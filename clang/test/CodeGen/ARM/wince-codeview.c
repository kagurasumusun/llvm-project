// RUN: %clang --target=arm-pc-wince -gcodeview -O0 -c %s -o %t.obj
// RUN: llvm-objdump -h %t.obj | FileCheck %s

// WinCE accepts CodeView debug info: the object must carry the CodeView
// sections (.debug$B line numbers, .debug$S symbol records, .debug$T type
// records).  lld-link -debug (added by the driver with -g) embeds them in
// the final image; lld has no PDB writer.

int add(int a, int b) { return a + b; }
int main(void) { return add(1, 2) - 3; }

// CHECK-DAG: .debug$B
// CHECK-DAG: .debug$S
// CHECK-DAG: .debug$T
