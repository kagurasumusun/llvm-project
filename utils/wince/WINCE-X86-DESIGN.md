# x86 Windows CE support: design and investigation

Status: DESIGN ONLY (2026-09-01). x86 CE remains a documented
non-goal until a concrete x86 CE application requirement exists. Both
current target apps (TECLIB/glpi-wince-agent, MaxSignal/Player) are
ARM. This document records what the investigation found and what an
implementation would require, so the scope decision is informed.

## Platform facts

* Windows CE shipped x86 (i386) platform builds through CE 5.0;
  CE 6.0 is ARM-only. (Fact — CE 6.0 source tree has no x86
  platform.)
* The CE kernel's exception model on x86 is **SEH** (structured
  exception handling, `EXCEPTION_REGISTRATION_RECORD` chain +
  `__except_handler4`-style handlers). There is no ARM EHABI on x86.
  (Fact — CE x86 images use the SEH directory; the compressed
  `IMAGE_CE_RUNTIME_FUNCTION_ENTRY` pdata format used on ARM is an
  ARM-side CE extension.)
* eVC++ for x86 CE compiled C++ exceptions **on top of SEH** (MSVC
  model), with Itanium-style inlining/mangling absent — i.e. an x86
  CE port is natively closer to a "desktop MSVC" configuration than
  to our ARM GNU-ABI build. (Fact — MSVC behavior; eVC++ is a
  reduced MSVC.)

## What already exists in the fork (ARM work that transfers)

* `llvm::Triple::WinCE` OS enum + `arm-pc-wince` parsing (Triple.cpp).
  The object-format switch already maps **x86/x86_64 + WinCE →
  COFF**. (Fact — verified in Triple.cpp.)
* `WinCETargetInfo` for the x86 branch already exists in
  `clang/lib/Basic/Targets.cpp` (line 601, `case Triple::WinCE` under
  x86) — the ARM side is `WinCEARMTargetInfo : WindowsARMTargetInfo`.
  (Fact — verified.)
* lld/COFF has the CE machinery we built: `-wince`, pdata compaction
  (ARM), CE text-bounds/exidx symbol binding, CE import thunks (ARM
  flavor), subsystem 9 handling, `-debug`/CodeView embedding.
  (Fact — lld/COFF.)
* The coredll def corpus (coredll3/4/5/6) is architecture-neutral in
  content (same exports, different implementations). (Fact — def
  files are symbol/ordinal tables.)

## What an x86 CE implementation would require (estimate, not a
## plan)

1. **Driver** (`clang/lib/Driver/ToolChains/WinCE.cpp`): today it is
   ARM-only (`arm` arch check). An i386 variant needs the i386 CE
   defaults: no `-mwince`-specific flags, i386 calling convention for
   Win32 stdcall imports (real 4-arch calling convention), entry
   points the same (`WinMainCRTStartup`), base/subsystem the same.
   (Estimate: days — mechanical, modeled on MinGW i386.)
2. **Exceptions — the critical design decision.** Options:
   a. **SEH frames + MSVC-style C++ EH** (`-fms-exceptions` on x86,
      which clang supports): matches what the CE x86 kernel and
      eVC++ objects expect. C code gets SEH frames; C++ exceptions
      run on the SEH handler. This departs from the ARM build's
      Itanium-EHABI model → one toolchain, two exception ABIs
      (like desktop MSVC vs. MinGW today).
   b. Itanium ABI + `.eh_frame` + libunwind-x86, asking the CE x86
      kernel to walk CFI: the CE x86 kernel does **not** provide a
      CFI-based unwinder for kernel-mode dispatch; user-mode
      propagation through C frames would need a userspace-only
      scheme (fragile for signals/CRT).
   Recommendation: (a). It is also what lets an x86 build ever
   interoperate with eVC++-era x86 code at the EH level (mangling
   still differs, though).
3. **lld/COFF i386**: machine 0x14C, i386 import thunks (standard
   16-bit/32-bit thunk form, no ARM pseudo-reloc interplay —
   `-runtime-pseudo-reloc` is an ARM loader mechanism; whether the x86
   CE loader needs the pseudo-reloc fixups must be verified against
   the CE 5.0 x86 loader source — OPEN), SEH directory emission (the
   upstream x86 SEH path largely exists for MSVC triples).
4. **sysroot**: recompile mingwrt + builtins for i386-pc-wince
   (mechanical); w32api headers are arch-neutral (some inline
   assembly is not); coredll def reuse (same exports).
5. **winh / C++ runtime**: the ARM winh is EHABI-based; the x86 build
   under option (a) needs the libunwind-x86 + SEH path instead.
6. **CI**: the Stage 1–5 pipeline generalizes (TARGET variable);
   device testing remains CI-only as on ARM.

**Order-of-magnitude estimate: several weeks** (driver + lld i386 +
SEH decision + sysroot + tests), with the exception-model decision
(#2) as the gating design point. No reference x86 CE toolchain is
available for cross-checking (CeGCC was ARM-only; eVC++ is
proprietary), so verification would be by construction + on-device,
which is the current open risk for the ARM build too.

## Recommendation

Keep x86 CE a non-goal until an x86 CE application enters scope. If
it does, start with the exception-model decision (#2) documented as a
separate design review before any implementation.
