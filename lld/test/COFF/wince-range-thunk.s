// LLD COFF: Windows CE ARM branch range-extension thunk.
//
// A32 B/BL reach +/-32 MB (24-bit word-aligned offset from PC+8). When the
// target ends up farther away than that (a large data section between the
// two code sections), the linker must insert a CE range-extension thunk -
// the binutils jmp_arm_bytes pair (ldr ip,[pc] / ldr pc,[ip], interworking
// via bit 0 of the literal) - right after the caller's section, and
// redirect the branch to it.

// REQUIRES: arm-registered-target

// RUN: llvm-mc -filetype=obj -triple=arm-pc-wince %s -o %t.obj
// RUN: lld-link /out:%t.exe /subsystem:windowsce /base:0x10000 %t.obj
// RUN: llvm-objdump -d --section=.text %t.exe | FileCheck %s

	.text
	.globl main
main:
	bl	callee
	bx	lr

// 36 MB of padding forces the branch above out of A32 range.
	.section .gap, "progbits"
	.space 0x900000

	.section .text2, "progbits"
	.globl callee
callee:
	bx	lr

// The BL is redirected to the range-extension thunk appended to .text
// (right after main), which carries the jmp_arm_bytes pair and the
// target's literal address.
// CHECK: bl
// CHECK: bx.*lr
// CHECK: ldr.*ip, \[pc
// CHECK: ldr.*pc, \[ip\]
