#===-- clang/cmake/caches/WinCE.cmake - Windows CE toolchain cache --------===#
#
# Minimal host stage of the LLVM/Clang Windows CE cross toolchain.
#
# Only what the WinCE target needs is enabled: the ARM backend, the Clang
# driver/frontend, LLD (COFF/lld-link), and the binary utilities used for
# sysroot assembly and verification.  Everything else (other backends,
# sanitizers, bindings, docs, benchmarks, examples) is disabled so the
# toolchain builds fast and small.
#
# Usage:
#   cmake -G Ninja -S llvm -B build --toolchain ... \
#     -C ../clang/cmake/caches/WinCE.cmake
#   ninja -C build clang lld llvm-ar llvm-ranlib llvm-dlltool \
#                llvm-readobj llvm-objdump llvm-nm llvm-mc FileCheck
#
# Stage 2 (WinCE sysroot) and stage 3 (C++ runtime) are driven by the
# wince-crt runtime project; see wince-crt/docs/BUILDING.md.
#
#===------------------------------------------------------------------------===#

# --- Projects: only the compiler, the linker; nothing else ----------------
set(LLVM_ENABLE_PROJECTS "clang;lld" CACHE STRING "")

# --- Backends: ARM only (WinCE devices); no native host backend needed ----
set(LLVM_TARGETS_TO_BUILD "ARM" CACHE STRING "")

# --- Runtimes are built as separate cross stages (wince-crt docs) ----------
set(LLVM_ENABLE_RUNTIMES "" CACHE STRING "")

# --- Disable everything the WinCE toolchain does not need ------------------
set(LLVM_INCLUDE_EXAMPLES OFF CACHE BOOL "")
set(LLVM_INCLUDE_BENCHMARKS OFF CACHE BOOL "")
set(LLVM_INCLUDE_DOCS OFF CACHE BOOL "")
set(LLVM_ENABLE_BINDINGS OFF CACHE BOOL "")
set(LLVM_ENABLE_PLUGINS OFF CACHE BOOL "")
set(LLVM_ENABLE_ZLIB OFF CACHE BOOL "")
set(LLVM_ENABLE_ZSTD OFF CACHE BOOL "")
set(LLVM_ENABLE_CURL OFF CACHE BOOL "")
set(LLVM_ENABLE_LIBXML2 OFF CACHE BOOL "")
set(LLVM_ENABLE_LIBEDIT OFF CACHE BOOL "")
set(LLVM_ENABLE_TERMINFO OFF CACHE BOOL "")
set(LLVM_ENABLE_LIBPFM OFF CACHE BOOL "")
set(LLVM_ENABLE_PEDANTIC OFF CACHE BOOL "")
set(LLVM_ENABLE_WARNINGS OFF CACHE BOOL "")
set(CLANG_ENABLE_STATIC_ANALYZER OFF CACHE BOOL "")
set(CLANG_ENABLE_ARCMIGRATE OFF CACHE BOOL "")
set(CLANG_ENABLE_PROTO_FUZZER OFF CACHE BOOL "")
set(LLD_ENABLE_THREADS ON CACHE BOOL "")

# --- Host toolchain sanity checks are done by the source distro's gcc -----
set(LLVM_INCLUDE_TESTS ON CACHE BOOL "") # FileCheck/lit for in-tree tests
set(LLVM_ENABLE_ASSERTIONS ON CACHE BOOL "")

# --- Keep the build lean and reproducible ---------------------------------
set(LLVM_APPEND_VC_REV OFF CACHE BOOL "")
set(LLVM_PARALLEL_LINK_JOBS 1 CACHE STRING "")
