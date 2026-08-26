# Building the LLVM/Clang Windows CE toolchain

This document describes a reproducible, from-source build of the complete
WinCE toolchain using only this repository (plus a host C++ compiler).
No Microsoft compiler/linker/CRT, no Platform Builder, and no GCC/CeGCC
binary is required at any stage.

## 0. Prerequisites

* Host: Linux with GCC/Clang >= 12, CMake >= 3.20, Ninja, Python 3.
* This monorepo with submodules initialized:

      git submodule update --init --depth 1

  The WinCE-side assets come from three submodules:

  | Submodule | Branch | Role |
  |---|---|---|
  | `third-party/mingwrt` | `master` | CeGCC mingw-runtime: COREDLL def files, CRT glue sources, mingwex, C headers |
  | `third-party/w32api` | `wip` | WinCE platform headers (w32api) + `libce/*.def` system-DLL definitions |
  | `third-party/binutils-gdb` | `ce-2.43.1` | CeGCC binutils fork (arm-wince-pe): the *normative reference* for the PE image format; not built, not required at run time |

  Only the WinCE-side assets of `mingwrt`/`w32api` are consumed (headers,
  def files, public-domain CRT sources).  The GCC/binutils compiler side
  of CeGCC is neither built nor depended upon: everything compiler- and
  linker-side is LLVM/Clang/LLD, and the final toolchain runs without any
  CeGCC component.

## 1. Stage 1: host LLVM + Clang + LLD (minimal)

The build is intentionally minimal: only the ARM backend, Clang, LLD and
the binary tools the sysroot needs.  The configuration lives in
`clang/cmake/caches/WinCE.cmake`:

    cmake -G Ninja -S llvm -B build \
      -C clang/cmake/caches/WinCE.cmake \
      -DCMAKE_BUILD_TYPE=Release
    ninja -C build clang lld llvm-ar llvm-ranlib llvm-dlltool \
                   llvm-readobj llvm-objdump llvm-nm llvm-mc FileCheck

Install the toolchain (the WinCE driver expects the sysroot at
`<prefix>/wince-sysroot` next to `bin/`):

    ninja -C build install

## 2. Stage 2: WinCE sysroot (C runtime + import libraries)

Standalone (the commands used for verification):

    cmake -B build-wince -S wince-crt \
        -DLLVM_TOOLS_DIR=$PWD/build/install/bin \
        -DCMAKE_C_COMPILER=$PWD/build/install/bin/clang \
        -DCMAKE_C_COMPILER_TARGET=arm-pc-wince
    cmake --build build-wince
    cp -r build-wince/wince-sysroot build/install/wince-sysroot

or, integrated in the LLVM runtimes build:

    cmake -G Ninja -S llvm -B build \
      -DLLVM_ENABLE_RUNTIMES=wince-crt \
      -DLLVM_RUNTIME_TARGETS=arm-pc-wince ...

Result: `wince-sysroot/` containing

    include/        mingw-runtime + w32api WinCE headers (+ pthread.h shim)
    lib/
      crt0.obj      EXE startup (WinMainCRTStartup / mainCRTStartup)
      dllcrt0.obj   DLL startup (DllMainCRTStartup)
      wce.lib       CRT glue: atexit, argv, WinMain adapter, pthread shim
      mingwex.lib   C library supplement (portable mingwex subset)
      coredll.lib   import library for COREDLL.dll (llvm-dlltool, armce)
      ceoldname.lib old-name import aliases
      *.lib         import libraries for all w32api/libce system DLLs

## 3. Stage 3: compiler runtime + C++ runtime

