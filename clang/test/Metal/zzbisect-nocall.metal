// TEMPORARY control probe: same kernel shape as the sqrt probes but with no
// builtin call. If this passes, the crash is specific to __metal_* call
// emission; if it crashes, the problem is in the kernel codegen itself.
// Delete once the builtin-arity.metal crash is root-caused.
//
// RUN: %clang_cc1 -x metal -triple air64_v28-apple-macosx26.0.0 \
// RUN:   -std=metal3.2 -emit-llvm -no-opaque-pointers -o /dev/null %s
kernel void p0(device float *f [[buffer(0)]]) {
  f[0] = f[0] + 1.0f;
}
