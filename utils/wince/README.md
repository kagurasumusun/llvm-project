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
that tree.  The vendored directory tracks the fork's `master`
(`06e7608`, current HEAD upstream) plus the three WinCE fixes - i.e. we
are on the newest available code of the newest available lineage.

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
| CPUs | ARM (v4/v4T/v5TE) and x86 (CEPC, i386-mingw32ce) are end-to-end (codegen + lld COFF import thunks + CRT).  **SH3/SH4 have no LLVM backend.  MIPS codegen exists but lld's COFF import thunks are x86/ARM/ARM64 only** - adding MIPS thunks in lld is the remaining linker-side work.  PPC (CE 1.0/2.0) has no LLVM COFF backend. |

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
entries. `thread_local`/`__thread` lower to emutls (`__emutls_v.*` + `__emutls_get_address` from compiler-rt, whose Windows path uses TlsAlloc), and `Triple::hasDefaultEmulatedTLS()` covers WinCE so `-femulated-tls` is the driver default. Caveat: mingwrt's `_errno()` remains a single shared static |
| profiling (`-pg`, `gcrt3.o`/`libgmon`) | not provided by the mingw32ce build of mingwrt (the `profile/` sources are desktop-CRT only); CeGCC's LIB_SPEC kept the hook but nothing satisfied it |

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

* **pthreads4w** (`wince-sysroot/pthread-win32`): `pthread_*`,
  `sem_*`, `sched_*` - threads, mutexes, condvars, semaphores, barriers,
  rwlocks, spinlocks, cancellation.
* **mingwrt's CE mingwex set** (linked as `libmingwex.a`): `open`/`read`/
  `write`/`close`/`lseek`/`access`/`chmod`/`stat`/`rename`/`mkdir`/
  `rmdir`/`unlink`/`utime`/`futime`/`fdopen`/`dirent`/`getopt`/
  `time`/`gmtime`/`localtime`/`mktime`/`strftime`/`gettimeofday`/
  `basename`/`dirname`/`tsearch` family, wide-char variants, `imax*`
  inttypes - each backed by the closest COREDLL Win32 call.
* **Implemented POSIX process/signal layer** (`wince-sysroot/posix/`,
  built as `libposix.a`, linked by default; headers: `sys/wait.h`):
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
Both live in `wince-sysroot/gmon/` and are built by the sysroot stage
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
  ABI-frozen change per the update policy).  MIPS/SH3/SH4 - the other
  historical CE CPUs - would additionally need lld COFF import thunks
  (lld supports x86/ARM/ARM64 today); codegen exists, the linker piece is
  the gap.

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
| SAL annotations (`_In_`/`_Out_` + old `__in` style) | **provided**: `wince-sysroot/include-overlay/sal.h` (standard no-op spellings, analysis-off mode, old-style kept); CE-era headers use none/old-style and now both compile |
| `<intrin.h>` (ARM) | **provided**: overlay `intrin.h` - MSVC spellings mapped (barrier/cache-flush/CLZ); `__dmb/__dsb/__isb` use native instructions on ARMv6+ and a CacheSync system barrier on the ARMv5TE baseline (the clang ACLE header has no v5 lowering) |
| MSVC CRT | **by design replaced** by mingwrt (that IS the CeGCC CRT contract); `_MSC_VER`-conditional MSVC-CRT-specific extensions (e.g. `__dbg` heap APIs) are out of scope |
| MSVC type mapping (`__int64`, `SIZE_T`, `DWORD_PTR`, ...) | provided: mingwrt/w32api headers define the full set; `__int64` is a clang keyword alias |
| `__uuidof` / `__declspec(uuid)` | clang supports both (Sema `ActOnCXXUuidof`, `UuidAttr`); `__uuidof` returns the compiler-generated GUID per C++ ABI (GenericARM Itanium mangling); MSVC-style `__uuidof` template capture works |
| operator new/delete from COREDLL | verified present in the def (mangled `??2@...`/`??3@...`/`??_U@...`/`??_V@...`); libc++ uses its own operators, C code never imports them |
| SEH (`__try`/`__except`/`__finally`) | **arch-gated, by design**: Sema/IR-gen support x86 and AArch64 WinEH; the ARM32 Windows-EH unwinder data (`R11`-chain) is not produced by this target, and CE never had ARM32 SEH unwinding (eVC used it only for kernel debugging). C++ exceptions on CE go through ARM EHABI + libunwind (implemented). `__try` on arm-pc-wince is diagnosed as unsupported rather than silently mis-generated |
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
* **Path B (in-tree, the "正規の機構" form): a real ARM MASM parser.**
  LLVM already ships `llvm/lib/MC/MCParser/MasmParser.cpp` (6269 lines)
  + `COFFMasmParser.cpp` + the `llvm-ml` tool: a full MASM dialect
  parser driving the same MCStreamer as the GNU parser.  It is x86-only
  only because `COFFMasmParser` implements the COFF section/symbol
  handlers that x86 needs and nobody registered ARM equivalents - the
  mnemonic parsing itself is done by the target's AsmParser, which for
  ARM already exists (ARMAsmParser, 13k lines).  Proper implementation =
  1. add `ARMCOFFMasmParser` (an `MCAsmParserExtension` like the COFF
  x86 one: `.AREA`->section, `PROC/ENDP/END`, `EXPORT/IMPORT`, DCD/DCB
  data directives, `;` comments, APCS register aliases), 2. register it
  in `createMCMasmParser` for the ARM triple, 3. teach
  `ARMAsmParser::parseDirectiveSyntax` to accept `.syntax divided`
  (currently hard-rejected) or normalize at the extension layer, 4. add
  `-masm=armasm` plumbing in the driver (the `-masm=` option already
  exists for intel/att).  Estimated diff: one new ~800-line parser
  extension + ~100 lines of plumbing.  That is the "in LLVM properly"
  route and is what should be upstreamed if you want it in-tree; the
  pre-assembler remains as the no-upstream fallback.  (Both can coexist;
  armasm's macro processor via MasmParser's existing macro engine.)

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
  WinCFI SEH coexist per function in one TU).  armel baseline:
  `llvm/test/CodeGen/ARM/wince-soft-float.ll` (default ARMv5TE CPU lowers
  float/double to `__aeabi_*` helpers and i64 mul to `__aeabi_lmul` — no
  VFP on the baseline), and the `arm926ej-s`/`+soft-float`/
  `+soft-float-abi`/`-mfloat-abi=soft` checks in `clang/test/Driver/wince.c`.
  All were authored source-level
  only (no build in this environment); the `.pdata`-layout expectations
  assume `.text` at 0x11000 and `.pdata` at 0x12000 under the default
  `/base:0x10000 /fixed` link and should be re-verified at first real build.
* On-device testing remains outstanding (no WinCE device in the build
  environment); see the procedure notes formerly in `wince-crt/docs`.
