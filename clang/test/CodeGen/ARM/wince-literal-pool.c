// RUN: %clang --target=arm-pc-wince -O1 -S -o - %s | FileCheck %s
// RUN: %clang --target=arm-pc-wince -mcpu=arm926ej-s -O1 -S -o - %s | FileCheck %s

// WinCE baseline devices (ARMv4T/ARMv5TE) have no MOVW/MOVT: global
// addresses must be materialized from literal pools (LowerGlobalAddress
// through the ELF-style path).

int g;

int *addr(void) { return &g; }

// CHECK-NOT: movt
// CHECK: ldr {{r[0-9]+}}, .LCPI
