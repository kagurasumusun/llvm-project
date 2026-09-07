@ RUN: llvm-mc -triple arm-pc-wince -filetype=obj -o - %s | llvm-readobj -S -r - | FileCheck %s


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

	.globl	g
g:
	.fnstart
	.cantunwind
	bx	lr
	.fnend

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

// CHECK: Relocations [
// CHECK: Section {{.*}}.ARM.exidx {
// CHECK-NEXT: 0x0 IMAGE_REL_ARM_ADDR32
// CHECK-NEXT: 0x8 IMAGE_REL_ARM_ADDR32
// CHECK-NEXT: 0x10 IMAGE_REL_ARM_ADDR32
// CHECK-NEXT: 0x14 IMAGE_REL_ARM_ADDR32
// CHECK-NEXT: }
// CHECK: Section {{.*}}.ARM.extab {
// CHECK-NEXT: 0x0 IMAGE_REL_ARM_ADDR32 __gxx_personality_v0
// CHECK-NEXT: }
// CHECK-NEXT: ]
