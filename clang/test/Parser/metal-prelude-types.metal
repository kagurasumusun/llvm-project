// RUN: %clang_cc1 -triple air64-apple-macosx26.0 -x metal -std=metal4.0 -fsyntax-only %s

float4 global_float4;
half4 global_half4;
uint4 global_uint4;
void use_vectors(float4 a, half4 b, uint4 c) {}

// User-facing `metal::` names come from Apple metal_stdlib.  The lightweight
// compiler prelude only provides global bootstrap aliases for focused frontend
// tests.

struct Uniforms {
  float4x4 mvp;
  float4 value;
  uint width;
  uint height;
};

constant bool fc_bool [[function_constant(0)]];
constant int fc_int [[function_constant(1)]];
