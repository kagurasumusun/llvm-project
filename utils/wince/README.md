# Windows CE toolchain: sysroot, runtime and compiler-side support

This directory holds the tooling that turns a freshly built LLVM/Clang/LLD
(the "stage 1" host toolchain, configured with
`clang/cmake/caches/WinCE.cmake`) into a complete, self-contained
Windows CE (Windows Embedded CE 6.0 focus) cross toolchain whose C runtime
and platform headers are the **unmodified** CeGCC-lineage

* [`kagurasumusun/mingwrt`](https://github.com/kagurasumusun/mingwrt) (mingwrt),
* [`kagurasumusun/w32api`](https://github.com/kagurasumusun/w32api) (w32api),

plus the pthreads4w static thread library.  There is no bespoke CRT: the
sysroot is assembled by building those trees **with their own
configure/make**, with Clang in place of GCC and the LLVM binary tools in
place of binutils — the same role CeGCC's GCC played.

```
stage 1  clang/lld/llvm-tools host build     (clang/cmake/caches/WinCE.cmake)
stage 2  sysroot: mingwrt + w32api + pthread (utils/wince/build-wince-sysroot.sh)
stage 3  compiler-rt + libunwind/libc++abi/libc++  (utils/wince/build-wince-runtimes.sh)
```

## Stage 2: the sysroot

    utils/wince/build-wince-sysroot.sh --toolchain <prefix>/bin \
        [--target arm-pc-wince] [--prefix <sysroot>] [--jobs N]

`<sysroot>` defaults to `<prefix>/../wince-sysroot`, the location the WinCE
driver probes.  The same flow is available through the standard LLVM
runtimes machinery — `wince-sysroot` is a registered runtime project
(`wince-sysroot/CMakeLists.txt`):

    cmake -G Ninja -S llvm -B build -C clang/cmake/caches/WinCE.cmake
    ninja -C build wince-sysroot           # or -DLLVM_ENABLE_RUNTIMES=wince-sysroot
    ninja -C build install/wince-sysroot   # stages to <install-prefix>/wince-sysroot

Resulting layout (CeGCC-compatible, GNU-named):

    include/              mingwrt C headers + w32api WinCE headers + pthreads
    lib/
      crt3.o dllcrt3.o    mingwrt CRT0S (EXE / DLL startup)
      CRT_noglob.o crtmt.o crtst.o
      libmingw32.a        mingwrt MINGW_OBJS(ce) incl. winmain_ce.o glue
      libmingwex.a        mingwrt CE object set (MATHCE/STDIO_CE, -D_IEEE_LIBM)
      libceoldname.a      old-name aliases via COREDLL (mingwrt rules)
      libcoredll.a libcoredll6.a   COREDLL import libraries (mingwrt)
      lib*.a              w32api libce/*.def import libraries (llvm-dlltool)
      libpthread.a        pthreads4w static library (PTW32_CLEANUP_C)

Tool substitutions (GNU -> LLVM):

| role    | tool                                            |
|---------|-------------------------------------------------|
| CC      | `clang --target=<triple> -march=armv5te -mfloat-abi=soft` |
| ar      | `llvm-ar`                                        |
| ranlib  | `llvm-ranlib`                                    |
| dlltool | `llvm-dlltool -m armce` via a tiny flag shim (`--def`->`-d`, `-U`/`--as` dropped) |

### Third-party source policy

The three trees are **vendored directly into this monorepo** (no
submodules), each with a provenance note in its `README.llvmvendor.md`:

* **`third-party/mingwrt`** — kagurasumusun/mingwrt @ `7c35691` plus one
  compiler-compat commit (also pushed upstream): `include/_mingw.h` accepts
  `__clang__` at the `#ifdef __declspec` probe that selects the
  `__DECLSPEC_SUPPORTED` / `__MINGW_IMPORT` / `_CRTIMP` declarations
  (GCC's PE targets predefine `__declspec` as a macro, Clang implements it
  as a keyword), and `Makefile.in` preprocesses the generated `.def` files
  without `-C` (the preserved C comments are outside the def-file grammar
  llvm-dlltool implements).  No runtime-semantic changes.
* **`third-party/w32api`** — kagurasumusun/w32api @ `51de0ad`, unmodified.
* **`third-party/pthread-win32`** — GerHobbelt/pthread-win32 (the actively
  maintained combined successor of pthreads-win32/pthreads4w; upstream
  pthreads4w is dormant) @ `06e7608` plus three WinCE 6.0 build fixes
  committed in-tree:
  1. thread entry/exit take the `_beginthreadex`/`_endthreadex` paths,
     which `implement.h` maps to `CreateThread`/`ExitThread` under
     `NEED_CREATETHREAD` (the guards keyed on `__MINGW32__`/`__MSVCRT__`
     only routed CeGCC-style builds to the CRTDLL `_beginthread()` that
     COREDLL does not export);
  2. the GNU interlocked block is restricted to x86 — it emits x86 inline
     assembly for any `__GNUC__` target; ARM must use the COREDLL
     `Interlocked*` exports;
  3. `_ptw32.h` accepts clang at the `#if ! defined __declspec` guard
     (same root cause as the mingwrt `_mingw.h` probe).

## Stage 3: compiler runtime + C++ runtime

    utils/wince/build-wince-runtimes.sh --toolchain <prefix>/bin \
        [--sysroot <sysroot>]

Cross-builds `compiler-rt/lib/builtins` (the `-lgcc` replacement) and the
static `libunwind + libc++abi + libc++` stack against the stage-2 sysroot
and stages them as `libclang_rt.builtins-<arch>.a`, `libunwind.a`,
`libc++abi.a`, `libc++.a` plus `include/c++/v1`.  libc++ uses the pthread
threading API (`LIBCXX_HAS_PTHREAD_API`), served by the stage-2
`libpthread.a`; the WinCE locale backend
(`libcxx/src/support/wince/locale_wince.cpp`) and clock paths are in-tree.

## Driver behavior (the Clang side)

`clang --target=arm-pc-wince` (alias `arm-mingw32ce`) selects the WinCE
toolchain (`clang/lib/Driver/ToolChains/WinCE.{h,cpp}`), which reproduces
the CeGCC link line on lld-link:

| CeGCC (gcc specs)                | clang driver                                        |
|----------------------------------|-----------------------------------------------------|
| `STARTFILE_SPEC` crt3/dllcrt3    | `crt3.o` (EXE) / `dllcrt3.o` (DLL) from the sysroot |
| `-e DllMainCRTStartup` (DLL)     | `/entry:DllMainCRTStartup`                          |
| pe.em subsystem-9 default entry  | `/entry:WinMainCRTStartup` (console: `mainCRTStartup`) |
| arm-wince emulation defaults     | `/subsystem:windowsce /base:0x10000 /fixed` (DLLs: `0x10000000`, keep `.reloc`) |
| `%{mthreads:-lmingwthrd} -lmingw32 -lgcc -lceoldname -lmingwex -lcoredll` | `libpthread.a` (`-mthreads`/`-pthread`, plus `-D_MT`), `libmingw32.a`, `libclang_rt.builtins-*.a`, `libceoldname.a`, `libmingwex.a`, `libcoredll.a` |
| `%{mthreads:-D_MT}` (CPP_SPEC)   | `-D_MT` at compile time                             |

Libraries are probed GNU-first (`lib<name>.a`) with MS-style (`<name>.lib`)
fallback so both sysroot layouts resolve.  `-Wl,` GNU spellings
(`-subsystem`, `-e/--entry`, `--image-base`, `--stack`, `--dll`, `--def`,
`--out-implib`, `--major/--minor-image-version`, `--dynamicbase`) are
translated to their lld-link forms.

## Target-level support ported from the CeGCC GCC fork

The compiler-side pieces of
[`salman-javed-nz/gcc@de6e918`](https://github.com/salman-javed-nz/gcc/commits/de6e9181807529b3034aa9cc56f97daa46bee303)
(the CeGCC GCC 14.2 revival) that have LLVM equivalents are implemented
in-tree; the rest target GCC/libstdc++/libgcc internals that have no
counterpart here (LLVM does not fuse sin+cos into `sincos`, the C++ runtime
is libc++, the builtins runtime is compiler-rt):

| CeGCC commit | LLVM/Clang equivalent here |
|---|---|
| `f363d9aa578` gcc: add support for WinCE targets (ARM) | pre-existing: `WinCEARMTargetInfo`, WinCE toolchain, lld COFF/ARM CE support |
| `68f61cad451` + `6c297094847` i386-mingw32ce defines, UNDER_CE alignment | new: `WinCETargetInfo` (x86-32) in `clang/lib/Basic/Targets/X86.{h,cpp}` — `_X86_`, `__CEGCC_VERSION__`, `__COREDLL__`, `__MINGW32(CE)__`, `UNDER_CE`/`_WIN32_WCE` (versioned), Unicode predefines, `__stdcall`->cdecl macro rewrite, calling conventions accepted-and-ignored |
| `3624235625c` sincos gate | N/A (LLVM has no sincos fusion pass) |
| `5892f02ab71`/`tsystem.h`, `db4740f6642`/`dccf64141cb` libgcc, `61d1f7ff138` libssp | N/A (compiler-rt builtins; no libgcc/libssp in this toolchain) |
| `d2e4720baf6`..`8736e9b611d` libstdc++ WinCE workarounds | covered by the libc++ WinCE configuration (locale backend, no filesystem, pthread API); libc++ needs none of the libstdc++-specific workarounds |

## MSVC syntax support additions

For maximum compatibility with eMbedded Visual C++ / Platform Builder era
sources (this target always compiles with `-fms-extensions
-fms-compatibility -fdelayed-template-parsing
-fms-compatibility-version=1900`), the following MSVC pragmas — previously
diagnosed as unknown — are now accepted and ignored (same treatment as
`#pragma runtime_checks`): `setlocale`, `check_stack`, `conform`,
`auto_inline` (see `clang/lib/Parse/ParsePragma.cpp`).

## Verification status

* End-to-end stage-2 verified with a host-clang stand-in for the target
  compiler: mingwrt CRT objects, `libmingw32.a`, full CE `libmingwex.a`,
  `libceoldname.a`, COREDLL import libraries, all 71 w32api libce import
  libraries, 420+ staged headers and `libpthread.a` build cleanly, and a
  client TU including `windows.h`/`stdio.h`/`pthread.h`/`semaphore.h`/
  `tchar.h` compiles against the staged sysroot alone.
* Stage 1/3 require a real build of this tree (`WinCE.cmake` cache); lit
  coverage: `clang/test/Driver/wince.c`, `clang/test/Driver/wince-x86.c`,
  `lld/test/COFF/wince-*.ll*(s)`, `llvm/test/MC/ARM/wince-*.s`.
* On-device testing remains outstanding (no WinCE device in the build
  environment); see the procedure notes formerly in `wince-crt/docs`.
