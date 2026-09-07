@ RUN: llvm-mc -triple thumb-pc-wince -filetype=obj -o %t.o %s
@ RUN: llvm-readobj -S -r --expand-relocs %t.o | FileCheck %s


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

	.globl	leaf
	.thumb_func
leaf:
	bx	lr

// CHECK: Sections [
// CHECK:   Name: .pdata

// CHECK:   Characteristics [
// CHECK-NOT: IMAGE_SCN_MEM_DISCARDABLE
// CHECK:     IMAGE_SCN_MEM_READ
// CHECK:   ]

// CHECK: Relocations [
// CHECK:   Section {{.*}} .pdata {
// CHECK:      Offset: 0x0
// CHECK-NEXT: Type: IMAGE_REL_ARM_ADDR32 (1)
// CHECK-NEXT: Symbol: .text
// CHECK:      Offset: 0x8
// CHECK-NEXT: Type: IMAGE_REL_ARM_CE_PDATA_FUNCLEN (23)
// CHECK:      Offset: 0xC
// CHECK-NEXT: Type: IMAGE_REL_ARM_CE_PDATA_PROLOG (24)
// CHECK:   }
// CHECK: ]
