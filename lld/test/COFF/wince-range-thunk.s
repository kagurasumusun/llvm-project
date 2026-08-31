// LLD COFF: Windows CE ARM branch range-extension thunk.
//
// A32 B/BL reach +/-32 MB (24-bit word-aligned offset from PC+8). A large
// uninitialized .bss gap between .text and .text2 forces the branch out of
// range; the linker inserts an ARM-mode range-extension thunk after the
// caller:
//   ldr ip, [pc]
//   bx  ip
//   .word <target VA, bit 0 = Thumb>
// The thunk is a single load of the target address plus bx (interworking).
// Do not use the import-thunk pair ldr pc,[ip] -- that double-loads.
// Thumb BL callers use wince-range-thunk-thumb.s (Thumb-1 veneer).

// REQUIRES: arm-registered-target

// RUN: llvm-mc -filetype=obj -triple=arm-pc-wince %s -o %t.obj
// RUN: lld-link /out:%t.exe /subsystem:windowsce /base:0x10000 %t.obj
// RUN: llvm-objdump -d --section=.text %t.exe | FileCheck %s

	.syntax unified
	.arm
	.text
	.globl main
main:
	bl	callee
	bx	lr

// 0x2200000 (> 32 MB) of BSS. Builtin section order is .text, .bss, ...
// then newly created .text2, so the gap sits between caller and callee
// in VA without bloating the object file.
	.bss
	.space	0x2200000

	.section	.text2,"xr"
	.globl	callee
callee:
	bx	lr

// The BL is redirected to the range-extension thunk appended to .text
// (right after main). llvm-objdump prints ip as r12.
// CHECK: bl
// CHECK: bx{{.*}}lr
// CHECK: ldr{{.*}}r12, [pc
// CHECK: bx{{.*}}r12
