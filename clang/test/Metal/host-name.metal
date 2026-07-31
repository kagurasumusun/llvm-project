// [[host_name("...")]] names the emitted LLVM symbol.
//
// Measured on
//   [[host_name("my_kernel")]] kernel void internal_name(...)
// which produces
//   define void @my_kernel(...)
//   !air.kernel = !{!{void (...)* @my_kernel, ...}}
// (reference/metal-ast-ios-air64/ir/
//  ios_air64_versioned_none_ios-metal2.2_cx_vis_host_name_26_0_llvm-ir.ll).
//
// RUN: %clang_cc1 -x metal -triple air64_v28-apple-macosx26.0.0 \
// RUN:   -std=metal3.2 -emit-llvm -no-opaque-pointers -o - %s | FileCheck %s

typedef unsigned int uint;

// The source name never appears; the host name is used for both the
// function and its air.kernel metadata reference.
//
// CHECK: define void @my_kernel(
// CHECK-NOT: define {{.*}}@internal_name
// CHECK: !air.kernel = !{![[K:[0-9]+]]}
// CHECK: ![[K]] = !{void ({{.*}})* @my_kernel,
[[host_name("my_kernel")]] kernel void internal_name(
    device uint *o [[buffer(0)]]) {
  o[0] = 42u;
}
