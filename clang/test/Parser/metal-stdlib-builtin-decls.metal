// RUN: %clang_cc1 -triple air64-apple-macosx26.0 -x metal -std=metal4.0 -fsyntax-only %s


int (*metal_abs_int)(int) = __metal_abs;
int (*metal_select_int)(int, int, bool) = __metal_select;
float (*metal_sin_float)(float) = __metal_sin;
float (*metal_cos_float)(float) = __metal_cos;
float (*metal_floor_float)(float) = __metal_floor;

// The lightweight Metal prelude declares __metal_* stdlib entry points gathered
// in MetalStdlibBuiltins.def.  The declarations are intentionally generic for
// bootstrap parsing; precise overloads/lowering can be refined later.
kernel void use_stdlib_builtin_decls(device int *out [[buffer(0)]]) {
  out[0] = __metal_abs(-7);
  out[1] = __metal_select(1, 2, true);
  float s = __metal_sin(1.0f) + __metal_cos(1.0f) + __metal_floor(1.5f);
  out[2] = int(s);
}
