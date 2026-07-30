// TEMPORARY bisect probe: f16 __metal_sqrt alone. Delete once the
// builtin-arity.metal crash is root-caused.
//
// RUN: %clang_cc1 -x metal -triple air64_v28-apple-macosx26.0.0 \
// RUN:   -std=metal3.2 -emit-llvm -no-opaque-pointers -o /dev/null %s
kernel void p2(device half *h [[buffer(0)]]) {
  h[0] = __metal_sqrt(h[0], 1);
}
