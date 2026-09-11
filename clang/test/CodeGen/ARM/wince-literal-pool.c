// RUN: %clang --target=arm-pc-wince -O1 -S -o - %s | FileCheck %s
// RUN: %clang --target=arm-pc-wince -mcpu=arm926ej-s -O1 -S -o - %s | FileCheck %s


int g;

int *addr(void) { return &g; }

// Neither half of the :lower16:/:upper16: pair appears for a core that has no
// MOVW or MOVT, which is the case the COFF bundling in ExpandMOV32BitImm exists
// for; the address comes out of the pool instead, and both CPUs below predate
// the ARMv6T2 encodings that pair needs.
// CHECK-NOT: movw
// CHECK-NOT: movt
// CHECK: ldr {{r[0-9]+}}, .LCPI
// CHECK-NOT: movw
// CHECK-NOT: movt
