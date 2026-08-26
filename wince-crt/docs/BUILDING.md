# Building the LLVM/Clang Windows CE toolchain

This document describes a reproducible, from-source build of the complete
WinCE toolchain using only this repository (plus a host C++ compiler).

## 0. Prerequisites

* Host: Linux or macOS with GCC/Clang >= 12, CMake >= 3.20, Ninja.
* This monorepo with submodules initialized:

      git submodule update --init third-party/cegcc-build

  (The submodules used are third-party/mingwrt and third-party/w32api;
  third-party/binutils-gdb is registered for reference only and is not
  checked out or built.)

## 1. Stage 1: host LLVM + Clang + LLD

    cmake -G Ninja -S llvm -B build \
      -DCMAKE_BUILD_TYPE=Release \
      -DLLVM_ENABLE_PROJECTS="clang;lld" \
      -DLLVM_TARGETS_TO_BUILD="ARM;X86" \
      -DLLVM_INCLUDE_TESTS=OFF -DLLVM_INCLUDE_EXAMPLES=OFF
    ninja -C build clang lld llvm-ar llvm-dlltool llvm-readobj llvm-objdump llvm-nm llvm-ar llvm-ranlib

This produces the cross compiler used for everything below.

## 2. Stage 2: WinCE sysroot (C runtime + import libraries)

As an LLVM runtime (integrated into the monorepo build):

    cmake -G Ninja -S llvm -B build-runtimes \
      -DLLVM_ENABLE_RUNTIMES="wince-crt" \
      -DLLVM_RUNTIME_TARGETS=arm-pc-wince \
      ... (cross toolchain settings; see stage 3)

or standalone:

    cmake -B build-wince -S wince-crt \
        -DLLVM_TOOLS_DIR=$PWD/build/bin \
        -DCMAKE_C_COMPILER=$PWD/build/bin/clang \
        -DCMAKE_C_COMPILER_TARGET=arm-pc-wince
    cmake --build build-wince

Result: `build-wince/wince-sysroot/` containing

    include/        mingw-runtime + w32api WinCE headers (+ pthread.h shim)
    lib/
      crt0.obj      EXE startup (WinMainCRTStartup / mainCRTStartup)
      dllcrt0.obj   DLL startup (DllMainCRTStartup)
      wce.lib       CRT glue: atexit, argv, WinMain adapter, pthread shim
      mingwex.lib   C library supplement (portable mingwex subset)
      coredll.lib   import library for COREDLL.dll (llvm-dlltool, armce)
      ceoldname.lib old-name import aliases
      *.lib         import libraries for all w32api/libce system DLLs

## 3. Stage 3: C++ runtime + compiler builtins

The C++ stack (libc++, libc++abi, libunwind) and the ARM builtins
(compiler-rt) are built by the LLVM runtimes machinery against the
sysroot produced in step 2:

    cmake -G Ninja -S llvm -B build-runtimes \
      -DCMAKE_BUILD_TYPE=Release \
      -DLLVM_ENABLE_RUNTIMES="compiler-rt;libunwind;libcxx;libcxxabi" \
      -DLLVM_RUNTIME_TARGETS=arm-pc-wince \
      -DCOMPILER_RT_BUILD_SANITIZERS=OFF -DCOMPILER_RT_BUILD_XRAY=OFF \
      -DCOMPILER_RT_DEFAULT_TARGET_ONLY=ON \
      -DCMAKE_SYSROOT=$PWD/build-wince/wince-sysroot \
      -DCMAKE_C_COMPILER=$PWD/build/bin/clang \
      -DCMAKE_CXX_COMPILER=$PWD/build/bin/clang++
    ninja -C build-runtimes

The resulting libraries are copied into the sysroot `lib/` (the runtimes
build installs them; the sysroot CMake can also consume prebuilt ones
from `WINCE_RUNTIME_LIBS`).

NOTE: as of this phase the runtimes build for `arm-pc-wince` still
requires the pieces listed as pending in `README.md` (libunwind's WinCE
exidx reader, pthread shim wiring in libc++'s threading config).  Those
are being landed incrementally; the C-only pipeline is complete and
verified.

## 4. Using the toolchain

    # C, WinMain-style GUI app
    clang --target=arm-pc-wince hello.c -o hello.exe

    # C++ with clang-cl (MSVC-style driver)
    clang-cl --target=arm-pc-wince /EHsc hello.cpp /Fe:hello.exe

The driver defaults to `--sysroot=<clang-prefix>/wince-sysroot`; override
with `--sysroot=`.  See `wince/README.md` for the image defaults that
LLD applies (subsystem 9, base 0x10000, fixed EXE, DllMainCRTStartup for
DLLs, etc.).
