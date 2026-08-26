#===- wince-crt/CMakeLists.txt - Windows CE C runtime ---------------------===#
#
# The Windows CE C runtime for the LLVM/Clang toolchain, following the LLVM
# runtime project conventions (buildable standalone, or as part of the LLVM
# runtimes build via LLVM_ENABLE_RUNTIMES=wince-crt).
#
# It assembles the complete WinCE sysroot:
#
#   * import libraries for the Windows CE system DLLs, generated from the
#     module definition files shipped in the third-party/mingwrt and
#     third-party/w32api submodules (llvm-dlltool, armce machine);
#   * the CRT startup objects (crt0.obj / dllcrt0.obj) and the static glue
#     library (wce.lib);
#   * the C library supplement (mingwex portable subset);
#   * the header overlay (mingw-runtime + w32api + pthread shim).
#
# The C library proper is COREDLL.dll (as on Windows CE): the runtime
# imports from it via coredll.lib exactly like the CeGCC runtime does.
# Files that CeGCC ships unchanged are compiled directly from the
# submodules; only the genuinely new port layer lives in lib/.
#
#===------------------------------------------------------------------------===#

cmake_minimum_required(VERSION 3.20)

if (CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
  # Standalone build.  Provide defaults matching the in-monorepo layout.
  # The compiler is a cross compiler with no sysroot yet; static-library
  # compiler checks avoid needing a working linker during configure.
  set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)
  project(wince-crt C)
  set(WINCE_CRT_STANDALONE ON)
  if (NOT LLVM_TOOLS_DIR)
    get_filename_component(LLVM_TOOLS_DIR "${CMAKE_C_COMPILER}" DIRECTORY)
  endif()
endif()

set(WINCERT_TARGETTriple "${CMAKE_C_COMPILER_TARGET}" CACHE STRING
    "WinCE target triple")
if (NOT WINCERT_TARGETTriple)
  set(WINCERT_TARGETTriple "arm-pc-wince")
endif()

set(MINGWRT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../third-party/mingwrt" CACHE PATH
    "Path to the mingwrt (CeGCC mingw-runtime) submodule")
set(W32API_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../third-party/w32api" CACHE PATH
    "Path to the w32api (WinCE headers) submodule")
set(WINCERT_SYSROOT "${CMAKE_CURRENT_BINARY_DIR}/wince-sysroot" CACHE PATH
    "Sysroot output directory")
set(WINCERT_ARCH_FLAGS "-march=armv4t" CACHE STRING
    "Baseline ARM architecture flags (ARMv4T runs on all CE devices)")

# --- Tool discovery -------------------------------------------------------

find_program(WINCE_CLANG clang
    HINTS "${LLVM_TOOLS_DIR}" "${CMAKE_C_COMPILER}" REQUIRED)
get_filename_component(WINCE_CLANG_DIR "${WINCE_CLANG}" DIRECTORY)
find_program(WINCE_LLVM_AR llvm-ar HINTS "${WINCE_CLANG_DIR}" REQUIRED)
find_program(WINCE_LLVM_DLLTOOL llvm-dlltool HINTS "${WINCE_CLANG_DIR}" REQUIRED)

# Host headers must never leak into the target build: -nostdlibinc plus
# explicit -isystem for the WinCE headers only.
set(WINCERT_C_FLAGS
    --target=${WINCERT_TARGETTriple}
    ${WINCERT_ARCH_FLAGS}
    -O2 -g0 -fno-ident
    -fgnu89-inline
    -nostdlibinc
    -isystem "${MINGWRT_DIR}/include"
    -isystem "${W32API_DIR}/include"
    -isystem "${CMAKE_CURRENT_SOURCE_DIR}/lib"
    -Wno-unknown-pragmas -Wno-unused-function
    CACHE STRING "" FORCE)

file(MAKE_DIRECTORY "${WINCERT_SYSROOT}/lib" "${CMAKE_CURRENT_BINARY_DIR}/obj")

#===------------------------------------------------------------------------===#
# Import libraries for the Windows CE system DLLs
#===------------------------------------------------------------------------===#

function(wince_implib def_file dll_name out_name)
  add_custom_command(
    OUTPUT "${WINCERT_SYSROOT}/lib/${out_name}"
    COMMAND ${WINCE_LLVM_DLLTOOL} -m armce -k
            --dllname "${dll_name}"
            --def "${def_file}"
            --output-lib "${WINCERT_SYSROOT}/lib/${out_name}"
    DEPENDS "${def_file}"
    VERBATIM)
  list(APPEND WINCERT_IMPLIB_TARGETS "${WINCERT_SYSROOT}/lib/${out_name}")
  set(WINCERT_IMPLIB_TARGETS "${WINCERT_IMPLIB_TARGETS}" PARENT_SCOPE)
