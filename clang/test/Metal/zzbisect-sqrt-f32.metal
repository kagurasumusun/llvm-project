// TEMPORARY bisect probe: f32 __metal_sqrt alone. Delete once the
// builtin-arity.metal crash is root-caused.
//
// The ast-dump RUN prints the callee subtree that Sema built and then fails
// on purpose, so the run-tests.sh FAIL annotation carries those lines. It
// deliberately runs first.
// RUN: %clang_cc1 -x metal -triple air64_v28-apple-macosx26.0.0 \
// RUN:   -std=metal3.2 -ast-dump -no-opaque-pointers %s -o /tmp/zz-f32.ast; \
// RUN:   grep -B2 '__metal_sqrt' /tmp/zz-f32.ast | tail -8; false
// RUN: %clang_cc1 -x metal -triple air64_v28-apple-macosx26.0.0 \
// RUN:   -std=metal3.2 -emit-llvm -no-opaque-pointers -o /dev/null %s
kernel void p1(device float *f [[buffer(0)]]) {
  f[0] = __metal_sqrt(f[0], 1);
}
