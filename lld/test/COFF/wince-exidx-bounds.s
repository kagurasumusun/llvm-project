# REQUIRES: arm-registered-target
# RUN: llvm-mc -triple arm-pc-wince -filetype=obj -o %t.obj %s
# RUN: lld-link -wince /out:%t.exe /subsystem:windowsce /entry:entry /base:0x10000 /fixed %t.obj
# RUN: llvm-readobj --symbols --sections %t.exe | FileCheck %s

## The WinCE C++ runtime (libunwind EHABI reader) locates the merged
## exception index through __exidx_start/__exidx_end, which the COFF
## writer binds to the .ARM.exidx output-section bounds (mirroring the
## ELF behavior).

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

## Two exidx entries, 8 bytes each, in the correctly-named output section.
# CHECK: Name: .ARM.exidx
# CHECK: VirtualSize: 0x10

# CHECK: Name: __exidx_start
# CHECK: Section: .ARM.exidx
# CHECK: Name: __exidx_end
# CHECK: Section: .ARM.exidx
