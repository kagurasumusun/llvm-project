// RUN: %clang --target=arm-pc-wince -gcodeview -O0 -c %s -o %t.obj
// RUN: llvm-objdump -h %t.obj | FileCheck %s --check-prefix=SECS
// RUN: llvm-objdump -s --section=.debug$S %t.obj | FileCheck %s --check-prefix=LINES








int add(int a, int b) { return a + b; }
int main(void) { return add(1, 2) - 3; }

// SECS: .debug$S
// SECS: .debug$T



// LINES: 02000000
