// simd/packed.h — Packed SIMD types
#ifndef _SIMD_PACKED_H_
#define _SIMD_PACKED_H_
#include <metal/metal_common>

struct simd_packed_float2 { float x, y; };
struct simd_packed_float3 { float x, y, z; };
struct simd_packed_float4 { float x, y, z, w; };
struct simd_packed_half2  { half x, y; };
struct simd_packed_half3  { half x, y, z; };
struct simd_packed_half4  { half x, y, z, w; };

#endif
