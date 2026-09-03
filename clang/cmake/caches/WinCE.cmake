#===-- clang/cmake/caches/WinCE.cmake - Windows CE toolchain cache --------===#
#
# Minimal host stage of the LLVM/Clang Windows CE cross toolchain.
#
# Only what the WinCE target needs is enabled: the ARM backend, the Clang
# driver/frontend, LLD (COFF/lld-link), and the binary utilities used for
# sysroot assembly and verification.  Everything else (other backends,
# sanitizers, bindings, docs, benchmarks, examples, optional compression/
# index/network libraries) is disabled so the toolchain builds fast and
# small.
#
# Usage:
#   cmake -G Ninja -S llvm -B build \
#     -C clang/cmake/caches/WinCE.cmake -DCMAKE_BUILD_TYPE=Release
#   ninja -C build clang lld llvm-ar llvm-ranlib llvm-dlltool \
#                llvm-readobj llvm-objdump llvm-nm llvm-mc FileCheck
#
# Stage 2 (WinCE sysroot: mingwrt + w32api + pthreads4w) and stage 3
# (compiler-rt builtins + libunwind/libc++abi/libc++) are driven by
# utils/wince/build-wince-sysroot.sh and utils/wince/build-wince-runtimes.sh;
# see utils/wince/README.md.
#
#===------------------------------------------------------------------------===#

# --- Projects: only the compiler, the linker; nothing else ----------------
set(LLVM_ENABLE_PROJECTS "clang;lld" CACHE STRING "")

# --- Backends: ARM + X86 (WinCE devices: armv4i + i386; CeGCC parity) -----
# CeGCC shipped two mingw32ce targets -- arm-mingw32ce and i386-mingw32ce --
# so the cross toolchain builds both the ARM and X86 backends.  The X86
# backend also lets the clang lit suite exercise the native x86-pc-wince EH
# paths (Microsoft C++ ABI __CxxFrameHandler3 / _CxxThrowException and the
# _except_handler3 SEH personality).
set(LLVM_TARGETS_TO_BUILD "ARM;X86" CACHE STRING "")

# --- Runtimes are built as separate cross stages (utils/wince) -------------
set(LLVM_ENABLE_RUNTIMES "" CACHE STRING "")

# --- Disable everything the WinCE toolchain does not need ------------------
# Optional support libraries: none are required.
set(LLVM_ENABLE_ZLIB OFF CACHE BOOL "")
set(LLVM_ENABLE_ZSTD OFF CACHE BOOL "")
set(LLVM_ENABLE_CURL OFF CACHE BOOL "")
set(LLVM_ENABLE_LIBXML2 OFF CACHE BOOL "")
set(LLVM_ENABLE_LIBEDIT OFF CACHE BOOL "")
set(LLVM_ENABLE_TERMINFO OFF CACHE BOOL "")
set(LLVM_ENABLE_LIBPFM OFF CACHE BOOL "")
set(LLVM_ENABLE_Z3 OFF CACHE BOOL "")
set(LLVM_ENABLE_FFI OFF CACHE BOOL "")
set(LLVM_ENABLE_DIA_SDK OFF CACHE BOOL "")
# Non-essential subprojects/features.
set(LLVM_INCLUDE_EXAMPLES OFF CACHE BOOL "")
set(LLVM_INCLUDE_BENCHMARKS OFF CACHE BOOL "")
set(LLVM_INCLUDE_DOCS OFF CACHE BOOL "")
set(LLVM_INCLUDE_GO_TESTS OFF CACHE BOOL "")
set(LLVM_ENABLE_BINDINGS OFF CACHE BOOL "")
set(LLVM_ENABLE_PLUGINS OFF CACHE BOOL "")
set(LLVM_ENABLE_IDE OFF CACHE BOOL "")
set(CLANG_ENABLE_STATIC_ANALYZER OFF CACHE BOOL "")
set(CLANG_ENABLE_ARCMIGRATE OFF CACHE BOOL "")
set(CLANG_ENABLE_PROTO_FUZZER OFF CACHE BOOL "")
# No shared LLVM library in the host stage.
set(LLVM_BUILD_LLVM_DYLIB OFF CACHE BOOL "")
set(LLVM_LINK_LLVM_DYLIB OFF CACHE BOOL "")
# Build noise/overhead trims.
set(LLVM_ENABLE_PEDANTIC OFF CACHE BOOL "")
set(LLVM_ENABLE_WARNINGS OFF CACHE BOOL "")
set(LLVM_ENABLE_ONDISK_CAS OFF CACHE BOOL "")
# Host tools are plain static executables: no PIC needed (faster compiles).
set(LLVM_ENABLE_PIC OFF CACHE BOOL "")
# Install only the toolchain binaries (not every tool/header).
set(LLVM_INSTALL_TOOLCHAIN_ONLY ON CACHE BOOL "")
set(LLD_ENABLE_THREADS ON CACHE BOOL "")

# --- Host toolchain sanity checks are done by the source distro's gcc -----
set(LLVM_INCLUDE_TESTS ON CACHE BOOL "") # FileCheck/lit for in-tree tests
# Release + optimized native tblgen: this cache builds host tools only to
# cross-compile WinCE code, so code size/perf of the host stage hardly
# matters and compile speed wins.
set(LLVM_OPTIMIZED_TABLEGEN ON CACHE BOOL "")

# --- Fast host stage --------------------------------------------------------
# This is the cross-toolchain *host* build, not the shipped artifact: the
# compiler/linker only need to run well enough to build and test WinCE
# code.  -O1 without assertions compiles roughly twice as fast as the
# -O3+assertions default on small build machines while keeping the tools
# plenty fast for target builds.  Flip to ON when debugging LLVM itself.
set(LLVM_ENABLE_ASSERTIONS OFF CACHE BOOL "")
set(CMAKE_C_FLAGS_RELEASE "-O1 -DNDEBUG" CACHE STRING "")
set(CMAKE_CXX_FLAGS_RELEASE "-O1 -DNDEBUG" CACHE STRING "")

# --- Keep the build lean and reproducible ---------------------------------
set(LLVM_APPEND_VC_REV OFF CACHE BOOL "")
set(LLVM_PARALLEL_LINK_JOBS 1 CACHE STRING "")

# --- Fast host linking: mold (memory-friendly on small build hosts) --------
if(NOT DEFINED LLVM_USE_LINKER)
  set(LLVM_USE_LINKER mold CACHE STRING "")
endif()

# --- Rebuild speed: ccache (kept outside the excluded build dirs) ----------
if(NOT DEFINED LLVM_CCACHE_BUILD)
  set(LLVM_CCACHE_BUILD ON CACHE BOOL "")
endif()
