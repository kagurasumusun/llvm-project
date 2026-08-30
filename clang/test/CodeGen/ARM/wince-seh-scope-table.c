// RUN: %clang --target=arm-pc-wince -Wno-wince-sysroot-missing -fms-extensions -S -o - %s | FileCheck %s

// Windows CE SEH scope-table + PDATA_EH emission (see
// utils/wince/WINEH-ABI-FACTS.md 4f/4g): the parent function's label is
// preceded by the scope table (count word + 16-byte absolute-address
// entries {BeginVA, EndVA, FilterOrFinally, Handler/Jump}) and the 8-byte
// PDATA_EH pair {__C_specific_handler, handler-data}, emitted by
// ARMAsmPrinter::emitCEHandlerData.  The CE kernel reads the pair from the
// 8 bytes before pFuncStart (exdsptch.c RtlLookupFunctionEntry) and only
// when the .pdata ExceptionFlag is set.
//
// Written source-level; exact encodings are re-verified at the first real
// build.

extern void might_crash(void);

// A non-constant filter is outlined into a normal (non-SEH) function; the
// scope table references it as the FilterFunction entry.
int except_func(int x) {
  int r = 0;
  __try {
    might_crash();
  } __except (x) {
    r = -1;
  }
  return r;
}

// Parent: scope table with the outlined filter as the FilterFunction entry,
// then the PDATA_EH pair immediately before the function label.
// CHECK: .Lexcept_func{{.*}}parent_frame_offset = {{[0-9]+}}
// CHECK: [[HD:.Lce_handlerdata_[0-9]+]]:
// CHECK-NEXT: .long ([[LE:.Llsda_end_[0-9]+]]-[[LB:.Llsda_begin_[0-9]+]])/16
// CHECK-NEXT: [[LB]]:
// CHECK-NEXT: .long {{.*}}
// CHECK-NEXT: .long {{.*}}
// CHECK-NEXT: .long {{.*}}__filt_except_func
// CHECK-NEXT: .long {{.*}}
// CHECK-NEXT: [[LE]]:
// CHECK-NEXT: .long __C_specific_handler
// CHECK-NEXT: .long [[HD]]
// CHECK-LABEL: except_func:
// CHECK: .seh_proc except_func
// CHECK: .seh_handler __C_specific_handler, %except
// CHECK: .seh_endproc

// A constant 1 filter is a catch-all: the FilterOrFinally slot holds the
// constant 1 (no filter function).
int catchall_func(void) {
  __try {
    might_crash();
  } __except (1) {
    return -1;
  }
  return 0;
}

// CHECK: .Lcatchall_func{{.*}}parent_frame_offset = {{[0-9]+}}
// CHECK: [[HD2:.Lce_handlerdata_[0-9]+]]:
// CHECK-NEXT: .long ([[LE2:.Llsda_end_[0-9]+]]-[[LB2:.Llsda_begin_[0-9]+]])/16
// CHECK-NEXT: [[LB2]]:
// CHECK-NEXT: .long {{.*}}
// CHECK-NEXT: .long {{.*}}
// CHECK-NEXT: .long 1
// CHECK-NEXT: .long {{.*}}
// CHECK-NEXT: [[LE2]]:
// CHECK-NEXT: .long __C_specific_handler
// CHECK-NEXT: .long [[HD2]]
// CHECK-LABEL: catchall_func:
// CHECK: .seh_proc catchall_func
// CHECK: .seh_handler __C_specific_handler, %except
// CHECK: .seh_endproc

// A finally entry has the funclet as FilterOrFinally and a null
// Handler/Jump slot (0): the runtime calls the finally function, it does
// not transfer control to an in-parent handler block.
int finally_func(void) {
  __try {
    might_crash();
  } __finally {
    ;
  }
  return 0;
}

// CHECK: .Lfinally_func{{.*}}parent_frame_offset = {{[0-9]+}}
// CHECK: [[HD3:.Lce_handlerdata_[0-9]+]]:
// CHECK-NEXT: .long ([[LE3:.Llsda_end_[0-9]+]]-[[LB3:.Llsda_begin_[0-9]+]])/16
// CHECK-NEXT: [[LB3]]:
// CHECK-NEXT: .long {{.*}}
// CHECK-NEXT: .long {{.*}}
// CHECK-NEXT: .long {{.*}}
// CHECK-NEXT: .long 0
// CHECK-NEXT: [[LE3]]:
// CHECK-NEXT: .long __C_specific_handler
// CHECK-NEXT: .long [[HD3]]
// CHECK-LABEL: finally_func:
// CHECK: .seh_proc finally_func
// CHECK: .seh_handler __C_specific_handler, %except
// CHECK: .seh_endproc
