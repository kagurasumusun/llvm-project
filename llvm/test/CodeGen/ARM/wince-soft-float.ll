; RUN: llc -mtriple=arm-pc-wince -verify-machineinstrs < %s | FileCheck %s

; Windows CE is the armel ABI: soft-float AAPCS on the ARMv5TE baseline
; (arm926ej-s / i.MX28).  The default CPU has no VFP, so floating-point
; arithmetic must lower to the __aeabi_* runtime helpers (compiler-rt
; builtins provide them; the EABI helper set is selected for the wince
; triple in compiler-rt/lib/builtins/CMakeLists.txt) and 64-bit integer
; arithmetic to the __aeabi_l* helpers - never to VFP instructions.
; See utils/wince/README.md "armel equivalence (source-verified)".
;
; Written source-level; the exact encodings are re-verified at the first
; real build.

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
; ARM926EJ-S has UMULL/UMLAL, so a 64-bit multiply expands in ISel
; rather than calling __aeabi_lmul.
; CHECK: umull
  %mul = mul i64 %a, %b
  ret i64 %mul
}
