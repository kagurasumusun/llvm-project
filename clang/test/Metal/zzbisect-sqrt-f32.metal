// TEMPORARY bisect probe: f32 __metal_sqrt alone. Delete once the
// builtin-arity.metal crash is root-caused.
//
// The -ast-dump RUN shows what Sema actually built for the builtin call,
// before CodeGen ever runs.
// RUN: %clang_cc1 -x metal -triple air64_v28-apple-macosx26.0.0 \
// RUN:   -std=metal3.2 -ast-dump -no-opaque-pointers %s | FileCheck %s
// RUN: %clang_cc1 -x metal -triple air64_v28-apple-macosx26.0.0 \
// RUN:   -std=metal3.2 -emit-llvm -no-opaque-pointers -o /dev/null %s
// CHECK: CallExpr {{.*}}
// CHECK-NEXT: {{(ImplicitCastExpr|DeclRefExpr)}}{{.*}}
kernel void p1(device float *f [[buffer(0)]]) {
  f[0] = __metal_sqrt(f[0], 1);
}
