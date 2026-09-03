// RUN: %clang --target=arm-pc-wince -Wno-wince-sysroot-missing -fexceptions -fcxx-exceptions -S -o - %s | FileCheck %s
// RUN: %clang --target=arm-pc-wince -Wno-wince-sysroot-missing -mthumb -fexceptions -fcxx-exceptions -S -o - %s | FileCheck %s
// RUN: %clang --target=arm-pc-wince -Wno-wince-sysroot-missing -fexceptions -fcxx-exceptions -c -o /dev/null %s
// RUN: %clang --target=arm-pc-wince -Wno-wince-sysroot-missing -mthumb -fexceptions -fcxx-exceptions -c -o /dev/null %s

// Windows CE C++ (Itanium) exceptions are unwound in USER SPACE by this
// toolchain's libunwind through the ARM EHABI tables, exactly as CeGCC/GCC did
// for arm-wince-pe -- NOT through the CE kernel exception dispatcher. So a C++
// function that throws/catches gets:
//   - the ARM EHABI frame (.fnstart/.personality __gxx_personality_v0/
//     .handlerdata/<LSDA>/.fnend) producing the .ARM.exidx/.ARM.extab entry the
//     unwinder binary-searches, and
//   - a WinCFI frame (.seh_proc/.seh_endprologue/.seh_endproc) producing a
//     compressed CE .pdata entry with ExceptionFlag=0, so the kernel can still
//     reverse-execution-unwind through the frame for a hardware fault or an
//     enclosing SEH __try.
// It must NOT claim a CE exception handler: a `.seh_handler` here would set
// ExceptionFlag=1 while ARMAsmPrinter emits no PDATA_EH pair for C++ functions
// (only for SEH funclet parents), so the kernel would read the 8 bytes before
// the function as a live handler pointer and jump into garbage. See
// utils/wince/WINEH-ABI-FACTS.md.
//
// The -c runs gate the object level (a .seh_endproc without .seh_endprologue is
// a hard error in CEEmitUnwindInfo).

extern "C" void might_throw(void);

int cpp_func(int x) {
  try {
    might_throw();
  } catch (...) {
    return -1;
  }
  return x;
}

// A C++ function with a catch: EHABI personality frame + WinCFI frame, and NO
// CE exception handler.
// CHECK-LABEL: _Z8cpp_funci:
// CHECK:      .seh_proc _Z8cpp_funci
// CHECK-NOT:  .seh_handler
// CHECK:      .fnstart
// CHECK:      .personality __gxx_personality_v0
// CHECK:      .handlerdata
// CHECK:      .fnend
// CHECK:      .seh_endproc

// A nested try/catch: same shape -- EHABI personality, no CE handler. An
// exception thrown in cpp_func unwinds through this frame in user space and is
// caught here.
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
