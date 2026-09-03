// RUN: %clang --target=arm-pc-wince -Wno-wince-sysroot-missing -fms-extensions -x c++ -fexceptions -fcxx-exceptions -S -o - %s | FileCheck %s
// RUN: %clang --target=arm-pc-wince -Wno-wince-sysroot-missing -mthumb -fms-extensions -x c++ -fexceptions -fcxx-exceptions -S -o - %s | FileCheck %s
// RUN: %clang --target=arm-pc-wince -Wno-wince-sysroot-missing -fms-extensions -x c++ -fexceptions -fcxx-exceptions -c -o /dev/null %s
// RUN: %clang --target=arm-pc-wince -Wno-wince-sysroot-missing -mthumb -fms-extensions -x c++ -fexceptions -fcxx-exceptions -c -o /dev/null %s

// Windows CE: C++ exceptions keep the ARM EHABI model (.fnstart/.fnend,
// .ARM.exidx) while MSVC-style SEH (__try/__except) functions additionally
// get a WinCFI frame (.seh_proc/.seh_endproc) encoded into the compressed
// CE .pdata format.  Both mechanisms coexist per function in one TU; see
// utils/wince/WINEH-ABI-FACTS.md 4c/4d.
//
// Since the all-function .pdata change, EVERY function carries both tables:
// the kernel needs a .pdata entry per function to unwind it, and the EHABI
// unwinder needs an .ARM.exidx entry per function to unwind through it (an
// SEH function without an exidx entry would make a C++ exception unwind
// with the *previous* function's opcodes).  SEH functions keep the SEH
// personality for .pdata only -- their EHABI entry has plain unwind
// opcodes and no personality.  The -c runs gate the object level, where a
// .seh_endproc without a preceding .seh_endprologue is a hard error.

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

// The SEH parent: WinCFI frame with handler first, then the EHABI frame
// (no .personality/.handlerdata: __C_specific_handler is dispatched by the
// kernel through PDATA_EH, not by the EHABI unwinder).
// CHECK-LABEL: seh_func:
// CHECK-NOT: .fnstart
// CHECK: .seh_proc seh_func
// CHECK: .seh_handler __C_specific_handler, %except
// CHECK: .fnstart
// CHECK-NOT: .personality
// CHECK-NOT: .handlerdata
// CHECK: .fnend
// CHECK: .seh_endproc

// The non-constant filter is outlined; the filter function is a plain
// function (no SEH personality): it keeps the EHABI frame and now gets a
// .pdata entry of its own as well.
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

// A C++ function: EHABI frame with personality, plus the WinCFI frame.
// CHECK-LABEL: _Z8cpp_funci:
// CHECK: .seh_proc _Z8cpp_funci
// CHECK: .fnstart
// CHECK: .personality __gxx_personality_v0
// CHECK: .handlerdata
// CHECK: .fnend
// CHECK: .seh_endproc
