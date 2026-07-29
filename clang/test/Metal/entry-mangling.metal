// Metal entry points and externally visible functions keep their source name;
// ordinary helpers are mangled as usual.
//
// Measured across 400 entry points in the reference corpus, none of which is
// mangled, e.g. reference/metal-ast-macos-air64/ir/...multi_entry_with_helpers:
//   define float @_Z10helper_mulff(float, float)   ; helper: mangled
//   define i32   @visible_fn(i32)                  ; [[visible]]: plain
//   define void  @k_using_add(...)                 ; kernel:      plain
//
// RUN: %clang_cc1 -x metal -triple air64_v28-apple-macosx26.0.0 \
// RUN:   -std=metal3.2 -emit-llvm -no-opaque-pointers -o - %s | FileCheck %s

static float helper_mul(float a, float b) { return a * b; }

[[visible]] int visible_fn(int v) { return v + 1; }

kernel void k_using_mul(device float *out [[buffer(0)]]) {
  out[0] = helper_mul(out[0], 2.0f);
}

// The kernel and the visible function keep their names.
// CHECK-DAG: define void @k_using_mul(
// CHECK-DAG: define{{.*}} i32 @visible_fn(

// The helper does not.
// CHECK-DAG: @_Z10helper_mulff
