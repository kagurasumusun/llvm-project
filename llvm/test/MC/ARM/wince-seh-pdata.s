@ RUN: llvm-mc -triple thumb-pc-wince -filetype=obj -o %t.o %s
@ RUN: llvm-readobj -S -r --expand-relocs %t.o | FileCheck %s

/// Windows CE SEH (.seh_proc/.seh_handler) must produce the compressed
/// WinCE .pdata layout that the CE6 kernel's RtlLookupFunctionEntry decodes
/// (see ce600 PRIVATE/.../CORE/DLL/exdsptch.c):
///
///   * .pdata entries are 8 bytes in the final image: word0 = pFuncStart
///     (ADDR32 absolute VA), word1 = PrologLen:8 | FuncLen:22 |
///     ThirtyTwoBit:1 | ExceptionFlag:1.  The object file carries a
///     16-byte-per-function intermediate form so the length pseudo
///     relocations (IMAGE_REL_ARM_CE_PDATA_FUNCLEN = 0x6,
///     IMAGE_REL_ARM_CE_PDATA_PROLOG = 0x7) can attach to their own words;
///     lld resolves them into the flags word, drops them, and compacts each
///     record to 8 bytes.
///   * The PDATA_EH pair (personality VA, handler-data VA) and the SEH scope
///     table are emitted by the compiler immediately before the function
///     body (ARMAsmPrinter), NOT into a shared section, so the pair sits in
///     the 8 bytes right before the function's first instruction for every
///     SEH function in the module.
///
/// --expand-relocs prints "Type: IMAGE_REL_ARM_* (N)".  WinCE pdata uses
/// ADDR32 for pFuncStart and the two CE_PDATA pseudo relocs for the
/// length/prolog fields (lld folds those into the flags word).

	.syntax unified
	.thumb
	.text
	.p2align 2
	.globl	sehfn
	.thumb_func
sehfn:
	.seh_proc sehfn
	.seh_handler __C_specific_handler, %except
	.seh_endprologue
	push	{r4, r5, lr}
	.seh_nop
	pop	{r4, r5, pc}
	.seh_endproc

/// A plain leaf function: it must produce no .pdata entry (no .seh frame).
	.globl	leaf
	.thumb_func
leaf:
	bx	lr

// CHECK: Sections [
// CHECK:   Name: .pdata

/// The CE6 kernel reads .pdata at runtime (RtlLookupFunctionEntry), so the
/// section must be mapped initialized data, NOT discardable.  A discardable
/// .pdata makes lld skip base relocations for it -- both ARM::addBaserels and
/// createRuntimePseudoRelocs early-return on IMAGE_SCN_MEM_DISCARDABLE -- so its
/// absolute pFuncStart VAs would be left unrelocated in any image that loads off
/// its preferred base (the usual runtime case).  See WINEH-ABI-FACTS.md 4m.
/// This guards the ARMWinCOFFStreamer::CEEmitUnwindInfo characteristics fix.
/// (CHECK-NOT is scoped from '[' to MEM_READ: MEM_DISCARDABLE 0x02000000 sorts
/// before MEM_READ 0x40000000, so a regressed discardable bit is always caught.)
// CHECK:   Characteristics [
// CHECK-NOT: IMAGE_SCN_MEM_DISCARDABLE
// CHECK:     IMAGE_SCN_MEM_READ
// CHECK:   ]

// The single .seh frame produces one 16-byte intermediate .pdata record.
// CHECK: Relocations [
// CHECK:   Section {{.*}} .pdata {
// pFuncStart -> function start (ADDR32 == 1)
// CHECK:      Offset: 0x0
// CHECK-NEXT: Type: IMAGE_REL_ARM_ADDR32 (1)
// CHECK-NEXT: Symbol: .text
// word1 pseudo relocations: FUNCLEN (6) then PROLOG (7)
// CHECK:      Offset: 0x8
// CHECK-NEXT: Type: IMAGE_REL_ARM_CE_PDATA_FUNCLEN (6)
// CHECK:      Offset: 0xC
// CHECK-NEXT: Type: IMAGE_REL_ARM_CE_PDATA_PROLOG (7)
// CHECK:   }
// CHECK: ]
