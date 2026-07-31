// TEMPORARY bisect probe: f32 __metal_sqrt alone. Delete once the
// builtin-arity.metal crash is root-caused.
//
// The first RUN line does -fsyntax-only, the second does full codegen.
// Both must pass (exit 0) for this test to be green.
// RUN: %clang_cc1 -x metal -triple air64_v28-apple-macosx26.0.0 \
// RUN:   -std=metal3.2 -fsyntax-only -no-opaque-pointers %s
// RUN: %clang_cc1 -x metal -triple air64_v28-apple-macosx26.0.0 \
// RUN:   -std=metal3.2 -emit-llvm -no-opaque-pointers -o /dev/null %s
kernel void p1(device float *f [[buffer(0)]]) {
  f[0] = __metal_sqrt(f[0], 1);
}
