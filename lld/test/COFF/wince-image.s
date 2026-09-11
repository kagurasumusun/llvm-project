
// REQUIRES: arm-registered-target

// RUN: llvm-mc -filetype=obj -triple=arm-pc-wince -mcpu=arm926ej-s %s -o %t.obj

// RUN: lld-link /out:%t.exe /subsystem:windowsce /entry:WinMainCRTStartup \
// RUN:   /base:0x10000 /fixed %t.obj
// RUN: llvm-readobj --headers --coff-imports --symbols %t.exe | FileCheck %s

// An image names the CE release it was built for in the subsystem version, and
// there is nothing in the inputs to derive that from, so a bare /subsystem:
// windowsce keeps the value the CE 6 generation's SDKs used.
// CHECK: Machine: IMAGE_FILE_MACHINE_ARM (0x1C0)
// CHECK: ImageBase: 0x10000
// CHECK: MajorSubsystemVersion: 6
// CHECK: MinorSubsystemVersion: 0
// CHECK: Subsystem: IMAGE_SUBSYSTEM_WINDOWS_CE_GUI (0x9)
// CHECK: BaseRelocationTableRVA: 0x0
// CHECK: BaseRelocationTableSize: 0x0

// Spelling the version out is what a CE 4.x build does, and the operating
// system version fields follow it, since nothing else sets them either.
// RUN: lld-link /out:%t-ver.exe /subsystem:windowsce,4.2 \
// RUN:   /entry:WinMainCRTStartup /base:0x10000 /fixed %t.obj
// RUN: llvm-readobj --headers %t-ver.exe | FileCheck %s --check-prefix=VER
// VER: MajorOperatingSystemVersion: 4
// VER: MinorOperatingSystemVersion: 2
// VER: MajorSubsystemVersion: 4
// VER: MinorSubsystemVersion: 2

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
