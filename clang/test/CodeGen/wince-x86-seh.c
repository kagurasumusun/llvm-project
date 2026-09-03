// RUN: %clang --target=i386-pc-wince -Wno-wince-sysroot-missing -fms-extensions -O1 -Xclang -disable-llvm-passes -S -emit-llvm -o - %s \
// RUN:     | FileCheck %s
// REQUIRES: x86-registered-target
//
// Windows CE x86 native SEH.
//
// __try / __except / __finally on x86-pc-wince get the x86 MSVC SEH
// personality _except_handler3 (coredll exports it together with
// _local_unwind2 / _local_unwind4 and __abnormal_termination), selected by
// getSEHPersonalityMSVC()'s x86 branch.  This is the platform-native SEH path
// -- the same fs:[0] exception-registration-chain mechanism desktop Win32 x86
// uses -- so no SJLJ/DWARF unwinder is involved.  (ARM WinCE uses the
// compressed .pdata SEH with __C_specific_handler instead; see
// clang/test/CodeGen/wince-seh.c.)  See utils/wince/WINEH-ABI-FACTS.md 4o.

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
