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

## 5. `__CxxFrameHandler` v1 — still blocked

`coredll6.def` exports `__CxxFrameHandler` (no `3` suffix), confirming the v1-era C++ EH
ABI as the handoff already noted from the def file alone. The struct layouts needed to
actually implement or interoperate with it (`FuncInfo`, `UnwindMap`, `TryBlockMap`,
`HandlerType`, `ThrowInfo`, `CatchableType`) are declared in `ehdata.h`, which is **not**
in the current `wince-source` extraction. This remains blocked until that header (or an
equivalent structural reference) is available. As the handoff already notes, this is
lower priority than SEH since C++ exceptions route through EHABI regardless.
