; RUN: llc -mtriple=arm-pc-wince -verify-machineinstrs < %s | FileCheck %s


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

define i64 @lmul(i64 %a, i64 %b) {
; CHECK-LABEL: lmul:
; CHECK: umull
  %mul = mul i64 %a, %b
  ret i64 %mul
}
