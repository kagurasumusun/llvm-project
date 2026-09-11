; A Windows CE image is COFF, and the COFF unwind model pins the frame pointer
; to a fixed register, so longjmp has to restore that register.  How it can do
; so depends on the core, not on the OS: reaching R11 and SP with a load needs
; the 32-bit encodings, which only exist from ARMv6T2 on, and Windows CE also
; covers cores that predate them.  There the same register has to be restored by
; way of the 16-bit encodings instead.
; RUN: llc -mtriple=thumbv5te-pc-wince -verify-machineinstrs -o - %s \
; RUN:     | FileCheck %s --check-prefix=THUMB1
; RUN: llc -mtriple=thumbv7-pc-wince -verify-machineinstrs -o - %s \
; RUN:     | FileCheck %s --check-prefix=THUMB2
; The desktop Windows on ARM releases start at ARMv7, so they keep taking the
; 32-bit sequence; that half is unchanged here.
; RUN: llc -mtriple=thumbv7-pc-windows-msvc -verify-machineinstrs -o - %s \
; RUN:     | FileCheck %s --check-prefix=THUMB2

define void @throw_out(ptr %buf) {
  call void @llvm.eh.sjlj.longjmp(ptr %buf)
  unreachable
}

declare void @llvm.eh.sjlj.longjmp(ptr)

; THUMB1-LABEL: throw_out:
; THUMB1: ldr r{{[0-9]+}}, [r{{[0-9]+}}, #8]
; THUMB1-NEXT: mov sp, r
; THUMB1-NEXT: ldr r{{[0-9]+}}, [r{{[0-9]+}}, #4]
; The pinned frame register, restored without a 32-bit encoding.
; THUMB1-NEXT: ldr r11, [r{{[0-9]+}}]
; THUMB1-NEXT: bx r{{[0-9]+}}
; THUMB1-NOT: ldr.w

; THUMB2-LABEL: throw_out:
; THUMB2: ldr.w r11, [{{\s*}}r{{[0-9]+}}]
; THUMB2-NEXT: ldr.w sp, [r{{[0-9]+}}, #8]
; THUMB2-NEXT: ldr.w pc, [r{{[0-9]+}}, #4]
