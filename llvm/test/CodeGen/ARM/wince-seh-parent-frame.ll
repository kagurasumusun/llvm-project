; RUN: llc -mtriple=arm-pc-wince -verify-machineinstrs -o - %s \
; RUN:     | FileCheck %s --check-prefix=ARM5
; RUN: llc -mtriple=thumbv7-pc-wince -verify-machineinstrs -o - %s \
; RUN:     | FileCheck %s --check-prefix=T2
;
; Windows CE SEH parent-frame access (see utils/wince/WINEH-ABI-FACTS.md 4f):
; outlined filters/finallys recover parent locals with llvm.eh.recoverfp
; (identity on ARM: the CE kernel hands the helpers the parent's entry SP as
; the establisher frame) plus llvm.localrecover, which materializes the
; frame-escape symbols the AsmPrinter assigns; llvm.localaddress returns the
; parent's entry SP (SP + $parent_frame_offset, where that symbol is assigned
; the frame size in emitCESpecificHandlerTable).  Written source-level; the
; exact encodings are re-verified at the first real build.

declare ptr @llvm.localaddress()
declare ptr @llvm.localrecover(ptr, ptr, i32)
declare void @llvm.localescape(...)
declare ptr @llvm.eh.recoverfp(ptr, ptr)
declare void @may_crash()
declare void @use_fp(ptr)
; The SEH personality runtime symbol used as the function personality.
declare void @__C_specific_handler(...)

; Outlined SEH filter: (exception_pointers, frame_pointer).  recoverfp is a
; no-op here, so the incoming frame_pointer (r1) is used directly as the
; base for the frame-escape offset.  No call is emitted for it.
define internal i32 @"?filt$0@0@alloc_func@@"(ptr %exception_pointers, ptr %frame_pointer) {
entry:
  %fp = call ptr @llvm.eh.recoverfp(ptr @alloc_func, ptr %frame_pointer)
  %a = call ptr @llvm.localrecover(ptr @alloc_func, ptr %fp, i32 0)
  %v = load i32, ptr %a, align 4
  ret i32 %v
}

; ARMv4T/ARMv5 (WinCE default) has no movw/movt: the absolute frame-escape
; symbol value is loaded from a literal pool entry.
; ARM5-LABEL: "??filt$0@0@alloc_func@@":
; ARM5-NOT: bl
; ARM5: ldr r[[O:[0-9]+]], .LCPI{{[0-9]+}}_{{[0-9]+}}
; ARM5-NEXT: add r[[A:[0-9]+]], r1, r[[O]]
; ARM5-NEXT: ldr r0, [r[[A]]]
; ARM5: .long .Lalloc_func$frame_escape_0

; Thumb-2 uses movw/movt for the same symbol.
; T2-LABEL: "??filt$0@0@alloc_func@@":
; T2-NOT: bl
; T2: movw r{{[0-9]+}}, :lower16:.Lalloc_func$frame_escape_0
; T2: movt r{{[0-9]+}}, :upper16:.Lalloc_func$frame_escape_0
; T2: add r{{[0-9]+}}, r1, r{{[0-9]+}}

define dso_local i32 @alloc_func() personality ptr @__C_specific_handler {
entry:
  %a = alloca i32, align 4
  call void (...) @llvm.localescape(ptr %a)
  store i32 42, ptr %a, align 4
  invoke void @may_crash()
          to label %cont unwind label %lpad

cont:
  %lp = call ptr @llvm.localaddress()
  call void @use_fp(ptr %lp)
  ret i32 0

lpad:
  %cs = catchswitch within none [label %catch] unwind to caller

catch:
  %p = catchpad within %cs [ptr @"?filt$0@0@alloc_func@@"]
  catchret from %p to label %cont
}

; The parent has an EH funclet, so the AsmPrinter assigns both the escaped
; local offset and the frame size ($parent_frame_offset) used by
; llvm.localaddress.  The frame-size assignment is emitted just before the
; function label (inside the CE handler-data emission); the literal pool
; holds the symbol for ARMv5.
; ARM5: .set .Lalloc_func$parent_frame_offset, {{[0-9]+}}
; ARM5-LABEL: alloc_func:
; ARM5: .set .Lalloc_func$frame_escape_0, {{-?[0-9]+}}
; ARM5: .long .Lalloc_func$parent_frame_offset

; T2: .set .Lalloc_func$parent_frame_offset, {{[0-9]+}}
; T2-LABEL: alloc_func:
; T2: .set .Lalloc_func$frame_escape_0, {{-?[0-9]+}}
; T2: movw r{{[0-9]+}}, :lower16:.Lalloc_func$parent_frame_offset
; T2: movt r{{[0-9]+}}, :upper16:.Lalloc_func$parent_frame_offset
