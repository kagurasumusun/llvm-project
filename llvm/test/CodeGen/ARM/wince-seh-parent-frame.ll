; RUN: llc -mtriple=arm-pc-wince -mcpu=arm926ej-s -verify-machineinstrs -o - %s \
; RUN:     | FileCheck %s --check-prefix=ARM5
; RUN: llc -mtriple=thumbv7-pc-wince -verify-machineinstrs -o - %s \
; RUN:     | FileCheck %s --check-prefix=T2
; The same image in ARM state, on a core that does have the MOVW and MOVT
; encodings.  A symbol is still taken from the constant pool there, which is
; what keeps the MOVW/MOVT guard of the constant island pass free of an ARM case
; rather than missing one.
; RUN: llc -mtriple=armv7-pc-wince -verify-machineinstrs -o - %s \
; RUN:     | FileCheck %s --check-prefix=A7

declare ptr @llvm.localaddress()
declare ptr @llvm.localrecover(ptr, ptr, i32)
declare void @llvm.localescape(...)
declare ptr @llvm.eh.recoverfp(ptr, ptr)
declare void @may_crash()
declare void @use_fp(ptr)
declare void @__C_specific_handler(...)

define internal i32 @"?filt$0@0@alloc_func@@"(ptr %exception_pointers, ptr %frame_pointer) {
entry:
  %fp = call ptr @llvm.eh.recoverfp(ptr @alloc_func, ptr %frame_pointer)
  %a = call ptr @llvm.localrecover(ptr @alloc_func, ptr %fp, i32 0)
  %v = load i32, ptr %a, align 4
  ret i32 %v
}

; ARM5-LABEL: {{.*}}filt{{\$}}0@0@alloc_func@@
; ARM5-NOT: bl
; ARM5: ldr r[[O:[0-9]+]], .LCPI{{[0-9]+}}_{{[0-9]+}}
; ARM5-NEXT: ldr r{{[0-9]+}}, [r1, r[[O]]]
; ARM5: .long .Lalloc_func$frame_escape_0

; T2-LABEL: {{.*}}filt{{\$}}0@0@alloc_func@@
; T2-NOT: bl
; T2: movw r{{[0-9]+}}, :lower16:.Lalloc_func$frame_escape_0
; T2: movt r{{[0-9]+}}, :upper16:.Lalloc_func$frame_escape_0
; T2: ldr r{{[0-9]+}}, [r1, r{{[0-9]+}}]

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

; ARM5: .Lalloc_func$parent_frame_offset = {{[0-9]+}}
; ARM5-LABEL: alloc_func:
; ARM5: .seh_proc alloc_func
; ARM5: .seh_handler __C_specific_handler, %except
; ARM5: .fnstart
; ARM5: .Lalloc_func$frame_escape_0 = {{-?[0-9]+}}

; T2: .Lalloc_func$parent_frame_offset = {{[0-9]+}}
; T2-LABEL: alloc_func:
; T2: .seh_proc alloc_func
; T2: .seh_handler __C_specific_handler, %except
; T2: .fnstart
; T2: .Lalloc_func$frame_escape_0 = {{-?[0-9]+}}
; T2: movw r{{[0-9]+}}, :lower16:.Lalloc_func$parent_frame_offset
; T2: movt r{{[0-9]+}}, :upper16:.Lalloc_func$parent_frame_offset

; A7-LABEL: {{.*}}filt{{\$}}0@0@alloc_func@@
; A7: .code 32
; A7-NOT: :lower16:
; A7: .Lalloc_func$parent_frame_offset = {{[0-9]+}}
; A7-LABEL: alloc_func:
; A7: .seh_proc alloc_func
; A7-NOT: :lower16:
; A7: .Lalloc_func$frame_escape_0 = {{-?[0-9]+}}
