// ===----------------------------------------------------------------------===//
// metal_custom_extensions.h — Custom Metal stdlib extensions
// Functions not in Apple's Metal stdlib but useful for Metal developers.
// Cleanroom implementation using __air_* builtins.
// ABI-compatible with Metal's calling convention.
// ===----------------------------------------------------------------------===//

#ifndef _METAL_CUSTOM_EXTENSIONS_H_
#define _METAL_CUSTOM_EXTENSIONS_H_

#include <metal/metal_common>
#include <metal/metal_math>

namespace metal {
namespace extensions {

// Safe division — returns 0 on division by zero instead of inf/nan
METAL_ALWAYS_INLINE float safe_div(float x, float y) { return (y == float(0)) ? float(0) : x / y; }
METAL_ALWAYS_INLINE float2 safe_div(float2 x, float2 y) { return (y == float2(0)) ? float2(0) : x / y; }
METAL_ALWAYS_INLINE float3 safe_div(float3 x, float3 y) { return (y == float3(0)) ? float3(0) : x / y; }
METAL_ALWAYS_INLINE float4 safe_div(float4 x, float4 y) { return (y == float4(0)) ? float4(0) : x / y; }
METAL_ALWAYS_INLINE half safe_div(half x, half y) { return (y == half(0)) ? half(0) : x / y; }
METAL_ALWAYS_INLINE half2 safe_div(half2 x, half2 y) { return (y == half2(0)) ? half2(0) : x / y; }
METAL_ALWAYS_INLINE half3 safe_div(half3 x, half3 y) { return (y == half3(0)) ? half3(0) : x / y; }
METAL_ALWAYS_INLINE half4 safe_div(half4 x, half4 y) { return (y == half4(0)) ? half4(0) : x / y; }

// Saturating absolute value — returns MAX on MIN_INT overflow
METAL_ALWAYS_INLINE int safe_abs(int x) { return x < 0 ? (x == int_MIN ? int_MAX : -x) : x; }
METAL_ALWAYS_INLINE int2 safe_abs(int2 x) { return x < 0 ? (x == int2_MIN ? int2_MAX : -x) : x; }
METAL_ALWAYS_INLINE int3 safe_abs(int3 x) { return x < 0 ? (x == int3_MIN ? int3_MAX : -x) : x; }
METAL_ALWAYS_INLINE int4 safe_abs(int4 x) { return x < 0 ? (x == int4_MIN ? int4_MAX : -x) : x; }

// Safe normalize — returns zero vector when length is zero
METAL_ALWAYS_INLINE float2 safe_normalize(float2 x) { float2 l = length(x); return l > float(0) ? x / l : float2(0); }
METAL_ALWAYS_INLINE float3 safe_normalize(float3 x) { float3 l = length(x); return l > float(0) ? x / l : float3(0); }
METAL_ALWAYS_INLINE float4 safe_normalize(float4 x) { float4 l = length(x); return l > float(0) ? x / l : float4(0); }
METAL_ALWAYS_INLINE half2 safe_normalize(half2 x) { half2 l = length(x); return l > half(0) ? x / l : half2(0); }
METAL_ALWAYS_INLINE half3 safe_normalize(half3 x) { half3 l = length(x); return l > half(0) ? x / l : half3(0); }
METAL_ALWAYS_INLINE half4 safe_normalize(half4 x) { half4 l = length(x); return l > half(0) ? x / l : half4(0); }

// Inverse linear interpolation
METAL_ALWAYS_INLINE float inverse_lerp(float a, float b, float x) { return (x - a) / (b - a); }
METAL_ALWAYS_INLINE float2 inverse_lerp(float2 a, float2 b, float2 x) { return (x - a) / (b - a); }
METAL_ALWAYS_INLINE float3 inverse_lerp(float3 a, float3 b, float3 x) { return (x - a) / (b - a); }
METAL_ALWAYS_INLINE float4 inverse_lerp(float4 a, float4 b, float4 x) { return (x - a) / (b - a); }
METAL_ALWAYS_INLINE half inverse_lerp(half a, half b, half x) { return (x - a) / (b - a); }
METAL_ALWAYS_INLINE half2 inverse_lerp(half2 a, half2 b, half2 x) { return (x - a) / (b - a); }
METAL_ALWAYS_INLINE half3 inverse_lerp(half3 a, half3 b, half3 x) { return (x - a) / (b - a); }
METAL_ALWAYS_INLINE half4 inverse_lerp(half4 a, half4 b, half4 x) { return (x - a) / (b - a); }

// Remap value from one range to another
METAL_ALWAYS_INLINE float remap(float x, float from_min, float from_max, float to_min, float to_max) { return to_min + (x - from_min) * (to_max - to_min) / (from_max - from_min); }
METAL_ALWAYS_INLINE float2 remap(float2 x, float2 from_min, float2 from_max, float2 to_min, float2 to_max) { return to_min + (x - from_min) * (to_max - to_min) / (from_max - from_min); }
METAL_ALWAYS_INLINE float3 remap(float3 x, float3 from_min, float3 from_max, float3 to_min, float3 to_max) { return to_min + (x - from_min) * (to_max - to_min) / (from_max - from_min); }
METAL_ALWAYS_INLINE float4 remap(float4 x, float4 from_min, float4 from_max, float4 to_min, float4 to_max) { return to_min + (x - from_min) * (to_max - to_min) / (from_max - from_min); }
METAL_ALWAYS_INLINE half remap(half x, half from_min, half from_max, half to_min, half to_max) { return to_min + (x - from_min) * (to_max - to_min) / (from_max - from_min); }
METAL_ALWAYS_INLINE half2 remap(half2 x, half2 from_min, half2 from_max, half2 to_min, half2 to_max) { return to_min + (x - from_min) * (to_max - to_min) / (from_max - from_min); }
METAL_ALWAYS_INLINE half3 remap(half3 x, half3 from_min, half3 from_max, half3 to_min, half3 to_max) { return to_min + (x - from_min) * (to_max - to_min) / (from_max - from_min); }
METAL_ALWAYS_INLINE half4 remap(half4 x, half4 from_min, half4 from_max, half4 to_min, half4 to_max) { return to_min + (x - from_min) * (to_max - to_min) / (from_max - from_min); }

// Check if value is power of two
METAL_ALWAYS_INLINE bool is_power_of_2(int x) { return x > 0 && (x & (x - 1)) == 0; }
METAL_ALWAYS_INLINE bool is_power_of_2(uint x) { return x > 0 && (x & (x - 1)) == 0; }
// Round up to next power of two
METAL_ALWAYS_INLINE uint next_power_of_2(uint x) { x--; x|=x>>1; x|=x>>2; x|=x>>4; x|=x>>8; x|=x>>16; return x+1; }

// Linear to sRGB conversion
METAL_ALWAYS_INLINE float linear_to_srgb(float c) { return c <= 0.0031308f ? 12.92f * c : 1.055f * pow(c, 1.0f/2.4f) - 0.055f; }
METAL_ALWAYS_INLINE float3 linear_to_srgb(float3 c) { return float3(linear_to_srgb(c.x), linear_to_srgb(c.y), linear_to_srgb(c.z)); }
METAL_ALWAYS_INLINE float4 linear_to_srgb(float4 c) { return float4(linear_to_srgb(c.xyz), c.w); }
// sRGB to linear conversion
METAL_ALWAYS_INLINE float srgb_to_linear(float c) { return c <= 0.04045f ? c / 12.92f : pow((c + 0.055f) / 1.055f, 2.4f); }
METAL_ALWAYS_INLINE float3 srgb_to_linear(float3 c) { return float3(srgb_to_linear(c.x), srgb_to_linear(c.y), srgb_to_linear(c.z)); }
METAL_ALWAYS_INLINE float4 srgb_to_linear(float4 c) { return float4(srgb_to_linear(c.xyz), c.w); }

// Quaternion multiply
METAL_ALWAYS_INLINE float4 quaternion_multiply(float4 q1, float4 q2) {
  return float4(
    q1.w*q2.x + q1.x*q2.w + q1.y*q2.z - q1.z*q2.y,
    q1.w*q2.y - q1.x*q2.z + q1.y*q2.w + q1.z*q2.x,
    q1.w*q2.z + q1.x*q2.y - q1.y*q2.x + q1.z*q2.w,
    q1.w*q2.w - q1.x*q2.x - q1.y*q2.y - q1.z*q2.z);
}
// Quaternion conjugate
METAL_ALWAYS_INLINE float4 quaternion_conjugate(float4 q) { return float4(-q.xyz, q.w); }
// Rotate vector by quaternion
METAL_ALWAYS_INLINE float3 quaternion_rotate(float4 q, float3 v) {
  float4 qv = float4(v, 0);
  float4 qr = quaternion_multiply(quaternion_multiply(q, qv), quaternion_conjugate(q));
  return qr.xyz;
}

// Hash-based pseudo-random float [0,1)
METAL_ALWAYS_INLINE float hash_float(uint x) {
  x = ((x >> 16) ^ x) * 0x45d9f3b; x = ((x >> 16) ^ x) * 0x45d9f3b; x = (x >> 16) ^ x;
  return float(x & 0xFFFFFF) / float(0x1000000);
}
METAL_ALWAYS_INLINE float2 hash_float2(uint2 v) { return float2(hash_float(v.x), hash_float(v.y)); }

} // namespace extensions
} // namespace metal
#endif // _METAL_CUSTOM_EXTENSIONS_H_