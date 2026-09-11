; The CPU is named because the OS no longer implies one, and it is a CPU
; without a floating-point unit: the soft-float ABI is CE's rule, but the
; absence of an FPU is the CPU's.
; RUN: llc -mtriple=arm-pc-wince -mcpu=arm926ej-s -verify-machineinstrs < %s | FileCheck %s


define float @fadd(float %a, float %b) {
; CHECK-LABEL: fadd:
; CHECK: bl __aeabi_fadd
  %add = fadd float %a, %b
  ret float %add
}

define double @dadd(double %a, double %b) {
; CHECK-LABEL: dadd:
; CHECK: bl __aeabi_dadd
  %add = fadd double %a, %b
  ret double %add
}

define double @ddiv(double %a, double %b) {
; CHECK-LABEL: ddiv:
; CHECK: bl __aeabi_ddiv
  %div = fdiv double %a, %b
  ret double %div
}

; Integer division goes to the AEABI helpers rather than to the desktop Windows
; RTABI ones, and it is that name which decides the argument order: the swap
; ARMTargetLowering::getDivRemArgList applies exists because __rt_sdiv and its
; kind take the two the other way round, so a CE call must not be given it.
; Pinning the helper family is what holds the rule up, since no call resolves to
; both.  A lone sdiv is not pinned by name here, as which AEABI override it gets
; depends on -target-abi, and llc is not run with one.
define i32 @idiv(i32 %a, i32 %b) {
; CHECK-LABEL: idiv:
; CHECK-NOT: __rt_
; CHECK: bl
  %d = sdiv i32 %a, %b
  ret i32 %d
}

define { i32, i32 } @idivmod(i32 %a, i32 %b) {
; CHECK-LABEL: idivmod:
; CHECK-NOT: __rt_
; CHECK: bl __aeabi_id
  %d = sdiv i32 %a, %b
  %r = srem i32 %a, %b
  %ins = insertvalue { i32, i32 } poison, i32 %d, 0
  %ins1 = insertvalue { i32, i32 } %ins, i32 %r, 1
  ret { i32, i32 } %ins1
}

define i64 @lmul(i64 %a, i64 %b) {
; CHECK-LABEL: lmul:
; CHECK: umull
  %mul = mul i64 %a, %b
  ret i64 %mul
}
