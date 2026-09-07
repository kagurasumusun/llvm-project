# REQUIRES: arm-registered-target
# RUN: llvm-mc -triple arm-pc-wince -mcpu=cortex-a8 -filetype=obj -o %t.obj %s
# RUN: lld-link /out:%t.exe /subsystem:windowsce /entry:entry /base:0x10000 /fixed %t.obj
# RUN: llvm-readobj --file-headers %t.exe | FileCheck %s --check-prefix=HDR
# RUN: llvm-objdump -d %t.exe | FileCheck %s


	.syntax unified
	.arm
	.text
	.globl	entry
entry:
	movw	r0, :lower16:dvar
	movt	r0, :upper16:dvar
	bl	callee
	blx	thumbcallee
	bx	lr
callee:
	bx	lr
	.thumb
	.thumb_func
thumbcallee:
	bx	lr

	.data
	.globl	dvar
dvar:
	.long	0x55667788

# HDR: Machine: IMAGE_FILE_MACHINE_ARM (0x1C0)
# HDR: ImageBase: 0x10000
# HDR: Subsystem: IMAGE_SUBSYSTEM_WINDOWS_CE_GUI (0x9)

# CHECK: e3020000
# CHECK: e3400001
# CHECK: bl 0x11014
# CHECK: blx 0x11018
