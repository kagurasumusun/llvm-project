# WinCE ARM WinEH (SEH) — compressed .pdata design

> **Provenance**: imported from branch `arena/wince-wineh-ce` (2026-08-30;
> see WINCE-HANDOFF.md §11.3). That branch's whole-target WinEH approach
> (`EncodingType::CE` wiring in AsmPrinter/AsmInfo) was **superseded** by the
> per-function WinCFI dispatch implemented on `llvm-wince`; no code from that
> branch was merged. This document is kept as a design reference. Its format
> claims were re-verified on 2026-08-30 against `wince-source` — see the
> verification addendum in WINEH-ABI-FACTS.md §4g (including a 4466-entry
> binary analysis of real CE-compiler `.pdata` sections).

CE6 kernel/coredll facts (from kagurasumusun/wince-source, ce600/PRIVATE):

## On-disk .pdata entry: 8 bytes (IMAGE_CE_RUNTIME_FUNCTION_ENTRY / PDATA)
  word0 = pFuncStart (absolute VA; thumb bit set for thumb funcs)
  word1 bitfields:
    PrologLen   : 8  (in INSTRUCTIONS: /4 for ARM32, /2 for Thumb)
    FuncLen     : 22 (in INSTRUCTIONS)
    ThirtyTwoBit: 1  (1 = ARM 32-bit inst (instsize 4); 0 = Thumb (instsize 2))
    ExceptionFlag:1 (MSB, 0x80000000)  => handler present

## Handler data (PDATA_EH): 8 bytes IMMEDIATELY BEFORE the function start VA
  (PEH = (PDATA_EH*)(pFuncStart & ~1) - 1):
    word at PEH+0 = pHandler     (absolute VA; personality e.g. __C_specific_handler)
    word at PEH+4 = pHandlerData (absolute VA; scope table for SEH, FuncInfo for C++ EH)

## Runtime
  RtlLookupFunctionEntry binary-searches .pdata (exception data dir);
  computes EndAddress   = pFuncStart + FuncLen*instsize
           PrologEnd    = pFuncStart + PrologLen*instsize
  reads PDATA_EH if ExceptionFlag; calls ExceptionHandler(EstablisherFrame...).
  Unwinder (unwind.c) reverse-executes the prolog machine code (no unwind opcodes).

## Our emission
  MC (ARMWinCOFFStreamer) per .seh frame:
    - in function's .text section, right before Begin label: 2 ADDR32 relocs
        => handler VA (+0), handlerData VA (+4)   (PDATA_EH)
    - in .pdata section:
        word0 = ADDR32 reloc -> Begin symbol (pFuncStart VA w/ thumb bit preserved)
        word1 = static bits 0x40000000 (ThirtyTwoBit if ARM) | 0x80000000 (if handler)
                + two LLD-internal pseudo relocs at same offset:
                  IMAGE_REL_ARM_CE_PDATA_FUNCLEN(0x6) -> End symbol   (fills FuncLen/PrologLen/ThirtyTwoBit)
                  IMAGE_REL_ARM_CE_PDATA_PROLOG(0x7)  -> PrologEnd    (fills PrologLen)
  LLD: finalize CE .pdata (config->wince): for each pseudo-reloc compute lengths,
       patch word1, drop pseudo relocs (no output reloc/base reloc for word1).
       Sort .pdata as 8-byte CE entries by word0 RVA.
