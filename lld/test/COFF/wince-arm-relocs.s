# REQUIRES: arm-registered-target
# RUN: llvm-mc -triple arm-pc-wince -mcpu=cortex-a8 -filetype=obj -o %t.obj %s
# RUN: lld-link /out:%t.exe /subsystem:windowsce /entry:entry /base:0x10000 /fixed %t.obj
# RUN: llvm-readobj --file-headers %t.exe | FileCheck %s --check-prefix=HDR
# RUN: llvm-objdump -d %t.exe | FileCheck %s

## Windows CE ARM images: linker applies the ARM-mode relocations
## (MOV32A literal formation, BRANCH24 BL/BLX, ADDR32 absolute data)
## exactly like the binutils arm-wince emulation does.

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
	.type	thumbcallee, %function
thumbcallee:
	bx	lr

	.data
	.globl	dvar
dvar:
	.long	0x55667788

## .text RVA 0x1000, .data RVA 0x2000 -> dvar VA 0x12000:
## movw picks up 0x2000, movt 0x0001.
# HDR: Machine: IMAGE_FILE_MACHINE_ARM (0x1C0)
# HDR: SubSystem: IMAGE_SUBSYSTEM_WINDOWS_CE_GUI (0x9)
# HDR: ImageBase: 0x10000

# CHECK: movw r0, #8192
# CHECK: movt r0, #1
# CHECK: bl {{.*}} <callee>
# CHECK: blx {{.*}} <thumbcallee>
