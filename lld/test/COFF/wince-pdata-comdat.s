# REQUIRES: arm-registered-target
# RUN: split-file %s %t
# RUN: llvm-mc -triple arm-pc-wince -mcpu=arm926ej-s -filetype=obj -o %t1.obj %t/1.s
# RUN: llvm-mc -triple arm-pc-wince -mcpu=arm926ej-s -filetype=obj -o %t2.obj %t/2.s
# RUN: lld-link /out:%t.exe /subsystem:windowsce /entry:entry /base:0x10000 /fixed %t1.obj %t2.obj
# RUN: llvm-readobj --headers %t.exe | FileCheck %s --check-prefix=HDR
# RUN: llvm-readobj --sections %t.exe | FileCheck %s --check-prefix=SEC


# HDR:      ExceptionTableSize: 0x10

# SEC:      Name: .pdata
# SEC-NEXT: VirtualSize: 0x20

#--- 1.s
	.syntax unified
	.arm
	.text
	.globl	entry
entry:
	.seh_proc entry
	push	{r4, lr}
	.seh_save_regs {r4, lr}
	.seh_endprologue
	mov	r0, #42
	pop	{r4, pc}
	.seh_endproc

	.section .text$same,"wx"
	.linkonce discard
	.globl	same
same:
	.seh_proc same
	push	{r4, lr}
	.seh_save_regs {r4, lr}
	.seh_endprologue
	mov	r0, #7
	pop	{r4, pc}
	.seh_endproc

#--- 2.s
	.syntax unified
	.arm
	.text
	.section .text$same,"wx"
	.linkonce discard
	.globl	same
same:
	.seh_proc same
	push	{r4, lr}
	.seh_save_regs {r4, lr}
	.seh_endprologue
	mov	r0, #7
	pop	{r4, pc}
	.seh_endproc
