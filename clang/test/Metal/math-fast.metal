// AIR math intrinsics carry a `fast_` infix for single precision and never for
// half precision, and the type suffix follows the call site.
//
// Measured across the reference IR corpus: `air.sqrt.f16` and
// `air.fast_sqrt.f32` both appear, but neither `air.fast_sqrt.f16` nor a bare
// `air.sqrt.f32` exists anywhere. 27 math stems take the infix. This matches
// the driver spelling -fmetal-math-fp32-functions=fast, which names single
// precision only.
//
// RUN: %clang_cc1 -x metal -triple air64_v28-apple-macosx26.0.0 \
// RUN:   -std=metal3.2 -emit-llvm -no-opaque-pointers -o - %s | FileCheck %s

// __metal_sqrt is a compiler builtin declared in BuiltinsMetal.def, so it
// must not be redeclared here; doing so is rejected with
// "cannot redeclare builtin function".

kernel void m(device float *f [[buffer(0)]], device __fp16 *h [[buffer(1)]]) {
  // CHECK: call{{.*}}@air.fast_sqrt.f32
  f[0] = __metal_sqrt(f[0], 1);
  // CHECK: call{{.*}}@air.sqrt.f16
  h[0] = __metal_sqrt(h[0], 1);
}
