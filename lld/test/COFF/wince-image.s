/// LLD COFF: Windows CE image layout.
/// The produced PE must match the WinCE loader requirements as implemented
/// by the CeGCC/binutils arm-wince emulation: machine 0x1c0, subsystem 9,
/// image base 0x10000, no base relocations for the fixed EXE, imports
/// from COREDLL.dll resolved via ARM import thunks.

// REQUIRES: arm-registered-target

// RUN: llvm-mc -filetype=obj -triple=arm-pc-wince %s -o %t.obj

// RUN: lld-link /out:%t.exe /subsystem:windowsce /entry:WinMainCRTStartup \
// RUN:   /base:0x10000 /fixed %t.obj
// RUN: llvm-readobj --headers --coff-imports %t.exe | FileCheck %s

// CHECK: Machine: IMAGE_FILE_MACHINE_ARM (0x1C0)
// CHECK: SubSystem: IMAGE_SUBSYSTEM_WINDOWS_CE_GUI (0x9)
// CHECK: ImageBase: 0x10000
// CHECK-NOT: BaseRelocationTable

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
