# REQUIRES: arm-registered-target
# RUN: llvm-mc -triple arm-pc-wince -filetype=obj -o %t.obj %s
# RUN: lld-link -wince /out:%t.exe /subsystem:windowsce /entry:entry /base:0x10000 /fixed %t.obj
# RUN: llvm-objdump -s --section=.ARM.exi %t.exe | FileCheck %s

## The ARM EHABI requires .ARM.exidx entries in ascending function order:
## unwinders binary-search the table (libunwind's EHABISectionUpperBound).
## Here the object's index order (f_hi's .fnend comes first in the file)
## disagrees with the .text layout (COFF $-grouping places .text$aa before
## .text$zz), so the linker must sort the merged table by function address:
## f_lo (0x1100c) must precede f_hi (0x11010).  Entry layout: word 0 =
## absolute function VA, word 1 = EXIDX_CANTUNWIND (0x1).
##
## The output section is named ".ARM.exi" in the PE headers: PE section
## names are 8 bytes inline, and lld truncates ".ARM.exidx" rather than
## emitting a string-table alias (same contract as wince-exidx-bounds.s;
## libunwind locates the table through the __exidx_start/__exidx_end
## symbols, not the section name, so the truncation is cosmetic).
## entry calls both functions so they stay reachable for /opt:ref-style
## dead stripping of unreferenced sections.

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

# CHECK: Contents of section .ARM.exi
# CHECK-NEXT: {{[0-9a-f]+}} 0c100100 01000000 10100100 01000000
