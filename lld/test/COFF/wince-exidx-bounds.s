# REQUIRES: arm-registered-target
# RUN: llvm-mc -triple arm-pc-wince -filetype=obj -o %t.obj %s
# RUN: lld-link -wince /out:%t.exe /subsystem:windowsce /entry:entry /base:0x10000 /fixed %t.obj
# RUN: llvm-readobj --symbols --sections %t.exe | FileCheck %s


	.syntax unified
	.arm
	.text
	.globl	entry
entry:
	.fnstart
	.cantunwind
	bx	lr
	.fnend

	.globl	f2
f2:
	.fnstart
	.cantunwind
	bx	lr
	.fnend

	.data
	.globl	bounds
bounds:
	.long	__exidx_start
	.long	__exidx_end

# CHECK: Name: .ARM.exidx
# CHECK: VirtualSize: 0x10
