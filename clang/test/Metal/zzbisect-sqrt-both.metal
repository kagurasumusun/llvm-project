// TEMPORARY bisect probe: f32 and f16 __metal_sqrt in one kernel, plus a
// second kernel using __metal_fabs -- the exact shape of builtin-arity.metal.
// Delete once the crash is root-caused.
//
// RUN: %clang_cc1 -x metal -triple air64_v28-apple-macosx26.0.0 \
// RUN:   -std=metal3.2 -emit-llvm -no-opaque-pointers -o /dev/null %s
kernel void p3(device float *f [[buffer(0)]], device half *h [[buffer(1)]]) {
  f[0] = __metal_sqrt(f[0], 1);
  h[0] = __metal_sqrt(h[0], 1);
}

kernel void p3b(device float *f [[buffer(0)]]) {
  f[0] = __metal_fabs(f[0], 1);
}
