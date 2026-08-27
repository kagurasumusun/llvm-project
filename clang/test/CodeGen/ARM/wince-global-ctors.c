/// Windows CE global constructors/destructors use the GNU .ctors/.dtors
/// convention (mingwrt's __main walks __CTOR_LIST__/__DTOR_LIST__; lld-link
/// -wince brackets the lists), NOT the MSVC .CRT$XCU tables.

// REQUIRES: arm-registered-target
// RUN: %clang --target=arm-pc-wince -S %s -o - 2>&1 \
// RUN:   | FileCheck %s

__attribute__((constructor)) void ctor(void) { return; }
__attribute__((destructor)) void dtor(void) { return; }

int main(void) { return 0; }

// CHECK: .section .ctors
// CHECK: .section .dtors
// CHECK-NOT: .CRT$XCU
// CHECK-NOT: .CRT$XTX
