# WinCE toolchain status (2026-09-04)

## 2026-09-04 — WinCE lit gate fully green (41/41); first end-to-end green run since the gate was added

cellvm-build CI
[33779871059](https://github.com/kagurasumusun/cellvm-build/actions/runs/33779871059)
(llvm `ad56d0510715`, cellvm-build `b7dad121e486`) finished **success** with all 26
steps green — Stage 1 configure/build, the WinCE lit gate at
`Passed: 41 (100.00%)`, the isolated install, Stage 2 sysroot assembly and its
25-row platform-header smoke, Stage 3 runtimes, the C++ header-collision and
CodeView smokes, packaging, and both real-world projects (glpi-wince-agent.exe and
easyrpg-player). Three artifacts uploaded: the toolchain (89.7 MB),
glpi-wince-agent (49.7 KB), easyrpg-player (6.7 MB).

The two failures that had been blocking the gate are closed (full evidence chain in
WINEH-ABI-FACTS.md §4p):

* **FAIL A — product defect, `7cb85c678b0c`.** `createX86MCAsmInfo` had no WinCE
  branch, so `i386-pc-wince` — which matches none of the MachO / ELF / MSVC-env /
  CoreCLR / UEFI / Cygwin-MinGW / Itanium tests — fell through to the ELF default
  and got an `X86ELFMCAsmInfo`. The AsmPrinter's first section switch then went
  through `MCAsmInfoELF::printSwitchToSection`: `-filetype=asm` segfaulted,
  `-filetype=obj` died in `MCAssembler::registerSymbol` under
  `MCWinCOFFStreamer::changeSection`, and only `-filetype=null` survived, which is
  what made the crash look intermittent. It now returns `X86MCAsmInfoGNUCOFF`,
  mirroring `createARMMCAsmInfo`'s `ARMCOFFMCAsmInfoGNU` for WinCE and the rest of
  this toolchain's GNU dialect.
* **FAIL B — test defect, `ad56d0510715`.** `wince-pdata-every-function.ll`'s
  `cpp_func` had a cleanup-only landing pad that resumed its own value immediately,
  which is exactly the shape `SimplifyCFGOpt::simplifySingleResume` converts into a
  plain call through `removeUnwindEdge` (no personality, clause or cleanup guard)
  and then deletes, gated only on `isCleanupBlockEmpty()`. With `getLandingPads()`
  empty, `ARMException::emitEHABIFunctionEnd` correctly emits no
  `.personality`/`.handlerdata`. `-print-after-all` named the pass: the pad is
  present after `atomic-expand` and gone after "Simplify the CFG (simplifycfg)" on
  armv7 but not on armv5te, while the personality stays in the IR on both. The pad
  now does real work (`extractvalue` plus a call to an external `cleanup_helper`),
  so the transform cannot fire at any target or optimization level. All 33 COMMON
  checks and all 6 RUN lines are unchanged. It was not XFAILed and no backend
  change was made: the directives were absent because they *should* have been.

Also in `b7dad121e486`, three defects that the new section-J diagnostic exposed in
my own GL smoke checks — two of them duplicated verbatim into Stage 2's block,
which had never run:

* `<EGL/eglext.h>` is not self-contained. It includes only `<EGL/eglplatform.h>`
  but uses `EGLBoolean`/`EGLDisplay`/`EGLSurface`/`EGLint`, which CE declares in
  `egl.h:22-28`; `eglplatform.h` supplies just the `EGLNative*` aliases and the
  `EGLAPIENTRY`/`EGLAPI` dllimport boilerplate. Both checks failed on "unknown type
  name 'EGLDisplay'" at `eglext.h:74-75`. `egl.h` now comes first — which is also
  upstream Khronos' order — so the header is left exactly as CE shipped it.
* GLES2's `glViewport` takes no `GLenum`: CE declares
  `glViewport (GLint x, GLint y, GLsizei width, GLsizei height)` at `gl2.h:647`.
  The check used the desktop OpenGL signature and got "incompatible function
  pointer types" on both `arm-pc-wince` and `i386-pc-wince`.
* `... 2>&1 | head -20` followed by `echo "  rc=$?"` reported `head`'s status, so
  all six rows printed `rc=0` while four of them were printing compiler errors;
  clang's own status is now captured into `$RC` first. Stage 2 additionally asserted
  `EGLint`-is-`int32_t` with `std::is_same` from `<type_traits>`, but Stage 2 runs
  before Stage 3 builds the runtimes, so libc++ headers are not installed yet and
  that check could never have compiled; it now uses the same hand-rolled two-line
  trait section J used.

Stage 2's header smoke consequently ran for the first time and passed all 25 rows,
including the five new EGL/GLES2 ones. The w32api import itself needed no change:
`egl.h`/`eglplatform.h` and both `.def` files (`libegl.def` 34 exports,
`libglesv2.def` 142) were already right, and `<stdint.h>` with `<EGL/egl.h>`
compiles in C with `EGLint` being `int32_t` as a *type* rather than merely the same
size — the whole point of the one deviation from CE's `eglplatform.h` text.

Branches: llvm `ad56d0510715`, mingwrt `a1477a2b2278`, w32api `e3158236bc3d`,
cellvm-build `b7dad121e486`. This documentation commit changes no build input, so
cellvm-build stays pinned at `ad56d0510715` instead of spending a run on it.


## 2026-09-03 — Option-B vestige removed; `.pdata` regression test; native SEH verified

Follow-ups closing out the EH work (details: WINEH-ABI-FACTS.md §4n):

* **Option-B vestige removed.** `mingwrt` `crt3.c` no longer includes
  `<wince_cxx_eh.h>`, and its top-level VEH no longer skips `WINCE_CXX_EH_NUMBER`:
  under Option A, C++ exceptions unwind in user space via libunwind/EHABI and never
  reach the kernel dispatcher or the VEH, so the skip was dead code. The VEH /
  `__except` crash logger itself is kept (it now logs only hardware faults and
  explicit RaiseException/SEH events). w32api's `include/wince_cxx_eh.h` (the
  Option-B `WINCE_CXX_EH_NUMBER`/`WINCE_CXX_EH_MAGIC`/`FuncInfoB`/handler-name
  header) is deleted — nothing referenced it after the crt3.c change.