endfunction()

wince_implib("${MINGWRT_DIR}/coredll.def"  COREDLL.dll coredll.lib)
wince_implib("${MINGWRT_DIR}/coredll6.def" COREDLL.dll coredll6.lib)

# The "old name" aliases are preprocessed from moldname.def.in exactly like
# the CeGCC build does (gcc -D__COREDLL__ -E).
add_custom_command(
  OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/obj/moldname-coredll.def"
  COMMAND ${WINCE_CLANG} --target=${WINCERT_TARGETTriple}
          -DRUNTIME=coredll -D__FILENAME__=moldname-coredll.def
          -D__COREDLL__ -C -E -P -x c-header
          "${MINGWRT_DIR}/moldname.def.in"
          -o "${CMAKE_CURRENT_BINARY_DIR}/obj/moldname-coredll.def"
  DEPENDS "${MINGWRT_DIR}/moldname.def.in"
  VERBATIM)
add_custom_command(
  OUTPUT "${WINCERT_SYSROOT}/lib/ceoldname.lib"
  COMMAND ${WINCE_LLVM_DLLTOOL} -m armce -k
          --dllname COREDLL.dll
          --def "${CMAKE_CURRENT_BINARY_DIR}/obj/moldname-coredll.def"
          --output-lib "${WINCERT_SYSROOT}/lib/ceoldname.lib"
  DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/obj/moldname-coredll.def"
  VERBATIM)

file(GLOB WINCERT_W32API_DEFS "${W32API_DIR}/libce/*.def")
foreach(def ${WINCERT_W32API_DEFS})
  get_filename_component(name "${def}" NAME_WE)
  # coredll comes from the mingwrt definition file above (the CeGCC
  # canonical copy; the two are kept in sync upstream).
  if (NOT name STREQUAL "coredll")
    wince_implib("${def}" "${name}.dll" "${name}.lib")
  endif()
endforeach()

#===------------------------------------------------------------------------===#
# CRT objects and glue library
#===------------------------------------------------------------------------===#

# The new port layer (LLVM/LLD-specific startup and glue).
# The new port layer: LLVM/LLD-specific startup and glue.
# crt0.c/dllcrt0.c are adapted from mingwrt crt3.c/dllcrt1.c; crtxcu.c
# dispatches .CRT$XCU/.CRT$XTU; init_args.c builds argv; winmain.c holds
# the WinMain<->main adapters; pthread.c is the coredll pthread shim;
# crt_extra.c provides MB_CUR_MAX for the mingw __COREDLL__ headers.
set(WINCERT_PORT_SOURCES
    lib/crt0.c
    lib/dllcrt0.c
    lib/crtxcu.c
    lib/init_args.c
    lib/winmain.c
    lib/pthread.c
    lib/crt_extra.c
)

# Files consumed unchanged from the CeGCC mingw-runtime (public domain):
# exactly the members of the CeGCC libmingw32.a that a WinCE build uses.
set(WINCERT_MINGWRT_SOURCES
    atexit.c
    assert.c
    coredll_stubs.c
    strcasecmp.c
    strncasecmp.c
)

set(WINCERT_CRT_OBJS)
foreach(src ${WINCERT_PORT_SOURCES})
  get_filename_component(obj "${src}" NAME_WE)
  set(obj "${CMAKE_CURRENT_BINARY_DIR}/obj/${obj}.obj")
  add_custom_command(
    OUTPUT "${obj}"
    COMMAND ${WINCE_CLANG} ${WINCERT_C_FLAGS} -c
            "${CMAKE_CURRENT_SOURCE_DIR}/${src}" -o "${obj}"
    DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/${src}"
    VERBATIM)
  list(APPEND WINCERT_CRT_OBJS "${obj}")
endforeach()

foreach(src ${WINCERT_MINGWRT_SOURCES})
  get_filename_component(obj "${src}" NAME_WE)
  set(obj "${CMAKE_CURRENT_BINARY_DIR}/obj/${obj}.obj")
  add_custom_command(
    OUTPUT "${obj}"
    COMMAND ${WINCE_CLANG} ${WINCERT_C_FLAGS} -c
            "${MINGWRT_DIR}/${src}" -o "${obj}"
    DEPENDS "${MINGWRT_DIR}/${src}"
    VERBATIM)
  list(APPEND WINCERT_CRT_OBJS "${obj}")
endforeach()

