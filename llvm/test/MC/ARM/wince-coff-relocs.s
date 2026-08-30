@ RUN: llvm-mc -triple arm-pc-wince -mcpu=cortex-a8 -filetype=obj -o - %s | llvm-readobj --file-headers -r - | FileCheck %s

/// Windows CE ARM objects: IMAGE_FILE_MACHINE_ARM (0x1c0) and the ARM-mode
/// (non-Thumb-2-only) relocation set.  ARM BL/BLX -> BRANCH24,
/// MOVW/MOVT pairs -> MOV32A (single reloc at the MOVW), Thumb BL/BLX/BEQ
/// -> BRANCH24T/BLX23T/BRANCH20T, data words -> ADDR32.

	.syntax unified
	.arm
	.text
	.globl	armfn
armfn:
	bl	ext
	blx	thumbext
	movw	r0, :lower16:extvar
	movt	r0, :upper16:extvar
	bx	lr
	.word	extvar

	.thumb
	.globl	thumbfn
thumbfn:
	bl	thumbext2
	blx	armext2
	beq	thumbext3
	bx	lr

// CHECK: Format: COFF-ARM
// CHECK: Machine: IMAGE_FILE_MACHINE_ARM (0x1C0)

// CHECK: Relocations [
// CHECK-NEXT: Section {{.*}}.text {
// CHECK-NEXT: 0x0 IMAGE_REL_ARM_BRANCH24 ext
// CHECK-NEXT: 0x4 IMAGE_REL_ARM_BRANCH24 thumbext
// CHECK-NEXT: 0x8 IMAGE_REL_ARM_MOV32A extvar
// CHECK-NEXT: 0x14 IMAGE_REL_ARM_ADDR32 extvar
// CHECK-NEXT: 0x1C IMAGE_REL_ARM_BRANCH24T thumbext2
// CHECK-NEXT: 0x20 IMAGE_REL_ARM_BLX23T armext2
// CHECK-NEXT: 0x24 IMAGE_REL_ARM_BRANCH20T thumbext3
// CHECK-NEXT: }
// CHECK-NEXT: ]
