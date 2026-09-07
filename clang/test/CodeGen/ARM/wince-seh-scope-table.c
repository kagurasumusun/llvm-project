// RUN: %clang --target=arm-pc-wince -Wno-wince-sysroot-missing -fms-extensions -S -o - %s | FileCheck %s


extern void might_crash(void);

int except_func(int x) {
  int r = 0;
  __try {
    might_crash();
  } __except (x) {
    r = -1;
  }
  return r;
}

// CHECK: .Lexcept_func{{.*}}parent_frame_offset = {{[0-9]+}}
// CHECK: [[HD:.Lce_handlerdata[0-9]+]]:
// CHECK-NEXT: .long ([[LE:.Llsda_end[0-9]+]]-[[LB:.Llsda_begin[0-9]+]])/16
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

int catchall_func(void) {
  __try {
    might_crash();
  } __except (1) {
    return -1;
  }
  return 0;
}

// CHECK: .Lcatchall_func{{.*}}parent_frame_offset = {{[0-9]+}}
// CHECK: [[HD2:.Lce_handlerdata[0-9]+]]:
// CHECK-NEXT: .long ([[LE2:.Llsda_end[0-9]+]]-[[LB2:.Llsda_begin[0-9]+]])/16
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

int finally_func(void) {
  __try {
    might_crash();
  } __finally {
    ;
  }
  return 0;
}

// CHECK: .Lfinally_func{{.*}}parent_frame_offset = {{[0-9]+}}
// CHECK: [[HD3:.Lce_handlerdata[0-9]+]]:
// CHECK-NEXT: .long ([[LE3:.Llsda_end[0-9]+]]-[[LB3:.Llsda_begin[0-9]+]])/16
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
