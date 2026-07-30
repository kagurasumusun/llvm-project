// Wrong arity on a Metal builtin is diagnosed.
//
// Split out from builtin-arity.metal: these declarations do not compile, so
// they cannot share a file with a -emit-llvm run. Codegen would be handed an
// invalid AST and crash.
//
// The counts come from parsing every call site in Apple's <metal_stdlib>;
// see docs-metal/data/builtin_arity.csv.
//
// RUN: %clang_cc1 -x metal -triple air64_v28-apple-macosx26.0.0 \
// RUN:   -std=metal3.2 -fsyntax-only -verify %s

// Getting the arity wrong is now diagnosed rather than silently accepted.
kernel void bad(device float *f [[buffer(0)]]) {
  f[0] = __metal_sqrt(f[0]); // expected-error {{too few arguments to function call, expected 2, have 1}}
  f[1] = __metal_fabs(f[0], 1, 1); // expected-error {{too many arguments to function call, expected 2, have 3}}
}

