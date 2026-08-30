# WinCE ARM WinEH/SEH — ABI Facts (clean-room extraction)

> Source: `kagurasumusun/wince-source` (`ce600/PRIVATE/WINCEOS/COREOS/CORE/DLL/ARM/unwind.c`,
> `.../CORELIBC/CRTW32/EH/*.cpp`, `.../CORELIBC/CRTW32/H/eh.h`).
> That repository is Microsoft shared-source licensed code. **No code from it is copied,
> transcribed, or closely paraphrased here or anywhere in this tree.** This document records
> only structural/interoperability facts (struct layouts, field names/order, control-flow
> facts, magic numbers) inferred by reading the implementation, the way one would document
> a wire format or binary ABI for interop purposes. Anyone implementing against this document
> writes their own code from scratch.
>
> Extraction date: 2026-08-28. Extracted subset: "AE600" (CE 6.0 R2/R3), not the full WinCE
> source tree.

## 1. Availability caveat

The extraction available to us does **not** include `ehdata.h` / `ehstate.h` (the headers
that declare the actual `FuncInfo`/`TryBlockMap`/`HandlerType`/`ThrowInfo`/`UnwindMap`
struct layouts used by `__CxxFrameHandler` v1). Only the *implementation* files
(`frame.cpp`, `throw.cpp`, `hooks.cpp`, `ehstate.cpp`, `unhandled.cpp`, `user.cpp`) and the
public `eh.h` are present. So: **no C++ `FuncInfo` v1 layout facts below** — that part of
§6.1 in the handoff is still blocked until those headers are available. Everything below
about ARM `RUNTIME_FUNCTION` / prologue unwinding comes from `DLL/ARM/unwind.c`, which *is*
present in full.

## 2. CE ARM `RUNTIME_FUNCTION` layout — differs from NT ARM

Inferred from field-access patterns (`FunctionEntry->BeginAddress`,
`FunctionEntry->PrologEndAddress`, `FunctionEntry->EndAddress`) in the Thumb and ARM
`*VirtualUnwind` routines:

```
struct RUNTIME_FUNCTION_CE {   // fact-derived name, not the original identifier
    ULONG BeginAddress;
    ULONG PrologEndAddress;
    ULONG EndAddress;
};
```

This is **three absolute addresses**, not the packed `{BeginAddress, UnwindData}` pair (or
`{BeginAddress, PackedUnwindData}` compact form) used by modern Windows-on-ARM NT unwind
info, and not an xdata-offset scheme either. There is no packed-opcode `UnwindData` word
and — as far as the available file shows — **no `.xdata` section is consulted by the
unwinder at all**. `EncodingType::CE`'s comment in LLVM ("Windows CE ARM, PowerPC, SH3,
SH4") is consistent with this being an older, simpler, non-table-driven scheme shared
across those CE-supported architectures.

Address handling notes (structural, not implementation):
- Both `BeginAddress` and `PrologEndAddress` have their low bit masked off
  (`& ~0x01`) before use — consistent with the low bit being a Thumb-mode marker on
  function-entry-style addresses elsewhere in the CE/Win32 ABI (same convention as
  NT Thumb entry points).
- A `VA_SECTION`-relative special case exists for addresses below `(1 << VA_SECTION)`,
  suggesting a "slot 0"/shared-address-space adjustment specific to CE's process model.
  This is a CE-kernel-internal detail; a compiler/linker producing `RUNTIME_FUNCTION_CE`
  entries does not need to reproduce it — it only needs correct absolute addresses.

## 3. CE ARM unwinding mechanism — prologue re-execution, not opcode tables

