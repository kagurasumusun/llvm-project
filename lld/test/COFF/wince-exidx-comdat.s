# REQUIRES: arm-registered-target
# RUN: split-file %s %t
# RUN: llvm-mc -triple arm-pc-wince -filetype=obj -o %t1.obj %t/1.s
# RUN: llvm-mc -triple arm-pc-wince -filetype=obj -o %t2.obj %t/2.s
# RUN: lld-link -wince /out:%t.exe /subsystem:windowsce /entry:entry /base:0x10000 /fixed %t1.obj %t2.obj
# RUN: llvm-readobj --sections %t.exe | FileCheck %s

## Two objects define the same COMDAT function, each with an .ARM.exidx
## entry for it (the real-world shape: template instantiations from two
## C++ TUs).  The COMDAT loser's function is discarded, and its exidx
## entry must be dropped with it - keeping it would leave a relocation
## into a section that is not in the image, and the EHABI unwinder walks
## the merged index unconditionally.  The link must succeed and the
## merged index must hold exactly two entries: one for entry and one for
## the surviving copy of the COMDAT function (8 bytes each).

# CHECK: Name: .ARM.exi
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
