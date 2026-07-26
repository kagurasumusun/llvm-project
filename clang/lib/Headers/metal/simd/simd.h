//===----------------------------------------------------------------------===//
// simd — Metal SIMD types and operations
// Vector types are compiler builtins; this header provides operations.
//===----------------------------------------------------------------------===//

#ifndef _SIMD_SIMD_H_
#define _SIMD_SIMD_H_

#include <metal/metal_common>
#include <metal/metal_math>

namespace simd {

// ---- Type aliases (compiler builtins) ----
typedef float2  simd_float2;
typedef float3  simd_float3;
typedef float4  simd_float4;
typedef half2   simd_half2;
typedef half3   simd_half3;
typedef half4   simd_half4;
typedef int2    simd_int2;
typedef int3    simd_int3;
typedef int4    simd_int4;
typedef uint2   simd_uint2;
typedef uint3   simd_uint3;
typedef uint4   simd_uint4;

// ---- Matrix types ----
typedef struct { simd_float4 columns[4]; } simd_float4x4;
typedef struct { simd_float4 columns[3]; } simd_float3x3;
typedef struct { simd_float4 columns[2]; } simd_float2x2;
typedef struct { simd_float3 columns[3]; } simd_float3x3_packed;

// ---- Dot product ----
METAL_ALWAYS_INLINE float simd_dot(simd_float2 a, simd_float2 b) { return metal::dot(a, b); }
METAL_ALWAYS_INLINE float simd_dot(simd_float3 a, simd_float3 b) { return metal::dot(a, b); }
METAL_ALWAYS_INLINE float simd_dot(simd_float4 a, simd_float4 b) { return metal::dot(a, b); }

// ---- Cross product ----
METAL_ALWAYS_INLINE simd_float3 simd_cross(simd_float3 a, simd_float3 b) { return metal::cross(a, b); }

// ---- Length ----
METAL_ALWAYS_INLINE float simd_length(simd_float2 v) { return metal::length(v); }
METAL_ALWAYS_INLINE float simd_length(simd_float3 v) { return metal::length(v); }
METAL_ALWAYS_INLINE float simd_length(simd_float4 v) { return metal::length(v); }

// ---- Normalize ----
METAL_ALWAYS_INLINE simd_float2 simd_normalize(simd_float2 v) { return metal::normalize(v); }
METAL_ALWAYS_INLINE simd_float3 simd_normalize(simd_float3 v) { return metal::normalize(v); }
METAL_ALWAYS_INLINE simd_float4 simd_normalize(simd_float4 v) { return metal::normalize(v); }

// ---- Min / Max ----
METAL_ALWAYS_INLINE simd_float2 simd_min(simd_float2 a, simd_float2 b) { return metal::fmin(a, b); }
METAL_ALWAYS_INLINE simd_float3 simd_min(simd_float3 a, simd_float3 b) { return metal::fmin(a, b); }
METAL_ALWAYS_INLINE simd_float4 simd_min(simd_float4 a, simd_float4 b) { return metal::fmin(a, b); }
METAL_ALWAYS_INLINE simd_float2 simd_max(simd_float2 a, simd_float2 b) { return metal::fmax(a, b); }
METAL_ALWAYS_INLINE simd_float3 simd_max(simd_float3 a, simd_float3 b) { return metal::fmax(a, b); }
METAL_ALWAYS_INLINE simd_float4 simd_max(simd_float4 a, simd_float4 b) { return metal::fmax(a, b); }

// ---- Clamp ----
METAL_ALWAYS_INLINE simd_float2 simd_clamp(simd_float2 x, simd_float2 lo, simd_float2 hi) {
  return metal::fmin(metal::fmax(x, lo), hi);
}
METAL_ALWAYS_INLINE simd_float3 simd_clamp(simd_float3 x, simd_float3 lo, simd_float3 hi) {
  return metal::fmin(metal::fmax(x, lo), hi);
}
METAL_ALWAYS_INLINE simd_float4 simd_clamp(simd_float4 x, simd_float4 lo, simd_float4 hi) {
  return metal::fmin(metal::fmax(x, lo), hi);
}

// ---- Saturate ----
METAL_ALWAYS_INLINE simd_float2 simd_saturate(simd_float2 x) { return simd_clamp(x, (simd_float2)0, (simd_float2)1); }
METAL_ALWAYS_INLINE simd_float3 simd_saturate(simd_float3 x) { return simd_clamp(x, (simd_float3)0, (simd_float3)1); }
METAL_ALWAYS_INLINE simd_float4 simd_saturate(simd_float4 x) { return simd_clamp(x, (simd_float4)0, (simd_float4)1); }

// ---- Matrix multiply ----
METAL_ALWAYS_INLINE simd_float4 simd_mul(simd_float4x4 m, simd_float4 v) {
  return m.columns[0] * v.x + m.columns[1] * v.y +
         m.columns[2] * v.z + m.columns[3] * v.w;
}

METAL_ALWAYS_INLINE simd_float4x4 simd_mul(simd_float4x4 a, simd_float4x4 b) {
  simd_float4x4 r;
  r.columns[0] = simd_mul(a, b.columns[0]);
  r.columns[1] = simd_mul(a, b.columns[1]);
  r.columns[2] = simd_mul(a, b.columns[2]);
  r.columns[3] = simd_mul(a, b.columns[3]);
  return r;
}

// ---- Identity matrix ----
METAL_ALWAYS_INLINE simd_float4x4 simd_matrix(float v) {
  simd_float4x4 m;
  m.columns[0] = simd_float4(v, 0, 0, 0);
  m.columns[1] = simd_float4(0, v, 0, 0);
  m.columns[2] = simd_float4(0, 0, v, 0);
  m.columns[3] = simd_float4(0, 0, 0, v);
  return m;
}

METAL_ALWAYS_INLINE simd_float4x4 simd_identity() { return simd_matrix(1.0f); }

// ---- Translation / rotation / scale matrices ----
METAL_ALWAYS_INLINE simd_float4x4 simd_matrix_translation(float x, float y, float z) {
  simd_float4x4 m = simd_identity();
  m.columns[3] = simd_float4(x, y, z, 1.0f);
  return m;
}

METAL_ALWAYS_INLINE simd_float4x4 simd_matrix_rotation(float angle, simd_float3 axis) {
  simd_float3 a = simd_normalize(axis);
  float c = metal::cos(angle);
  float s = metal::sin(angle);
  float t = 1.0f - c;
  simd_float4x4 m;
  m.columns[0] = simd_float4(t*a.x*a.x + c,     t*a.x*a.y + s*a.z, t*a.x*a.z - s*a.y, 0);
  m.columns[1] = simd_float4(t*a.x*a.y - s*a.z, t*a.y*a.y + c,     t*a.y*a.z + s*a.x, 0);
  m.columns[2] = simd_float4(t*a.x*a.z + s*a.y, t*a.y*a.z - s*a.x, t*a.z*a.z + c,     0);
  m.columns[3] = simd_float4(0, 0, 0, 1);
  return m;
}

METAL_ALWAYS_INLINE simd_float4x4 simd_matrix_scale(float sx, float sy, float sz) {
  simd_float4x4 m;
  m.columns[0] = simd_float4(sx, 0, 0, 0);
  m.columns[1] = simd_float4(0, sy, 0, 0);
  m.columns[2] = simd_float4(0, 0, sz, 0);
  m.columns[3] = simd_float4(0, 0, 0, 1);
  return m;
}

} // namespace simd

#endif // _SIMD_SIMD_H_
