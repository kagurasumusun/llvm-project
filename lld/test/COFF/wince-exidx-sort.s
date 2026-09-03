# REQUIRES: arm-registered-target
# RUN: llvm-mc -triple arm-pc-wince -filetype=obj -o %t.obj %s
# RUN: lld-link -wince /out:%t.exe /subsystem:windowsce /entry:entry /base:0x10000 /fixed %t.obj
# RUN: llvm-objdump -s --section=.ARM.exidx %t.exe | FileCheck %s

## The ARM EHABI requires .ARM.exidx entries in ascending function order:
## unwinders binary-search the table (libunwind's EHABISectionUpperBound).
## Here the object's index order (f_hi's .fnend comes first in the file)
## disagrees with the .text layout (COFF $-grouping sorts .text$aa before
## .text$zz), so the linker must sort the merged table by function address:
## f_lo (0x1100c) must precede f_hi (0x11010).  Entry layout: word 0 =
## absolute function VA, word 1 = EXIDX_CANTUNWIND (0x1).
## entry calls both functions: lld-link's default /opt:ref would otherwise
## dead-strip .text$aa/.text$zz, and the exidx cull then drops their index
## entries along with the dead functions.

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
