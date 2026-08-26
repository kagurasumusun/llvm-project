// RUN: %clang --target=arm-pc-wince -S -o - %s | FileCheck %s
// RUN: %clang --target=arm-pc-wince -mthumb -S -o - %s | FileCheck %s

// Windows CE: every function gets a .ARM.exidx entry (userland EHABI
// unwinding is mandatory: the unwinder walks C frames too).

int leaf(int x) { return x; }

// CHECK: .fnstart
// CHECK: .fnend

int frame(int a, int b) {
  int arr[16];
  for (int i = 0; i < 16; ++i)
    arr[i] = a * b + i;
  int s = 0;
  for (int i = 0; i < 16; ++i)
    s += arr[i];
  return s;
}

// CHECK: .fnstart
// CHECK: .setfp r11, sp
// CHECK: .fnend