add_custom_command(
  OUTPUT "${WINCERT_SYSROOT}/lib/wce.lib"
  COMMAND ${WINCE_LLVM_AR} rc "${WINCERT_SYSROOT}/lib/wce.lib"
          ${WINCERT_CRT_OBJS}
  DEPENDS ${WINCERT_CRT_OBJS}
  VERBATIM)

add_custom_command(
  OUTPUT "${WINCERT_SYSROOT}/lib/crt0.obj" "${WINCERT_SYSROOT}/lib/dllcrt0.obj"
  COMMAND ${CMAKE_COMMAND} -E copy
          "${CMAKE_CURRENT_BINARY_DIR}/obj/crt0.obj"
          "${WINCERT_SYSROOT}/lib/crt0.obj"
  COMMAND ${CMAKE_COMMAND} -E copy
          "${CMAKE_CURRENT_BINARY_DIR}/obj/dllcrt0.obj"
          "${WINCERT_SYSROOT}/lib/dllcrt0.obj"
  DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/obj/crt0.obj"
          "${CMAKE_CURRENT_BINARY_DIR}/obj/dllcrt0.obj"
  VERBATIM)

#===------------------------------------------------------------------------===#
# mingwex supplement (portable subset; COREDLL.dll provides the rest)
#===------------------------------------------------------------------------===#

set(WINCERT_MINGWEX_SOURCES
    _Exit.c atoll.c basename.c dirname.c btowc.c fwide.c getopt.c
    gettimeofday.c imaxabs.c imaxdiv.c isblank.c iswblank.c
    lltoa.c lltow.c ulltoa.c ulltow.c
    mbrtowc.c mbsinit.c wcrtomb.c wctob.c wctrans.c wctype.c
    wcstoimax.c wcstoumax.c strtoimax.c strtoumax.c
    tdelete.c tfind.c tsearch.c twalk.c
    mingw-aligned-malloc.c usleep.c
    wmemchr.c wmemcmp.c wmemcpy.c wmemmove.c wmemset.c)

set(WINCERT_MINGWEX_OBJS)
foreach(src ${WINCERT_MINGWEX_SOURCES})
  get_filename_component(obj "${src}" NAME_WE)
  set(obj "${CMAKE_CURRENT_BINARY_DIR}/obj/mingwex_${obj}.obj")
  add_custom_command(
    OUTPUT "${obj}"
    COMMAND ${WINCE_CLANG} ${WINCERT_C_FLAGS}
            -c "${MINGWRT_DIR}/mingwex/${src}" -o "${obj}"
    DEPENDS "${MINGWRT_DIR}/mingwex/${src}"
    VERBATIM)
  list(APPEND WINCERT_MINGWEX_OBJS "${obj}")
endforeach()

add_custom_command(
  OUTPUT "${WINCERT_SYSROOT}/lib/mingwex.lib"
  COMMAND ${WINCE_LLVM_AR} rc "${WINCERT_SYSROOT}/lib/mingwex.lib"
          ${WINCERT_MINGWEX_OBJS}
  DEPENDS ${WINCERT_MINGWEX_OBJS}
  VERBATIM)

#===------------------------------------------------------------------------===#
# Header overlay
#===------------------------------------------------------------------------===#

add_custom_command(
  OUTPUT "${WINCERT_SYSROOT}/include/.stamp"
  COMMAND ${CMAKE_COMMAND} -E copy_directory
          "${MINGWRT_DIR}/include" "${WINCERT_SYSROOT}/include"
  COMMAND ${CMAKE_COMMAND} -E copy_directory
          "${W32API_DIR}/include" "${WINCERT_SYSROOT}/include"
  COMMAND ${CMAKE_COMMAND} -E copy
          "${CMAKE_CURRENT_SOURCE_DIR}/lib/pthread.h"
          "${WINCERT_SYSROOT}/include/pthread.h"
  COMMAND ${CMAKE_COMMAND} -E touch "${WINCERT_SYSROOT}/include/.stamp"
  VERBATIM)

#===------------------------------------------------------------------------===#
# Top-level targets
#===------------------------------------------------------------------------===#

add_custom_target(wince-crt
  DEPENDS
    "${WINCERT_SYSROOT}/include/.stamp"
    "${WINCERT_SYSROOT}/lib/crt0.obj"
    "${WINCERT_SYSROOT}/lib/dllcrt0.obj"
    "${WINCERT_SYSROOT}/lib/wce.lib"
    "${WINCERT_SYSROOT}/lib/mingwex.lib"
    ${WINCERT_IMPLIB_TARGETS})

if (WINCE_CRT_STANDALONE)
  install(DIRECTORY "${WINCERT_SYSROOT}/" DESTINATION .)
endif()
