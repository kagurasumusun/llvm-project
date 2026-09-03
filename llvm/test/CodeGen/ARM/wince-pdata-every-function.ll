; RUN: llc -mtriple=armv5te-pc-wince -o - %s | FileCheck %s --check-prefixes=COMMON,ARM
; RUN: llc -mtriple=armv5te-pc-wince -filetype=obj -o /dev/null %s
; RUN: llc -mtriple=thumbv5te-pc-wince -o - %s | FileCheck %s --check-prefixes=COMMON,T1
; RUN: llc -mtriple=thumbv5te-pc-wince -filetype=obj -o /dev/null %s
; RUN: llc -mtriple=thumbv7-pc-wince -o - %s | FileCheck %s --check-prefixes=COMMON,T2
; RUN: llc -mtriple=thumbv7-pc-wince -filetype=obj -o /dev/null %s
;
; Windows CE: EVERY function carries both unwind tables (see
; ARMWinCFI.h functionNeedsWinCFIFrame):
;  - a WinCFI frame (.seh_proc/.seh_endprologue/.seh_endproc) that the
;    ARMWinCOFFStreamer turns into a compressed CE .pdata entry.  The
;    kernel unwinds all frames through .pdata (eVC emits .pdata for every
;    function too); ExceptionFlag/PDATA_EH are only set for __try
;    functions.
;  - an ARM EHABI frame (.fnstart/.fnend) producing the .ARM.exidx entry
;    for the toolchain's own unwinder (libunwind / C++ exceptions).
; The -filetype=obj runs are the real gate: CEEmitUnwindInfo errors out if
; a function reaches .seh_endproc without .seh_endprologue, so a broken
; prologue marker fails the build instead of producing an unwinding hazard.
;
; GHC functions have no prologue at all and stay EHABI-only.

declare void @may_crash()
declare i32 @__gxx_personality_v0(...)
declare void @__C_specific_handler(...)

; A leaf function: empty prologue.  SEH_PrologEnd sits at the function
; start (PrologLen 0 -- the kernel's "no prolog" path).
define i32 @leaf(i32 %x) {
entry:
  ret i32 %x
}

; COMMON-LABEL: leaf:
; COMMON: .seh_proc leaf
; COMMON: .fnstart
; COMMON: .seh_endprologue
; COMMON: .fnend
; COMMON: .seh_endproc

; A function with a stack frame and a call.
define i32 @frame(i32 %a, i32 %b) {
entry:
  %buf = alloca [64 x i32], align 4
  call void @may_crash()
  %p = ptrtoint ptr %buf to i32
  ret i32 %p
}

; COMMON-LABEL: frame:
; COMMON: .seh_proc frame
; COMMON: .fnstart
; COMMON: .seh_endprologue
; COMMON: .fnend
; COMMON: .seh_endproc

; A C++ function: EHABI personality + LSDA on top of the WinCFI frame.
; The WinCFI frame never suppresses the EHABI one.
define i32 @cpp_func(i32 %x) personality ptr @__gxx_personality_v0 {
entry:
  invoke void @may_crash() to label %cont unwind label %lpad

cont:
  ret i32 %x

lpad:
  %lp = landingpad { ptr, i32 }
          cleanup
  resume { ptr, i32 } %lp
}

; COMMON-LABEL: cpp_func:
; COMMON: .seh_proc cpp_func
; COMMON: .fnstart
; COMMON: .personality __gxx_personality_v0
; COMMON: .handlerdata
; COMMON: .fnend
; COMMON: .seh_endproc

; An SEH (__try-style) parent: .seh_handler + PDATA_EH pair for the
; kernel, and an EHABI entry WITHOUT a personality (the SEH personality is
; not an EHABI one) so C++ exceptions can still unwind through this frame.
define internal i32 @"?filt$0@0@seh_func@@"(ptr %exception_pointers, ptr %frame_pointer) {
entry:
  ret i32 1
}

define dso_local i32 @seh_func() personality ptr @__C_specific_handler {
entry:
  %a = alloca i32, align 4
  store i32 42, ptr %a
  invoke void @may_crash() to label %cont unwind label %lpad

cont:
  %v = load i32, ptr %a
  ret i32 %v

lpad:
  %cs = catchswitch within none [label %catch] unwind to caller

catch:
  %p = catchpad within %cs [ptr @"?filt$0@0@seh_func@@"]
  catchret from %p to label %cont
}

; COMMON-LABEL: seh_func:
; COMMON: .seh_proc seh_func
; COMMON: .seh_handler __C_specific_handler, %except
; COMMON: .fnstart
; COMMON-NOT: .personality
; COMMON-NOT: .handlerdata
; COMMON: .fnend
; COMMON: .seh_endproc

; GHC calling convention: no prologue, no WinCFI frame, EHABI only.
define ghccc i32 @ghc_func(i32 %x) {
entry:
  ret i32 %x
}

; COMMON-LABEL: ghc_func:
; COMMON-NOT: .seh_proc
; COMMON: .fnstart
; COMMON: .fnend
; COMMON-NOT: .seh_endproc