The ARM builtins (compiler-rt) and the C++ stack (libunwind, libc++abi,
libc++) are cross-built by the LLVM runtimes machinery against the stage-2
sysroot.  The compiler driver finds its sysroot automatically once the
sysroot is at `<prefix>/wince-sysroot`:

    cmake -G Ninja -S runtimes -B build-runtimes \
      -DCMAKE_BUILD_TYPE=Release \
      -DLLVM_ENABLE_RUNTIMES="compiler-rt;libunwind;libcxxabi;libcxx" \
      -DLLVM_RUNTIME_TARGETS=arm-pc-wince \
      -DCMAKE_C_COMPILER=$PWD/build/install/bin/clang \
      -DCMAKE_CXX_COMPILER=$PWD/build/install/bin/clang++ \
      -DCMAKE_AR=$PWD/build/install/bin/llvm-ar \
      -DCMAKE_RANLIB=$PWD/build/install/bin/llvm-ranlib \
      -DCOMPILER_RT_BUILD_SANITIZERS=OFF \
      -DCOMPILER_RT_BUILD_XRAY=OFF \
      -DCOMPILER_RT_BUILD_LIBFUZZER=OFF \
      -DCOMPILER_RT_BUILD_PROFILE=OFF \
      -DCOMPILER_RT_BUILD_MEMPROF=OFF \
      -DCOMPILER_RT_BUILD_ORC=OFF \
      -DCOMPILER_RT_BUILD_GWP_ASAN=OFF \
      -DCOMPILER_RT_DEFAULT_TARGET_ONLY=ON \
      -DCOMPILER_RT_BAREMETAL=ON \
      -DLIBUNWIND_ENABLE_SHARED=OFF \
      -DLIBCXXABI_ENABLE_SHARED=OFF \
      -DLIBCXX_ENABLE_SHARED=OFF \
      -DLIBCXX_ENABLE_STATIC_ABI_LIBRARY=ON \
      -DLIBCXXABI_ENABLE_PIC=OFF \
      -DLIBUNWIND_ENABLE_PIC=OFF \
      -DLIBCXX_ENABLE_PIC=OFF \
      -DLIBCXX_ENABLE_MONOTONIC_CLOCK=ON \
      -DLIBCXX_HAS_WIN32_THREAD_API=ON
    ninja -C build-runtimes

The resulting static libraries (`clang_rt.builtins-arm.lib`,
`unwind.lib`, `libc++abi.lib`, `c++.lib`) are copied into the sysroot
`lib/` directory, where the WinCE driver's default link line expects
them (`clang_rt.builtins-arm.lib`) and where `-lc++ -lc++abi -lunwind`
resolve.

Notes:

* `COMPILER_RT_BAREMETAL` selects compile-only target probing in
  compiler-rt's configure (the WinCE target cannot execute host-side
  configure binaries); the builtins themselves are ordinary ARM code.
* Threading in libc++ uses the Win32/CE API through the pthread shim in
  `wce.lib` (`LIBCXX_HAS_WIN32_THREAD_API` maps libc++ onto
  `CreateThread`/`WaitForSingleObject` coredll exports directly).
* Exceptions use the ARM EHABI (`.ARM.exidx`/`.ARM.extab`) through
  LLVM libunwind; `__exidx_start`/`__exidx_end` are bound by LLD.

## 4. Using the toolchain

    # C, WinMain-style GUI app
    clang --target=arm-pc-wince hello.c -o hello.exe

    # C, console-style app (main(); argv from GetCommandLineW)
    clang --target=arm-pc-wince -mconsole hello.c -o hello.exe

    # DLL
    clang --target=arm-pc-wince -shared lib.c -o lib.dll

    # C++ (libc++ static, EHABI exceptions)
    clang++ --target=arm-pc-wince hello.cpp -o hello.exe

    # C++ with clang-cl (MSVC-style driver)
    clang-cl --target=arm-pc-wince /EHsc hello.cpp /Fe:hello.exe

The driver defaults to `--sysroot=<clang-prefix>/wince-sysroot`; override
with `--sysroot=`.  See `wince-crt/docs/README.md` for the image defaults
that LLD applies (subsystem 9, base 0x10000, fixed EXE, DllMainCRTStartup
for DLLs, etc.).

## 5. Running the tests

* Host toolchain + in-tree tests:

      ninja -C build check-clang-driver check-lld-coff
      llvm-lit llvm/test/MC/ARM -filter=wince

* End-to-end pipeline (compile → archive → link → inspect):

      wince-crt/test/e2e/run-e2e.sh build/install/bin build/install/wince-sysroot

* On-device procedure: `wince-crt/docs/DEVICE-TESTING.md`.
