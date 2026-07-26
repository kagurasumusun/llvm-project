//===----------------------------------------------------------------------===//
// metal_numeric — MSL §8.1.33–41 numeric utility functions
//===----------------------------------------------------------------------===//
#ifndef _METAL_NUMERIC_H_
#define _METAL_NUMERIC_H_
#include <metal/metal_common>
#include <metal/metal_math>

namespace metal {

// fmod / modf / ldexp / frexp / ilogb — delegated to metal_math

// degrees / radians — delegated to metal_math

// fdim
METAL_ALWAYS_INLINE float  fdim(float x, float y)   { return x > y ? x - y : 0.0f; }
METAL_ALWAYS_INLINE half   fdim(half x, half y)     { return x > y ? x - y : (half)0; }
METAL_ALWAYS_INLINE float2 fdim(float2 x, float2 y) { return float2(fdim(x.x,y.x), fdim(x.y,y.y)); }
METAL_ALWAYS_INLINE float3 fdim(float3 x, float3 y) { return float3(fdim(x.x,y.x), fdim(x.y,y.y), fdim(x.z,y.z)); }
METAL_ALWAYS_INLINE float4 fdim(float4 x, float4 y) { return float4(fdim(x.x,y.x), fdim(x.y,y.y), fdim(x.z,y.z), fdim(x.w,y.w)); }
METAL_ALWAYS_INLINE half2  fdim(half2 x, half2 y)   { return half2(fdim(x.x,y.x), fdim(x.y,y.y)); }
METAL_ALWAYS_INLINE half3  fdim(half3 x, half3 y)   { return half3(fdim(x.x,y.x), fdim(x.y,y.y), fdim(x.z,y.z)); }
METAL_ALWAYS_INLINE half4  fdim(half4 x, half4 y)   { return half4(fdim(x.x,y.x), fdim(x.y,y.y), fdim(x.z,y.z), fdim(x.w,y.w)); }

// fma / fmuladd / mad — delegated to metal_math

// fmax3 / fmin3 / fmedian3 / max3 / min3 / median3
METAL_ALWAYS_INLINE float fmax3(float a, float b, float c) { return fmax(fmax(a, b), c); }
METAL_ALWAYS_INLINE float fmin3(float a, float b, float c) { return fmin(fmin(a, b), c); }
METAL_ALWAYS_INLINE float fmedian3(float a, float b, float c) {
  if (a > b) { float t=a; a=b; b=t; }
  if (b > c) { float t=b; b=c; c=t; }
  if (a > b) { float t=a; a=b; b=t; }
  return b;
}
METAL_ALWAYS_INLINE int   max3(int a, int b, int c)   { return max(max(a, b), c); }
METAL_ALWAYS_INLINE int   min3(int a, int b, int c)   { return min(min(a, b), c); }
METAL_ALWAYS_INLINE int   median3(int a, int b, int c) { return fmedian3((float)a,(float)b,(float)c); }
METAL_ALWAYS_INLINE uint  max3(uint a, uint b, uint c) { return max(max(a, b), c); }
METAL_ALWAYS_INLINE uint  min3(uint a, uint b, uint c) { return min(min(a, b), c); }
METAL_ALWAYS_INLINE uint  median3(uint a, uint b, uint c) { return fmedian3((float)a,(float)b,(float)c); }

METAL_ALWAYS_INLINE float2 fmax3(float2 a, float2 b, float2 c) { return fmax(fmax(a,b),c); }
METAL_ALWAYS_INLINE float3 fmax3(float3 a, float3 b, float3 c) { return fmax(fmax(a,b),c); }
METAL_ALWAYS_INLINE float4 fmax3(float4 a, float4 b, float4 c) { return fmax(fmax(a,b),c); }
METAL_ALWAYS_INLINE float2 fmin3(float2 a, float2 b, float2 c) { return fmin(fmin(a,b),c); }
METAL_ALWAYS_INLINE float3 fmin3(float3 a, float3 b, float3 c) { return fmin(fmin(a,b),c); }
METAL_ALWAYS_INLINE float4 fmin3(float4 a, float4 b, float4 c) { return fmin(fmin(a,b),c); }

// nextafter
METAL_ALWAYS_INLINE float nextafter(float x, float y) { return __builtin_nextafterf(x, y); }
METAL_ALWAYS_INLINE half  nextafter(half x, half y)   { return (half)__builtin_nextafterf((float)x, (float)y); }
METAL_ALWAYS_INLINE float2 nextafter(float2 x, float2 y) { return float2(nextafter(x.x,y.x), nextafter(x.y,y.y)); }
METAL_ALWAYS_INLINE float3 nextafter(float3 x, float3 y) { return float3(nextafter(x.x,y.x), nextafter(x.y,y.y), nextafter(x.z,y.z)); }
METAL_ALWAYS_INLINE float4 nextafter(float4 x, float4 y) { return float4(nextafter(x.x,y.x), nextafter(x.y,y.y), nextafter(x.z,y.z), nextafter(x.w,y.w)); }

} // namespace metal
#endif