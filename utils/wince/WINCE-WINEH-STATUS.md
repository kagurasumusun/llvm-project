# WinCE ARM WinEH (SEH) implementation status

> **Provenance**: imported from branch `arena/wince-wineh-ce` (2026-08-30;
> see WINCE-HANDOFF.md §11.3). That branch's whole-target WinEH approach was
> **superseded** by the per-function WinCFI dispatch implemented on
> `llvm-wince`; no code from that branch was merged. Kept as a design
> reference. Format claims re-verified on 2026-08-30 against `wince-source`
> (WINEH-ABI-FACTS.md §4g verification addendum, incl. a 4466-entry binary
> analysis of real CE-compiler `.pdata` sections).

Branch: `arena/wince-wineh-ce`.  References: `utils/wince/WINCE-WINEH-DESIGN.md`,
and the ce600 shared source in kagurasumusun/wince-source:
- `PRIVATE/WINCEOS/COREOS/CORE/DLL/exdsptch.c`  (user-mode dispatcher,
  compressed .pdata decoder `RtlLookupFunctionEntry`, `RtlDispatchException`)
- `PRIVATE/WINCEOS/COREOS/CORE/DLL/ARM/unwind.c` (prolog reverse-execution)
- `PRIVATE/WINCEOS/COREOS/CORE/CORELIBC/CRTW32/EH/frame.cpp` (C++ EH runtime)
- `PUBLIC/COMMON/SDK/INC/winnt.h` (`IMAGE_CE_RUNTIME_FUNCTION_ENTRY`)

## Verified format facts

- On-disk .pdata == 8-byte `IMAGE_CE_RUNTIME_FUNCTION_ENTRY` per function:
  word0 pFuncStart (absolute VA; thumb bit set for Thumb),
  word1 = PrologLen:8 | FuncLen:22 | ThirtyTwoBit:1 | ExceptionFlag:1
  (lengths in *instructions*: /4 for ARM32, /2 for Thumb).
- Handled functions have an 8-byte PDATA_EH pair {pHandler, pHandlerData}
  immediately before the function start VA
  (`peh = (PDATA_EH*)(pFuncStart & ~1) - 1`).
- Unwinder uses no unwind opcodes: prolog machine code is reverse-executed,
  so only function/prologue extents + handler location are needed.
- `__C_specific_handler` (SEH) and `__CxxFrameHandler` (C++ EH) are exported
  by coredll6.def. The ce600 CRT's `__CxxFrameHandler` is a thin wrapper over
  the v3 handler and reads the same v3 FuncInfo tables.

## Implemented (this branch)

- `EncodingType::CE` real wiring:
  - `ARMCOFFMCAsmInfoGNU(IsWinCE=true)` sets ExceptionsType=WinEH +
    EncodingType::CE.
  - AsmPrinter routes EncodingType::CE to `WinException` (funclet/.seh path).
- MC: `ARMWinCOFFStreamer` gains a CE unwind emitter (`CEEmitUnwindInfo`):
  - in-text PDATA_EH pair (personality ADDR32 + handler-data ADDR32) into a
    `.text$di` code section that lld sorts before `.text$mn`;
  - compressed 8-byte .pdata entry: word0 ADDR32 to function begin;
    word1 static ThirtyTwoBit/ExceptionFlag bits plus two internal pseudo
    relocations `IMAGE_REL_ARM_CE_PDATA_FUNCLEN`(0x6)/`..._PROLOG`(0x7) that
    lld resolves to the instruction-count lengths and discards.
- `WinException`: CE routes SEH scope tables into `.text$di` (instead of
  .xdata) so they sit before the function body.
- COFF: new reloc constants + MCSymbolRefExpr VK_CE_PDATA_FUNCLEN/PROLOG;
  ARM WinCOFF writer maps them and keeps ADDR32 absolute across sections.
- lld/COFF: resolves the pseudo relocs (lengths in instructions), excludes
  them from base relocs, and sorts the compressed 8-byte .pdata
  (`sortCEExceptionTable`, keyed on pFuncStart).
- Clang: `isSEHTrySupported()` allows WinCE ARM/Thumb; WinCE toolchain
  GetExceptionModel() returns WinEH so __try/__except funclets + .seh are
  emitted; `__CxxFrameHandler` recognized as an MSVC C++ personality.

## Verified at MC level (llvm-mc, thumb-pc-wince)

A `.seh_proc/.seh_handler __C_specific_handler,%except/.seh_handlerdata`
function produces:
- `.text$di`: ADDR32 reloc to `__C_specific_handler` + ADDR32 to handler data;
- `.pdata`: ADDR32 reloc to .text at offset 0, pseudo 0x6 at 8, 0x7 at 12.
(Raw reloc types confirmed by parsing the object bytes; llvm-readobj prints
numeric types because the machine is IMAGE_FILE_MACHINE_ARM, not ARMNT.)

## Still TODO

- End-to-end clang __try/__except compile + lld link test (clang binary build
  on the 2-core/2GB sandbox is the bottleneck; CI builds it).
- lld link verification: confirm PDATA_EH lands directly before the function
  VA and pseudo relocs resolve to correct instruction counts.
- C++ EH (.seh_funclet + $cppxdata FuncInfo) data placement into .text$di;
  personality name __CxxFrameHandler3 vs coredll's __CxxFrameHandler (the
  runtime reads v3 tables either way).
- libunwind/runtime side is unchanged (the OS provides RtlDispatchException
  / __C_specific_handler); no compiler-rt EH work needed for SEH.

## Status (2026-08-30, llvm-wince)

- The "Still TODO" items above are addressed on `llvm-wince` by the
  per-function WinCFI implementation: the stage-1 CI build now succeeds
  (the two MCParser compile errors were fixed in `6b5cc2feb`), the WinCE lit
  suite runs in CI, and the scope table / PDATA_EH placement is covered by
  `llvm/test/MC/ARM/wince-seh-pdata.s` and `lld/test/COFF/wince-pdata.test`.
- The `__CxxFrameHandler` claim ("thin wrapper over the v3 handler") is
  confirmed against `ce600/.../CORELIBC/CRTW32/EH/frame.cpp`: on non-x86 the
  exported `__CxxFrameHandler` is a one-line pass-through to
  `__CxxFrameHandler3`, which reads the tables as
  `FuncInfo *pFuncInfo = (FuncInfo*)pDC->FunctionEntry->HandlerData`. The
  `FuncInfo` layout itself remains blocked on the absent `ehdata.h` (see
  WINEH-ABI-FACTS.md §4g).
