// RUN: %clang --target=arm-pc-wince -Wno-wince-sysroot-missing -fexceptions -fcxx-exceptions -S -o - %s | FileCheck %s
// RUN: %clang --target=arm-pc-wince -Wno-wince-sysroot-missing -mthumb -fexceptions -fcxx-exceptions -S -o - %s | FileCheck %s
// RUN: %clang --target=arm-pc-wince -Wno-wince-sysroot-missing -fexceptions -fcxx-exceptions -c -o /dev/null %s
// RUN: %clang --target=arm-pc-wince -Wno-wince-sysroot-missing -mthumb -fexceptions -fcxx-exceptions -c -o /dev/null %s


extern "C" void might_throw(void);

int cpp_func(int x) {
  try {
    might_throw();
  } catch (...) {
    return -1;
  }
  return x;
}

// CHECK-LABEL: _Z8cpp_funci:
// CHECK:      .seh_proc _Z8cpp_funci
// CHECK-NOT:  .seh_handler
// CHECK:      .fnstart
// CHECK:      .personality __gxx_personality_v0
// CHECK:      .handlerdata
// CHECK:      .fnend
// CHECK:      .seh_endproc

// CHECK-LABEL: _Z10pass_alongi:
// CHECK:      .seh_proc _Z10pass_alongi
// CHECK-NOT:  .seh_handler
// CHECK:      .personality __gxx_personality_v0

int pass_along(int x) {
  try {
    return cpp_func(x);
  } catch (...) {
    return 42;
  }
}
