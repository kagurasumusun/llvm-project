// TEMPORARY bisect probe: f32 __metal_sqrt alone. Delete once the
// builtin-arity.metal crash is root-caused.
//
// CASTCHK asserts what Sema SHOULD have built for the builtin call: a
// DeclRefExpr typed by the lazily created FunctionDecl, wrapped in the
// function-to-pointer decay. If the real AST differs (the suspicion is a
// bare DeclRefExpr typed '__write_only image1d_array_t'), FileCheck fails
// and the mismatch lines land in the CI FAIL annotation.
// RUN: %clang_cc1 -x metal -triple air64_v28-apple-macosx26.0.0 \
// RUN:   -std=metal3.2 -ast-dump -no-opaque-pointers %s | FileCheck %s --check-prefix=CASTCHK
// RUN: %clang_cc1 -x metal -triple air64_v28-apple-macosx26.0.0 \
// RUN:   -std=metal3.2 -emit-llvm -no-opaque-pointers -o /dev/null %s
// CASTCHK-LABEL: CallExpr 0x{{.*}} 'float'
// CASTCHK-NEXT: ImplicitCastExpr 0x{{.*}} 'void (*)(...) noexcept' <FunctionToPointerDecay>
// CASTCHK-NEXT: DeclRefExpr 0x{{.*}} 'void (...) noexcept' lvalue Function 0x{{.*}} '__metal_sqrt'
kernel void p1(device float *f [[buffer(0)]]) {
  f[0] = __metal_sqrt(f[0], 1);
}
