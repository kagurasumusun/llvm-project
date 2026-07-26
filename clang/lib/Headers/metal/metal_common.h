// metal_common — MSL common functions (cleanroom)
#ifndef _METAL_COMMON_H_
#define _METAL_COMMON_H_
#include <metal/metal_math>
namespace metal {
namespace fast {
METAL_FUNC float clamp(float x, float lo, float hi) { return __air_clamp(x, lo, hi, _AIR_FAST_MATH_); }
METAL_FUNC float saturate(float x) { return __air_saturate(x, _AIR_FAST_MATH_); }
METAL_FUNC float mix(float x, float y, float a) { return __air_mix(x, y, a, _AIR_FAST_MATH_); }
METAL_FUNC float sign(float x) { return __air_sign(x, _AIR_FAST_MATH_); }
METAL_FUNC float smoothstep(float e0, float e1, float x) { return __air_smoothstep(e0, e1, x, _AIR_FAST_MATH_); }
METAL_FUNC float step(float edge, float x) { return __air_step(edge, x, _AIR_FAST_MATH_); }
METAL_FUNC float2 clamp(float2 x, float2 lo, float2 hi) { return __air_clamp(x, lo, hi, _AIR_FAST_MATH_); }
METAL_FUNC float2 saturate(float2 x) { return __air_saturate(x, _AIR_FAST_MATH_); }
METAL_FUNC float2 mix(float2 x, float2 y, float2 a) { return __air_mix(x, y, a, _AIR_FAST_MATH_); }
METAL_FUNC float2 sign(float2 x) { return __air_sign(x, _AIR_FAST_MATH_); }
METAL_FUNC float2 smoothstep(float2 e0, float2 e1, float2 x) { return __air_smoothstep(e0, e1, x, _AIR_FAST_MATH_); }
METAL_FUNC float2 step(float2 edge, float2 x) { return __air_step(edge, x, _AIR_FAST_MATH_); }
METAL_FUNC float3 clamp(float3 x, float3 lo, float3 hi) { return __air_clamp(x, lo, hi, _AIR_FAST_MATH_); }
METAL_FUNC float3 saturate(float3 x) { return __air_saturate(x, _AIR_FAST_MATH_); }
METAL_FUNC float3 mix(float3 x, float3 y, float3 a) { return __air_mix(x, y, a, _AIR_FAST_MATH_); }
METAL_FUNC float3 sign(float3 x) { return __air_sign(x, _AIR_FAST_MATH_); }
METAL_FUNC float3 smoothstep(float3 e0, float3 e1, float3 x) { return __air_smoothstep(e0, e1, x, _AIR_FAST_MATH_); }
METAL_FUNC float3 step(float3 edge, float3 x) { return __air_step(edge, x, _AIR_FAST_MATH_); }
METAL_FUNC float4 clamp(float4 x, float4 lo, float4 hi) { return __air_clamp(x, lo, hi, _AIR_FAST_MATH_); }
METAL_FUNC float4 saturate(float4 x) { return __air_saturate(x, _AIR_FAST_MATH_); }
METAL_FUNC float4 mix(float4 x, float4 y, float4 a) { return __air_mix(x, y, a, _AIR_FAST_MATH_); }
METAL_FUNC float4 sign(float4 x) { return __air_sign(x, _AIR_FAST_MATH_); }
METAL_FUNC float4 smoothstep(float4 e0, float4 e1, float4 x) { return __air_smoothstep(e0, e1, x, _AIR_FAST_MATH_); }
METAL_FUNC float4 step(float4 edge, float4 x) { return __air_step(edge, x, _AIR_FAST_MATH_); }
METAL_FUNC float8 clamp(float8 x, float8 lo, float8 hi) { return __air_clamp(x, lo, hi, _AIR_FAST_MATH_); }
METAL_FUNC float8 saturate(float8 x) { return __air_saturate(x, _AIR_FAST_MATH_); }
METAL_FUNC float8 mix(float8 x, float8 y, float8 a) { return __air_mix(x, y, a, _AIR_FAST_MATH_); }
METAL_FUNC float8 sign(float8 x) { return __air_sign(x, _AIR_FAST_MATH_); }
METAL_FUNC float8 smoothstep(float8 e0, float8 e1, float8 x) { return __air_smoothstep(e0, e1, x, _AIR_FAST_MATH_); }
METAL_FUNC float8 step(float8 edge, float8 x) { return __air_step(edge, x, _AIR_FAST_MATH_); }
METAL_FUNC float16 clamp(float16 x, float16 lo, float16 hi) { return __air_clamp(x, lo, hi, _AIR_FAST_MATH_); }
METAL_FUNC float16 saturate(float16 x) { return __air_saturate(x, _AIR_FAST_MATH_); }
METAL_FUNC float16 mix(float16 x, float16 y, float16 a) { return __air_mix(x, y, a, _AIR_FAST_MATH_); }
METAL_FUNC float16 sign(float16 x) { return __air_sign(x, _AIR_FAST_MATH_); }
METAL_FUNC float16 smoothstep(float16 e0, float16 e1, float16 x) { return __air_smoothstep(e0, e1, x, _AIR_FAST_MATH_); }
METAL_FUNC float16 step(float16 edge, float16 x) { return __air_step(edge, x, _AIR_FAST_MATH_); }
} // namespace fast

namespace precise {
METAL_FUNC float clamp(float x, float lo, float hi) { return __air_clamp(x, lo, hi, _AIR_PRECISE_MATH_); }
METAL_FUNC float saturate(float x) { return __air_saturate(x, _AIR_PRECISE_MATH_); }
METAL_FUNC float mix(float x, float y, float a) { return __air_mix(x, y, a, _AIR_PRECISE_MATH_); }
METAL_FUNC float sign(float x) { return __air_sign(x, _AIR_PRECISE_MATH_); }
METAL_FUNC float smoothstep(float e0, float e1, float x) { return __air_smoothstep(e0, e1, x, _AIR_PRECISE_MATH_); }
METAL_FUNC float step(float edge, float x) { return __air_step(edge, x, _AIR_PRECISE_MATH_); }
METAL_FUNC float2 clamp(float2 x, float2 lo, float2 hi) { return __air_clamp(x, lo, hi, _AIR_PRECISE_MATH_); }
METAL_FUNC float2 saturate(float2 x) { return __air_saturate(x, _AIR_PRECISE_MATH_); }
METAL_FUNC float2 mix(float2 x, float2 y, float2 a) { return __air_mix(x, y, a, _AIR_PRECISE_MATH_); }
METAL_FUNC float2 sign(float2 x) { return __air_sign(x, _AIR_PRECISE_MATH_); }
METAL_FUNC float2 smoothstep(float2 e0, float2 e1, float2 x) { return __air_smoothstep(e0, e1, x, _AIR_PRECISE_MATH_); }
METAL_FUNC float2 step(float2 edge, float2 x) { return __air_step(edge, x, _AIR_PRECISE_MATH_); }
METAL_FUNC float3 clamp(float3 x, float3 lo, float3 hi) { return __air_clamp(x, lo, hi, _AIR_PRECISE_MATH_); }
METAL_FUNC float3 saturate(float3 x) { return __air_saturate(x, _AIR_PRECISE_MATH_); }
METAL_FUNC float3 mix(float3 x, float3 y, float3 a) { return __air_mix(x, y, a, _AIR_PRECISE_MATH_); }
METAL_FUNC float3 sign(float3 x) { return __air_sign(x, _AIR_PRECISE_MATH_); }
METAL_FUNC float3 smoothstep(float3 e0, float3 e1, float3 x) { return __air_smoothstep(e0, e1, x, _AIR_PRECISE_MATH_); }
METAL_FUNC float3 step(float3 edge, float3 x) { return __air_step(edge, x, _AIR_PRECISE_MATH_); }
METAL_FUNC float4 clamp(float4 x, float4 lo, float4 hi) { return __air_clamp(x, lo, hi, _AIR_PRECISE_MATH_); }
METAL_FUNC float4 saturate(float4 x) { return __air_saturate(x, _AIR_PRECISE_MATH_); }
METAL_FUNC float4 mix(float4 x, float4 y, float4 a) { return __air_mix(x, y, a, _AIR_PRECISE_MATH_); }
METAL_FUNC float4 sign(float4 x) { return __air_sign(x, _AIR_PRECISE_MATH_); }
METAL_FUNC float4 smoothstep(float4 e0, float4 e1, float4 x) { return __air_smoothstep(e0, e1, x, _AIR_PRECISE_MATH_); }
METAL_FUNC float4 step(float4 edge, float4 x) { return __air_step(edge, x, _AIR_PRECISE_MATH_); }
METAL_FUNC float8 clamp(float8 x, float8 lo, float8 hi) { return __air_clamp(x, lo, hi, _AIR_PRECISE_MATH_); }
METAL_FUNC float8 saturate(float8 x) { return __air_saturate(x, _AIR_PRECISE_MATH_); }
METAL_FUNC float8 mix(float8 x, float8 y, float8 a) { return __air_mix(x, y, a, _AIR_PRECISE_MATH_); }
METAL_FUNC float8 sign(float8 x) { return __air_sign(x, _AIR_PRECISE_MATH_); }
METAL_FUNC float8 smoothstep(float8 e0, float8 e1, float8 x) { return __air_smoothstep(e0, e1, x, _AIR_PRECISE_MATH_); }
METAL_FUNC float8 step(float8 edge, float8 x) { return __air_step(edge, x, _AIR_PRECISE_MATH_); }
METAL_FUNC float16 clamp(float16 x, float16 lo, float16 hi) { return __air_clamp(x, lo, hi, _AIR_PRECISE_MATH_); }
METAL_FUNC float16 saturate(float16 x) { return __air_saturate(x, _AIR_PRECISE_MATH_); }
METAL_FUNC float16 mix(float16 x, float16 y, float16 a) { return __air_mix(x, y, a, _AIR_PRECISE_MATH_); }
METAL_FUNC float16 sign(float16 x) { return __air_sign(x, _AIR_PRECISE_MATH_); }
METAL_FUNC float16 smoothstep(float16 e0, float16 e1, float16 x) { return __air_smoothstep(e0, e1, x, _AIR_PRECISE_MATH_); }
METAL_FUNC float16 step(float16 edge, float16 x) { return __air_step(edge, x, _AIR_PRECISE_MATH_); }
METAL_FUNC half clamp(half x, half lo, half hi) { return __air_clamp(x, lo, hi, _AIR_PRECISE_MATH_); }
METAL_FUNC half saturate(half x) { return __air_saturate(x, _AIR_PRECISE_MATH_); }
METAL_FUNC half mix(half x, half y, half a) { return __air_mix(x, y, a, _AIR_PRECISE_MATH_); }
METAL_FUNC half sign(half x) { return __air_sign(x, _AIR_PRECISE_MATH_); }
METAL_FUNC half smoothstep(half e0, half e1, half x) { return __air_smoothstep(e0, e1, x, _AIR_PRECISE_MATH_); }
METAL_FUNC half step(half edge, half x) { return __air_step(edge, x, _AIR_PRECISE_MATH_); }
METAL_FUNC half2 clamp(half2 x, half2 lo, half2 hi) { return __air_clamp(x, lo, hi, _AIR_PRECISE_MATH_); }
METAL_FUNC half2 saturate(half2 x) { return __air_saturate(x, _AIR_PRECISE_MATH_); }
METAL_FUNC half2 mix(half2 x, half2 y, half2 a) { return __air_mix(x, y, a, _AIR_PRECISE_MATH_); }
METAL_FUNC half2 sign(half2 x) { return __air_sign(x, _AIR_PRECISE_MATH_); }
METAL_FUNC half2 smoothstep(half2 e0, half2 e1, half2 x) { return __air_smoothstep(e0, e1, x, _AIR_PRECISE_MATH_); }
METAL_FUNC half2 step(half2 edge, half2 x) { return __air_step(edge, x, _AIR_PRECISE_MATH_); }
METAL_FUNC half3 clamp(half3 x, half3 lo, half3 hi) { return __air_clamp(x, lo, hi, _AIR_PRECISE_MATH_); }
METAL_FUNC half3 saturate(half3 x) { return __air_saturate(x, _AIR_PRECISE_MATH_); }
METAL_FUNC half3 mix(half3 x, half3 y, half3 a) { return __air_mix(x, y, a, _AIR_PRECISE_MATH_); }
METAL_FUNC half3 sign(half3 x) { return __air_sign(x, _AIR_PRECISE_MATH_); }
METAL_FUNC half3 smoothstep(half3 e0, half3 e1, half3 x) { return __air_smoothstep(e0, e1, x, _AIR_PRECISE_MATH_); }
METAL_FUNC half3 step(half3 edge, half3 x) { return __air_step(edge, x, _AIR_PRECISE_MATH_); }
METAL_FUNC half4 clamp(half4 x, half4 lo, half4 hi) { return __air_clamp(x, lo, hi, _AIR_PRECISE_MATH_); }
METAL_FUNC half4 saturate(half4 x) { return __air_saturate(x, _AIR_PRECISE_MATH_); }
METAL_FUNC half4 mix(half4 x, half4 y, half4 a) { return __air_mix(x, y, a, _AIR_PRECISE_MATH_); }
METAL_FUNC half4 sign(half4 x) { return __air_sign(x, _AIR_PRECISE_MATH_); }
METAL_FUNC half4 smoothstep(half4 e0, half4 e1, half4 x) { return __air_smoothstep(e0, e1, x, _AIR_PRECISE_MATH_); }
METAL_FUNC half4 step(half4 edge, half4 x) { return __air_step(edge, x, _AIR_PRECISE_MATH_); }
METAL_FUNC half8 clamp(half8 x, half8 lo, half8 hi) { return __air_clamp(x, lo, hi, _AIR_PRECISE_MATH_); }
METAL_FUNC half8 saturate(half8 x) { return __air_saturate(x, _AIR_PRECISE_MATH_); }
METAL_FUNC half8 mix(half8 x, half8 y, half8 a) { return __air_mix(x, y, a, _AIR_PRECISE_MATH_); }
METAL_FUNC half8 sign(half8 x) { return __air_sign(x, _AIR_PRECISE_MATH_); }
METAL_FUNC half8 smoothstep(half8 e0, half8 e1, half8 x) { return __air_smoothstep(e0, e1, x, _AIR_PRECISE_MATH_); }
METAL_FUNC half8 step(half8 edge, half8 x) { return __air_step(edge, x, _AIR_PRECISE_MATH_); }
METAL_FUNC half16 clamp(half16 x, half16 lo, half16 hi) { return __air_clamp(x, lo, hi, _AIR_PRECISE_MATH_); }
METAL_FUNC half16 saturate(half16 x) { return __air_saturate(x, _AIR_PRECISE_MATH_); }
METAL_FUNC half16 mix(half16 x, half16 y, half16 a) { return __air_mix(x, y, a, _AIR_PRECISE_MATH_); }
METAL_FUNC half16 sign(half16 x) { return __air_sign(x, _AIR_PRECISE_MATH_); }
METAL_FUNC half16 smoothstep(half16 e0, half16 e1, half16 x) { return __air_smoothstep(e0, e1, x, _AIR_PRECISE_MATH_); }
METAL_FUNC half16 step(half16 edge, half16 x) { return __air_step(edge, x, _AIR_PRECISE_MATH_); }
} // namespace precise

METAL_FUNC float clamp(float x, float lo, float hi) { return __air_clamp(x, lo, hi, _AIR_ACTIVE_MATH_); }
METAL_FUNC float saturate(float x) { return __air_saturate(x, _AIR_ACTIVE_MATH_); }
METAL_FUNC float mix(float x, float y, float a) { return __air_mix(x, y, a, _AIR_ACTIVE_MATH_); }
METAL_FUNC float sign(float x) { return __air_sign(x, _AIR_ACTIVE_MATH_); }
METAL_FUNC float smoothstep(float e0, float e1, float x) { return __air_smoothstep(e0, e1, x, _AIR_ACTIVE_MATH_); }
METAL_FUNC float step(float edge, float x) { return __air_step(edge, x, _AIR_ACTIVE_MATH_); }
METAL_FUNC float2 clamp(float2 x, float2 lo, float2 hi) { return __air_clamp(x, lo, hi, _AIR_ACTIVE_MATH_); }
METAL_FUNC float2 saturate(float2 x) { return __air_saturate(x, _AIR_ACTIVE_MATH_); }
METAL_FUNC float2 mix(float2 x, float2 y, float2 a) { return __air_mix(x, y, a, _AIR_ACTIVE_MATH_); }
METAL_FUNC float2 sign(float2 x) { return __air_sign(x, _AIR_ACTIVE_MATH_); }
METAL_FUNC float2 smoothstep(float2 e0, float2 e1, float2 x) { return __air_smoothstep(e0, e1, x, _AIR_ACTIVE_MATH_); }
METAL_FUNC float2 step(float2 edge, float2 x) { return __air_step(edge, x, _AIR_ACTIVE_MATH_); }
METAL_FUNC float3 clamp(float3 x, float3 lo, float3 hi) { return __air_clamp(x, lo, hi, _AIR_ACTIVE_MATH_); }
METAL_FUNC float3 saturate(float3 x) { return __air_saturate(x, _AIR_ACTIVE_MATH_); }
METAL_FUNC float3 mix(float3 x, float3 y, float3 a) { return __air_mix(x, y, a, _AIR_ACTIVE_MATH_); }
METAL_FUNC float3 sign(float3 x) { return __air_sign(x, _AIR_ACTIVE_MATH_); }
METAL_FUNC float3 smoothstep(float3 e0, float3 e1, float3 x) { return __air_smoothstep(e0, e1, x, _AIR_ACTIVE_MATH_); }
METAL_FUNC float3 step(float3 edge, float3 x) { return __air_step(edge, x, _AIR_ACTIVE_MATH_); }
METAL_FUNC float4 clamp(float4 x, float4 lo, float4 hi) { return __air_clamp(x, lo, hi, _AIR_ACTIVE_MATH_); }
METAL_FUNC float4 saturate(float4 x) { return __air_saturate(x, _AIR_ACTIVE_MATH_); }
METAL_FUNC float4 mix(float4 x, float4 y, float4 a) { return __air_mix(x, y, a, _AIR_ACTIVE_MATH_); }
METAL_FUNC float4 sign(float4 x) { return __air_sign(x, _AIR_ACTIVE_MATH_); }
METAL_FUNC float4 smoothstep(float4 e0, float4 e1, float4 x) { return __air_smoothstep(e0, e1, x, _AIR_ACTIVE_MATH_); }
METAL_FUNC float4 step(float4 edge, float4 x) { return __air_step(edge, x, _AIR_ACTIVE_MATH_); }
METAL_FUNC float8 clamp(float8 x, float8 lo, float8 hi) { return __air_clamp(x, lo, hi, _AIR_ACTIVE_MATH_); }
METAL_FUNC float8 saturate(float8 x) { return __air_saturate(x, _AIR_ACTIVE_MATH_); }
METAL_FUNC float8 mix(float8 x, float8 y, float8 a) { return __air_mix(x, y, a, _AIR_ACTIVE_MATH_); }
METAL_FUNC float8 sign(float8 x) { return __air_sign(x, _AIR_ACTIVE_MATH_); }
METAL_FUNC float8 smoothstep(float8 e0, float8 e1, float8 x) { return __air_smoothstep(e0, e1, x, _AIR_ACTIVE_MATH_); }
METAL_FUNC float8 step(float8 edge, float8 x) { return __air_step(edge, x, _AIR_ACTIVE_MATH_); }
METAL_FUNC float16 clamp(float16 x, float16 lo, float16 hi) { return __air_clamp(x, lo, hi, _AIR_ACTIVE_MATH_); }
METAL_FUNC float16 saturate(float16 x) { return __air_saturate(x, _AIR_ACTIVE_MATH_); }
METAL_FUNC float16 mix(float16 x, float16 y, float16 a) { return __air_mix(x, y, a, _AIR_ACTIVE_MATH_); }
METAL_FUNC float16 sign(float16 x) { return __air_sign(x, _AIR_ACTIVE_MATH_); }
METAL_FUNC float16 smoothstep(float16 e0, float16 e1, float16 x) { return __air_smoothstep(e0, e1, x, _AIR_ACTIVE_MATH_); }
METAL_FUNC float16 step(float16 edge, float16 x) { return __air_step(edge, x, _AIR_ACTIVE_MATH_); }
METAL_FUNC half clamp(half x, half lo, half hi) { return __air_clamp(x, lo, hi, _AIR_ACTIVE_MATH_); }
METAL_FUNC half saturate(half x) { return __air_saturate(x, _AIR_ACTIVE_MATH_); }
METAL_FUNC half mix(half x, half y, half a) { return __air_mix(x, y, a, _AIR_ACTIVE_MATH_); }
METAL_FUNC half sign(half x) { return __air_sign(x, _AIR_ACTIVE_MATH_); }
METAL_FUNC half smoothstep(half e0, half e1, half x) { return __air_smoothstep(e0, e1, x, _AIR_ACTIVE_MATH_); }
METAL_FUNC half step(half edge, half x) { return __air_step(edge, x, _AIR_ACTIVE_MATH_); }
METAL_FUNC half2 clamp(half2 x, half2 lo, half2 hi) { return __air_clamp(x, lo, hi, _AIR_ACTIVE_MATH_); }
METAL_FUNC half2 saturate(half2 x) { return __air_saturate(x, _AIR_ACTIVE_MATH_); }
METAL_FUNC half2 mix(half2 x, half2 y, half2 a) { return __air_mix(x, y, a, _AIR_ACTIVE_MATH_); }
METAL_FUNC half2 sign(half2 x) { return __air_sign(x, _AIR_ACTIVE_MATH_); }
METAL_FUNC half2 smoothstep(half2 e0, half2 e1, half2 x) { return __air_smoothstep(e0, e1, x, _AIR_ACTIVE_MATH_); }
METAL_FUNC half2 step(half2 edge, half2 x) { return __air_step(edge, x, _AIR_ACTIVE_MATH_); }
METAL_FUNC half3 clamp(half3 x, half3 lo, half3 hi) { return __air_clamp(x, lo, hi, _AIR_ACTIVE_MATH_); }
METAL_FUNC half3 saturate(half3 x) { return __air_saturate(x, _AIR_ACTIVE_MATH_); }
METAL_FUNC half3 mix(half3 x, half3 y, half3 a) { return __air_mix(x, y, a, _AIR_ACTIVE_MATH_); }
METAL_FUNC half3 sign(half3 x) { return __air_sign(x, _AIR_ACTIVE_MATH_); }
METAL_FUNC half3 smoothstep(half3 e0, half3 e1, half3 x) { return __air_smoothstep(e0, e1, x, _AIR_ACTIVE_MATH_); }
METAL_FUNC half3 step(half3 edge, half3 x) { return __air_step(edge, x, _AIR_ACTIVE_MATH_); }
METAL_FUNC half4 clamp(half4 x, half4 lo, half4 hi) { return __air_clamp(x, lo, hi, _AIR_ACTIVE_MATH_); }
METAL_FUNC half4 saturate(half4 x) { return __air_saturate(x, _AIR_ACTIVE_MATH_); }
METAL_FUNC half4 mix(half4 x, half4 y, half4 a) { return __air_mix(x, y, a, _AIR_ACTIVE_MATH_); }
METAL_FUNC half4 sign(half4 x) { return __air_sign(x, _AIR_ACTIVE_MATH_); }
METAL_FUNC half4 smoothstep(half4 e0, half4 e1, half4 x) { return __air_smoothstep(e0, e1, x, _AIR_ACTIVE_MATH_); }
METAL_FUNC half4 step(half4 edge, half4 x) { return __air_step(edge, x, _AIR_ACTIVE_MATH_); }
METAL_FUNC half8 clamp(half8 x, half8 lo, half8 hi) { return __air_clamp(x, lo, hi, _AIR_ACTIVE_MATH_); }
METAL_FUNC half8 saturate(half8 x) { return __air_saturate(x, _AIR_ACTIVE_MATH_); }
METAL_FUNC half8 mix(half8 x, half8 y, half8 a) { return __air_mix(x, y, a, _AIR_ACTIVE_MATH_); }
METAL_FUNC half8 sign(half8 x) { return __air_sign(x, _AIR_ACTIVE_MATH_); }
METAL_FUNC half8 smoothstep(half8 e0, half8 e1, half8 x) { return __air_smoothstep(e0, e1, x, _AIR_ACTIVE_MATH_); }
METAL_FUNC half8 step(half8 edge, half8 x) { return __air_step(edge, x, _AIR_ACTIVE_MATH_); }
METAL_FUNC half16 clamp(half16 x, half16 lo, half16 hi) { return __air_clamp(x, lo, hi, _AIR_ACTIVE_MATH_); }
METAL_FUNC half16 saturate(half16 x) { return __air_saturate(x, _AIR_ACTIVE_MATH_); }
METAL_FUNC half16 mix(half16 x, half16 y, half16 a) { return __air_mix(x, y, a, _AIR_ACTIVE_MATH_); }
METAL_FUNC half16 sign(half16 x) { return __air_sign(x, _AIR_ACTIVE_MATH_); }
METAL_FUNC half16 smoothstep(half16 e0, half16 e1, half16 x) { return __air_smoothstep(e0, e1, x, _AIR_ACTIVE_MATH_); }
METAL_FUNC half16 step(half16 edge, half16 x) { return __air_step(edge, x, _AIR_ACTIVE_MATH_); }

} // namespace metal
#endif