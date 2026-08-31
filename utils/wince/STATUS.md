# WinCE toolchain status (2026-09-01)

Branch: `llvm-wince`.
First full-pipeline green: [33364840614](https://github.com/kagurasumusun/llvm-project/actions/runs/33364840614)
at `b10d3fa` (Stage 1–3 + package). Third-party app + no Absolute-0
`__text_start__`: [33437887749](https://github.com/kagurasumusun/llvm-project/actions/runs/33437887749)
at `7b59bc2c`.

This file is the current snapshot. Older notes in this directory
(`README.md` inventory, `WINCE-WINEH-STATUS.md`) are historical unless
they match a heading here.

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

* **On-device execution** — CI only compiles/links; no CE hardware.
* **Thumb far-branch veneer** — implemented (A32 stub + Thumb-1 `bx pc` /
  `ldr pc`); not executed on a device.
* **WinEH / compressed `.pdata` on a real CE image** — lit + object dumps
  only.
* **libc++ `std::thread` / filesystem** — explicitly off.
* **App CI coverage** — GLPI-Agent (unmodified `make`) plus MaxSignal/Player
  `0.6.2.3-wince` (official zip, Clang Makefile overlay, no audio). `make cab`
  not run.
