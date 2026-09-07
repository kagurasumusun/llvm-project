
// REQUIRES: arm-registered-target

// RUN: llvm-mc -filetype=obj -triple=arm-pc-wince %s -o %t.obj
// RUN: lld-link /out:%t.exe /subsystem:windowsce /base:0x10000 /entry:main %t.obj
// RUN: llvm-objdump -d --section=.text %t.exe | FileCheck %s

	.syntax unified
	.arm
	.text
	.globl main
main:
	bl	callee
	bx	lr

	.bss
	.space	0x2200000

	.section	.text2,"xr"
	.globl	callee
callee:
	bx	lr

// CHECK: bl
// CHECK: e12fff1e
// CHECK: ldr{{.*}}r12, [pc
// CHECK: e12fff1c
