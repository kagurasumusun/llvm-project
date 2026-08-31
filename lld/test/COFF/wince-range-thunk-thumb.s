// LLD COFF: Windows CE Thumb-1 branch range-extension veneer.
//
// Thumb BL (IMAGE_REL_ARM_BRANCH24T) stays in Thumb, so it cannot land
// on the ARM-mode RangeExtensionThunkARMCE. The veneer is GNU ld's v4t
// Thumb long-branch (ARMv5TE / arm926ej-s has no Thumb-2 movw/movt):
//   bx  pc
//   nop
//   ldr pc, [pc, #-4]
//   .word <target VA, bit 0 = Thumb>
//
// REQUIRES: arm-registered-target
//
// RUN: llvm-mc -filetype=obj -triple=thumb-pc-wince %s -o %t.obj
// RUN: lld-link /out:%t.exe /subsystem:windowsce /base:0x10000 %t.obj
// RUN: llvm-objdump -s --section=.text %t.exe | FileCheck %s

	.syntax unified
	.thumb
	.text
	.globl main
	.thumb_func
main:
	bl	callee
	bx	lr

// 0x2200000 (> 16 MB) of BSS so the Thumb BL is out of range.
	.bss
	.space	0x2200000

	.section	.text2,"xr"
	.thumb
	.globl	callee
	.thumb_func
callee:
	bx	lr

// Thumb-1 veneer bytes: bx pc; nop; ldr pc, [pc, #-4]
// CHECK: {{78 47 c0 46 04 f0 1f e5|7847c046 04f01fe5|7847c04604f01fe5}}
