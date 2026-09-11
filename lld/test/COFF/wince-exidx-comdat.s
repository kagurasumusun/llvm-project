# REQUIRES: arm-registered-target
# RUN: split-file %s %t
# RUN: llvm-mc -triple arm-pc-wince -mcpu=arm926ej-s -filetype=obj -o %t1.obj %t/1.s
# RUN: llvm-mc -triple arm-pc-wince -mcpu=arm926ej-s -filetype=obj -o %t2.obj %t/2.s
# RUN: lld-link /out:%t.exe /subsystem:windowsce /entry:entry /base:0x10000 /fixed %t1.obj %t2.obj
# RUN: llvm-readobj --sections %t.exe | FileCheck %s


# CHECK: Name: .ARM.exidx
# CHECK: VirtualSize: 0x10

#--- 1.s
	.syntax unified
	.arm
	.text
	.globl	entry
entry:
	.fnstart
	.cantunwind
	bx	lr
	.fnend

	.section .text$same,"wx"
	.linkonce discard
	.globl	same
same:
	.fnstart
	.cantunwind
	bx	lr
	.fnend

#--- 2.s
	.syntax unified
	.arm
	.text
	.section .text$same,"wx"
	.linkonce discard
	.globl	same
same:
	.fnstart
	.cantunwind
	bx	lr
	.fnend
