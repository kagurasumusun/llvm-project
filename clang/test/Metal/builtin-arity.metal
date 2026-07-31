// Metal builtins are generic, and their arity comes from measurement.
//
// The builtins are never declared in Apple's headers -- the compiler supplies
// them -- so the only way to learn their signatures from the reference set is
// to read every call site in <metal_stdlib>. Doing that
// (docs-metal/verify/extract_builtin_arity.py) yields 651 names covering all
// 650 entries of BuiltinsMetal.def with no gaps, recorded in
// docs-metal/data/builtin_arity.csv. Two shapes account for every call:
//
//   return __metal_abs(x);                       // 1 argument
//   return __metal_sqrt(x, __METAL_FAST_MATH__); // value first, flag trailing
//
// The declarations keep the generic "v." signature -- "signature is
// meaningless, use custom typechecking" -- because a single declaration is
// called on float, half and the integer types. Sema::CheckMetalBuiltinCall
// checks the arity and derives the result type from the first argument, which
// is what stops a call whose result is used from being typed void and
// crashing CodeGen.
//
// RUN: %clang_cc1 -x metal -triple air64_v28-apple-macosx26.0.0 \
// RUN:   -std=metal3.2 -emit-llvm -no-opaque-pointers -o - %s | FileCheck %s

#include <metal_stdlib>

typedef unsigned int uint;

// The result type follows the first argument, so the f32 and f16 forms select
// different AIR intrinsics. The `fast_` infix is tied to the element type:
// f32 always takes it, f16 never does.
//
// CHECK-LABEL: define void @m(
// CHECK: call{{.*}}@air.fast_sqrt.f32
// CHECK: call{{.*}}@air.sqrt.f16
kernel void m(device float *f [[buffer(0)]], device half *h [[buffer(1)]]) {
  f[0] = __metal_sqrt(f[0], 1);
  h[0] = __metal_sqrt(h[0], 1);
}

// fabs is measured at 2 arguments as well (60 call sites, all of the form
// `__metal_fabs(x, __METAL_FAST_MATH__)`).
//
// CHECK-LABEL: define void @a(
// CHECK: call{{.*}}@air.fast_fabs.f32
kernel void a(device float *f [[buffer(0)]]) {
  f[0] = __metal_fabs(f[0], 1);
}
