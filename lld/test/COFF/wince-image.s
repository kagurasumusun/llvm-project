/// LLD COFF: Windows CE image layout.
/// The produced PE must match the WinCE loader requirements as implemented
/// by the CeGCC/binutils arm-wince emulation: machine 0x1c0, subsystem 9,
/// image base 0x10000, no base relocations for the fixed EXE, imports
/// from COREDLL.dll resolved via ARM import thunks.

// REQUIRES: arm-registered-target

// RUN: llvm-mc -filetype=obj -triple=arm-pc-wince %s -o %t.obj

// RUN: lld-link /out:%t.exe /subsystem:windowsce /entry:WinMainCRTStartup \
// RUN:   /base:0x10000 /fixed %t.obj
// RUN: llvm-readobj --headers --coff-imports --symbols %t.exe | FileCheck %s

// CHECK: Machine: IMAGE_FILE_MACHINE_ARM (0x1C0)
// CHECK: ImageBase: 0x10000
// CHECK: Subsystem: IMAGE_SUBSYSTEM_WINDOWS_CE_GUI (0x9)
// CHECK-NOT: BaseRelocationTable
// CHECK: Name: __text_start__
// CHECK: Name: __text_end__

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

// Force the CE text-bounds symbols to be referenced like mingwrt
// pseudo-reloc.o does. lld must define them before reporting undefines.
	.globl	ref_text_bounds
ref_text_bounds:
	.long	__text_start__
	.long	__text_end__
