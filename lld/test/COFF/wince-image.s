
// REQUIRES: arm-registered-target

// RUN: llvm-mc -filetype=obj -triple=arm-pc-wince %s -o %t.obj

// RUN: lld-link /out:%t.exe /subsystem:windowsce /entry:WinMainCRTStartup \
// RUN:   /base:0x10000 /fixed %t.obj
// RUN: llvm-readobj --headers --coff-imports --symbols %t.exe | FileCheck %s

// CHECK: Machine: IMAGE_FILE_MACHINE_ARM (0x1C0)
// CHECK: ImageBase: 0x10000
// CHECK: Subsystem: IMAGE_SUBSYSTEM_WINDOWS_CE_GUI (0x9)
// CHECK: BaseRelocationTableRVA: 0x0
// CHECK: BaseRelocationTableSize: 0x0

	.text
	.globl	WinMainCRTStartup
WinMainCRTStartup:
	mov	r0, #0
	bx	lr

	.globl	getmsg
getmsg:
	ldr	r0, .Lmsg
	ldr	r0, [r0]
	bx	lr
.Lmsg:
	.long	message

	.comm	message, 4, 2

	.globl	ref_text_bounds
ref_text_bounds:
	.long	__text_start__
	.long	__text_end__
