// RUN: %clang --target=arm-pc-wince -Wno-wince-sysroot-missing -fms-extensions -x c++ -fexceptions -fcxx-exceptions -S -o - %s | FileCheck %s
// RUN: %clang --target=arm-pc-wince -Wno-wince-sysroot-missing -mthumb -fms-extensions -x c++ -fexceptions -fcxx-exceptions -S -o - %s | FileCheck %s
// RUN: %clang --target=arm-pc-wince -Wno-wince-sysroot-missing -fms-extensions -x c++ -fexceptions -fcxx-exceptions -c -o /dev/null %s
// RUN: %clang --target=arm-pc-wince -Wno-wince-sysroot-missing -mthumb -fms-extensions -x c++ -fexceptions -fcxx-exceptions -c -o /dev/null %s


extern "C" void might_crash(void);

extern "C" int seh_func(int x) {
  int r = 0;
  __try {
    might_crash();
  } __except (x) {
    r = -1;
  }
  return r;
}

// CHECK-LABEL: seh_func:
// CHECK-NOT: .fnstart
// CHECK: .seh_proc seh_func
// CHECK: .seh_handler __C_specific_handler, %except
// CHECK: .fnstart
// CHECK-NOT: .personality
// CHECK-NOT: .handlerdata
// CHECK: .fnend
// CHECK: .seh_endproc

// CHECK-LABEL: __filt_seh_func:
// CHECK: .seh_proc __filt_seh_func
// CHECK: .fnstart
// CHECK: .fnend
// CHECK: .seh_endproc

int cpp_func(int x) {
  try {
    might_crash();
  } catch (...) {
    return -1;
  }
  return x;
}

// CHECK-LABEL: _Z8cpp_funci:
// CHECK: .seh_proc _Z8cpp_funci
// CHECK: .fnstart
// CHECK: .personality __gxx_personality_v0
// CHECK: .handlerdata
// CHECK: .fnend
// CHECK: .seh_endproc
