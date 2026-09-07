; RUN: llc -mtriple=thumbv7-pc-windows-gnu -verify-machineinstrs %s -o - | FileCheck %s --check-prefix=WIN
; RUN: llc -mtriple=thumbv7-pc-windows-gnu -verify-machineinstrs -filetype=obj %s -o %t.gnu.obj
; RUN: llc -mtriple=thumbv7-pc-windows-msvc -verify-machineinstrs %s -o - | FileCheck %s --check-prefix=WIN
; RUN: llc -mtriple=thumbv7-pc-windows-msvc -verify-machineinstrs -filetype=obj %s -o %t.msvc.obj
; RUN: llc -mtriple=armv5te-pc-wince -verify-machineinstrs %s -o - | FileCheck %s --check-prefix=CE
; RUN: llc -mtriple=armv5te-pc-wince -verify-machineinstrs -filetype=obj %s -o %t.ce-arm.obj
; RUN: llc -mtriple=thumbv5te-pc-wince -verify-machineinstrs %s -o - | FileCheck %s --check-prefix=CE
; RUN: llc -mtriple=thumbv5te-pc-wince -verify-machineinstrs -filetype=obj %s -o %t.ce-thumb.obj
; RUN: llc -mtriple=thumbv7-pc-wince -verify-machineinstrs %s -o - | FileCheck %s --check-prefix=CE
; RUN: llc -mtriple=thumbv7-pc-wince -verify-machineinstrs -filetype=obj %s -o %t.ce-thumb2.obj

; WIN-LABEL: no_unwind:
; WIN-NOT: .seh_
; WIN-LABEL: explicit_unwind:
; WIN: .seh_proc explicit_unwind
; WIN: .seh_endprologue
; WIN: .seh_endproc
; WIN-LABEL: may_unwind:
; WIN: .seh_proc may_unwind
; WIN: .seh_endprologue
; WIN: .seh_endproc

; CE-LABEL: no_unwind:
; CE: .seh_proc no_unwind
; CE: .fnstart
; CE: .seh_endprologue
; CE: .fnend
; CE: .seh_endproc
; CE-LABEL: explicit_unwind:
; CE: .seh_proc explicit_unwind
; CE: .seh_endproc
; CE-LABEL: may_unwind:
; CE: .seh_proc may_unwind
; CE: .seh_endproc

define i32 @no_unwind(i32 %x) nounwind optsize "frame-pointer"="none" {
entry:
  %slot = alloca i32, align 4
  store volatile i32 %x, ptr %slot, align 4
  %value = load volatile i32, ptr %slot, align 4
  %result = add i32 %value, 1
  ret i32 %result
}

define i32 @explicit_unwind(i32 %x) nounwind optsize uwtable "frame-pointer"="none" {
entry:
  %slot = alloca i32, align 4
  store volatile i32 %x, ptr %slot, align 4
  %value = load volatile i32, ptr %slot, align 4
  %result = add i32 %value, 1
  ret i32 %result
}

define i32 @may_unwind(i32 %x) optsize "frame-pointer"="none" {
entry:
  %value = call i32 @callee(i32 %x)
  %result = add i32 %value, 1
  ret i32 %result
}

declare i32 @callee(i32)
