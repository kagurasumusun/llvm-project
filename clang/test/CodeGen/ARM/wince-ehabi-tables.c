// RUN: %clang --target=arm-pc-wince -S -o - %s | FileCheck %s
// RUN: %clang --target=arm-pc-wince -mthumb -S -o - %s | FileCheck %s
// RUN: %clang --target=arm-pc-wince -c -o /dev/null %s
// RUN: %clang --target=arm-pc-wince -mthumb -c -o /dev/null %s


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
