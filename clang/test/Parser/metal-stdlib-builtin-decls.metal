// RUN: %clang_cc1 -triple air64-apple-macosx26.0 -x metal -std=metal4.0 -fsyntax-only %s

// The lightweight Metal prelude declares __metal_* stdlib entry points gathered
// in MetalStdlibBuiltins.def.  The declarations are intentionally generic for
// bootstrap parsing; precise overloads/lowering can be refined later.
kernel void use_stdlib_builtin_decls(device int *out [[buffer(0)]]) {
  out[0] = __metal_abs(-7);
  out[1] = __metal_select(1, 2, true);
}
