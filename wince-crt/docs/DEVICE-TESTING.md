# Windows CE device testing procedure

**Status: this environment has no Windows CE device or emulator, so
nothing in this repository has been executed on Windows CE hardware.
Everything verified so far is host-side: compilation, linking, and
binary-structure inspection against the CeGCC linker's documented
behavior.**  This document is the procedure to close that gap; it is
part of the deliverable so the verification is reproducible.

## Test matrix

1. `app-winmain.exe` — C, WinMain entry, coredll message box.
2. `app-main.exe` — C, main() entry, printf/malloc/argv.
3. `simpdll.dll` — DLL with `__declspec(dllexport)` functions.
4. `cpp-app.exe` — C++ with libc++ static, global ctors, exceptions,
   threads (requires the C++ runtime phase to be complete).
5. `pthread-test.exe` — mutex/condvar/once/TLS exercise.
6. `cpp-interop.exe` — C++ EXE calling into simpdll.dll.

Each sample lives in `wince-crt/test/device/` (source only; binaries are
built with the completed toolchain).

## Deployment

Copy the built EXE/DLL files to the device (`\\pc\` share via
Windows Mobile Device Center, a storage card, or `CeCopy`).  ARM builds
target ARMv4T baseline and run on ARM9/StrongARM/XScale devices
(Windows CE 4.2/5.0/6.0); choose `-march=armv5te` for XScale-specific
builds.

## Verification checklist on device

* [ ] EXE loads (loader accepts machine/subsystem/imports) — visible by
      the program starting at all.
* [ ] `app-main.exe` prints argc/argv correctly; exit code 0 (check with
      a process-exit code viewer or by chaining).
* [ ] `simpdll.dll` loads via LoadLibrary and exported functions return
      correct values (cpp-interop checks adder(2,3)==5 && dllmul(2,3)==6).
* [ ] `pthread-test.exe` passes all 12 internal checks (prints PASS).
* [ ] C++ test: exceptions caught across function boundaries; global
      constructors ran (printed banner); `std::thread` joins cleanly.
* [ ] long-run test: 1000 allocate/free cycles, thread churn, no
      memory growth (WinCE has no page file; leaks are fatal).

## Debugging on device

* Debug output goes to the debug port: `OutputDebugStringW` is
  delivered by `DebugView`-style tools (e.g. ZoneRingo's "CE Debug
  Monitor", or Platform Builder-attached KITL if available).
* Exit codes: use a launcher EXE printing `GetExitCodeProcess`.
* The CRT writes nothing to a console by default on CE; `printf` output
  requires a console manager (total commander console plugin etc.) — the
  device tests use files/MessageBox/OutputDebugString instead of stdout
  where possible.

## Known risks to watch for on device

* Import resolution of `COREDLL.dll` — the loader is strict about DLL
  names; our import libraries name `COREDLL.dll` exactly like CeGCC's.
* WinCE 5 vs 6 kernel differences for `TlsCall`-based TLS
  (`kfuncs.h` inlines cover both).
* Absolute-address `.ARM.exidx` entries (our COFF EHABI variant) — if a
  CE kernel ever applies image relocation to a DLL, the `.reloc` entry
  for those words must relocate them; verified structurally, re-check at
  runtime with the C++ tests.
