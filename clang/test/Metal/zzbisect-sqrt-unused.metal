// TEMPORARY bisect probe: f32 __metal_sqrt with the result unused, to tell
// apart a crash inside builtin emission from one in the surrounding store.
// Delete once the builtin-arity.metal crash is root-caused.
//
// RUN: %clang_cc1 -x metal -triple air64_v28-apple-macosx26.0.0 \
// RUN:   -std=metal3.2 -emit-llvm -no-opaque-pointers -o /dev/null %s
kernel void p5(device float *f [[buffer(0)]]) {
  __metal_sqrt(f[0], 1);
}
