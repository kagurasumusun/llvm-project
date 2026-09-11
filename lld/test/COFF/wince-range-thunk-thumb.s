// REQUIRES: arm-registered-target
// RUN: llvm-mc -filetype=obj -triple=thumb-pc-wince -mcpu=arm926ej-s %s -o %t.obj
// RUN: lld-link /out:%t.exe /subsystem:windowsce /base:0x10000 /entry:main /export:callee %t.obj
// RUN: llvm-objdump -s --section=.text %t.exe | FileCheck %s --check-prefix=HEX
// RUN: llvm-readobj --coff-exports %t.exe | FileCheck %s --check-prefix=EXP
// RUN: llvm-objdump -s --section=.text %t.exe > %t.text
// RUN: llvm-readobj --coff-exports %t.exe > %t.exports
// RUN: %python %S/wince-range-thunk-thumb-check.py %t.exports %t.text

	.syntax unified
	.thumb
	.text
	.globl main
	.thumb_func
main:
	bl	callee
	bx	lr

	.bss
	.space	0x2200000

	.section	.text2,"xr"
	.thumb
	.globl	callee
	.thumb_func
callee:
	bx	lr

// HEX: 7847c046 00c09fe5
// HEX-NEXT: 1cff2fe1
// HEX-NOT: 1cff2fe1 00000000
// EXP: Name: callee
// EXP: RVA: 0x
