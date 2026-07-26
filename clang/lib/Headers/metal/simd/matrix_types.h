// simd/matrix_types.h — SIMD matrix type definitions
#ifndef _SIMD_MATRIX_TYPES_H_
#define _SIMD_MATRIX_TYPES_H_
#include <metal/metal_common>

typedef struct { simd_float4 columns[4]; } simd_float4x4;
typedef struct { simd_float4 columns[3]; } simd_float3x4;
typedef struct { simd_float3 columns[3]; } simd_float3x3;
typedef struct { simd_float3 columns[2]; } simd_float2x3;
typedef struct { simd_float2 columns[2]; } simd_float2x2;
typedef struct { simd_half4 columns[4]; } simd_half4x4;
typedef struct { simd_half4 columns[3]; } simd_half3x4;
typedef struct { simd_half3 columns[3]; } simd_half3x3;
typedef struct { simd_half2 columns[2]; } simd_half2x2;

#endif
