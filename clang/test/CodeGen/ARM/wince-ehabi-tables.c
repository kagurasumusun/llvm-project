// RUN: %clang --target=arm-pc-wince -S -o - %s | FileCheck %s
// RUN: %clang --target=arm-pc-wince -mthumb -S -o - %s | FileCheck %s
// RUN: %clang --target=arm-pc-wince -c -o /dev/null %s
// RUN: %clang --target=arm-pc-wince -mthumb -c -o /dev/null %s

// Windows CE: every function gets BOTH unwind tables (see
// ARMWinCFI.h functionNeedsWinCFIFrame):
//  - a .ARM.exidx entry (userland EHABI unwinding is mandatory: the
//    toolchain's unwinder walks C frames too), and
//  - a WinCFI frame (.seh_proc/.seh_endproc) encoded by the
//    ARMWinCOFFStreamer into a compressed CE .pdata entry, which the
//    kernel needs to unwind any frame at all (eVC emits .pdata for every
//    function too).
// The -c runs also gate the object level: a function reaching
// .seh_endproc without .seh_endprologue is a hard MC error.

int leaf(int x) { return x; }

// CHECK: .seh_proc leaf
// CHECK: .fnstart
// CHECK: .fnend
// CHECK: .seh_endproc

int frame(int a, int b) {
  int arr[16];
  for (int i = 0; i < 16; ++i)
    arr[i] = a * b + i;
  int s = 0;
  for (int i = 0; i < 16; ++i)
    s += arr[i];
  return s;
}

// CHECK: .seh_proc frame
// CHECK: .fnstart
// CHECK: .setfp r11, sp
// CHECK: .fnend
// CHECK: .seh_endproc
