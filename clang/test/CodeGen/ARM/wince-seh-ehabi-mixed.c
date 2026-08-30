// RUN: %clang --target=arm-pc-wince -Wno-wince-sysroot-missing -fms-extensions -x c++ -fexceptions -fcxx-exceptions -S -o - %s | FileCheck %s
// RUN: %clang --target=arm-pc-wince -Wno-wince-sysroot-missing -mthumb -fms-extensions -x c++ -fexceptions -fcxx-exceptions -S -o - %s | FileCheck %s

// Windows CE: C++ exceptions keep the ARM EHABI model (.fnstart/.fnend,
// .ARM.exidx) while MSVC-style SEH (__try/__except) functions switch to a
// WinCFI frame (.seh_proc/.seh_endproc) encoded into the compressed CE
// .pdata format.  Both mechanisms coexist per function in one TU; see
// utils/wince/WINEH-ABI-FACTS.md 4c/4d.

extern "C" void might_crash(void);

int seh_func(int x) {
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
// CHECK: .seh_endproc

// The non-constant filter is outlined; the filter function is a plain
// function (no SEH personality), so it keeps the EHABI frame.
// CHECK-LABEL: "??filt$0@0@seh_func@@":
// CHECK: .fnstart
// CHECK: .fnend

int cpp_func(int x) {
  try {
    might_crash();
  } catch (...) {
    return -1;
  }
  return x;
}

// CHECK-LABEL: cpp_func:
// CHECK: .fnstart
// CHECK: .fnend
