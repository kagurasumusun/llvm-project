# REQUIRES: arm-registered-target
# RUN: split-file %s %t
# RUN: llvm-mc -triple arm-pc-wince -filetype=obj -o %t1.obj %t/1.s
# RUN: llvm-mc -triple arm-pc-wince -filetype=obj -o %t2.obj %t/2.s
# RUN: lld-link -wince /out:%t.exe /subsystem:windowsce /entry:entry /base:0x10000 /fixed %t1.obj %t2.obj
# RUN: llvm-readobj --headers %t.exe | FileCheck %s --check-prefix=HDR
# RUN: llvm-readobj --sections %t.exe | FileCheck %s --check-prefix=SEC

## Two objects define the same COMDAT function, each with a Windows CE
## .pdata entry (the real-world shape: template instantiations from two C++
## TUs, which on Windows CE carry a .pdata entry per function).  The COMDAT
## loser's function is discarded by the linker; its .pdata entry's three
## relocations (pFuncStart, FUNCLEN, PROLOG) would otherwise point into a
## section that is not in the image and fail the link with "relocation
## against symbol in discarded section".  The cull must drop the loser's
## .pdata table exactly as it drops the .ARM.exidx table, so the link
## succeeds and the merged .pdata holds exactly the two live functions
## (entry and the surviving copy of the COMDAT function): 2 entries * 8
## bytes = 0x10.

# HDR:      ExceptionTableSize: 0x10

# SEC:      Name: .pdata
# SEC-NEXT: VirtualSize: 0x10

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