This is the single most important architectural fact, and it changes the priority/shape
of the LLVM-side work versus what the handoff assumed ("恐らく同一またはサブセット" — CE
was assumed possibly identical or a subset of NT ARM's packed-unwind scheme).

**Fact: CE's ARM unwinder does not consume any encoded unwind-opcode stream at all.**
Given a `RUNTIME_FUNCTION_CE` triple, it:
1. Computes prologue length as `PrologEndAddress - BeginAddress`.
2. Reads the actual prologue *machine code* bytes from `BeginAddress` (via a
   `READ_CODE`-style safe copy).
3. Walks those instructions **in reverse**, pattern-matching each decoded instruction
   against a small fixed set of recognized prologue idioms, and undoes each one against
   the live `CONTEXT` (e.g., a recognized push-register-list instruction becomes "pop
   these registers back off the stack and restore `Sp`"; a recognized stack-pointer
   decrement becomes "add that amount back to `Sp`"; an unrecognized instruction is
   logged and skipped).
4. Separate decoders exist for Thumb-encoded prologues and ARM-encoded prologues
   (`ThumbVirtualUnwind` / `ArmVirtualUnwind`-shaped routines), each recognizing the
   idioms for its instruction set.
5. The recognized Thumb idiom set includes (categories, not exhaustive):
   register push (with optional LR), small immediate SP adjust, a "large frame" idiom
   (load a large immediate via literal pool / negate it / apply it to SP across a small
   instruction sequence), a register-save **call to a millicode/helper routine**
   recognized specifically as `bx pc` into an ARM-mode `stm sp!, {reglist}` / `bx lr`
   sequence (this is the classic "register save helper" calling convention used to
   compress prologues), and a plain `bx` (used to detect end-of-recognized-prologue).
   Any instruction outside this recognized set is simply ignored by the unwinder rather
   than treated as an error — meaning **an unrecognized prologue silently fails to
   unwind correctly** rather than raising a diagnosable error.

**Implication for the compiler/toolchain**: correctness here is not "emit the right
encoded unwind opcodes" (there are none to emit) — it is "emit function prologues built
only from instruction idioms the CE OS unwinder's pattern matcher recognizes," plus
correct `RUNTIME_FUNCTION_CE` triples in `.pdata` pointing at prologue start/end/function
end. No `.xdata` section is needed for this mechanism.

## 4. Practical consequence for LLVM's `EncodingType::CE`

- `EncodingType::CE` should **not** reuse the NT ARM packed/xdata-based `.pdata` emitter
  logic (`Win64EH::ARMUnwindEmitter` / `MCWin64EH.cpp`'s `ARMEmitUnwindInfo`) as-is — that
  logic's job (encoding push/stack-adjust into a compact opcode stream + `.xdata`) has no
  consumer on CE.
- A CE-targeting `.pdata` emitter's job is much narrower: emit one
  `{BeginAddress, PrologEndAddress, EndAddress}` triple per function that needs unwind
  info, with relocations against the function-start, prologue-end, and function-end
  labels. `PrologEndAddress` in particular is a new label CodeGen needs to be able to
  mark (end of the recognized prologue sequence), distinct from what NT ARM WinEH tracks.
- Because this is a **best-effort pattern-matched interpreter**, not a validated table
  scheme, the practical implementation risk lives entirely on the codegen side (does the
  backend's actual generated prologue sequence stay within the recognized idiom set?),
  not on the encoding side. This is worth flagging explicitly to whoever picks up the
  codegen work next.
- Scope carved out for `EncodingType::CE`, per this repo's design (§3.5 of
  `WINCE-HANDOFF.md`): C++ exceptions already use ARM EHABI (`.ARM.exidx`/`.ARM.extab`)
  via this toolchain's own libunwind/libc++abi and do **not** need this mechanism.
  `EncodingType::CE` is only required for interop with **OS/kernel-level SEH**
  (`__try`/`__except` in code that must unwind through frames the CE kernel itself
  walks — e.g. inside `coredll`/OAL/driver code compiled against this toolchain,
  or any frame the kernel's fault dispatcher walks through).

## 4a. Implementation status (added 2026-08-28; **superseded** 2026-08-30)

> **⚠ This section describes code that no longer exists.**  `EmitCE()` was
> removed again in `84aec8c1` ("remove the dead, ABI-incorrect
> `EncodingType::CE` emitter from MCWin64EH") because nothing ever selected
> `EncodingType::CE` - see §4b for the gate that made it unreachable.  The
> CE `.pdata` support that *is* in the tree takes a different route:
> `ARMWinCOFFStreamer::CEEmitUnwindInfo()` (compressed 16-byte intermediate
> records, compacted to 8 bytes by lld), driven per function by
> `functionUsesWinCFI()` (`llvm/lib/Target/ARM/ARMWinCFI.h`).  §4f lists the
> commits.  What survives from this section is the ABI analysis in §2/§3,
> which the implementation is still based on.

`llvm::Win64EH::ARMUnwindEmitter::EmitCE()` existed
(`llvm/lib/MC/MCWin64EH.{h,cpp}`) implementing exactly the narrow scope from
§4 above: two passes over `Streamer.getWinFrameInfos()`, first validating
each `WinEH::FrameInfo` has `PrologEnd` (from `.seh_endprologue`, already
generic MC infrastructure) and `FuncletOrFuncEnd` set, then emitting a
`{Begin, PrologEnd, FuncletOrFuncEnd}` triple of `IMGREL32` relocations per
function into the associated `.pdata` section via
`ARMEmitCERuntimeFunction`. No `.xdata` section is touched at all, matching
§3's finding. This is written from scratch against the ABI facts above; it
shares no code with `ARMEmitUnwindInfo`/`ARMEmitRuntimeFunction` (the NT ARM
packed/xdata path) beyond calling the same low-level symbol-relocation
helpers (`EmitSymbolRefWithOfs`) that both paths already used.

**Status: 実装済み (untested/build-unverified) — not yet wired to any
target.** Specifically still missing before this can fire for real code:

1. **Encoding-type selection**: nothing currently sets
   `WinEH::EncodingType::CE` on any `MCAsmInfo`, and `WinCE`'s target-triple
   factory (`ARMMCTargetDesc.cpp`) currently forces
   `ExceptionHandling::ARM` (EHABI) unconditionally, which is a different
   `ExceptionsType` than `ExceptionHandling::WinEH` (required for any
   `WinEH::EncodingType` to apply at all). Since C++ exceptions must keep
   using EHABI (§4), this needs a **per-function** or **per-attribute**
   selection mechanism (e.g. driven by `__try`/`__seh`-style codegen
   attributes), not a blanket per-target switch. This is unresolved
   architecture, not yet designed in detail.
2. **Dispatch wiring**: `ARMWinCOFFStreamer::emitWindowsUnwindTables()` calls
   the class's `Emit()`, not `EmitCE()` — nothing calls `EmitCE()` yet.
   `AsmPrinter.cpp`'s `WinEH::EncodingType` switch (line ~647) also has no
   `CE` case.
3. **Clang-side `__try`/`__except` support for ARM**: unverified whether
   Sema/CodeGen currently lower MS `__try` at all for the ARM target under
   any exception model; this needs investigating before the above is
   useful for anything.
4. **No test coverage** — `llvm/test/MC/ARM/` has no CE-EncodingType test
   yet, and none can usefully exist until (1)-(2) make this reachable from
   real assembly/IR input.

Given this session's build is explicitly out of scope, this code has been
written carefully (mirrored line-by-line against the existing NT ARM
`ARMEmitRuntimeFunction`/`ARMEmitUnwindInfo` patterns, checked for
brace/paren balance and field-type consistency by hand) but **has not been
compiled**. Treat it as a reviewed draft, not verified-working code, until
someone builds `check-llvm-mc` (or equivalent) against it.

## 4b. The real remaining blocker — precisely located (2026-08-28, this session)

Traced exactly where SEH/EHABI coexistence breaks down, to avoid a previous
overly-vague "architecture undesigned" framing.

**Good news — personality selection is already per-function, not per-target.**
`clang::EHPersonality::get()` (`clang/lib/CodeGen/CGException.cpp`) already
picks `MSVC_C_specific_handler` for any individual `FunctionDecl` where
`FD->usesSEHTry()` is true, and the plain C++ personality otherwise --
per-function, already generic, already works regardless of target arch (the
non-x86 branch of `getSEHPersonalityMSVC()` already returns
`MSVC_C_specific_handler` for ARM). So Sema/CodeGen-level `__try` support
and mixed SEH+EHABI-C++ in the same translation unit is **not** blocked at
that layer.

**The actual blocker is one level down, in a target-wide (not per-function)
MC gate.** Whether the backend ever emits `.seh_*` WinCFI directives for a
function (which populate `WinEH::FrameInfo::PrologEnd`/`FuncletOrFuncEnd` --
exactly what `EmitCE()` from section 4a needs) is controlled by
`MCAsmInfo::usesWindowsCFI()`:

```
bool usesWindowsCFI() const {
  return ExceptionsType == ExceptionHandling::WinEH &&
         (WinEHEncodingType != WinEH::EncodingType::Invalid &&
          WinEHEncodingType != WinEH::EncodingType::X86);
}
```

This is a **single boolean derived from the whole target's `MCAsmInfo`**,
not something any per-function codegen decision can currently override.
Since `ARMMCTargetDesc.cpp` forces `ExceptionsType = ExceptionHandling::ARM`
unconditionally for `isWindowsCE()`, `usesWindowsCFI()` is always false on
this target today -- so no function, no matter its personality, can
currently trigger `.seh_endprologue`/`.seh_endproc`/etc. emission, and
`EmitCE()` from section 4a has no way to become reachable as things stand.

By contrast, the *existing* ARM EHABI `.fnstart`/`.fnend` emission
(`ARMWinCOFFStreamer::isEHABI()`) is gated purely on
`getTargetTriple().isWindowsCE()` -- triple-based, not `ExceptionsType`-based
-- which is exactly why EHABI already works unconditionally today regardless
of this issue.

**Two design options, not yet chosen (needs a build-and-test iteration
loop, which was out of scope this session):**

1. Make `usesWindowsCFI()` (generic LLVM MC code, shared by every Windows
   target: x86, x64, ARM64, ARM) sensitive to something finer-grained than
   a single per-target `ExceptionsType`/`WinEHEncodingType` pair -- e.g. a
   per-`MachineFunction` or per-`MCContext` override. This is a
   target-independent core-MC change with real blast radius; wrong-by-inspection
   risk is high without building and running the existing x86/AArch64 WinEH
   test suites to check for regressions.
2. Keep `usesWindowsCFI()` untouched, and instead add ARM/WinCE-specific
   logic (contained to `ARMAsmPrinter`/ARM `SelectionDAG` lowering) that
   calls the WinCFI-emission APIs (`emitARMWinCFIPrologEnd` etc.) directly
   for SEH-personality functions on this target, bypassing the generic gate
   entirely for this one case. Smaller blast radius (ARM-target-local), more
   plausible to land without destabilizing other Windows targets, but
   requires locating exactly where `ARMAsmPrinter`/ISel currently branches
   on `isEHABI()`-equivalent state during function prologue/epilogue codegen
   to add the parallel SEH-personality branch -- not yet located in this
   pass.

Recommendation for whoever picks this up next: **option 2**, and start by
finding where in `ARMAsmPrinter.cpp` / `ARMFrameLowering.cpp` the decision
to call EHABI `.fnstart` happens, since the SEH branch needs to sit right
next to it, keyed off `EHPersonality::get(...)`-equivalent per-function
state rather than the target-wide `usesWindowsCFI()`.

## 4c. Mixed-mode dispatch — confirmed unsafe to attempt without a build (2026-08-28)

Checked option 1 from §4b concretely before ruling it out for real, rather
than by inspection-guess alone: **flipping `ExceptionsType` to `WinEH`
target-wide would silently disable the EHABI unwind-directive translation
that this toolchain's existing, working C++ exceptions depend on.**

Confirmed by direct code reading, not speculation: in
`ARMAsmPrinter.cpp`'s per-`MachineInstr` unwinding-instruction translator
(`ATS.emitRegSave(...)`, `ATS.emitSetFP(...)`, `ATS.emitPad(...)` --
i.e. the code that turns prologue `MachineInstr`s into `.save`/`.setfp`/
`.pad` EHABI directives), **every one of these call sites is individually
gated on `MAI->getExceptionHandlingType() == ExceptionHandling::ARM`**
(`ARMAsmPrinter.cpp` lines ~1363, ~1414, and others following the same
pattern). `MAI->getExceptionHandlingType()` returns the single
target-wide `ExceptionsType` value -- so setting it to `WinEH` anywhere
would make all of these branches stop firing everywhere, for every
function, immediately. That is a silent, toolchain-wide correctness
regression to already-working C++ exception handling, not a contained
change. **Option 1 is confirmed unsafe; do not attempt it as a simple
target-wide flip.**

This means real mixed-mode support (EHABI for C++, WinEH/CE-SEH for
`__try` functions, coexisting per-function in the same translation unit)
requires threading **per-function** state through every one of these
`ExceptionHandlingType`-gated call sites, replacing the blanket
`MAI->getExceptionHandlingType() == ExceptionHandling::ARM` checks with
something that also asks "does *this* function use EHABI or WinCFI",
keyed off the same `EHPersonality`/`needsUnwindTableEntry()`-style
per-function state that `ARMFrameLowering.cpp`'s `needsWinCFI()` already
uses (see §4b) -- for at least:

- `ARMAsmPrinter.cpp` ~1363, ~1414 (and any sibling branches in the same
  unwinding-instruction translator not yet enumerated one-by-one here)
- `AsmPrinter.cpp` ~610-625, ~633-643 (module-level EH streamer selection
  switch), ~1344, ~1371-1374, ~4644
- `ARMWinCOFFStreamer::isEHABI()` itself may need to become
  function-aware too (currently pure triple check), or the two output
  paths (`.fnstart`/`.fnend` EHABI vs `.seh_*`/pdata-triple CE) need a
  clean per-function switch above this layer instead

This is a real, multi-site refactor across shared ARM backend code with
genuine regression risk to every existing WinCE C++ program if any one
site is missed or miswired -- it needs a build-and-test iteration loop
(building against the existing WinCE C++ exception test coverage, if any
exists, plus new SEH-specific tests) that was out of scope this session.
Writing this blind, without being able to compile and run it, is exactly
the kind of unverified change this project's own conventions (see
WINCE-HANDOFF.md/instructions: never claim untested code as complete, and
especially never risk regressing a previously-working mechanism) argue
against. Recommendation for whoever has build capability next: implement
the per-function dispatch as a small, isolated helper (e.g.
`bool ARMAsmPrinter::functionUsesEHABI(const MachineFunction &MF)`) and
thread it into each site above one at a time, verifying with
`check-llvm-codegen-arm` plus a hand-written mixed EHABI+SEH test case
after each site.

## 4d. Implemented: safe additive per-function dispatch (2026-08-28, this session)

Added `llvm/lib/Target/ARM/ARMWinCFI.h` — a shared `functionUsesWinCFI(MF)`
predicate — and wired it into the two clearly-understood, minimal-risk
sites:

- `ARMFrameLowering.cpp`'s `needsWinCFI()` (controls whether
  `SEH_*` pseudo-instructions get inserted into a function's prologue/
  epilogue at all).
- `ARMAsmPrinter.cpp`'s `emitInstruction()` gate on
  `EmitUnwindingInstruction` (the EHABI `.save`/`.setfp`/`.pad` MachineInstr
  translator) -- now also excludes functions where `functionUsesWinCFI`
  is true, which is required for correctness: those functions' FrameSetup
  instructions include `SEH_*` pseudo-ops that the EHABI translator's
  switch does not recognize and would otherwise hit its
  `llvm_unreachable("Unsupported opcode for unwinding information")`
  fallback -- i.e. without this exclusion, a compiled CE `__try` function
  would crash the compiler, not just emit wrong output.

**Why these two sites are safe to have written without a build**: for
every function that is not "Windows CE and has an `MSVC_TableSEH`/
`MSVC_X86SEH` IR personality" (i.e. every currently-reachable CE function
today, since nothing currently produces that combination end-to-end -- see
§4e), `functionUsesWinCFI(MF)` evaluates to exactly what
`MAI->usesWindowsCFI()` evaluated to before this change (`false` for CE).
Both edits are therefore behaviorally inert for all existing/tested code
paths; they only add a new, previously-unreachable branch. This is a much
narrower risk profile than option 1's target-wide flip (§4c).

## 4e. Open question, not resolved by inspection: does CE's EHABI table
actually get generated for *compiled* C++ today?

While tracing where WinCFI's `.seh_*` directives get emitted from (to
find the corresponding EHABI `.fnstart`/`.fnend` call site to also gate),
`ARMTargetStreamer::emitFnStart()`/`emitFnEnd()` -- the calls that open
and close a `.ARM.exidx`/`.ARM.extab` table entry
(`ARMWinCOFFStreamer::EHABIemitFnStart`/`EHABIemitFnEnd`) -- were found to
be called **only from `ARMAsmParser.cpp`** (i.e. when assembling `.s`
source containing an explicit `.fnstart`/`.fnend` directive, such as
`armasm`-authored files). No call site was found in `ARMAsmPrinter.cpp`,
generic `AsmPrinter.cpp`, `ARMISelLowering.cpp`, or `ARMFrameLowering.cpp`
that would invoke this for a function CodeGen compiled directly from C/C++
source.

