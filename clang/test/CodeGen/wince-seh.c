// RUN: %clang_cc1 %s -triple arm-pc-wince -fms-extensions -emit-llvm -O1 -disable-llvm-passes -o - \
// RUN:     | FileCheck %s
// RUN: %clang_cc1 %s -triple arm-pc-wince -fms-extensions -fsyntax-only -o - 2>&1 \
// RUN:     | FileCheck %s --check-prefix=SYNTAX
//
// Windows CE on ARM uses the compressed .pdata SEH mechanism with
// __C_specific_handler, like desktop Windows on ARM64: __try/__except/
// __finally are accepted, the parent function gets the SEH personality, and
// outlined filters/finallys recover parent locals through the
// llvm.eh.recoverfp / llvm.localrecover / llvm.localaddress intrinsics
// whose ARM backend lowering is implemented in llvm/lib/Target/ARM (see
// utils/wince/WINEH-ABI-FACTS.md 4f).  The backend stage of this test is
// exercised by llvm/test/CodeGen/ARM/wince-seh-parent-frame.ll (added with
// the backend implementation; both files were authored source-level and are
// executed at the first real build).

void might_crash(void);
int g;

int catch_all(void) {
  int r = 0;
  __try {
    might_crash();
  } __except (1) {
    r = -1;
  }
  return r;
}

// CHECK-LABEL: define dso_local arm_aapcscc i32 @catch_all()
// CHECK-SAME: personality ptr @__C_specific_handler
// CHECK: invoke {{.*}}void @might_crash()
// CHECK: catchpad within {{.*}} [ptr null]
// CHECK: catchret

int filter_touches_parent(void) {
  int local = 7;
  __try {
    might_crash();
  } __except (g == local) {
    local = 0;
  }
  return local;
}

// A non-constant filter is outlined into an internal helper that receives
// (exception_pointers, frame_pointer); touching the parent's local emits the
// parent-frame intrinsics.
// CHECK-LABEL: define internal arm_aapcscc i32 @__filt_filter_touches_parent
// CHECK: call ptr @llvm.eh.recoverfp
// CHECK: call ptr @llvm.localrecover

int filter_exception_code(void) {
  __try {
    might_crash();
  } __except (_exception_code() == 5) {
    return 5;
  }
  return 0;
}

// _exception_code() also goes through the parent frame: the exception-code
// slot lives in the parent, so the filter recovers the parent FP and
// localrecover the slot, then reads ExceptionRecord->ExceptionCode from the
// EXCEPTION_POINTERS first argument.
// CHECK-LABEL: define internal arm_aapcscc i32 @__filt_filter_exception_code
// CHECK: call ptr @llvm.eh.recoverfp
// CHECK: call ptr @llvm.localrecover
// CHECK: getelementptr
// CHECK: load i32, ptr %{{.*}}
// CHECK: icmp eq i32 %{{.*}}, 5

void finally_basic(void) {
  __try {
    might_crash();
  } __finally {
    might_crash();
  }
}

// The parent calls the outlined finally with (abnormal_termination, FP) on
// both paths; FP comes from llvm.localaddress on the normal path.
// CHECK-LABEL: define dso_local {{.*}}void @finally_basic()
// CHECK: call ptr @llvm.localaddress()
// CHECK: call void @__fin_finally_basic({{i8 noundef( zeroext)?}} 0, ptr noundef %{{.*}})
// CHECK: define internal void @__fin_finally_basic

// SYNTAX-NOT: error:
