// RUN: %clang --target=i386-pc-wince -Wno-wince-sysroot-missing -fms-extensions -O1 -Xclang -disable-llvm-passes -S -emit-llvm -o - %s \
// RUN:     | FileCheck %s
// REQUIRES: x86-registered-target

void might_crash(void);

int catch_all(void) {
  int r = 0;
  __try {
    might_crash();
  } __except (1) {
    r = -1;
  }
  return r;
}

// CHECK-LABEL: define{{.*}}@catch_all()
// CHECK-SAME: personality ptr @_except_handler3
// CHECK: invoke{{.*}}@might_crash
