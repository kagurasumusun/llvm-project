@ RUN: llvm-mc -triple arm-pc-wince -filetype=obj -o - %s | llvm-readobj -S -r - | FileCheck %s

/// Windows CE uses userland ARM EHABI unwinding on COFF: the WinCE ARM
/// COFF streamer implements the .fnstart/.fnend state machine and emits
/// .ARM.exidx/.ARM.extab sections whose words reference functions and
/// unwind data with absolute ADDR32 relocations (the WinCE encoding that
/// libunwind reads back).

	.syntax unified
	.arm
	.text
	.globl	f
f:
	.fnstart
	.save	{r4, r11, lr}
	.setfp	r11, sp, #0
	pop	{r4, r11, pc}
	.fnend

/// A leaf function that cannot unwind.
	.globl	g
g:
	.fnstart
	.cantunwind
	bx	lr
	.fnend

/// A function with a C++ personality routine.
	.globl	h
h:
	.fnstart
	.personality	__gxx_personality_v0
	.pad	#0
	.handlerdata
	.fnend

// CHECK: Sections [
// CHECK: Name: .ARM.exidx
// CHECK: Name: .ARM.extab

/// .ARM.exidx: one 8-byte entry per function.  f: [fnstart ADDR32][inline
/// PR0 opcodes]; g: [fnstart ADDR32][EXIDX_CANTUNWIND const]; h: [fnstart
/// ADDR32][extab ADDR32].
// CHECK: Relocations [
// CHECK: Section .ARM.exidx {
// CHECK-NEXT: 0x0 IMAGE_REL_ARM_ADDR32
// CHECK-NEXT: 0x8 IMAGE_REL_ARM_ADDR32
// CHECK-NEXT: 0x10 IMAGE_REL_ARM_ADDR32
// CHECK-NEXT: 0x14 IMAGE_REL_ARM_ADDR32
// CHECK-NEXT: }
/// .ARM.extab: h's unwind opcodes preceded by the personality pointer.
// CHECK: Section .ARM.extab {
// CHECK-NEXT: 0x0 IMAGE_REL_ARM_ADDR32 __gxx_personality_v0
// CHECK-NEXT: }
// CHECK-NEXT: ]
