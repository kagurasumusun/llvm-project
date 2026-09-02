# Windows CE toolchain: sysroot, runtime and compiler-side support

> **Current status (2026-08-31, repo reorganization complete):** this
> repository holds the **compiler side only** (driver, cmake cache,
> lld/COFF, lit tests, docs; CI = Stage 1 + WinCE lit gate). The full
> toolchain pipeline (sysroot, runtimes, packaging, TECLIB/glpi,
> EasyRPG Player) now runs in
> **[kagurasumusun/cellvm-build](https://github.com/kagurasumusun/cellvm-build)**,
> which consumes `llvm-project@llvm-wince` plus
> [`mingwrt`](https://github.com/kagurasumusun/mingwrt),
> [`w32api`](https://github.com/kagurasumusun/w32api) and
> [`pthread-win32`](https://github.com/kagurasumusun/pthread-win32) as
> submodules (the WinCE fixes are pushed to those repositories).
> See **[STATUS.md](STATUS.md)**. Where this README disagrees, STATUS.md wins.


This directory holds the tooling that turns a freshly built LLVM/Clang/LLD
(the "stage 1" host toolchain, configured with
`clang/cmake/caches/WinCE.cmake`) into a complete, self-contained
Windows CE (Windows Embedded CE 6.0 focus) cross toolchain whose C runtime
and platform headers are the **unmodified** CeGCC-lineage

* [`kagurasumusun/mingwrt`](https://github.com/kagurasumusun/mingwrt) (mingwrt),
* [`kagurasumusun/w32api`](https://github.com/kagurasumusun/w32api) (w32api),

plus the pthread-win32 static thread library.  There is no bespoke CRT: the
sysroot is assembled by building those trees **with their own
configure/make**, with Clang in place of GCC and the LLVM binary tools in
place of binutils — the same role CeGCC's GCC played.

Since the 2026-08-31 reorganization the pipeline is not in this
repository: it lives in `kagurasumusun/cellvm-build` (submodule + build
scripts + CI, in the cegcc-build style).  The stages, as executed there:

```
stage 1  clang/lld/llvm-tools host build     (this repo, clang/cmake/caches/WinCE.cmake)
stage 2  sysroot: mingwrt + w32api + pthread (cellvm-build/build-wince-sysroot.sh)
stage 3  compiler-rt + libunwind/libc++abi/libc++  (cellvm-build/build-wince-runtimes.sh)
stage 4  unmodified TECLIB/glpi-wince-agent   (cellvm-build CI)
stage 5  MaxSignal/EasyRPG Player 0.6.2.3-wince (cellvm-build CI)
```

The in-house sysroot code (gmon, posix, include-overlay) and the build
scripts that used to live under `wince-sysroot/` / `utils/wince/` in this
repository moved to `cellvm-build` as well; this README remains the
authoritative spec for how the pieces fit.

## Scope and non-goals (read this before anything else)

**The one supported deployment target of this toolchain is Windows CE on
32-bit ARM, ARMv5TE-class, the `armel` ABI (little-endian, soft-float,
AAPCS), default CPU `arm926ej-s` (the Freescale i.MX28 family).** See
"Architecture baseline (ARM926EJ-S / i.MX28 / ARMv5TE / armel)" below.

Everything else is **out of scope — a non-goal**. Do not implement,
"complete", extend, or verify any of the following as part of this
toolchain:

* **x86 / i386 (CEPC, `i386-mingw32ce`, `i386-pc-wince`)** — at most a
  TargetInfo/driver stub that lets Clang *parse and compile* for the x86
  spelling. There is no CE-specific x86 runtime path (the compressed
  `.pdata` SEH emitter, the entry/export Thumb-bit convention, and the CE
  machine dispatch are all ARM-only). It is **not** a supported end-to-end
  Windows CE target and is not to be finished.
* **x86-64 / AArch64 / ARM64 / ARM64EC / ARM64X** — desktop Windows on ARM
  and 64-bit Windows. Not a Windows CE target at all.
* **armhf / hard-float, big-endian** — CE has no big-endian variant and the
  soft-float AAPCS (armel) ABI is the only supported one.
* **cross-CPU "completeness"**: MIPS / SH3 / SH4 / PPC import thunks in lld,
  or any port of the CE runtime to a non-ARM CPU. Codegen that happens to
  exist for these elsewhere in LLVM is irrelevant here.

When a review, evaluation, or audit tables x86/64/ARM64 items (for example
"WinCE x86", "the AArch64/ARM64EC machine-dispatch sites", "ARM64EC export
thunks"), read them as **context notes** about code that already exists in
upstream LLVM — **not** as work items for this toolchain. Do not let them
divert effort from the ARMv5TE/armel target.

## Stage 2: the sysroot

    cellvm-build: sh build-wince-sysroot.sh --toolchain <prefix>/bin \
        [--target arm-pc-wince] [--prefix <sysroot>] [--jobs N]

`<sysroot>` defaults to `<prefix>/wince-sysroot` (a sibling of the `bin`
dir the compiler is installed in), which is exactly the location the WinCE
driver probes.  The script lives in the `cellvm-build` repository together
with the mingwrt/w32api/pthread-win32 submodules it consumes; it is not an
LLVM runtime project and there is no `wince-sysroot/CMakeLists.txt` in this
repository (the in-tree copy and its CMake registration were removed in the
2026-08-31 reorganization).

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

The trees are the actual repositories (submodules of `cellvm-build`); the
WinCE changes that used to live only in this repository's vendored copy
were pushed to them on 2026-08-31 (see WINCE-HANDOFF.md §14.2):

* **`kagurasumusun/mingwrt` @ `69043bc`** (master) — upstream `7c35691`
  plus: clang build support (the `__declspec` probe in `_mingw.h` accepts
  `__clang__`; `Makefile.in` preprocesses the generated `.def` files
  without `-C`), clang/libc++ header compatibility (float.h / stdlib.h /
  `__small`), the COREDLL def completion (30 export names in
  coredll.def/coredll6.def), the CE math set (8 objects in
  mingwex/wince + Makefile registration), and C17-named setlocale params
  in coredll_stubs.c.
* **`kagurasumusun/w32api` @ `7192b73`** (wip) — upstream `51de0ad` plus
  the same COREDLL def completion in libce/coredll.def.
* **`kagurasumusun/pthread-win32` @ `4ae6417`** (master) — GerHobbelt's
  actively maintained combined successor of pthreads-win32/pthreads4w
  (upstream pthreads4w is dormant) at `06e7608` plus three WinCE 6.0 build
  fixes:
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

## PDB / CodeView debug info (2026-08-31)

* **DWARF** (the `-g` default): emitted as before (DWARF4 on COFF). Works,
  but is not the native Windows format; WCE debug servers do not read
  DWARF.
* **CodeView** (`-gcodeview`): accepted for `arm-pc-wince`; the object
  carries the CodeView sections (`.debug$B` line numbers, `.debug$S`
  symbol records, `.debug$T` type records) — verified by
  `clang/test/CodeGen/ARM/wince-codeview.c`.
* **Link**: the driver passes `-debug` to `lld-link` whenever the user
  asks for debug info (`-g`, `-g1..3`, `-gcodeview`, ...; not `-g0`),
  mirroring the MSVC convention. `lld-link -debug` merges the CodeView
  sections from the objects and writes the debug directory into the
  image. End-to-end check in the cellvm-build CI (Stage 3 smoke).
* **PDB**: lld has no PDB writer, so a standalone `.pdb` file is NOT
  produced. The CodeView data embedded in the image is the PDB's raw
  material; a debugger/tool that needs a PDB would have to synthesize
  one from the image (no in-tree tool does this yet). This is the
  known gap against the MSVC toolchain for CE debugging.

## Stage 3: compiler runtime + C++ runtime

    cellvm-build: bash build-wince-runtimes.sh --toolchain <prefix>/bin \
        [--sysroot <sysroot>]

Cross-builds `compiler-rt/lib/builtins` (the `-lgcc` replacement) and the
static `libunwind + libc++abi + libc++` stack against the stage-2 sysroot
and stages them as `libclang_rt.builtins-<arch>.a`, `libunwind.a`,
`libc++abi.a`, `libc++.a` plus `include/c++/v1`.  These sit on **bare CE**
(mingwrt + COREDLL).  libc++/libunwind are `ENABLE_THREADS=OFF`;
pthreads4w is an optional extra, not the C++ thread API.  CE is not
`_LIBCPP_WIN32API` / `_LIBCPP_MSVCRT_LIKE`.  Static libc++ is hermetic
(`LIBCXX_HERMETIC_STATIC_LIBRARY=ON`).  compiler-rt CRT objects stay off
(`COMPILER_RT_BUILD_CRT=OFF`; PE startup is mingwrt `crt3.o`).

## Driver behavior (the Clang side)

`clang --target=arm-pc-wince` (alias `arm-mingw32ce`) selects the WinCE
toolchain (`clang/lib/Driver/ToolChains/WinCE.{h,cpp}`), which reproduces
the CeGCC link line on lld-link:

| CeGCC (gcc specs)                | clang driver                                        |
|----------------------------------|-----------------------------------------------------|
| `STARTFILE_SPEC` crt3/dllcrt3    | `crt3.o` (EXE) / `dllcrt3.o` (DLL) from the sysroot |
| `-e DllMainCRTStartup` (DLL)     | `/entry:DllMainCRTStartup`                          |
| pe.em subsystem-9 default entry  | `/entry:WinMainCRTStartup` (always; mingwrt `winmain_ce.o` bridges `main`) |
| arm-wince emulation defaults     | `/subsystem:windowsce /base:0x10000 /fixed` (DLLs: `0x10000000`, keep `.reloc`) |
| `%{mthreads:-lmingwthrd} -lmingw32 -lgcc -lceoldname -lmingwex -lcoredll` | `-mthreads`/`-pthread`: `libmingwthrd.a` (mingwrt, as CeGCC) + `libpthread.a` (pthreads4w) and `-D_MT`; then `libmingw32.a`, `libclang_rt.builtins-*.a`, `libceoldname.a`, `libmingwex.a`, `libcoredll.a` |
| `%{mthreads:-D_MT}` (CPP_SPEC)   | `-D_MT` at compile time                             |
| (GNU ld pe.em: __CTOR_LIST__/__DTOR_LIST__ bracketing) | clang emits global ctors/dtors in the GNU convention for WinCE (`.ctors`/`.dtors`, priority subsections `.ctors.NNNNN`, associative grouping); lld-link `-wince` sorts and brackets them with the -1 head sentinel and 0 terminator, `__CTOR_LIST__`/`__DTOR_LIST__` point at the head, and mingwrt's `__main` (`gccmain.c`) walks them: global C++ constructors run before `main`/`WinMain`, destructors run through the atexit table.  (The MSVC `.CRT$XCU`/`.CRT$XTX` tables would need `__xc_a` startup objects that the CE runtime does not provide — verified not used for this target.) |
| (GCC default: gnu89 inline)      | `-fgnu89-inline` by default — the pre-C99 `extern __inline` convention used by eMbedded Visual C++ / old mingwrt headers keeps its external-definition semantics (`-fno-gnu89-inline` to override) |

Libraries are GNU-named (`lib<name>.a`) only — no MS `.lib` aliases.
`-mthreads`/`-pthread` add `libmingwthrd.a` + `libpthread.a`; `libposix.a`
is **not** on the default line.  `-Wl,` GNU spellings
(`-subsystem`, `-e/--entry`, `--image-base`, `--stack`, `--dll`, `--def`,
`--out-implib`, `--major/--minor-image-version`, `--dynamicbase`) are
translated to their lld-link forms.

## Linkage/runtime chain audit (symbol-level, exhaustive)

Beyond the behavioral chains below, the sysroot stage is audited at the
COFF/ELF symbol level: every archive object's undefined references are
extracted and checked against (a) the other archives and (b) the
COREDLL import surface.  As of the current tree the residual is **zero**
(only linker-provided symbols - __CTOR_LIST__/__DTOR_LIST__/__ImageBase/
__text_start__/main - and compiler-rt `__aeabi_*` helpers remain, by
design).  Breakages this audit caught and fixed:

* `pseudo-reloc.o -> __text_start__` - now bound by lld-link -wince
  (.text bounds; mingwrt derives the image base from it on WinMo 6.1+
  loaders).
* `pthread.o -> errno` (data import) - pthreads4w's need_errno.h picks
  the `extern int errno` branch without `-D_MT`; the stage now builds
  with `-D_MT` so errno resolves through mingwrt's `_errno()`.
* mingwex CE set was missing `copysign/finite/hypot/ecvt/fcvt`, the
  whole `*l` math family, `carg/cargf` (referenced by clog/cpow despite
  the CE filter), `wcsdup` (coredll6.def exports `_wcsdup` - a desktop
  underscore leftover - so it never matched) and the `_imp__pow`
  dllimport-pointer used by cpow.  All provided additively in
  `mingwex/wince/` (policy: additive, ABI untouched).

Known caveats from the audit (not fixable without semantic changes):

* `errno` on this runtime is a **single shared static** (mingwrt's
  `_errno()` returns `&static`): two threads doing failing operations
  race on errno.  A thread-aware `_errno()` would need a TLS slot,
  which CE does not allocate (see TLS row above).  Documented;
  changing it is a runtime-semantics decision, not a link fix.
* pthreads4w + raw `CreateThread` mixing is supported for API calls,
  but `pthread_exit` must not be called from an implicitly-created
  thread (pthreads4w documented limitation on all platforms).

### Naming note

`pthread-win32` and `pthreads4w` are the same code lineage: the project
was renamed pthreads4w for v3 (sourceware -> GitHub), and
`GerHobbelt/pthread-win32` is the actively maintained combined fork of
that tree.  `kagurasumusun/pthread-win32` tracks the fork's `master`
(upstream `06e7608`) plus the three WinCE fixes, and `cellvm-build` pins it
as a submodule - i.e. we are on the newest available code of the newest
available lineage.

### fork emulation: ecosystem survey

No existing project implements fork on Windows CE (surveyed: Cygwin
itself needed kernel cooperation it only partly got on Windows; WSL1
implemented fork *inside the Windows kernel*; no CE equivalent exists -
CE has no COW, no handle enumeration, no child-image primitive, and the
required process-memory APIs are not in the COREDLL def surface).  The
only CE-relevant finds are emulators (`gweslab/cerf`) and the CeGCC
continuation forks (`salman-javed-nz/cegcc-build`, GCC 14.2) - neither
provides fork.  Conclusion stands: exec/spawn substitution only.

### mingwrt language-generation update: DONE (C17, one pass)

The entire CE build set (CRT0S, MINGW_OBJS(ce), the full CE mingwex
set, pthreads4w, gmon, posix - 290+ objects) is now **pinned and
verified at C17**: the sysroot stage compiles with explicit
`-std=c17`/`-std=gnu17` (GNU inline semantics stay pinned via
`-fgnu89-inline`), and the strict pass compiles the whole set with
**zero compiler warnings and zero errors**.  Fixes made during the
pass: named parameters in coredll_stubs.c, the gmon CONTEXT alignment
spelling, posix `_strdup`/stdarg dependencies, popen "w"-mode
determinism (child deferred to pclose), and system() returning the
POSIX wait-status encoding.  Device behavior is unchanged (ABI frozen;
same exports, same semantics).

### Performance

* Language refresh: no measurable change (same optimizer, same codegen;
  potentially marginal gains from clearer aliasing).
* emutls (`thread_local`): each access is a TlsGetValue + indirect load -
  measurably slower than native TLS and slower than desktop emutls-
  with-caching; use thread_local sparingly on CE.
* `-auto-import`: dllimport data gets a .refptr indirection (same cost
  CeGCC paid).
* `-pg`: sampling runs in a below-normal-priority thread; expect a few
  percent under profiling.
* Everything else (ctor/dtor bracketing, def completion) is link/load
  time only.

Updating the vendored mingwrt sources to modern C (C99/C11/C23
conformance) is compile-time-only: the device-visible behavior changes
only if (a) an exported symbol's ABI changes or (b) code semantics
differ.  Under the ABI-freeze policy (exports/CRT0S/MINGW_OBJS frozen)
plus a behavioral-equivalence review, refreshed sources produce
binaries that run identically on real devices.  The practical risks to
gate in review are C23 keyword absorption (`bool`/`true`/`false`
becoming keywords), inline-semantics drift (we pin `-fgnu89-inline`
everywhere the CRT builds), and implicit-declaration removal - all
mechanical, all verifiable by the device test procedure.

### WinCE generations 1-6 and historical CPUs

| generation | build | notes |
|---|---|---|
| CE 6.0 | yes (default) | `coredll6` surface |
| CE 5.0 | yes | `arm-pc-wince5.0` -> `coredll` surface |
| CE 4.2 / 4.1 / 4.0 (.NET) | yes, with caveat | `arm-pc-wince4.2` + `_WIN32_WCE=0x400/420`; import surface is still the CE 5.0 def - a generation-matched `coredll4.def` (def-only, ABI-frozen) closes it fully |
| CE 3.0 (Pocket PC 2000/2002, HPC2000) | same as 4.x | ARMv4T CPUs (ARM720T/ARM920T) via `-march=armv4t`; StrongARM SA-1100/1110 via `-mcpu=strongarm`/`-march=armv4` (no Thumb, codegen supports it) |
| CE 2.x / 1.0 | same mechanism | radically smaller API surface; needs a generation def file; H/PC Pro era |
| CPUs | **ARM (v4/v4T/v5TE) is the only supported deployment target** (default arm926ej-s / ARMv5TE, `armel` soft-float; override with `-march=armv4t` etc. for older ARM hardware). x86 (CEPC, i386-mingw32ce) is a TargetInfo/driver compile stub only — there is no CE-specific x86 runtime (compressed `.pdata` SEH, entry/export Thumb-bit convention, CE machine dispatch) — it is **out of scope**, not end-to-end. SH3/SH4 have no LLVM backend; MIPS/SH3/SH4/PPC import thunks in lld are **non-goals** (see "Scope and non-goals"). |

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
| branch range extension (far A32 B/BL) | added 2026-08-30 (covered by lld lit in Stage 1): `RangeExtensionThunkARMCE` in lld - the same jmp_arm_bytes pair as the import thunk plus the target's absolute address literal (HIGHLOW base relocation on non-fixed images); A32 branch callers are thunked, and the CE symbol-value convention (bit 0 = Thumb) interworks the stub to either code mode. Out-of-range branches from Thumb-mode callers (T1/T32) still fail with "relocation out of range" until Thumb sections are placed at odd RVAs (WINCE-HANDOFF.md section 13.6). The PE entry point / export table keep the symbol's bit 0 for CE (no forced `|= 1` as on ARMNT - that would corrupt ARM entries). Tests: `lld/test/COFF/wince-range-thunk.s`, `lld/test/COFF/wince-thumb-bit.s` |
| thread-local storage | **supported via emutls**: CE has exported `TlsAlloc`/`TlsFree` since CE 1.0 (MSDN, "Windows CE OS 1.0 and later") - the CeGCC `coredll.def` simply omitted them; the vendored def files list them now.

### COREDLL def completeness (web-verified)

The authoritative completeness reference is a `dumpbin /EXPORTS
coredll.dll` dump of a real CE image (the 2010 CE5/WM6 dump archived on
cnblogs: 1799 functions).  **All 9 chunks have now been ingested: every
one of the ~1799 export names was diffed against the vendored defs, and
the complete missing set - 30 names (CeMapArgument, allPrivilege, and
28 C++-mangled operator-new/delete / std-internal / IME exports that
CeGCC omitted on purpose) - has been added.**  Zero gaps remain
(27KB of mangled names are inert under libc++: import libraries resolve
only referenced symbols).  `utils/wince/audit-coredll.py` mechanizes
re-verification: run it against a `dumpbin /EXPORTS` of YOUR device's
CoreDLL.dll (OEM variation is real) and it lists def gaps/extra
entries. `thread_local`/`__thread` lower to emutls (`__emutls_v.*` + `__emutls_get_address` from compiler-rt, whose Windows path uses TlsAlloc), and `Triple::hasDefaultEmulatedTLS()` covers WinCE so `-femulated-tls` is the driver default.

(The `-pg` profiler row of the old table here was stale: `libgmon.a` IS
provided now - the in-house sampler in cellvm-build `sysroot/gmon/`, which
the driver links by default.  And the `_errno()` caveat is obsolete: this
fork's `coredll_stubs.c` provides `_errno` as a per-thread TlsAlloc slot,
not a shared static.)

## Version policy (nothing is hard-wired to a single generation)

| knob | default | override |
|---|---|---|
| CE OS generation | Windows Embedded CE 6.0 (`_WIN32_WCE=0x600`) | versioned triple `arm-pc-wince5.0` / `arm-pc-wince6.0` (also feeds `_WIN32_WCE`), or `-D_WIN32_WCE=...` |
| COREDLL import surface | `libcoredll6.a` (CE 6.0-only exports: `CeGetThreadPriority`, `FindFirstDevice`, ...) | link-time: chosen from the triple version - `arm-pc-wince5.0` selects `libcoredll.a` (the CE 5.0 surface) |
| MSVC compat persona | VS2015 (`-fms-compatibility-version=1900`) | `-fms-compatibility-version=1400` when importing eVC4/VS2005-era SDK headers that probe `_MSC_VER` |
| GCC persona | GCC 14.2 (`-fgnuc-version=14.2`) | `-fgnuc-version=X.Y` for headers with `__GNUC__`-version gates |
| CPU | arm926ej-s (ARMv5TE) | `-march=`/`-mcpu=`; sysroot stage: `WINCE_ARCH_FLAGS` |

So neither the MSVC generation, the GCC persona, the CE generation nor the
CPU is baked in - the defaults simply match the dominant deployment
(CE 6.0 on i.MX28-class hardware).  Tools driven by these macros (mingwrt/
w32api headers) check capabilities, not personas, and behave identically
for any of the overrides above.

## POSIX surface (beyond pthreads)

The POSIX surface comes from two already-vendored layers - no additional
runtime was written:

* **pthreads4w** (`cellvm-build` submodule `pthread-win32/`): `pthread_*`,
  `sem_*`, `sched_*` - threads, mutexes, condvars, semaphores, barriers,
  rwlocks, spinlocks, cancellation.
* **mingwrt's CE mingwex set** (linked as `libmingwex.a`): `open`/`read`/
  `write`/`close`/`lseek`/`access`/`chmod`/`stat`/`rename`/`mkdir`/
  `rmdir`/`unlink`/`utime`/`futime`/`fdopen`/`dirent`/`getopt`/
  `time`/`gmtime`/`localtime`/`mktime`/`strftime`/`gettimeofday`/
  `basename`/`dirname`/`tsearch` family, wide-char variants, `imax*`
  inttypes - each backed by the closest COREDLL Win32 call.
* **Implemented POSIX process/signal layer** (`cellvm-build/sysroot/posix/`,
  built as `libposix.a`, **optional extra — not on the default driver
  link line**; headers: `sys/wait.h`):
  * `execv`/`execvp`/`execl`/`execlp` - CreateProcess-based image
    replacement approximation (create, wait, exit with the child's
    code, so a waiting parent observes the right status).
  * `system()` - CreateProcess directly (CE has no shell; documented).
  * `waitpid` - runtime child table (exec/system/popen children) over
    WaitForSingleObject + GetExitCodeProcess; WNOHANG supported.
  * `popen`/`pclose` - implemented with a documented platform limit:
    CE cannot redirect a child's stdout (no `CreatePipe`, no child
    std-handle inheritance), so the stream carries nothing from the
    child; pclose still returns the real exit status.
  * `signal`/`raise`/`alarm` - registry + cooperative delivery,
    SIGALRM via a timer thread; fault-delivered SIGSEGV/SIGFPE need
    the vectored-exception export which the CeGCC def does not list
    (kernel-level availability would be a def-only follow-up).
  * `fork`: impossible - CE has no COW/child-image primitive; the
    CreateProcess + state-rebuild route (the Cygwin model) would
    require tracking every allocation/handle/thread - an OS-layer
    project, not a runtime function.

## POSIX / native mixing

Mixing POSIX-style and native Win32 code in one binary is safe by
construction, with two documented caveats:

* File descriptors ARE kernel handles: mingwrt's CE `open()` family
  (`mingwex/wince/open.c`) returns the `HANDLE` from `CreateFileW` cast
  to `int`, and `_get_osfhandle(fd)` is the identity (`coredll_stubs.c`).
  So `read(fd)` on an fd you created with `CreateFileW` works, and
  mixing `fopen`/`fread` with `CreateFileW`/`ReadFile` on the same file
  is ordinary Win32 sharing semantics.
* Caveat 1 - errno: mingwrt's `_errno()` returns a single shared
  static (thread-aware errno would need a TLS slot; now that
  TlsAlloc is in the def set this is implementable - a runtime
  semantics decision, parked).
* Caveat 2 - threads: pthreads4w threads are plain CE threads
  (`NEED_CREATETHREAD` -> `CreateThread`), so raw `CreateThread` threads
  may call pthread mutexes/condvars/semaphores; they must not call
  `pthread_exit` (implicit-thread limitation, all platforms).

## -pg profiling (implemented)

`-pg` is fully wired: the driver links `gcrt3.o` (mingwrt's crt3 startup
wrapped with the profiler lifecycle, replacing `crt3.o` per CeGCC's
`%{pg:gcrt3%O%s}`) and `libgmon.a` (CeGCC's `%{pg:-lgmon}` position).
Both live in `cellvm-build/sysroot/gmon/` and are built by the sysroot stage
(user-mode only; no WinCE platform behavior is touched).

* What you get: a BSD/gprof `gmon.out` written next to the executable at
  exit, containing a `GMON_TAG_TIME_HIST` histogram over the module image
  (`gprof ./program.exe gmon.out` gives the flat profile).  The sampler
  runs in a below-normal-priority thread (10 ms interval,
  `SuspendThread`/`GetThreadContext`/`ResumeThread`), so it covers
  `main`/`WinMain` and the atexit/destructor phase alike.
* What you do not get: the call-graph arc table.  clang's `-pg` emits a
  plain `bl mcount` without the EABI `__gnu_mcount_nc` frame contract, so
  `mcount` cannot recover `frompc`; it is provided as a no-op so `-pg`
  links everywhere the attribute is emitted.  (Sampled-PC histograms are
  exactly what gprof falls back to when arcs are absent.)
* History of CE generations: the toolchain targets the COREDLL surface as
  of CE 5.0/6.0 (CeGCC lineage).  CE 4.2 is reachable (`arm-pc-wince4.2`,
  `_WIN32_WCE=0x420`, ARMv4T `-march=armv4t` for SA-1110/ARM720T), but the
  import surface comes from the CE 5.0 `coredll.def`, so CE <= 4.1 devices
  need a generation-matched `coredll3.def` added to mingwrt (a def-only,
  ABI-frozen change per the update policy).  MIPS/SH3/SH4/PPC and x86/ARM64
  import thunks are **not part of this toolchain** (see "Scope and
  non-goals"); the supported CPU is 32-bit ARM only.

## mingw-w64 update safety

Copying code from mingw-w64 is safe **only for self-contained parts**
(gdtoa, `_pformat`, pure computation): they make no OS calls.  Anything
that touches startup, TLS, process/environment, console or heap - i.e.
anything against msvcrt/desktop APIs (`VirtualAlloc` with
MEM_RESET, `FlsAlloc`, `GetConsoleWindow`, `_beginthreadex`, ...) - will
not run on WinCE and must not be ported into the CE mingwrt.  The same
rule that protects the ABI also protects the platform boundary.

## MSVC source-compat inventory (the full checklist)

| MSVC feature | status here |
|---|---|
| SAL annotations (`_In_`/`_Out_` + old `__in` style) | **provided**: `cellvm-build/sysroot/include-overlay/sal.h` (standard no-op spellings, analysis-off mode, old-style kept); CE-era headers use none/old-style and now both compile |
| `<intrin.h>` (ARM) | **provided**: overlay `intrin.h` - MSVC spellings mapped (barrier/cache-flush/CLZ); `__dmb/__dsb/__isb` use native instructions on ARMv6+ and a CacheSync system barrier on the ARMv5TE baseline (the clang ACLE header has no v5 lowering) |
| MSVC CRT | **by design replaced** by mingwrt (that IS the CeGCC CRT contract); `_MSC_VER`-conditional MSVC-CRT-specific extensions (e.g. `__dbg` heap APIs) are out of scope |
| MSVC type mapping (`__int64`, `SIZE_T`, `DWORD_PTR`, ...) | provided: mingwrt/w32api headers define the full set; `__int64` is a clang keyword alias |
| `__uuidof` / `__declspec(uuid)` | clang supports both (Sema `ActOnCXXUuidof`, `UuidAttr`); `__uuidof` returns the compiler-generated GUID per C++ ABI (GenericARM Itanium mangling); MSVC-style `__uuidof` template capture works |
| operator new/delete from COREDLL | verified present in the def (mangled `??2@...`/`??3@...`/`??_U@...`/`??_V@...`); libc++ uses its own operators, C code never imports them |
| SEH (`__try`/`__except`/`__finally`) | **implemented on ARM**: `__try`/`__except`/`__finally` parse and lower to the CE compressed `.pdata` SEH mechanism with `__C_specific_handler`, **per function**, so EHABI C++ exceptions (libc++ + libunwind) and SEH coexist in one TU (`utils/wince/WINEH-ABI-FACTS.md` §4d/4f/4g; tests: `clang/test/CodeGen/wince-seh.c`, `clang/test/CodeGen/ARM/wince-seh-{scope-table,ehabi-mixed}.c`, `llvm/test/MC/ARM/wince-seh-pdata.s`, `lld/test/COFF/wince-pdata.test`). Known limits (source-verified, not device-verified): nested `__try` inside an outlined funclet and variable-sized frames are unsupported, and the x86 CE path is not connected to the CE `.pdata` emitter (ARM only) |
| `<fcntl.h>` `<conio.h>` `<io.h>` `<process.h>` | provided by mingwrt (verified: `_getch/_kbhit/_putch`, `_findfirst` family, spawn/exec decls) |
| `<tchar.h>` | provided (87 `_tcs*` mappings, `_T()`), UNICODE build assumed like eVC |
| `<wincrypt.h>` | provided (w32api CE), backed by the Crypt* exports in the def (verified) |
| `<windows.h>` | provided (w32api CE 255-header SDK: windef/winbase/wingdi/winuser/winsock/... + pshpack/poppack) |
| C++ (libc++) | C++11/14/17 + most C++20; `thread_local` via emutls |

## armasm as a proper LLVM mechanism

Two integration paths exist; the LLVM codebase already contains 80% of
the second one:

* **Path A (shipped here): the pre-assembler**
  (`utils/wince/armasm/armasm-convert.py`).  Zero LLVM changes,
  maintains upstream cleanliness, full armasm surface covered.  This is
  what the build uses today.
* **Path B (in-tree, opt-in): `ARMCOFFMasmParser`**, an
  `MCAsmParserExtension` on top of the regular ARM AsmParser
  (`llvm/lib/MC/MCParser/ARMCOFFMasmParser.cpp`), reached with
  `clang -masm=armasm` or `llvm-mc -masm-armasm`.

  **Status: 部分実装 (partial).**  It implements armasm's structural
  directives *and* the basic statement syntax built on top of them:

  * **structural**: `AREA` (including armasm's `|name|` spelling),
    `ALIGN`, `EXPORT`/`GLOBAL`, `IMPORT`/`EXTERN`, `EXPORTAS`, `ENTRY`,
    `PROC`/`ENDP`/`END`; `PRESERVE8`/`REQUIRE8`/`CODE16`/`CODE32`/`ARM`/
    `THUMB`/`OPT`/`TTL`/`SUBT`/`ROUT`/`KEEP`/`NOFP` are ignored.
  * **statements**: `;` comments (added to, not replacing, this target's
    own `@`), a label written in front of a directive with no `:` after
    it (`Foo PROC`, `Bar DCD 1`, `Baz EQU 4`), and a label standing
    alone on a line.
  * **data**: `DCD`/`DCW`/`DCB`/`DCQ` (DCB also takes a quoted string),
    `DCFS`/`DCFD`, `SPACE`, `FILL n{, value}` and `EQU`.
  * **literals**: `&FF` [hex], `%1010` [binary] and `n_xxxx` [base n],
    on top of the `0x` prefix the generic lexer already handles.

  Mnemonics keep coming from the ARM instruction parser.
  `llvm/test/MC/ARM/wince-armasm.s` (structural) and
  `llvm/test/MC/ARM/wince-armasm-labels.s` (statements + data) pin that
  down.

  Since 2026-09-01 it also covers, through the existing generic machinery
  (no private re-implementation): `GBLA`/`GBLL`/`GBLS` and the local
  `LCLA`/`LCLL`/`LCLS` declarations, `SETA`/`SETL` assignments (`EQU` is an
  equivalence, like `.equ`), the unaligned `DCBU`/`DCWU`/`DCDU`/`DCQU`/
  `DCFU`/`DCFSU`/`DCFDU` data forms, and `IF`/`ELSEIF`/`ELSE`/`ENDIF`/
  `IFDEF`/`IFNDEF` as aliases of the generic conditional assembly (same
  nesting and skip logic; `addAliasForDirective` is an existing
  `MCAsmParser` API).

  Still missing, so it is **not** yet a substitute for Path A: the macro
  processor (`MACRO`/`MEND`, `WHILE`/`WEND`, `GET`/`INCLUDE`/`LTORG` are
  diagnosed by name, never silently ignored) and the `SETS`/`SETB`
  spellings plus the literal `IF :DEF:` form (`IFDEF`/`IFNDEF` are the
  spellings that work).  Because of that the driver
  does **not** default WinCE assembly to it: `-masm=armasm` must be
  requested explicitly, and Platform Builder sources should keep going
  through Path A.
  (Making the dispatch reachable was the first step: `AsmParser::
  parseStatement` only looked directives up when the identifier starts
  with `.`, so the extension, which registers bare names, could never
  fire at all.)

  The "full MASM-family parser" route that was sketched here before -
  LLVM already ships `MasmParser.cpp` + `COFFMasmParser.cpp` +
  `llvm-ml`, x86-only only because nobody registered ARM equivalents -
  remains the right endgame if a complete in-tree armasm assembler is
  wanted (`llvm-ml` drives its own parser and lexer, which is what the
  missing `;`-comment and macro support needs).  Path B is a
  directive-level foundation on top of the GNU parser, not that parser.

A complete armasm -> GNU unified-syntax translator.  Pipeline:
`armasm source -> armasm-convert.py -> GNU .s -> clang -x
assembler-with-cpp (integrated ARM as) -> COFF object`.

Covered (full armasm statement surface):

* **Directives**: AREA (CODE/DATA/READONLY/READWRITE/NOINIT/ALIGN=n ->
  .section with flags + .align), PROC/FUNC & ENDP/ENDFUNC (framed with
  .size), EXPORT/GLOBAL/IMPORT/EXTERN/EXPORTAS, DCD/DCDU/DCI, DCB
  (strings + numbers), DCW/DCWU, DCQ/DCQU, DCFS/DCFD, SPACE/FILL,
  ALIGN, EQU (`*`/`=` forms), RN/CN/CP register aliasing, GBLA/GBLL/
  GBLS/LCLA/LCLL/LCLS/SETA/SETL/SETS (passed to cpp), IF/ELSE/ENDIF and
  WHILE/WEND (constant-folded, :DEF: aware), **MACRO/MEND** (positional
  and keyword parameters, `$`-substitution with longest-first
  word-boundary matching), GET/INCLUDE (files inlined), LNK, INCBIN,
  ASSERT (-> .error), ATTR, PRESERVE8/REQUIRE8, THUMB/ARM/CODE16/
  CODE32, LTORG, NOFP, ENTRY, ROUT, KEEP, NOCROSSREF, OPT/TTL/SUBT.
* **Expressions**: `{PC}`->`.`, `{TRUE}/{FALSE}`, `:CHR:`,
  `:LOWERCASE:`/`:UPPERCASE:`, `:DEF:`, `:AND:/:OR:/:EOR:/:MOD:/:SHL:/
  :SHR:`, `2_1010`/`%1010`/`&ff` binary/hex markers, `*` = current
  location.
* **Registers**: full APCS/ATPCS spellings (a1-a4, v1-v8, sb/sl/fp/ip/
  sp/lr/pc) auto-mapped to r0-r15.
* **Comments**: `;` -> `@` (string- and `||`-aware).
* **Mnemonics**: SWI->SVC spelling update; condition codes are already
  GNU-compatible.

Verified end-to-end: a CE-driver-style source (AREA/PROC/IMPORT/EXPORT,
APCS aliases, literal pools `=imm`/`=label`, conditional branches,
DCB strings, DCD tables, ALIGN/SPACE) and a macro test (EQU, IF/ELSE
constant folding, MACRO with `$`-parameters) both translate and
assemble to ARM COFF objects with the in-tree toolchain.

Known simplification: labels that collide with mnemonic prefixes are
disambiguated heuristically (bare identifier = label; identifier +
directive = labeled directive; identifier + operands = instruction) -
armasm itself has the same ambiguity, and PB sources place such labels
on their own line.

## OS build platform: feasibility with self-provided full CE sources

With the OS full source tree provided by you (not the MS shared-source
tree), the entire chain is implementable as an open build platform:

* **Compiler/assembler/linker**: this toolchain (clang/llvm-mc/lld-link)
  + the armasm path above.  Kernel-safe subset documented separately.
* **build.exe replacement**: CE's `build.exe` is a directory-tree walker
  driven by `sources`/`dirs` files with `_TGTCPU`/`_TGTPLAT`
  environment expansion.  A Python/Make reimplementation is mechanical
  (~1-2k lines); it just compiles each directory with fixed flags.
* **sysgen**: SYStem GENeration - the phase that turns SYSGEN_XXX
  environment variables (set by the OS design) into *filtered* .def
  files and link lists.  Concretely: `cesysgen.bat`+`nmake` run
  `cefilter` over e.g. `COREDLL.DEF`, keeping only exports whose
  component SYSGEN var is set (that is why our def files looked
  "incomplete" - the pristine tree is the full surface; sysgen prunes
  it per design).  Reimplementable as: read SYSGEN vars -> per-module
  component lists (from CEBAT/ceconfig data) -> filter .def + generate
  link libs.  Pure text processing, fully open.
* **makeimg / BIB / REG / DAT / DB**: all are documented text formats
  compiled into the NK.BIN image (BIB = module/file sections with
  memory layout; REG = registry hive source; DAT = filesystem init;
  DB = database init).  Each compiler (romimage/makeimg, regcomp...)
  is a self-contained converter; reimplementations exist in the
  community (e.g. CE6 imaging tools) and the formats are stable.  The
  loader (BIN format with e32/o32 headers) is understood - lld already
  emits the PE/COFF the records point at.
* **Catalog items**: the IDE's component database is just metadata
  mapping SYSGEN vars <-> optional modules; with self-owned sources
  you define your own manifest (YAML/JSON) and the sysgen tool above
  consumes it.
* **HAL/OAL**: normal C + the armasm path; links against the kernel
  (kitl, coredll) your sources provide.
* **What remains MSVC-only even with full sources**: nothing in the
  *build tooling* - but your kernel sources must avoid MSVC-only
  extensions (`__try` in kernel code, SEH unwinding info in Nk.lib,
  MSVC name mangling for C++ kernel components).  If your CE source
  tree is a clean-room/free implementation (e.g. a CE-compatible
  kernel), everything is buildable with this toolchain end to end.

Summary: OS image building = compiler(this repo) + build.exe clone +
sysgen clone(def filter) + makeimg/BIB/REG compilers + your own
component manifest.  All implementable; none require Microsoft tools
once the sources are yours.

Feasible in scope: the PB *driver/application* build (sources file ->
cl/armasm -> link against coredll) is reproducible with this toolchain
(a `sources`-file parser + Make/CMake generator is mechanical, and the
CE-target compile flags are fully emulated here).  What is NOT
reproducible: the OS-image build (sysgen, makeimg, BIB/REG/DAT
compilation, catalog items, the kernel HAL, and the MSVC-only kernel
components) - that is an OS-build platform, not a compiler, and the CE6
shared-source kernel still expects armasm + MSVC for several
components.  A `wince-build` tool covering driver/app projects is
another parked, implementable tool.

Already in the sysroot: libc++ (C++11/14/17, most of C++20 minus
`thread_local`), pthreads4w, the mingwex POSIX subset, gprof profiling.
Feasible additions (evaluations, not commitments):

* `threads.h` (C11 threads) - a thin header over pthreads4w; trivial,
  safe, additive.
* `std::filesystem` for CE - libc++'s backend maps to C++17 filesystem
  APIs; a CE backend over `CreateFileW`/`FindFirstFile`/`RemoveDirectory`
  is implementable (CE 6.0 has a real object store with paths).  Medium
  effort; the blocker is libc++'s per-platform filesystem config points.
* `backtrace()`/`backtrace_symbols()` - a thin wrapper over libunwind
  (`unw_step`), which is already linked for exceptions; small and safe.
* Sanitizers (ASan/UBSan) - not feasible: they need page-granular
  memory control (`VirtualAlloc` granularity is 4 KB on CE but the
  kernel lacks guard-page commit semantics and ASan's shadow mapping
  assumptions); UBSan alone (no ASan) might work but is untested.
* Objective-C / OpenMP / sanitizer-coverage - require target work with
  no known WinCE demand; not pursued.

## mingwrt update policy (no WinCE-behavior changes)

Upgrading/refreshing the vendored mingwrt is possible under the "WinCE
spec never changes" constraint, as long as updates are limited to:

1. warning/C99-conformance fixes and errno/locale additions (the existing
   upstream activity on the fork),
2. mechanical refreshes of self-contained third-party parts (gdtoa,
   `_pformat`) from newer mingw-w64 - license-compatible, no CRT0S /
   MINGW_OBJS(ce) / export-set changes,
3. keeping `CRT0S(ce)`, `MINGW_OBJS(ce)`, the `LIBS(ce)` set and
   `coredll*.def` byte-identical - these define the platform ABI.

Any change to (3) would be a WinCE-spec change and is rejected by policy.

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

### armel equivalence (source-verified)

The `arm-pc-wince` target is byte-for-byte the Debian-style **armel**
ABI (ARM EABI, little-endian, soft-float), verified end-to-end in the
source:

* **Little-endian only**: `Triple::WinCE` maps to `WinCEARMTargetInfo`
  (clang/lib/Basic/Targets.cpp), derived from `ARMleTargetInfo`; CE has
  no big-endian variant (the CE loader is LE-only).  `arm-pc-wince`
  canonicalizes to `arm` (LE).
* **C ABI = AAPCS**: `ARMTargetInfo::ARMTargetInfo` calls
  `setABI("aapcs")` for every Windows OS (clang/lib/Basic/Targets/ARM.cpp),
  and `__ARM_EABI__` is defined for WinCE (full EABI runtime
  conventions — `__aeabi_*` helper functions from the compiler-rt
  builtins stage, and the ARM EHABI unwind tables).
* **Soft-float**: `FloatABI::Soft` is the WinCE default
  (clang/lib/Driver/ToolChains/Arch/ARM.cpp, `Triple::WinCE` case) —
  no VFP, exactly the COREDLL floating-point calling convention.  This
  is the defining characteristic of armel (vs. armhf).
* **`va_list` = AAPCS `char *`**: `WindowsARMTargetInfo::getBuiltinVaListKind()`
  returns `CharPtrBuiltinVaList`, the AAPCS stdarg form used by armel
  targets.
* **C++ ABI**: Generic ARM (Itanium-based) via `TargetCXXABI::GenericARM`,
  served by libc++/libc++abi — the open-source C++ runtime this
  toolchain builds (an armel-conventional C++ ABI, no MSVC name
  decoration; `_M_ARM_NT` is deliberately not defined).
* **Default CPU**: `arm926ej-s` via the `Triple::WinCE` case of
  `ARM::getARMCPUForArch` (llvm/lib/TargetParser/ARMTargetParser.cpp),
  so the clang driver and llvm-mc agree.  The core is ARMv5TE
  (Jazelle-capable per the `arm926ej-s` entry, which maps to ARMV5TEJ —
  a superset of ARMv5TE, so plain ARMv5TE devices run the same code).
* **Interworking**: `__THUMB_INTERWORK__` is defined for 5 <= arch <= 8
  on WinCE (the CE loader switches mode on the Thumb bit), matching the
  binutils arm-wince convention.

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
| `d2e4720baf6`..`8736e9b611d` libstdc++ WinCE workarounds | covered by the libc++ WinCE configuration (no filesystem, `ENABLE_THREADS=OFF`, CE is not WIN32API/MSVCRT); libc++ needs none of the libstdc++-specific workarounds |

## MSVC syntax support additions

GNU clang on this target defaults to `-fms-extensions -fms-compatibility
-fms-compatibility-version=1900` (w32api `__declspec` plus the Clang
lookup libc++ ctype overloads need).  **Delayed template parsing is off**
on the GNU driver and on for clang-cl.  eMbedded Visual C++ / Platform
Builder-era MSVC pragmas are implemented (see
`clang/lib/Parse/ParsePragma.cpp`):

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

* **CI green (2026-08-31, `b10d3fa`, run 33364840614):** Stage 1 lit,
  Stage 2 sysroot/smoke, Stage 3 runtimes, package, `/opt` sanity.
  Details: [STATUS.md](STATUS.md).
* Lit coverage: `clang/test/Driver/wince.c`, `clang/test/Driver/wince-x86.c`,
  `lld/test/COFF/wince-*.ll*(s)`, `llvm/test/MC/ARM/wince-*.s`.  SEH-specific:
  `llvm/test/MC/ARM/wince-seh-pdata.s` (CE `.pdata` 16-byte records with
  FUNCLEN/PROLOG pseudo-relocations), `lld/test/COFF/wince-pdata.test`
  (compaction 16->8 bytes, pFuncStart sort, pair placement, exception
  directory size), `clang/test/CodeGen/wince-seh.c` (CE ARM accepts
  `__try`/`__except`/`__finally`; parents get the `__C_specific_handler`
  personality; outlined filters/finallys use the parent-frame intrinsics),
  `llvm/test/CodeGen/ARM/wince-seh-parent-frame.ll` (backend ISel of
  `llvm.eh.recoverfp`/`llvm.localrecover`/`llvm.localaddress`/frame-escape
  assignments on ARMv4T/ARMv5 and Thumb-2), and
  `clang/test/CodeGen/ARM/wince-seh-ehabi-mixed.c` (EHABI C++ exceptions and
  WinCFI SEH coexist per function in one TU), and
  `clang/test/CodeGen/ARM/wince-seh-scope-table.c` (parent functions emit
  the scope table — count word + 16-byte absolute-address entries
  `{BeginVA, EndVA, FilterOrFinally, Handler/Jump}` — plus the 8-byte
  `PDATA_EH` pair `{__C_specific_handler, handler-data}` immediately before
  the function label; filter / catch-all constant 1 / finally null-jump
  entries).  armel baseline:
  `llvm/test/CodeGen/ARM/wince-soft-float.ll` (default ARMv5TE CPU lowers
  float/double to `__aeabi_*` helpers and i64 mul to `__aeabi_lmul` — no
  VFP on the baseline), and the `arm926ej-s`/`+soft-float`/
  `+soft-float-abi`/`-mfloat-abi=soft` checks in `clang/test/Driver/wince.c`.
  These lit files run in Stage 1 of `.github/workflows/main.yml` before
  package.  `.pdata` layout still wants a dump from a real CE image
  (FileCheck is not a device).
* armasm: `llvm/test/MC/ARM/wince-armasm.s` covers the structural
  directives and `llvm/test/MC/ARM/wince-armasm-labels.s` the statement
  and data syntax that `-masm=armasm` implements (see the armasm
  section).
* On-device testing remains outstanding (no WinCE device in the build
  environment); see the procedure notes formerly in `wince-crt/docs`.