* **`.pdata` non-discardable regression test.** `llvm/test/MC/ARM/wince-seh-pdata.s`
  now CHECKs the `.pdata` section characteristics (`IMAGE_SCN_MEM_READ` present,
  `IMAGE_SCN_MEM_DISCARDABLE` absent) so the §4m ARMWinCOFFStreamer fix cannot
  silently regress.
* **Native WinCE SEH verified viable + correctly implemented** (§4n).
  `__try/__except/__finally` resolves `__C_specific_handler` from CE6 ARM coredll
  (corelib1.def `#else` branch); clang's CE scope table matches the documented MSVC
  `SCOPE_TABLE` ABI (`{Count,{Begin,End,HandlerAddress,JumpTarget}[]}`,
  HandlerAddress = filter | 1 | finally-funclet, JumpTarget = body | 0); absolute
  addressing is forced-correct because CE's `DISPATCHER_CONTEXT` has no `ImageBase`;
  `__finally` needs no user-visible unwind helper; the filter intrinsics are clang
  `MSLangBuiltin`s. Only coredll's closed handler internals remain device-only. No
  code change was needed — SEH was always the sound path; Option B (C++) was the
  broken one.

Branches: `llvm-wince`, mingwrt `master`, w32api `wip`; cellvm-build pins bumped to match.


## 2026-09-03 — C++ EH switched to Option A (EHABI self-unwind); `.pdata` discardable bug fixed

The WinCE C++ exception model was decided and implemented (details:
WINEH-ABI-FACTS.md §4l/§4m, §2 correction):

* **Option A adopted.** WinCE C++ (Itanium) exceptions now unwind in *user space* via
  libunwind + the ARM EHABI `.ARM.exidx`/`.ARM.extab` tables (the CeGCC model). The
  "Option B" kernel-dispatcher bridge (`RaiseException(WINCE_CXX_EH_NUMBER)` +
  `__wince_cxx_frame_handler` + per-frame `PDATA_EH`) was **removed**: even with its
  emission gate fixed, the CE kernel resumes a frame handler with **R0 = ExceptionCode**
  (`CONTEXT_TO_RETVAL` = R0, nkarm.h), clobbering the `_Unwind_Exception*` an Itanium
  landing pad needs in R0, and no compiler-side landing-pad wrapper exists to reload it —
  so `catch (T obj)` / rethrow stayed broken. A C++ frame keeps a `.pdata` entry with
  `ExceptionFlag=0` (the kernel can still unwind it for a hardware fault or an enclosing
  SEH `__try`). llvm-wince `e73be4d975` + deletions `9a3ab21740`/`a2fc4a2b34`;
  CI **33728492946** green (full toolchain + WinCE lit + glpi-wince-agent + easyrpg-player).
