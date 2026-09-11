# REQUIRES: arm-registered-target
# RUN: llvm-mc -triple arm-pc-wince -mcpu=arm926ej-s -filetype=obj -o %t.obj %s
# RUN: lld-link /out:%t.exe /subsystem:windowsce /entry:entry /base:0x10000 /fixed %t.obj
# RUN: llvm-objdump -s --section=.ARM.exidx %t.exe | FileCheck %s


	.syntax unified
	.arm

	.section .text$zz,"x"
	.globl	f_hi
f_hi:
	.fnstart
	.cantunwind
	bx	lr
	.fnend

	.section .text$aa,"x"
	.globl	f_lo
f_lo:
	.fnstart
	.cantunwind
	bx	lr
	.fnend

	.text
	.globl	entry
entry:
	bl	f_lo
	bl	f_hi
	bx	lr

# CHECK: Contents of section .ARM.exidx
# CHECK-NEXT: {{[0-9a-f]+}} 0c100100 01000000 10100100 01000000
