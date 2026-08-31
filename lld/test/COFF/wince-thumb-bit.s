// LLD COFF: Windows CE entry point and export Thumb bit.
//
// Windows CE ARM images interwork between ARM and Thumb code: the producer
// marks Thumb function symbols with bit 0 set in the COFF symbol value (see
// WinCOFFObjectWriter), and lld keeps that bit in the final address
// (DefinedCOFF::getRVA = section RVA + symbol value). Unlike ARMNT - where
// lld unconditionally ORs bit 0 into the PE entry point because ARMNT
// images are uniformly Thumb - a Windows CE image must NOT force the bit:
// an ARM entry/export must stay even, while a Thumb one must stay odd.

// REQUIRES: arm-registered-target

// RUN: llvm-mc -filetype=obj -triple=arm-pc-wince %s -o %t.obj
// RUN: lld-link /out:%t.exe /subsystem:windowsce /base:0x10000 \
// RUN:   /entry:thumb_entry /export:thumb_entry /export:arm_entry %t.obj
// RUN: llvm-readobj --headers --coff-exports %t.exe | FileCheck %s

// CHECK: Machine: IMAGE_FILE_MACHINE_ARM (0x1C0)
// CHECK: AddressOfEntryPoint: 0x1001
// CHECK: Name: thumb_entry
// CHECK: Ordinal: 1
// CHECK: RVA: 0x1001
// CHECK: Name: arm_entry
// CHECK: Ordinal: 2
// CHECK: RVA: 0x1008

	.text
	.globl thumb_entry
.thumb_func
thumb_entry:
	movs	r0, #0
	bx	lr

	.globl arm_entry
.arm
arm_entry:
	mov	r0, #0
	bx	lr
