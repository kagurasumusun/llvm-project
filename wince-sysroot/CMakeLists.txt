#===- wince-sysroot/CMakeLists.txt - Windows CE sysroot ------------------===#
#
# The Windows CE sysroot as a first-class LLVM runtime project: it assembles
# <sysroot>/{include,lib} from the trees vendored under third-party/
# (mingwrt, w32api, pthread-win32) by building them with their own
# configure/make using the LLVM/Clang WinCE cross compiler in place of
# GCC/binutils.  The build steps live in
# utils/wince/build-wince-sysroot.sh; this project drives them through the
# standard LLVM runtimes flow (LLVM_ENABLE_RUNTIMES=wince-sysroot) or
# standalone, and installs the result to <install-prefix>/wince-sysroot,
# where the WinCE driver finds it by default.
#
#===------------------------------------------------------------------------===#

cmake_minimum_required(VERSION 3.20)

if (CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
  # Standalone build: provide defaults matching the in-monorepo layout.
  project(wince-sysroot C)
  set(WINCE_SYSROOT_STANDALONE ON)
endif()

set(WINCE_SYSROOT_TARGET "arm-pc-wince" CACHE STRING
    "Windows CE target triple (arm-pc-wince or i386-pc-wince)")
set(WINCE_SYSROOT_SCRIPT "${CMAKE_CURRENT_SOURCE_DIR}/../utils/wince/build-wince-sysroot.sh")
set(WINCE_SYSROOT_OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/wince-sysroot")

if (WINCE_SYSROOT_STANDALONE)
  # Standalone: discover the cross toolchain next to CMAKE_C_COMPILER, or
  # take an explicit prefix.
  set(LLVM_TOOLS_BINARY_DIR "" CACHE PATH
      "Directory containing the stage-1 clang/lld/llvm tools")
else()
  # In the runtimes umbrella, the just-built host tools are in the build tree.
  get_filename_component(LLVM_TOOLS_BINARY_DIR "${CMAKE_C_COMPILER}" DIRECTORY CACHE)
endif()

if (NOT EXISTS "${LLVM_TOOLS_BINARY_DIR}/clang")
  message(FATAL_ERROR
      "wince-sysroot: clang not found in ${LLVM_TOOLS_BINARY_DIR}; "
      "build the stage-1 toolchain first (clang/cmake/caches/WinCE.cmake) "
      "and point LLVM_TOOLS_BINARY_DIR at its bin directory.")
endif()
if (NOT EXISTS "${WINCE_SYSROOT_SCRIPT}")
  message(FATAL_ERROR "wince-sysroot: builder script missing: ${WINCE_SYSROOT_SCRIPT}")
endif()

find_program(WINCE_SHELL sh DOC "POSIX shell used to drive the sysroot build")
if (NOT WINCE_SHELL)
  message(FATAL_ERROR
      "wince-sysroot: the sysroot assembly drives mingwrt/w32api's own "
      "configure/make and requires a POSIX shell (sh); on Windows use "
      "WSL/Cygwin/MSYS or run utils/wince/build-wince-sysroot.sh manually.")
endif()

if (NOT CMAKE_BUILD_PARALLEL_LEVEL)
  set(CMAKE_BUILD_PARALLEL_LEVEL 2)
endif()

# The sysroot only needs the headers/CRT at build time of target code; it is
# a byproduct-style target rather than part of `all` (mirrors how runtimes
# with heavy external builds are handled).
add_custom_target(wince-sysroot
    COMMAND "${WINCE_SHELL}" "${WINCE_SYSROOT_SCRIPT}"
            --toolchain "${LLVM_TOOLS_BINARY_DIR}"
            --target "${WINCE_SYSROOT_TARGET}"
            --prefix "${WINCE_SYSROOT_OUTPUT}"
            --jobs "${CMAKE_BUILD_PARALLEL_LEVEL}"
    BYPRODUCTS "${WINCE_SYSROOT_OUTPUT}/lib/crt3.o"
               "${WINCE_SYSROOT_OUTPUT}/lib/libcoredll.a"
               "${WINCE_SYSROOT_OUTPUT}/lib/libpthread.a"
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/.."
    COMMENT "Assembling the Windows CE sysroot (mingwrt + w32api + pthreads4w)"
    VERBATIM
    USES_TERMINAL)

install(DIRECTORY "${WINCE_SYSROOT_OUTPUT}/"
        DESTINATION wince-sysroot
        COMPONENT wince-sysroot)
