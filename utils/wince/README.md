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
place of binutils — the same role CeGCC's GCC played.  All three trees are
vendored in-tree under `wince-sysroot/` (this runtime project), matching
LLVM's structure: `wince-sysroot` is a registered runtime
(`LLVM_ENABLE_RUNTIMES=wince-sysroot`) whose subdirectories hold the
imported sources.

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
      libmingwthrd.a      mingwrt's -mthreads CRT glue (crtmt.o)
      libpthread.a        pthreads4w static library (PTW32_CLEANUP_C)

Tool substitutions (GNU -> LLVM):

| role    | tool                                            |
|---------|-------------------------------------------------|
| CC      | `clang --target=<triple> -march=armv5te -mfloat-abi=soft` |
| ar      | `llvm-ar`                                        |
| ranlib  | `llvm-ranlib`                                    |
| dlltool | `llvm-dlltool -m armce` via a tiny flag shim (`--def`->`-d`, `-U`/`--as` dropped) |

### Third-party source policy

Each tree carries a provenance note in its `README.llvmvendor.md`:

* **`wince-sysroot/mingwrt`** — kagurasumusun/mingwrt @ `7c35691` plus one
  compiler-compat commit (also pushed upstream): `include/_mingw.h` accepts
  `__clang__` at the `#ifdef __declspec` probe that selects the
  `__DECLSPEC_SUPPORTED` / `__MINGW_IMPORT` / `_CRTIMP` declarations
  (GCC's PE targets predefine `__declspec` as a macro, Clang implements it
  as a keyword), and `Makefile.in` preprocesses the generated `.def` files
  without `-C` (the preserved C comments are outside the def-file grammar
  llvm-dlltool implements).  No runtime-semantic changes.
