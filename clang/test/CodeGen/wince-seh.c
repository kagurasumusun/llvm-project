// RUN: %clang --target=arm-pc-wince -Wno-wince-sysroot-missing -fms-extensions -O1 -Xclang -disable-llvm-passes -S -emit-llvm -o - %s \
// RUN:     | FileCheck %s
// RUN: %clang --target=arm-pc-wince -Wno-wince-sysroot-missing -fms-extensions -fsyntax-only %s

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

// CHECK-LABEL: define internal arm_aapcscc i32 @__filt_filter_exception_code
// CHECK: getelementptr
// CHECK: load i32, ptr %{{.*}}
// CHECK: icmp eq i32 {{.*}}, 5

void finally_basic(void) {
  __try {
    might_crash();
  } __finally {
    might_crash();
  }
}

// CHECK-LABEL: define dso_local {{.*}}void @finally_basic()
// CHECK: call ptr @llvm.localaddress()
// CHECK: call arm_aapcscc void @__fin_finally_basic({{i8 noundef( zeroext)?}} 0, ptr noundef %{{.*}})
// CHECK: define internal arm_aapcscc void @__fin_finally_basic