This was **not resolved** -- it needs either (a) confirming there's a
legitimate implicit/generic mechanism elsewhere that this pass didn't
find, by tracing further or by building and inspecting the `.ARM.exidx`
section of a compiled test binary, or (b) concluding that CodeGen-driven
EHABI table generation for compiled (not hand-assembled) WinCE C++ is not
actually wired up yet, independent of anything to do with SEH. Given the
significance either way (this would be foundational to the "C++
exceptions already work via EHABI" premise this whole SEH investigation
has been resting on), **this should be checked with an actual build and
a minimal `try { throw ...; } catch { ... }` compile-and-disassemble test
before any further EH work on this target proceeds**, rather than assumed
in either direction. Flagging this explicitly rather than guessing was
judged safer than either asserting it works or silently leaving it
unmentioned.

## 4g. CE exception dispatch & PDATA_EH — verified against exdsptch.c (2026-08-29)

**Addendum (2026-08-30, Phase 2):** three defects in the emitter/linker side
of this table were found by source analysis and fixed; all three are the kind
of thing only a real build or the lld test would have shown, and none has been
executed yet:

* `pFuncStart` did **not** carry the Thumb marker.  `Frame->Begin`,
  `FuncEnd` and `PrologEnd` are temporary labels, so llvm-mc emits them as
  *section* relocations (`WinCOFFObjectWriter.cpp`: "Turn relocations for
  temporary symbols into section relocations") and no symbol value carried
  bit 0.  `CEEmitUnwindInfo` now folds `+1` into the emitted expression for
  Thumb functions; the length relocations round the span up, so the marker
  does not inflate FuncLen/PrologLen.
* lld measured both lengths **from the start of `.text`**: for the same
  reason, `s` in `applyRelARM` is the section RVA and the label offset is an
  inline addend in the slot, which the code ignored.
* lld patched the **wrong word** for `PROLOG`: the record start was computed
  as `off - 8` for both relocations, but PROLOG lives at record offset 12, so
  PrologLen never reached the flags word (and the values it computed started
  from a `beginRVA` read out of the flags word).

~~Unverified premise that these fixes rely on: `dwSlot` in the COMPRESSED
lookup below (see item 2) — `wince-source` was not consulted in this
session.~~ **RESOLVED 2026-08-30: `dwSlot = 0` (exdsptch.c:1341/1447) — see
the verification addendum at the end of this section.**

---

The CE kernel-side dispatch rules for the 8-byte PDATA_EH pair, traced in
`PRIVATE/WINCEOS/COREOS/CORE/DLL/exdsptch.c` (`RtlLookupFunctionEntry`,
COMPRESSED_PDATA path ~line 1387; COMBINED_PDATA path ~line 1544):

1. **Layout**: `PDATA_EH = { PEXCEPTION_ROUTINE pHandler; PVOID pHandlerData; }`
   (exdsptch.c:933-936) — 8 bytes, sitting in the **8 bytes immediately
   before the function's first instruction** (pFuncStart).
2. **Lookup**: COMPRESSED path reads it as
   `(PPDATA_EH)((pFuncStart & ~(InstSize-1)) + dwSlot) - 1`, i.e. it masks
   off the Thumb +1 bit (and aligns for ARM) before backing up one slot.
   COMBINED path reads `(PPDATA_EH)(pFuncStart + dwSlot) - 1` (does not
   mask the thumb bit — relevant only if COMBINED tables are in use;
   lld's output is the COMPRESSED 8-byte form, matching the COMPRESSED
   path).
3. **Gate**: the pair is consulted **only when the function's .pdata entry
   has the ExceptionFlag bit set** (exdsptch.c:1385-1391); otherwise
   ExceptionHandler/HandlerData are zero.
4. **Invocation**: `RtlDispatchException` calls
   `RtlpExecuteHandlerForException(pExr, (PVOID)EstablisherFrame, pCtx,
   &DispatcherContext, FunctionEntry->ExceptionHandler)` with
   `DispatcherContext.EstablisherFrame = EstablisherFrame` (ARM/rtlsup.s
   `RtlpExecuteHandlerForException`: handler invoked with r0-r3 =
   ExceptionRecord, EstablisherFrame, ContextRecord, DispatcherContext —
   **the x64-style 4-argument handler convention**).  The CE non-x86
   `DISPATCHER_CONTEXT` (SDK excpt.h) is
   `{ ControlPc, FunctionEntry, EstablisherFrame, ContextRecord }` — it
   has **no HandlerData field** (unlike x64), so the handler reads the
   scope table via `DispatcherContext.FunctionEntry->HandlerData`
   (RtlLookupFunctionEntry fills `prf->ExceptionHandler`/`prf->HandlerData`
   from the PDATA_EH pair).  Dispositions are the standard
   ExceptionContinueSearch/ExecuteHandler/NestedException set;
   ExecuteHandler unwinds with `RtlUnwind(EstablisherFrame,
   DispatcherContext.ControlPc, ...)`.

Correspondence with the implementation (all pushed):

| Rule | Implementation |
|------|----------------|
| Pair = 8B {pHandler, pHandlerData}, before function start | `ARMAsmPrinter::emitFunctionEntryLabel` emits PDATA_EH (scope table then pair) immediately before the function label, gated on `functionUsesWinCFI && MF->hasEHFunclets` (parent functions only; ARMAsmPrinter.cpp:81-88) |
| ExceptionFlag set ⇔ handler present | `ARMWinCOFFStreamer::CEEmitUnwindInfo`: `HasHandler = Frame->HandlesExceptions && Frame->ExceptionHandler` sets `CE_PDATA_EXCEPTION_FLAG` (ARMWinCOFFStreamer.cpp:363-420); funclets (no .seh_handler) carry no flag — matching the dispatcher's gate |
| pHandler = `__C_specific_handler` | personality of the parent function (coredll6.def exports it); HandlerData = scope table count word (`emitCSpecificHandlerTable`) |
| EstablisherFrame = entry SP | `RtlVirtualUnwind`/`unwind.c` sets `*EstablisherFrame = Register[13]` at function entry; matches the entry-SP convention of §4f item 1 |

Still unverified (source not available): the body of
`__C_specific_handler` itself — how it walks the scope table and invokes
filters/finallys (the x64-convention assumption from §4f item 5 stands,
and the dispatcher-side convention above is now confirmed, but the CRT
handler internals are not in the reference extraction; crtdummy.cpp has
only a stub).

**Update 2026-08-29 — `__C_specific_handler` source does not exist in the
CE 6.0 public tree (now confirmed, not just missing from the
extraction):** the full `wince-source` tree was searched (git grep over
`ce600/PRIVATE/...` and the whole repo). The only source hit is
`CORE/DLL/CRT/crtdummy.cpp`'s `DUMMY(__C_specific_handler)`, where the
`DUMMY` macro generates a link-through stub with signature
`void *fn(void *, const void *, size_t)` — not even the real handler
signature, i.e. purely a linker placeholder. The export itself is
`CORE/DLL/CRT/corelib1.def:115` (`CRT(__C_specific_handler)`), which
routes the name into the built (k.)coredll image; the actual
implementation ships as part of the closed coredll binary. What *is*
confirmed from the SDK headers:

- `__C_specific_handler` is **the** SEH handler for all non-x86 CE
  (SDK excpt.h: `#ifdef _X86_ _except_handler3 #else __C_specific_handler
  #endif`), with the 4-argument `EXCEPTION_ROUTINE` signature.
- The kernel-side convention is fully confirmed (PDATA_EH location,
  ExceptionFlag gate, 4-arg invocation, DispatcherContext layout).
  The one remaining unknown is internal to the closed binary: which
  values it passes to filter/finally functions (the x64 convention —
  establisher frame as the filter's 2nd argument — is the working
  assumption, to be confirmed on device or by disassembling coredll),
  and how it walks `{Begin,End,FilterOrFinally,Handler/Jump}` scope
  entries. Nothing in the public tree contradicts the assumed format;
  x86's `rtlsupsafeseh.asm` only marks C handlers safe via `.SAFESEH`
  (x86-only mechanism, irrelevant on ARM).


`coredll6.def` exports `__CxxFrameHandler` (no `3` suffix). **Verified
2026-08-30 against `CORE/CORELIBC/CRTW32/EH/frame.cpp`:** on non-x86 the
exported `__CxxFrameHandler` is a one-line pass-through to
`__CxxFrameHandler3` (commented "Old entrypoint to the runtime"), which —
like NT's v3 personality — reads the per-function tables as
`FuncInfo *pFuncInfo = (FuncInfo*)pDC->FunctionEntry->HandlerData`. So the
CE handler consumes v3-style `FuncInfo` tables, not a v1 layout; the
no-suffix export name is just the CE-era symbol. The `FuncInfo`/`UnwindMap`/
`TryBlockMap`/`HandlerType`/`ThrowInfo`/`CatchableType` struct layouts are
declared in `ehdata.h`, which is **not** in the `wince-source` tree (only
the `CRTW32/EH/*.cpp` implementation files are present), so the exact layout
remains blocked until that header (or an equivalent structural reference) is
available. As the handoff already notes, this is lower priority than SEH
since C++ exceptions route through EHABI regardless.

**Verification addendum (2026-08-30): §4g cross-checked against `wince-source`
(`ce600/PRIVATE/WINCEOS/COREOS/CORE/DLL/exdsptch.c`, `.../DLL/ARM/rtlsup.s`,
`.../CORE/DLL/CRT/{crtdummy.cpp,corelib1.def}`, `ce600/PUBLIC/COMMON/SDK/INC/excpt.h`)
and against real CE-compiler ARM binaries from the same tree. Every item below was a
source-level assumption until now; all are confirmed:**

1. **`dwSlot = 0`** — exdsptch.c:1341 (COMPRESSED) and :1447 (COMBINED):
   `DWORD dwSlot = 0; //FunctionTableAddr - ZeroPtr (FunctionTableAddr)`.
   Table addresses are zero-based within the DLL image, so the slot offset
   stays 0.
2. **The PDATA record is a single 8-byte structure; all bitfields live in
   word1** (exdsptch.c:42-48): word0 = `pFuncStart`; word1 = `PrologLen:8`
   (bits 0-7) | `FuncLen:22` (bits 8-29) | `ThirtyTwoBits:1` (bit 30) |
   `ExceptionFlag:1` (bit 31). The flag fields share word1 because the first
   30 bits leave room in the same allocation unit — there is **no third
   word**. `FuncLen`/`PrologLen` are in *instructions*:
   `InstSize = entry->ThirtyTwoBits ? 4 : 2` (exdsptch.c:1378) and
   `EndAddress = pFuncStart + FuncLen*InstSize` (:1380-1381).
   Verified against real CE-compiler ARM images in the tree
   (`OTHERS/DOTNETV2/ARMV4I/{cgacutil.exe,mscoree.dll}`,
   `OTHERS/EDB/ARMV4I/sqlceme30.dll`,
   `OTHERS/DOTNETV35/ARMV4I/mscoree3_5.dll`): **4466/4466 entries** are
   consistent with exactly this layout (pfs + FuncLen*InstSize chains to the
   next entry in 898/903 adjacent pairs; bit 30 set iff 32-bit function;
   bit 31 set only on the SEH functions). This also confirms the emitter
   constants `CE_PDATA_THIRTY_TWO_BIT = 0x40000000` /
   `CE_PDATA_EXCEPTION_FLAG = 0x80000000` in
   `ARMWinCOFFStreamer.cpp` were correct.
3. **PDATA_EH pair**: the 8 bytes immediately before pFuncStart hold
   `{ handler, handlerData }` (exdsptch.c:1387-1390). Observed in 20 real SEH
   entries: every `handler` is the same image-local `__C_specific_handler`
   VA; `handlerData` points at the scope table.
4. **Scope table format**: 4-byte count word, then 16-byte entries
   `{ BeginVA, EndVA, FilterOrFinally, HandlerJump }`, all absolute VAs.
   `FilterOrFinally` is the address of the outlined filter/finally function;
   for `__finally` blocks `HandlerJump == End` of the finally block (the
   "null jump" form). Confirmed on all three cgacutil.exe SEH functions.
5. **Lookup/gate verbatim**: COMPRESSED
   `peh = (PPDATA_EH)((entry->pFuncStart & ~(InstSize-1)) + dwSlot) - 1`
   (exdsptch.c:1387); COMBINED
   `(PPDATA_EH)(NewEntry->pFuncStart + dwSlot) - 1` (:1544, no thumb-bit
   mask — matches the note above); gate on `entry->ExceptionFlag`
   (:1385-1391).
6. **Kernel invocation convention** (DLL/ARM/rtlsup.s:43-104):
   `RtlpExecuteHandlerForException` takes r0-r3 = ExceptionRecord,
   EstablisherFrame, ContextRecord, DispatcherContext; the routine itself is
   passed via the stack; the handler is invoked in system or Thumb mode per
   the Interworking/Thumbing build flag. Matches the 4-argument
   `EXCEPTION_ROUTINE` in `PUBLIC/COMMON/SDK/INC/excpt.h`, whose non-x86
   `DISPATCHER_CONTEXT` is exactly
   `{ ControlPc, FunctionEntry, EstablisherFrame, ContextRecord }`.
7. **`__C_specific_handler` provenance**: `CRT/crtdummy.cpp:114-131`
   `DUMMY(__C_specific_handler)` (stub `void* fn(void*, const void*,
   size_t)` — a link-through placeholder, not the real handler) and
   `CRT/corelib1.def:115` `CRT(__C_specific_handler)` under
   `#ifndef _X86_`. The real handler remains closed-source, as previously
   noted.

**Not covered by the binary sample**: all four sampled images are pure ARM
(machine 0x01c2); no Thumb entry was observed, so the Thumb side (`pfs` bit 0
set, `ThirtyTwoBits = 0`, `InstSize = 2`) rests on the kernel code path above
plus the standard CE/Win32 Thumb entry convention.

## 4f. Resolution of 4e + Phase 2 implementation status (2026-08-29, this session)

**4e resolved (source-level):** CodeGen *does* drive EHABI table generation for
compiled C/C++. `ARMException::beginFunction()` calls
`getTargetStreamer().emitFnStart()` and `ARMException::endFunction()` calls
`emitFnEnd()` whenever `MAI->getExceptionHandlingType() == ExceptionHandling::ARM`
(which is the case for WinCE per `ARMMCTargetDesc.cpp`). The 4e search missed
`ARMException.cpp` because the call goes through the `ARMTargetStreamer` virtual
interface, not directly to `ARMWinCOFFStreamer::EHABIemitFnStart`. So the
".ARM.exidx entries exist for compiled C++" premise holds: `.fnstart` opens an
entry in `ARMWinCOFFStreamer::EHABIemitFnStart` and `.fnend` closes it (entry
persisted via `PendingFnStarts` and flushed in `finishImpl`). Still worth a build
check, but the wiring exists.

**Phase 2 SEH implementation (all pushed to origin/llvm-wince):**

| Area | What | Commit |
|------|------|--------|
| MC relocs/constants | `IMAGE_REL_ARM_CE_PDATA_FUNCLEN/PROLOG` (0x6/0x7), `VK_COFF_CE_PDATA_*`, `FrameInfo::CEEmitted` | 292f230d |
| Object writer | CE specifiers -> pseudo relocs; CE cross-section FK_Data_4 stays ADDR32 (fixed-address images) | 292f230d, 7bf00de0 |
| MC streamer | `CEEmitUnwindInfo` emits 16-byte intermediate .pdata records; `.seh_*` allowed on WinCE despite ARM EHABI MAI; error if .seh_endprologue/.seh_endproc missing | 292f230d, a1250eba |
| Sema/passes | `isSEHTrySupported` for WinCE; `createWinEHPass()` added to the ARM case (self-gating on MSVC_TableSEH personalities) | 8db91016 |
| AsmPrinter | SEH functions open a WinCFI frame (ARMException); scope table + PDATA_EH pair emitted immediately before the function label (ARMAsmPrinter::emitCEHandlerData), absolute ADDR32 entries | 2ee56ac1 |
| lld | resolves the pseudo relocations into the flags word, compacts 16->8 bytes, sorts by pFuncStart, exception dir size halved | ffbd4141 |
| Cleanup | dead ABI-incorrect `EncodingType::CE` triple emitter removed from MCWin64EH | 84aec8c1 |
| Parent frame | `llvm.eh.recoverfp` identity + `llvm.localaddress`=SP+frame size + `ISD::LOCAL_RECOVER` via `ARMISD::Wrapper`; `LDRLIT_ga_abs` taught to take non-GlobalValue symbol operands (`ARMConstantPoolSymbol`); Thumb-2 covered by the existing `HasV8MBaseline` texternalsym pattern (implied by V6T2 — the extra pattern added initially was dropped) | b31ba42f, e39ae2af |
| Tests | MC `.pdata` reloc test (`--expand-relocs`; readobj prints "Unknown" for all IMAGE_FILE_MACHINE_ARM reloc names), lld link test with two SEH functions linked in reverse order (sort/compaction/bitfields/pair placement), clang IR test (`wince-seh.c` incl. `_exception_code()`), backend ISel test (`wince-seh-parent-frame.ll`), EHABI+SEH mixed-TU test (`wince-seh-ehabi-mixed.c`) | 538df378, ffbd4141, fee05cf4, 86d9849a, 2ae5675e |

**Known limitations (v1, by design, to be re-checked with a build):**

1. *Funclet parent-frame references* (implemented 2026-08-29, commits
   b31ba42f + e39ae2af): `llvm.eh.recoverfp` (filters) lowers to the
   identity and `llvm.localaddress` lowers to SP + frame size, with the
   frame size assigned by the AsmPrinter to the `$parent_frame_offset`
   symbol.  This adopts an **entry-SP convention** (CE's establisher
   frame), matching AArch64's SEH model; `LOCAL_ESCAPE` offsets are
   entry-SP-relative (`MFI.getObjectOffset`), and `ISD::LOCAL_RECOVER`
   materializes the absolute frame-escape symbol value via
   `ARMISD::Wrapper` (movw/movt via the existing texternalsym patterns —
   `HasV8MBaselineOps` is implied by V6T2, so they cover v6T2+/Thumb-2;
   literal-pool load on ARMv4T/ARMv5, where `LDRLIT_ga_abs` falls back to
   `ARMConstantPoolSymbol` for non-GlobalValue operands).  Rationale: the
   CE kernel computes the establisher frame as the entry SP (prologue
   reverse-executed), which differs from x64 (SP after prologue); an x64
   style "+SEHSetFrameOffset" recoverfp would be off by the caller's frame
   size in nested helpers, so the identity + entry-SP convention is the
   only one consistent between the normal path (localaddress) and the
   exception path (establisher frame).
2. *Nested SEH helpers* (a `__try`/`__except` inside an outlined
   `__finally`): **not verified** (v1 limitation).  The identity recoverfp
   is actually friendlier to nesting than x64's "+offset" form (no
   caller-frame-size adjustment is needed), and a nested `__finally` with
   its own `__try` becomes an EH-funclet parent itself, so its
   `$parent_frame_offset` would be assigned by the same path as any other
   parent; but the whole nested-helper codegen path (EmitCapturedLocals
   recovering the parent finally's `frame_pointer` argument via
   `llvm.localrecover`) has not been traced end-to-end and needs a build
   check.  Until then it is treated as unsupported.
3. *Variable-sized objects* in a parent with SEH helpers: `llvm.localaddress`
   assumes a constant frame size (FIXME in ARMISelLowering; same AArch64
   limitation).
4. *Nested `__try` inside funclets*: `ARMException` has no `beginFunclet`
   implementation (no per-funclet .seh frames). Funclets are emitted inside the
   parent's single WinCFI frame (they are EH regions of the same MachineFunction
   in current LLVM, so the parent's .pdata FuncLen covers them).
5. *Filter functions* are outlined by clang into normal functions (2nd arg =
   parent FP per CE/ARM64 convention); they carry no PDATA_EH pair and are
   invoked by `__C_specific_handler` as plain callbacks.  The assumption
   that CE's `__C_specific_handler` forwards the kernel's establisher frame
   to filters/finallys (x64 convention) is unverified by source: the
   implementation is not in the reference extraction (crtdummy.cpp provides
   only a stub).

**Mixed EHABI + WinCFI in one TU (per-function dispatch, added 2026-08-29):**
the dispatch is per-function, so a single TU (and object file) can freely
mix EHABI functions and CE SEH functions:

- *Non-SEH functions*: `ARMException::beginFunction()`/`endFunction()`
  emit `.fnstart`/`.fnend`, opening a `.ARM.exidx` entry (verified in
  `llvm/test/CodeGen/ARM/wince-ehabi-tables.c`).