* **`wince-sysroot/w32api`** — kagurasumusun/w32api @ `51de0ad`, unmodified.
* **`wince-sysroot/pthread-win32`** — GerHobbelt/pthread-win32 (the actively
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
| `%{mthreads:-lmingwthrd} -lmingw32 -lgcc -lceoldname -lmingwex -lcoredll` | `-mthreads`/`-pthread`: `libmingwthrd.a` (mingwrt, as CeGCC) + `libpthread.a` (pthreads4w) and `-D_MT`; then `libmingw32.a`, `libclang_rt.builtins-*.a`, `libceoldname.a`, `libmingwex.a`, `libcoredll.a` |
| `%{mthreads:-D_MT}` (CPP_SPEC)   | `-D_MT` at compile time                             |
| (GNU ld pe.em: __CTOR_LIST__/__DTOR_LIST__ bracketing) | clang emits global ctors/dtors in the GNU convention for WinCE (`.ctors`/`.dtors`, priority subsections `.ctors.NNNNN`, associative grouping); lld-link `-wince` sorts and brackets them with the -1 head sentinel and 0 terminator, `__CTOR_LIST__`/`__DTOR_LIST__` point at the head, and mingwrt's `__main` (`gccmain.c`) walks them: global C++ constructors run before `main`/`WinMain`, destructors run through the atexit table.  (The MSVC `.CRT$XCU`/`.CRT$XTX` tables would need `__xc_a` startup objects that the CE runtime does not provide — verified not used for this target.) |
| (GCC default: gnu89 inline)      | `-fgnu89-inline` by default — the pre-C99 `extern __inline` convention used by eMbedded Visual C++ / old mingwrt headers keeps its external-definition semantics (`-fno-gnu89-inline` to override) |

Libraries are probed GNU-first (`lib<name>.a`) with MS-style (`<name>.lib`)
fallback so both sysroot layouts resolve.  `-Wl,` GNU spellings
(`-subsystem`, `-e/--entry`, `--image-base`, `--stack`, `--dll`, `--def`,
`--out-implib`, `--major/--minor-image-version`, `--dynamicbase`) are
translated to their lld-link forms.

## Linkage/runtime chain audit

Every startup path from the compiler through the linker to the runtime,
audited end to end:

| chain | status |
|---|---|
| C/C++ global constructors & destructors | complete: clang emits GNU `.ctors`/`.dtors` (priority subsections, associative) -> lld-link `-wince` brackets `__CTOR_LIST__`/`__DTOR_LIST__` (-1 head, 0 tail) -> mingwrt `__main` walks them; dtors via `atexit` |
| dllimport data (`__declspec(dllimport)` direct references) | complete: lld-link `-auto-import` (CeGCC's binutils default `--enable-auto-import`) synthesizes `.refptr` per-object stubs and `-runtime-pseudo-reloc` emits `__RUNTIME_PSEUDO_RELOC_LIST__`, processed at startup by mingwrt's `_pei386_runtime_relocator` (in libmingw32) |
| C++ exceptions / unwinding | complete: ARM EHABI `.ARM.exidx` from clang, `__exidx_start/__exidx_end` bound by lld-link, libunwind + libc++abi (`GenericARM` C++ ABI) |
| atexit / `_onexit` | complete: mingwrt's private atexit table (COREDLL exports neither), flushed by `_cexit` at the end of `crt3`/`dllcrt1` |
| argv / WinMain | complete: mingwrt `crt3.c` dispatches to `WinMain`; `winmain_ce.o` in libmingw32 provides the `WinMain -> main` adapter, `mainCRTStartup` builds argv from `GetCommandLineW` |
| runtime pseudo relocations & import thunks | complete: ARM-mode `ldr ip,[pc]` thunks identical to binutils `jmp_arm_bytes`, `_pei386_runtime_relocator` handles imported-data offsets |
| thread-local storage | **unsupported by the platform**: COREDLL exports no `TlsAlloc`/`TlsFree` (CE allocates TLS per-DLL through the `DllMain` `reserved` parameter), so neither native TLS nor the emutls fallback (whose Windows path requires `TlsAlloc`) can run; `__thread`/`thread_local` are diagnosed as unsupported, exactly as eMbedded Visual C++ did. The WinCE targets document this and do NOT claim emutls lowering |
| profiling (`-pg`, `gcrt3.o`/`libgmon`) | not provided by the mingw32ce build of mingwrt (the `profile/` sources are desktop-CRT only); CeGCC's LIB_SPEC kept the hook but nothing satisfied it |

## Architecture baseline (ARM926EJ-S / i.MX28 / ARMv5TE / armel)

* Default CPU: **arm926ej-s** — the ARMv5TE generation core of the
  Freescale i.MX28 family (the dominant WinCE 5.0/6.0 SoC and the one
  eVC4/Platform Builder shipped for).  Implemented in
  `llvm/lib/TargetParser/ARMTargetParser.cpp` (`Triple::WinCE` case of
  the OS-default-CPU rule), so it applies to the clang driver and to
  llvm-mc alike.
* Default ABI: soft-float (`FloatABI::Soft` for WinCE in
  `clang/lib/Driver/ToolChains/Arch/ARM.cpp`) — AAPCS with no VFP, i.e.
  the **armel**-equivalent ABI and the COREDLL floating-point calling
  convention.  The sysroot stage builds with `-march=armv5te
  -mfloat-abi=soft` by default.
* Older hardware: override per invocation — `-march=armv4t` (ARM920T /
  i.MX21, i.MX1/S3C2410-class), `-mcpu=arm920t`, or any other ARMv4T/5TE
  CPU; the sysroot stage honors `WINCE_ARCH_FLAGS` the same way.
* Thumb: supported by the code generator, but CeGCC's own toolchain
  shipped it as broken on WinCE ("Thumb support for arm-wince-pe does not
  appear to be working in binutils yet"), so ARM mode remains the
  default; interworking data is emitted so Thumb objects still link.

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
-fms-compatibility-version=1900`), the era's MSVC pragmas are fully
implemented (see `clang/lib/Parse/ParsePragma.cpp`):

* `#pragma auto_inline([on|off])` — functions in an `off` range get
  `noinline` (`__forceinline` still wins, matching MSVC); Sema applies it
  range-based, exactly like `#pragma optimize`.
* `#pragma check_stack([on|off])` — functions in an `off` range get
  `no_stack_protector` (the attribute behind `__declspec(safebuffers)`).
* `#pragma setlocale("<locale>")` — full MSVC grammar with diagnostics;
  clang fixes the narrow-literal charset per translation unit via
  `-fexec-charset`, so the mapping is reported (MSVC codepage equivalent).
* `#pragma conform(name, on|off[, push|pop[, id]])` — full MSVC grammar
  with diagnostics; `'for'`-scope conformance is fixed by the language
  mode, and the clang mapping is reported.
* `#pragma runtime_checks` remains accepted-and-ignored (it guards
  /RTC checks, which have no clang equivalent).

Storage-class syntax: redundant identical specifiers — `extern extern`,
`static static`, `inline extern` orderings, `extern __inline`, and
`extern __forceinline` — are accepted with the extension warning
(`-Wduplicate-decl-specifier`), matching MSVC's tolerance for these
machine-generated and ported-header patterns; mixing *different* storage
classes remains an error (MSVC C2159 likewise).
See `clang/test/Sema/ms-extern.c`.

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
