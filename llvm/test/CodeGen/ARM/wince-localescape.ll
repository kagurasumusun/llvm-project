; RUN: llc -mtriple=armv5te-unknown-wince -verify-machineinstrs -filetype=obj %s -o %t.arm.obj
; RUN: llc -mtriple=thumbv5te-unknown-wince -verify-machineinstrs -filetype=obj %s -o %t.thumb.obj
; RUN: llc -mtriple=thumbv7-unknown-wince -verify-machineinstrs -filetype=obj %s -o %t.thumb2.obj
; RUN: llc -mtriple=armv5te-unknown-wince -verify-machineinstrs %s -o - | FileCheck %s
; RUN: llc -mtriple=thumbv5te-unknown-wince -verify-machineinstrs %s -o - | FileCheck %s
; RUN: llc -mtriple=thumbv7-unknown-wince -verify-machineinstrs %s -o - | FileCheck %s

; CHECK: .Lescape_local$frame_escape_0 = -{{[0-9]+}}

define void @escape_local() nounwind {
  %slot = alloca i32, align 4
  call void (...) @llvm.localescape(ptr %slot)
  ret void
}

declare void @llvm.localescape(...)