* **`.pdata` no longer `IMAGE_SCN_MEM_DISCARDABLE`** (`b73cd8d97e56`). The discardable bit
  made lld skip base relocations for `.pdata` (`addBaserels`/`createRuntimePseudoRelocs`
  skip discardable sections), so a CE module loaded off its preferred base (any DLL) would
  keep unrelocated `pFuncStart` VAs and break kernel unwinding; `/fixed` EXEs hid it. Now
  `data|r`, matching the PE/COFF spec and the sibling `.ARM.exidx`/`.ARM.extab`.
* **WINEH-ABI-FACTS.md §2 corrected.** The CE `_RUNTIME_FUNCTION` is the 5-field nkarm.h
  struct `{BeginAddress, EndAddress, ExceptionHandler, HandlerData, PrologEndAddress}`
  (= w32api `excpt.h`), expanded by `RtlLookupFunctionEntry` from the on-disk compressed
  8-byte `IMAGE_CE_RUNTIME_FUNCTION_ENTRY`; the earlier 3-field derivation was only the
  subset the prologue-re-execution unwinder touches.
* **WinceClock caveat closed.** `GetSystemTime` / `SystemTimeToFileTime` /
  `FileTimeToSystemTime` / `QueryPerformanceCounter` / `QueryPerformanceFrequency` /
  `GetTickCount` are all exported by `libce/coredll*.def` (`GetSystemTimeAsFileTime` is
  CE6-only but unused by libcxx's WinCE `system_clock`). The `system_clock` (µs) /
  `steady_clock` (QPC) math was already correct.

Branch `llvm-wince`; cellvm-build `llvm-project` pin bumped to match.


## 2026-09-03 first real on-device report (imx28, CE 6.0, ARM, 12MB RAM)

User-reported: EasyRPG Player (Stage 5 build) starts briefly then
crashes on real hardware — the first ever on-device signal this
toolchain has received. Findings and fixes: WINCE-HANDOFF.md §19.

* **WINEH-ABI-FACTS.md §4e corrected**: the "CodeGen-driven EHABI table
  generation may not be wired up" open question was a false alarm from
  an incomplete grep (missed `ARMException.cpp`). Verified against
  source: it is wired up and lit-tested
  (`clang/test/CodeGen/ARM/wince-ehabi-tables.c`). See §4h for where the
  remaining EHABI-shaped risk actually is.
* **Top-level `__try/__except` crash logger added** to `mingwrt`'s
  `crt3.c` (`WinMainCRTStartup`): every WinCE program built by this
  toolchain now writes `<exe>.crash.log` with the exception code/address
  on an unhandled hardware or software exception, instead of the process
  silently disappearing. Not yet exercised on a real build/device.
* ARMv5TE strict-alignment audit of the app-side dependencies
  (pixman/libpng/zlib/liblcf/SDL) — planned next, not yet started.

Branch: `llvm-wince`.
First full-pipeline green: [33364840614](https://github.com/kagurasumusun/llvm-project/actions/runs/33364840614)
at `b10d3fa` (Stage 1–3 + package). Third-party app + no Absolute-0
`__text_start__`: [33437887749](https://github.com/kagurasumusun/llvm-project/actions/runs/33437887749)
at `7b59bc2c`.
**First Stage 5 green (EasyRPG Player links)**: llvm `afc91e89` /
cellvm-build `c83b9f4`
([33600018503](https://github.com/kagurasumusun/cellvm-build/actions/runs/33600018503));
`easyrpg-player.exe` 5,346,816 bytes, PE verified (ARM, subsystem 9,
entry `.text+0`, import/IAT resolved, merged `.ARM.exidx`/`.ARM.extab`).
Causes and fixes: WINCE-HANDOFF.md §18.

This file is the current snapshot. Older notes in this directory
(`README.md` inventory, `WINCE-WINEH-STATUS.md`) are historical unless
they match a heading here.

## 2026-09-02 collision / duplication sweep

Systematic audit of every cross-tree name collision, duplication and
process conflict (full results in WINCE-HANDOFF.md §17):

* **`__small` macro collision** (the only live one of 374 candidate CE-header
  macros × 9308 C++-runtime identifiers): w32api `basetyps.h` no longer
  defines it under clang, mirroring the `_mingw.h` precedent.  A
  windows.h + libc++ `<functional>` TU now compiles.
* **Driver**: `arm-pc-wince` OS versions below 4.0 now fail the link with
  `err_drv_unsupported_wince_version` instead of emitting a line against the
  nonexistent `libcoredll3.a` (CE 3.0 was shelved in mingwrt/w32api; the
  driver case was the last remnant).
* **Headers**: mingwrt's x86-legacy `excpt.h` deleted (w32api's ARM-shaped
  one is canonical and is what `windows.h` includes); the COREDLL
  `coredll{,4,6}.def` mirror in `w32api/libce` is now guarded by a
  byte-equality check in the sysroot build.
* **cellvm-build Stage 5**: the missing `wincehelper.cpp` overlay copy was
  restored (it had landed as an unreferenced commit), the `(void*)0` typo in
  it fixed, and MSVC-style include spellings (`Windows.h`, `Shellapi.h`)
  are covered by case-alias forwarders plus a generator script.

## Stage 5 greening (2026-09-02, full table in WINCE-HANDOFF.md §18)

Each round exposed the next structural cause; every fix is a mechanism or a
single-sourcing, not a local dodge:

* libc++ `std::to_string` ambiguity → wincehelper shim overloads deleted
  (`5cc3d9d`).
* Driver ignored `-L` for the wince linker job → translated to `/libpath:`
  with a claim, lit-checked (`219e86dd`).
* `strerror` duplicate symbol → `ce-strerror.c` copy removed; mingwrt
  `coredll_stubs` is the sole provider (`bbce715`).
* `-lmmtimer.lib is not allowed in .drectve` → SDL patch forces the
  threaded-timer path; no MM timer dependency (`bbce715`).
* `wcstold` / `__mingw_aligned_malloc` undefined → mingwrt CE `LIB_OBJS`
  omissions fixed (`993c7e4`).
* `AudioSeCache`/`AudioDecoder`/`Struct<RPG::TestBattler>` undefined →
  three missing sources in the overlay Makefiles (`66db512`, `e2d95ba`).
* `WIN_GL_*` undefined → `SDL_dibvideo.c` GL call sites guarded; GL is
  not built for WINDIB (`66db512`).
* `.ARM.exidx` relocations against COMDAT-discarded functions → lld/COFF
  now culls exidx/extab entries whose function was discarded, ELF-style,
  lit-checked `wince-exidx-comdat.s` (`afc91e89`).

## Repository layout (2026-08-31 reorganization)

* **This repository = compiler side only**: driver, cmake cache,
  lld/COFF, lit tests, docs. CI = Stage 1 + WinCE lit gate.
* **`kagurasumusun/cellvm-build`** = the full pipeline (sysroot, runtimes,
  packaging, TECLIB/glpi, EasyRPG Player) with submodules
  `llvm-project@llvm-wince` + `mingwrt@master` + `w32api@wip` +
  `pthread-win32@master`, plus the in-house sysroot code
  (`sysroot/{gmon,posix,include-overlay}`) and the build scripts that used
  to live in `wince-sysroot/` / `utils/wince/` here.
* The WinCE fixes to mingwrt/w32api/pthread-win32 are pushed to
  `kagurasumusun/{mingwrt,w32api,pthread-win32}` (see WINCE-HANDOFF.md §14).
  `wince-sysroot/` is deleted from this repository.

## What is green

| Layer | Result |
|---|---|
| Stage 1 clang+lld, WinCE lit | pass |
| Stage 2 mingwrt/w32api sysroot + smoke | pass |
| Stage 3 compiler-rt builtins + libunwind + libc++abi + libc++ | pass; `libc++.a` in the sysroot |
| Package + `/opt` compile sanity | pass |
| Stage 4 TECLIB/glpi-wince-agent (unmodified `make`) | pass; artifacts uploaded |
| Stage 5 MaxSignal/Player 0.6.2.3-wince (Clang Makefile overlay) | **pass; `easyrpg-player.exe` links** (audio off, SDL 1.2 WINDIB) |
| CPU / ABI | `arm926ej-s`, soft-float, GNU `lib*.a` |

## Runtime layering (do not flatten)

1. **OS/CRT**: COREDLL + this tree's mingwrt. CeGCC-like baseline.
2. **Compiler runtimes**: clang, compiler-rt builtins, libunwind (EHABI), **libc++**.
3. **Extras**: pthreads4w and the posix shim. **Not** CE pthread/POSIX.
   The driver links `libpthread.a` / `libmingwthrd.a` only with `-mthreads` /
   `-pthread`. `libposix.a` is **not** on the default link line.

libc++ and libunwind are built with **`ENABLE_THREADS=OFF`**. No
CE-specific dummy RWMutex. Soft-float: do not emit VFP save/restore or
COFF `.fpu`.

## libc++: CE is not NT/MSVCRT (`728da332`)

* `_LIBCPP_WIN32API` / `_LIBCPP_MSVCRT_LIKE` only when `_WIN32 && !_WIN32_WCE`.
* CE keeps `_LIBCPP_SHORT_WCHAR` and `_LIBCPP_PROVIDES_DEFAULT_RUNE_TABLE`.
* Entropy: `CeGenRandom`. Clocks: `GetSystemTime` + `QueryPerformanceCounter`.
* Aligned new: `__mingw_aligned_malloc`.
* `fstream` native `HANDLE` stays behind `_LIBCPP_WIN32API`.
* `-DLIBCXX_HERMETIC_STATIC_LIBRARY=ON`.

## Driver language defaults (`b10d3fa`)

GNU `clang --target=arm-pc-wince`:

| Flag | GNU default | clang-cl |
|---|---|---|
| `-fms-extensions` | on | on |
| `-fms-compatibility` | **on** | on |
| `-fms-compatibility-version` | `1900` | `1900` |
| `-fdelayed-template-parsing` | **off** | on |

`-fms-compatibility` stays because libc++ `__locale::isblank` vs
`using ::isblank` (CI 33364441376), not because "maximum MSVC dialect"
is a requirement.

Link line: GNU archive names only. EXE entry `WinMainCRTStartup`.
Subsystem `windowsce`.

## Third-party app CI

GitHub Actions clones **unmodified**
[TECLIB/glpi-wince-agent](https://github.com/TECLIB/glpi-wince-agent)
(GPLv2 WinCE inventory agent; its Makefile already expects
`arm-mingw32ce-gcc`) and runs **their** `make`. The toolchain only
installs CeGCC-style names that bind `--target=arm-pc-wince`. The app
tree is not patched.

Stage 5 downloads the official
[MaxSignal/Player 0.6.2.3-wince](https://github.com/MaxSignal/Player/archive/refs/tags/0.6.2.3-wince.zip)
zip and copies an LLVM/Clang `Makefile` over it. Player C++ is not patched.
Audio stays off (`SUPPORT_AUDIO` is not defined). UI is SDL 1.2 WINDIB. Deps
(zlib, libpng, pixman, libiconv, SDL 1.2.15, liblcf 0.6.2) are built into a
separate prefix, not the CRT sysroot.

## Do not revive

Empty `__cdecl`/`__stdcall`; `#ifdef __clang__` skip of `include_next`;
ELF crtbegin; `wince-cc` as a flag dump; `-nostdlibinc`; pthreads4w as
platform pthread; MS `.lib` aliases; CE dummy RWMutex; VFP/COFF `.fpu`;
force-push.

## Still open (not blocking CI)

* **PDB generation** — **done (lit-gated)**: `lld-link -debug` writes a
  standalone `.pdb` (streams + DBI modules + publics), verified with
  `llvm-pdbutil` in `lld/test/COFF/wince-pdb.test`; the Stage 5 CI emits
  `easyrpg-player.pdb` and the rolling `wince-latest` release ships it.
* **On-device execution** — CI only compiles/links; no CE hardware.
* **Thumb far-branch veneer** — implemented (A32 stub + Thumb-1 `bx pc` /
  `ldr pc`); not executed on a device.
* **WinEH / compressed `.pdata` on a real CE image** — lit + object dumps
  only.
* **libc++ `std::thread`** — explicitly off (runtimes built with
  `ENABLE_THREADS=OFF`).
* **libc++ `<filesystem>`** — in progress, not off: `LIBCXX_ENABLE_FILESYSTEM=ON`
  in the Stage 3 runtimes build since 2026-09-01, with the POSIX declaration
  surface and the `mingwex/wince` file-API shims it needs landed in mingwrt
  and the CE branches in `libcxx` (mkdir/mtime, `_LIBCPP_HAS_OPEN_WITH_WCHAR`).
  Compile-level green; not yet exercised by an app in CI.
* **App CI coverage** — GLPI-Agent (unmodified `make`) plus MaxSignal/Player
  `0.6.2.3-wince` (official zip, Clang Makefile overlay, no audio). `make cab`
  not run.
