
// REQUIRES: arm-registered-target
// RUN: %clang --target=arm-pc-wince -Wno-wince-sysroot-missing -S %s -o - \
// RUN:   | FileCheck %s

__attribute__((constructor)) void ctor(void) { return; }
__attribute__((destructor)) void dtor(void) { return; }

int main(void) { return 0; }

// CHECK: .section .ctors
// CHECK: .section .dtors
// CHECK-NOT: .CRT$XCU
// CHECK-NOT: .CRT$XTX