- *SEH functions*: the same `ARMException` methods branch on
  `hasWinCFI` first and emit `SEH_StartProc`/`SEH_EndProc` instead
  (ARMException.cpp beginFunction/endFunction, lines 31-96);
  `ARMAsmPrinter::emitInstruction` suppresses the EHABI `.fnstart`
  emission for these functions (ARMAsmPrinter.cpp:1997-2006) and emits
  the 8-byte PDATA_EH pair (`{pHandler, pHandlerData}`) immediately
  before the function label (ARMAsmPrinter.cpp:81-88, emitCEHandlerData);
  `ARMWinCOFFStreamer::emitWindowsUnwindTables` dispatches on `isCEEH`
  to `CEEmitUnwindInfo` (ARMWinCOFFStreamer.cpp:447-463).
- *Parent SEH functions*: `WinException::endFuncletImpl` emits
  `.seh_handlerdata` + `emitCSpecificHandlerTable`; funclets end with
  `.seh_endproc`; the `.seh_handler %except` marker uses `%` in ARM/Thumb
  assembly (MCAsmStreamer.cpp:2200-2204).
- *Test*: `clang/test/CodeGen/ARM/wince-seh-ehabi-mixed.c` — one function
  with EHABI try/catch (`.fnstart`/`.fnend`) and another with
  `__try`/`__except` (`.seh_proc`/`.seh_handler %except`/`.seh_endproc`)
  in the same TU, proving the two mechanisms coexist per-function.

