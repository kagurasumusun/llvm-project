//===----------------------------------------------------------------------===//
// metal_math.h — Metal Shading Language math functions (cleanroom)
// Independently written. ABI-compatible with Apple metalfe 32023.883.
// Uses __air_* builtins (our own compiler intrinsics) instead of __metal_*.
// Each __air_* builtin maps to the same air.* intrinsic as Apple's __metal_*.
//
// Math mode: _AIR_FAST_MATH_ / _AIR_PRECISE_MATH_ (our own naming)
// MSL Spec 4.1 §8.1 compliance
//===----------------------------------------------------------------------===//

#ifndef _METAL_MATH_H_
#define _METAL_MATH_H_
#include <metal/metal_limits>
#include <metal/metal_relational>

// AIR math mode macros (independent naming, same semantics as Apple's)
#if defined(__AIR_MATH_FP32_FUNCTIONS_FAST__)
#define _AIR_FAST_MATH_ 1
#define _AIR_PRECISE_MATH_ 0
#else
#define _AIR_FAST_MATH_ 0
#define _AIR_PRECISE_MATH_ 1
#endif

// MSL Spec: fast/precise namespaces control math precision
// The default namespace uses whichever mode is active
#define _AIR_ACTIVE_MATH_ (__AIR_MATH_FP32_FUNCTIONS_FAST__ ? _AIR_FAST_MATH_ : _AIR_PRECISE_MATH_)

namespace metal {

namespace fast {
METAL_ALWAYS_INLINE float abs(float x) { return __air_abs(x, _AIR_FAST_MATH_); }
METAL_ALWAYS_INLINE float2 abs(float2 x) { return __air_abs(x, _AIR_FAST_MATH_); }
METAL_ALWAYS_INLINE float3 abs(float3 x) { return __air_abs(x, _AIR_FAST_MATH_); }
METAL_ALWAYS_INLINE float4 abs(float4 x) { return __air_abs(x, _AIR_FAST_MATH_); }
METAL_ALWAYS_INLINE float8 abs(float8 x) { return __air_abs(x, _AIR_FAST_MATH_); }
METAL_ALWAYS_INLINE float16 abs(float16 x) { return __air_abs(x, _AIR_FAST_MATH_); }
METAL_ALWAYS_INLINE float acos(float x) { return __air_acos(x, _AIR_FAST_MATH_); }
METAL_ALWAYS_INLINE float2 acos(float2 x) { return __air_acos(x, _AIR_FAST_MATH_); }
METAL_ALWAYS_INLINE float3 acos(float3 x) { return __air_acos(x, _AIR_FAST_MATH_); }
METAL_ALWAYS_INLINE float4 acos(float4 x) { return __air_acos(x, _AIR_FAST_MATH_); }
METAL_ALWAYS_INLINE float8 acos(float8 x) { return __air_acos(x, _AIR_FAST_MATH_); }
METAL_ALWAYS_INLINE float16 acos(float16 x) { return __air_acos(x, _AIR_FAST_MATH_); }
METAL_ALWAYS_INLINE float acosh(float x) { return __air_acosh(x, _AIR_FAST_MATH_); }
METAL_ALWAYS_INLINE float2 acosh(float2 x) { return __air_acosh(x, _AIR_FAST_MATH_); }
METAL_ALWAYS_INLINE float3 acosh(float3 x) { return __air_acosh(x, _AIR_FAST_MATH_); }
METAL_ALWAYS_INLINE float4 acosh(float4 x) { return __air_acosh(x, _AIR_FAST_MATH_); }
METAL_ALWAYS_INLINE float8 acosh(float8 x) { return __air_acosh(x, _AIR_FAST_MATH_); }
METAL_ALWAYS_INLINE float16 acosh(float16 x) { return __air_acosh(x, _AIR_FAST_MATH_); }
METAL_ALWAYS_INLINE float asin(float x) { return __air_asin(x, _AIR_FAST_MATH_); }
METAL_ALWAYS_INLINE float2 asin(float2 x) { return __air_asin(x, _AIR_FAST_MATH_); }
METAL_ALWAYS_INLINE float3 asin(float3 x) { return __air_asin(x, _AIR_FAST_MATH_); }
METAL_ALWAYS_INLINE float4 asin(float4 x) { return __air_asin(x, _AIR_FAST_MATH_); }
METAL_ALWAYS_INLINE float8 asin(float8 x) { return __air_asin(x, _AIR_FAST_MATH_); }
METAL_ALWAYS_INLINE float16 asin(float16 x) { return __air_asin(x, _AIR_FAST_MATH_); }
METAL_ALWAYS_INLINE float asinh(float x) { return __air_asinh(x, _AIR_FAST_MATH_); }
METAL_ALWAYS_INLINE float2 asinh(float2 x) { return __air_asinh(x, _AIR_FAST_MATH_); }
METAL_ALWAYS_INLINE float3 asinh(float3 x) { return __air_asinh(x, _AIR_FAST_MATH_); }
METAL_ALWAYS_INLINE float4 asinh(float4 x) { return __air_asinh(x, _AIR_FAST_MATH_); }
METAL_ALWAYS_INLINE float8 asinh(float8 x) { return __air_asinh(x, _AIR_FAST_MATH_); }
METAL_ALWAYS_INLINE float16 asinh(float16 x) { return __air_asinh(x, _AIR_FAST_MATH_); }
METAL_ALWAYS_INLINE float atan(float x) { return __air_atan(x, _AIR_FAST_MATH_); }
METAL_ALWAYS_INLINE float2 atan(float2 x) { return __air_atan(x, _AIR_FAST_MATH_); }
METAL_ALWAYS_INLINE float3 atan(float3 x) { return __air_atan(x, _AIR_FAST_MATH_); }
METAL_ALWAYS_INLINE float4 atan(float4 x) { return __air_atan(x, _AIR_FAST_MATH_); }
METAL_ALWAYS_INLINE float8 atan(float8 x) { return __air_atan(x, _AIR_FAST_MATH_); }
METAL_ALWAYS_INLINE float16 atan(float16 x) { return __air_atan(x, _AIR_FAST_MATH_); }
METAL_ALWAYS_INLINE float atanh(float x) { return __air_atanh(x, _AIR_FAST_MATH_); }
METAL_ALWAYS_INLINE float2 atanh(float2 x) { return __air_atanh(x, _AIR_FAST_MATH_); }
METAL_ALWAYS_INLINE float3 atanh(float3 x) { return __air_atanh(x, _AIR_FAST_MATH_); }
METAL_ALWAYS_INLINE float4 atanh(float4 x) { return __air_atanh(x, _AIR_FAST_MATH_); }
METAL_ALWAYS_INLINE float8 atanh(float8 x) { return __air_atanh(x, _AIR_FAST_MATH_); }
METAL_ALWAYS_INLINE float16 atanh(float16 x) { return __air_atanh(x, _AIR_FAST_MATH_); }
METAL_ALWAYS_INLINE float ceil(float x) { return __air_ceil(x, _AIR_FAST_MATH_); }
METAL_ALWAYS_INLINE float2 ceil(float2 x) { return __air_ceil(x, _AIR_FAST_MATH_); }
METAL_ALWAYS_INLINE float3 ceil(float3 x) { return __air_ceil(x, _AIR_FAST_MATH_); }
METAL_ALWAYS_INLINE float4 ceil(float4 x) { return __air_ceil(x, _AIR_FAST_MATH_); }
METAL_ALWAYS_INLINE float8 ceil(float8 x) { return __air_ceil(x, _AIR_FAST_MATH_); }
METAL_ALWAYS_INLINE float16 ceil(float16 x) { return __air_ceil(x, _AIR_FAST_MATH_); }
METAL_ALWAYS_INLINE float cos(float x) { return __air_cos(x, _AIR_FAST_MATH_); }
METAL_ALWAYS_INLINE float2 cos(float2 x) { return __air_cos(x, _AIR_FAST_MATH_); }
METAL_ALWAYS_INLINE float3 cos(float3 x) { return __air_cos(x, _AIR_FAST_MATH_); }
METAL_ALWAYS_INLINE float4 cos(float4 x) { return __air_cos(x, _AIR_FAST_MATH_); }
METAL_ALWAYS_INLINE float8 cos(float8 x) { return __air_cos(x, _AIR_FAST_MATH_); }
METAL_ALWAYS_INLINE float16 cos(float16 x) { return __air_cos(x, _AIR_FAST_MATH_); }
METAL_ALWAYS_INLINE float cosh(float x) { return __air_cosh(x, _AIR_FAST_MATH_); }
METAL_ALWAYS_INLINE float2 cosh(float2 x) { return __air_cosh(x, _AIR_FAST_MATH_); }
METAL_ALWAYS_INLINE float3 cosh(float3 x) { return __air_cosh(x, _AIR_FAST_MATH_); }
METAL_ALWAYS_INLINE float4 cosh(float4 x) { return __air_cosh(x, _AIR_FAST_MATH_); }
METAL_ALWAYS_INLINE float8 cosh(float8 x) { return __air_cosh(x, _AIR_FAST_MATH_); }
METAL_ALWAYS_INLINE float16 cosh(float16 x) { return __air_cosh(x, _AIR_FAST_MATH_); }
METAL_ALWAYS_INLINE float cospi(float x) { return __air_cospi(x, _AIR_FAST_MATH_); }
METAL_ALWAYS_INLINE float2 cospi(float2 x) { return __air_cospi(x, _AIR_FAST_MATH_); }
METAL_ALWAYS_INLINE float3 cospi(float3 x) { return __air_cospi(x, _AIR_FAST_MATH_); }
METAL_ALWAYS_INLINE float4 cospi(float4 x) { return __air_cospi(x, _AIR_FAST_MATH_); }
METAL_ALWAYS_INLINE float8 cospi(float8 x) { return __air_cospi(x, _AIR_FAST_MATH_); }
METAL_ALWAYS_INLINE float16 cospi(float16 x) { return __air_cospi(x, _AIR_FAST_MATH_); }
METAL_ALWAYS_INLINE float divide(float x) { return __air_divide(x, _AIR_FAST_MATH_); }
METAL_ALWAYS_INLINE float2 divide(float2 x) { return __air_divide(x, _AIR_FAST_MATH_); }
METAL_ALWAYS_INLINE float3 divide(float3 x) { return __air_divide(x, _AIR_FAST_MATH_); }
METAL_ALWAYS_INLINE float4 divide(float4 x) { return __air_divide(x, _AIR_FAST_MATH_); }
METAL_ALWAYS_INLINE float8 divide(float8 x) { return __air_divide(x, _AIR_FAST_MATH_); }
METAL_ALWAYS_INLINE float16 divide(float16 x) { return __air_divide(x, _AIR_FAST_MATH_); }
METAL_ALWAYS_INLINE float exp(float x) { return __air_exp(x, _AIR_FAST_MATH_); }
METAL_ALWAYS_INLINE float2 exp(float2 x) { return __air_exp(x, _AIR_FAST_MATH_); }
METAL_ALWAYS_INLINE float3 exp(float3 x) { return __air_exp(x, _AIR_FAST_MATH_); }
METAL_ALWAYS_INLINE float4 exp(float4 x) { return __air_exp(x, _AIR_FAST_MATH_); }
METAL_ALWAYS_INLINE float8 exp(float8 x) { return __air_exp(x, _AIR_FAST_MATH_); }
METAL_ALWAYS_INLINE float16 exp(float16 x) { return __air_exp(x, _AIR_FAST_MATH_); }
METAL_ALWAYS_INLINE float exp10(float x) { return __air_exp10(x, _AIR_FAST_MATH_); }
METAL_ALWAYS_INLINE float2 exp10(float2 x) { return __air_exp10(x, _AIR_FAST_MATH_); }
METAL_ALWAYS_INLINE float3 exp10(float3 x) { return __air_exp10(x, _AIR_FAST_MATH_); }
METAL_ALWAYS_INLINE float4 exp10(float4 x) { return __air_exp10(x, _AIR_FAST_MATH_); }
METAL_ALWAYS_INLINE float8 exp10(float8 x) { return __air_exp10(x, _AIR_FAST_MATH_); }
METAL_ALWAYS_INLINE float16 exp10(float16 x) { return __air_exp10(x, _AIR_FAST_MATH_); }
METAL_ALWAYS_INLINE float exp2(float x) { return __air_exp2(x, _AIR_FAST_MATH_); }
METAL_ALWAYS_INLINE float2 exp2(float2 x) { return __air_exp2(x, _AIR_FAST_MATH_); }
METAL_ALWAYS_INLINE float3 exp2(float3 x) { return __air_exp2(x, _AIR_FAST_MATH_); }
METAL_ALWAYS_INLINE float4 exp2(float4 x) { return __air_exp2(x, _AIR_FAST_MATH_); }
METAL_ALWAYS_INLINE float8 exp2(float8 x) { return __air_exp2(x, _AIR_FAST_MATH_); }
METAL_ALWAYS_INLINE float16 exp2(float16 x) { return __air_exp2(x, _AIR_FAST_MATH_); }
METAL_ALWAYS_INLINE float fabs(float x) { return __air_fabs(x, _AIR_FAST_MATH_); }
METAL_ALWAYS_INLINE float2 fabs(float2 x) { return __air_fabs(x, _AIR_FAST_MATH_); }
METAL_ALWAYS_INLINE float3 fabs(float3 x) { return __air_fabs(x, _AIR_FAST_MATH_); }
METAL_ALWAYS_INLINE float4 fabs(float4 x) { return __air_fabs(x, _AIR_FAST_MATH_); }
METAL_ALWAYS_INLINE float8 fabs(float8 x) { return __air_fabs(x, _AIR_FAST_MATH_); }
METAL_ALWAYS_INLINE float16 fabs(float16 x) { return __air_fabs(x, _AIR_FAST_MATH_); }
METAL_ALWAYS_INLINE float floor(float x) { return __air_floor(x, _AIR_FAST_MATH_); }
METAL_ALWAYS_INLINE float2 floor(float2 x) { return __air_floor(x, _AIR_FAST_MATH_); }
METAL_ALWAYS_INLINE float3 floor(float3 x) { return __air_floor(x, _AIR_FAST_MATH_); }
METAL_ALWAYS_INLINE float4 floor(float4 x) { return __air_floor(x, _AIR_FAST_MATH_); }
METAL_ALWAYS_INLINE float8 floor(float8 x) { return __air_floor(x, _AIR_FAST_MATH_); }
METAL_ALWAYS_INLINE float16 floor(float16 x) { return __air_floor(x, _AIR_FAST_MATH_); }
METAL_ALWAYS_INLINE float fract(float x) { return __air_fract(x, _AIR_FAST_MATH_); }
METAL_ALWAYS_INLINE float2 fract(float2 x) { return __air_fract(x, _AIR_FAST_MATH_); }
METAL_ALWAYS_INLINE float3 fract(float3 x) { return __air_fract(x, _AIR_FAST_MATH_); }
METAL_ALWAYS_INLINE float4 fract(float4 x) { return __air_fract(x, _AIR_FAST_MATH_); }
METAL_ALWAYS_INLINE float8 fract(float8 x) { return __air_fract(x, _AIR_FAST_MATH_); }
METAL_ALWAYS_INLINE float16 fract(float16 x) { return __air_fract(x, _AIR_FAST_MATH_); }
METAL_ALWAYS_INLINE float ilogb(float x) { return __air_ilogb(x, _AIR_FAST_MATH_); }
METAL_ALWAYS_INLINE float2 ilogb(float2 x) { return __air_ilogb(x, _AIR_FAST_MATH_); }
METAL_ALWAYS_INLINE float3 ilogb(float3 x) { return __air_ilogb(x, _AIR_FAST_MATH_); }
METAL_ALWAYS_INLINE float4 ilogb(float4 x) { return __air_ilogb(x, _AIR_FAST_MATH_); }
METAL_ALWAYS_INLINE float8 ilogb(float8 x) { return __air_ilogb(x, _AIR_FAST_MATH_); }
METAL_ALWAYS_INLINE float16 ilogb(float16 x) { return __air_ilogb(x, _AIR_FAST_MATH_); }
METAL_ALWAYS_INLINE float log(float x) { return __air_log(x, _AIR_FAST_MATH_); }
METAL_ALWAYS_INLINE float2 log(float2 x) { return __air_log(x, _AIR_FAST_MATH_); }
METAL_ALWAYS_INLINE float3 log(float3 x) { return __air_log(x, _AIR_FAST_MATH_); }
METAL_ALWAYS_INLINE float4 log(float4 x) { return __air_log(x, _AIR_FAST_MATH_); }
METAL_ALWAYS_INLINE float8 log(float8 x) { return __air_log(x, _AIR_FAST_MATH_); }
METAL_ALWAYS_INLINE float16 log(float16 x) { return __air_log(x, _AIR_FAST_MATH_); }
METAL_ALWAYS_INLINE float log10(float x) { return __air_log10(x, _AIR_FAST_MATH_); }
METAL_ALWAYS_INLINE float2 log10(float2 x) { return __air_log10(x, _AIR_FAST_MATH_); }
METAL_ALWAYS_INLINE float3 log10(float3 x) { return __air_log10(x, _AIR_FAST_MATH_); }
METAL_ALWAYS_INLINE float4 log10(float4 x) { return __air_log10(x, _AIR_FAST_MATH_); }
METAL_ALWAYS_INLINE float8 log10(float8 x) { return __air_log10(x, _AIR_FAST_MATH_); }
METAL_ALWAYS_INLINE float16 log10(float16 x) { return __air_log10(x, _AIR_FAST_MATH_); }
METAL_ALWAYS_INLINE float log2(float x) { return __air_log2(x, _AIR_FAST_MATH_); }
METAL_ALWAYS_INLINE float2 log2(float2 x) { return __air_log2(x, _AIR_FAST_MATH_); }
METAL_ALWAYS_INLINE float3 log2(float3 x) { return __air_log2(x, _AIR_FAST_MATH_); }
METAL_ALWAYS_INLINE float4 log2(float4 x) { return __air_log2(x, _AIR_FAST_MATH_); }
METAL_ALWAYS_INLINE float8 log2(float8 x) { return __air_log2(x, _AIR_FAST_MATH_); }
METAL_ALWAYS_INLINE float16 log2(float16 x) { return __air_log2(x, _AIR_FAST_MATH_); }
METAL_ALWAYS_INLINE float rint(float x) { return __air_rint(x, _AIR_FAST_MATH_); }
METAL_ALWAYS_INLINE float2 rint(float2 x) { return __air_rint(x, _AIR_FAST_MATH_); }
METAL_ALWAYS_INLINE float3 rint(float3 x) { return __air_rint(x, _AIR_FAST_MATH_); }
METAL_ALWAYS_INLINE float4 rint(float4 x) { return __air_rint(x, _AIR_FAST_MATH_); }
METAL_ALWAYS_INLINE float8 rint(float8 x) { return __air_rint(x, _AIR_FAST_MATH_); }
METAL_ALWAYS_INLINE float16 rint(float16 x) { return __air_rint(x, _AIR_FAST_MATH_); }
METAL_ALWAYS_INLINE float round(float x) { return __air_round(x, _AIR_FAST_MATH_); }
METAL_ALWAYS_INLINE float2 round(float2 x) { return __air_round(x, _AIR_FAST_MATH_); }
METAL_ALWAYS_INLINE float3 round(float3 x) { return __air_round(x, _AIR_FAST_MATH_); }
METAL_ALWAYS_INLINE float4 round(float4 x) { return __air_round(x, _AIR_FAST_MATH_); }
METAL_ALWAYS_INLINE float8 round(float8 x) { return __air_round(x, _AIR_FAST_MATH_); }
METAL_ALWAYS_INLINE float16 round(float16 x) { return __air_round(x, _AIR_FAST_MATH_); }
METAL_ALWAYS_INLINE float rsqrt(float x) { return __air_rsqrt(x, _AIR_FAST_MATH_); }
METAL_ALWAYS_INLINE float2 rsqrt(float2 x) { return __air_rsqrt(x, _AIR_FAST_MATH_); }
METAL_ALWAYS_INLINE float3 rsqrt(float3 x) { return __air_rsqrt(x, _AIR_FAST_MATH_); }
METAL_ALWAYS_INLINE float4 rsqrt(float4 x) { return __air_rsqrt(x, _AIR_FAST_MATH_); }
METAL_ALWAYS_INLINE float8 rsqrt(float8 x) { return __air_rsqrt(x, _AIR_FAST_MATH_); }
METAL_ALWAYS_INLINE float16 rsqrt(float16 x) { return __air_rsqrt(x, _AIR_FAST_MATH_); }
METAL_ALWAYS_INLINE float sin(float x) { return __air_sin(x, _AIR_FAST_MATH_); }
METAL_ALWAYS_INLINE float2 sin(float2 x) { return __air_sin(x, _AIR_FAST_MATH_); }
METAL_ALWAYS_INLINE float3 sin(float3 x) { return __air_sin(x, _AIR_FAST_MATH_); }
METAL_ALWAYS_INLINE float4 sin(float4 x) { return __air_sin(x, _AIR_FAST_MATH_); }
METAL_ALWAYS_INLINE float8 sin(float8 x) { return __air_sin(x, _AIR_FAST_MATH_); }
METAL_ALWAYS_INLINE float16 sin(float16 x) { return __air_sin(x, _AIR_FAST_MATH_); }
METAL_ALWAYS_INLINE float sinh(float x) { return __air_sinh(x, _AIR_FAST_MATH_); }
METAL_ALWAYS_INLINE float2 sinh(float2 x) { return __air_sinh(x, _AIR_FAST_MATH_); }
METAL_ALWAYS_INLINE float3 sinh(float3 x) { return __air_sinh(x, _AIR_FAST_MATH_); }
METAL_ALWAYS_INLINE float4 sinh(float4 x) { return __air_sinh(x, _AIR_FAST_MATH_); }
METAL_ALWAYS_INLINE float8 sinh(float8 x) { return __air_sinh(x, _AIR_FAST_MATH_); }
METAL_ALWAYS_INLINE float16 sinh(float16 x) { return __air_sinh(x, _AIR_FAST_MATH_); }
METAL_ALWAYS_INLINE float sinpi(float x) { return __air_sinpi(x, _AIR_FAST_MATH_); }
METAL_ALWAYS_INLINE float2 sinpi(float2 x) { return __air_sinpi(x, _AIR_FAST_MATH_); }
METAL_ALWAYS_INLINE float3 sinpi(float3 x) { return __air_sinpi(x, _AIR_FAST_MATH_); }
METAL_ALWAYS_INLINE float4 sinpi(float4 x) { return __air_sinpi(x, _AIR_FAST_MATH_); }
METAL_ALWAYS_INLINE float8 sinpi(float8 x) { return __air_sinpi(x, _AIR_FAST_MATH_); }
METAL_ALWAYS_INLINE float16 sinpi(float16 x) { return __air_sinpi(x, _AIR_FAST_MATH_); }
METAL_ALWAYS_INLINE float sqrt(float x) { return __air_sqrt(x, _AIR_FAST_MATH_); }
METAL_ALWAYS_INLINE float2 sqrt(float2 x) { return __air_sqrt(x, _AIR_FAST_MATH_); }
METAL_ALWAYS_INLINE float3 sqrt(float3 x) { return __air_sqrt(x, _AIR_FAST_MATH_); }
METAL_ALWAYS_INLINE float4 sqrt(float4 x) { return __air_sqrt(x, _AIR_FAST_MATH_); }
METAL_ALWAYS_INLINE float8 sqrt(float8 x) { return __air_sqrt(x, _AIR_FAST_MATH_); }
METAL_ALWAYS_INLINE float16 sqrt(float16 x) { return __air_sqrt(x, _AIR_FAST_MATH_); }
METAL_ALWAYS_INLINE float tan(float x) { return __air_tan(x, _AIR_FAST_MATH_); }
METAL_ALWAYS_INLINE float2 tan(float2 x) { return __air_tan(x, _AIR_FAST_MATH_); }
METAL_ALWAYS_INLINE float3 tan(float3 x) { return __air_tan(x, _AIR_FAST_MATH_); }
METAL_ALWAYS_INLINE float4 tan(float4 x) { return __air_tan(x, _AIR_FAST_MATH_); }
METAL_ALWAYS_INLINE float8 tan(float8 x) { return __air_tan(x, _AIR_FAST_MATH_); }
METAL_ALWAYS_INLINE float16 tan(float16 x) { return __air_tan(x, _AIR_FAST_MATH_); }
METAL_ALWAYS_INLINE float tanh(float x) { return __air_tanh(x, _AIR_FAST_MATH_); }
METAL_ALWAYS_INLINE float2 tanh(float2 x) { return __air_tanh(x, _AIR_FAST_MATH_); }
METAL_ALWAYS_INLINE float3 tanh(float3 x) { return __air_tanh(x, _AIR_FAST_MATH_); }
METAL_ALWAYS_INLINE float4 tanh(float4 x) { return __air_tanh(x, _AIR_FAST_MATH_); }
METAL_ALWAYS_INLINE float8 tanh(float8 x) { return __air_tanh(x, _AIR_FAST_MATH_); }
METAL_ALWAYS_INLINE float16 tanh(float16 x) { return __air_tanh(x, _AIR_FAST_MATH_); }
METAL_ALWAYS_INLINE float tanpi(float x) { return __air_tanpi(x, _AIR_FAST_MATH_); }
METAL_ALWAYS_INLINE float2 tanpi(float2 x) { return __air_tanpi(x, _AIR_FAST_MATH_); }
METAL_ALWAYS_INLINE float3 tanpi(float3 x) { return __air_tanpi(x, _AIR_FAST_MATH_); }
METAL_ALWAYS_INLINE float4 tanpi(float4 x) { return __air_tanpi(x, _AIR_FAST_MATH_); }
METAL_ALWAYS_INLINE float8 tanpi(float8 x) { return __air_tanpi(x, _AIR_FAST_MATH_); }
METAL_ALWAYS_INLINE float16 tanpi(float16 x) { return __air_tanpi(x, _AIR_FAST_MATH_); }
METAL_ALWAYS_INLINE float trunc(float x) { return __air_trunc(x, _AIR_FAST_MATH_); }
METAL_ALWAYS_INLINE float2 trunc(float2 x) { return __air_trunc(x, _AIR_FAST_MATH_); }
METAL_ALWAYS_INLINE float3 trunc(float3 x) { return __air_trunc(x, _AIR_FAST_MATH_); }
METAL_ALWAYS_INLINE float4 trunc(float4 x) { return __air_trunc(x, _AIR_FAST_MATH_); }
METAL_ALWAYS_INLINE float8 trunc(float8 x) { return __air_trunc(x, _AIR_FAST_MATH_); }
METAL_ALWAYS_INLINE float16 trunc(float16 x) { return __air_trunc(x, _AIR_FAST_MATH_); }

METAL_ALWAYS_INLINE float atan2(float x, float y) { return __air_atan2(x, y, _AIR_FAST_MATH_); }
METAL_ALWAYS_INLINE float2 atan2(float2 x, float2 y) { return __air_atan2(x, y, _AIR_FAST_MATH_); }
METAL_ALWAYS_INLINE float3 atan2(float3 x, float3 y) { return __air_atan2(x, y, _AIR_FAST_MATH_); }
METAL_ALWAYS_INLINE float4 atan2(float4 x, float4 y) { return __air_atan2(x, y, _AIR_FAST_MATH_); }
METAL_ALWAYS_INLINE float8 atan2(float8 x, float8 y) { return __air_atan2(x, y, _AIR_FAST_MATH_); }
METAL_ALWAYS_INLINE float16 atan2(float16 x, float16 y) { return __air_atan2(x, y, _AIR_FAST_MATH_); }
METAL_ALWAYS_INLINE float copysign(float x, float y) { return __air_copysign(x, y, _AIR_FAST_MATH_); }
METAL_ALWAYS_INLINE float2 copysign(float2 x, float2 y) { return __air_copysign(x, y, _AIR_FAST_MATH_); }
METAL_ALWAYS_INLINE float3 copysign(float3 x, float3 y) { return __air_copysign(x, y, _AIR_FAST_MATH_); }
METAL_ALWAYS_INLINE float4 copysign(float4 x, float4 y) { return __air_copysign(x, y, _AIR_FAST_MATH_); }
METAL_ALWAYS_INLINE float8 copysign(float8 x, float8 y) { return __air_copysign(x, y, _AIR_FAST_MATH_); }
METAL_ALWAYS_INLINE float16 copysign(float16 x, float16 y) { return __air_copysign(x, y, _AIR_FAST_MATH_); }
METAL_ALWAYS_INLINE float fdim(float x, float y) { return __air_fdim(x, y, _AIR_FAST_MATH_); }
METAL_ALWAYS_INLINE float2 fdim(float2 x, float2 y) { return __air_fdim(x, y, _AIR_FAST_MATH_); }
METAL_ALWAYS_INLINE float3 fdim(float3 x, float3 y) { return __air_fdim(x, y, _AIR_FAST_MATH_); }
METAL_ALWAYS_INLINE float4 fdim(float4 x, float4 y) { return __air_fdim(x, y, _AIR_FAST_MATH_); }
METAL_ALWAYS_INLINE float8 fdim(float8 x, float8 y) { return __air_fdim(x, y, _AIR_FAST_MATH_); }
METAL_ALWAYS_INLINE float16 fdim(float16 x, float16 y) { return __air_fdim(x, y, _AIR_FAST_MATH_); }
METAL_ALWAYS_INLINE float fmax(float x, float y) { return __air_fmax(x, y, _AIR_FAST_MATH_); }
METAL_ALWAYS_INLINE float2 fmax(float2 x, float2 y) { return __air_fmax(x, y, _AIR_FAST_MATH_); }
METAL_ALWAYS_INLINE float3 fmax(float3 x, float3 y) { return __air_fmax(x, y, _AIR_FAST_MATH_); }
METAL_ALWAYS_INLINE float4 fmax(float4 x, float4 y) { return __air_fmax(x, y, _AIR_FAST_MATH_); }
METAL_ALWAYS_INLINE float8 fmax(float8 x, float8 y) { return __air_fmax(x, y, _AIR_FAST_MATH_); }
METAL_ALWAYS_INLINE float16 fmax(float16 x, float16 y) { return __air_fmax(x, y, _AIR_FAST_MATH_); }
METAL_ALWAYS_INLINE float fmin(float x, float y) { return __air_fmin(x, y, _AIR_FAST_MATH_); }
METAL_ALWAYS_INLINE float2 fmin(float2 x, float2 y) { return __air_fmin(x, y, _AIR_FAST_MATH_); }
METAL_ALWAYS_INLINE float3 fmin(float3 x, float3 y) { return __air_fmin(x, y, _AIR_FAST_MATH_); }
METAL_ALWAYS_INLINE float4 fmin(float4 x, float4 y) { return __air_fmin(x, y, _AIR_FAST_MATH_); }
METAL_ALWAYS_INLINE float8 fmin(float8 x, float8 y) { return __air_fmin(x, y, _AIR_FAST_MATH_); }
METAL_ALWAYS_INLINE float16 fmin(float16 x, float16 y) { return __air_fmin(x, y, _AIR_FAST_MATH_); }
METAL_ALWAYS_INLINE float fmod(float x, float y) { return __air_fmod(x, y, _AIR_FAST_MATH_); }
METAL_ALWAYS_INLINE float2 fmod(float2 x, float2 y) { return __air_fmod(x, y, _AIR_FAST_MATH_); }
METAL_ALWAYS_INLINE float3 fmod(float3 x, float3 y) { return __air_fmod(x, y, _AIR_FAST_MATH_); }
METAL_ALWAYS_INLINE float4 fmod(float4 x, float4 y) { return __air_fmod(x, y, _AIR_FAST_MATH_); }
METAL_ALWAYS_INLINE float8 fmod(float8 x, float8 y) { return __air_fmod(x, y, _AIR_FAST_MATH_); }
METAL_ALWAYS_INLINE float16 fmod(float16 x, float16 y) { return __air_fmod(x, y, _AIR_FAST_MATH_); }
METAL_ALWAYS_INLINE float max(float x, float y) { return __air_max(x, y, _AIR_FAST_MATH_); }
METAL_ALWAYS_INLINE float2 max(float2 x, float2 y) { return __air_max(x, y, _AIR_FAST_MATH_); }
METAL_ALWAYS_INLINE float3 max(float3 x, float3 y) { return __air_max(x, y, _AIR_FAST_MATH_); }
METAL_ALWAYS_INLINE float4 max(float4 x, float4 y) { return __air_max(x, y, _AIR_FAST_MATH_); }
METAL_ALWAYS_INLINE float8 max(float8 x, float8 y) { return __air_max(x, y, _AIR_FAST_MATH_); }
METAL_ALWAYS_INLINE float16 max(float16 x, float16 y) { return __air_max(x, y, _AIR_FAST_MATH_); }
METAL_ALWAYS_INLINE float min(float x, float y) { return __air_min(x, y, _AIR_FAST_MATH_); }
METAL_ALWAYS_INLINE float2 min(float2 x, float2 y) { return __air_min(x, y, _AIR_FAST_MATH_); }
METAL_ALWAYS_INLINE float3 min(float3 x, float3 y) { return __air_min(x, y, _AIR_FAST_MATH_); }
METAL_ALWAYS_INLINE float4 min(float4 x, float4 y) { return __air_min(x, y, _AIR_FAST_MATH_); }
METAL_ALWAYS_INLINE float8 min(float8 x, float8 y) { return __air_min(x, y, _AIR_FAST_MATH_); }
METAL_ALWAYS_INLINE float16 min(float16 x, float16 y) { return __air_min(x, y, _AIR_FAST_MATH_); }
METAL_ALWAYS_INLINE float nextafter(float x, float y) { return __air_nextafter(x, y, _AIR_FAST_MATH_); }
METAL_ALWAYS_INLINE float2 nextafter(float2 x, float2 y) { return __air_nextafter(x, y, _AIR_FAST_MATH_); }
METAL_ALWAYS_INLINE float3 nextafter(float3 x, float3 y) { return __air_nextafter(x, y, _AIR_FAST_MATH_); }
METAL_ALWAYS_INLINE float4 nextafter(float4 x, float4 y) { return __air_nextafter(x, y, _AIR_FAST_MATH_); }
METAL_ALWAYS_INLINE float8 nextafter(float8 x, float8 y) { return __air_nextafter(x, y, _AIR_FAST_MATH_); }
METAL_ALWAYS_INLINE float16 nextafter(float16 x, float16 y) { return __air_nextafter(x, y, _AIR_FAST_MATH_); }
METAL_ALWAYS_INLINE float pow(float x, float y) { return __air_pow(x, y, _AIR_FAST_MATH_); }
METAL_ALWAYS_INLINE float2 pow(float2 x, float2 y) { return __air_pow(x, y, _AIR_FAST_MATH_); }
METAL_ALWAYS_INLINE float3 pow(float3 x, float3 y) { return __air_pow(x, y, _AIR_FAST_MATH_); }
METAL_ALWAYS_INLINE float4 pow(float4 x, float4 y) { return __air_pow(x, y, _AIR_FAST_MATH_); }
METAL_ALWAYS_INLINE float8 pow(float8 x, float8 y) { return __air_pow(x, y, _AIR_FAST_MATH_); }
METAL_ALWAYS_INLINE float16 pow(float16 x, float16 y) { return __air_pow(x, y, _AIR_FAST_MATH_); }
METAL_ALWAYS_INLINE float powr(float x, float y) { return __air_powr(x, y, _AIR_FAST_MATH_); }
METAL_ALWAYS_INLINE float2 powr(float2 x, float2 y) { return __air_powr(x, y, _AIR_FAST_MATH_); }
METAL_ALWAYS_INLINE float3 powr(float3 x, float3 y) { return __air_powr(x, y, _AIR_FAST_MATH_); }
METAL_ALWAYS_INLINE float4 powr(float4 x, float4 y) { return __air_powr(x, y, _AIR_FAST_MATH_); }
METAL_ALWAYS_INLINE float8 powr(float8 x, float8 y) { return __air_powr(x, y, _AIR_FAST_MATH_); }
METAL_ALWAYS_INLINE float16 powr(float16 x, float16 y) { return __air_powr(x, y, _AIR_FAST_MATH_); }

METAL_ALWAYS_INLINE float fma(float x, float y, float z) { return __air_fma(x, y, z, _AIR_FAST_MATH_); }
METAL_ALWAYS_INLINE float2 fma(float2 x, float2 y, float2 z) { return __air_fma(x, y, z, _AIR_FAST_MATH_); }
METAL_ALWAYS_INLINE float3 fma(float3 x, float3 y, float3 z) { return __air_fma(x, y, z, _AIR_FAST_MATH_); }
METAL_ALWAYS_INLINE float4 fma(float4 x, float4 y, float4 z) { return __air_fma(x, y, z, _AIR_FAST_MATH_); }
METAL_ALWAYS_INLINE float8 fma(float8 x, float8 y, float8 z) { return __air_fma(x, y, z, _AIR_FAST_MATH_); }
METAL_ALWAYS_INLINE float16 fma(float16 x, float16 y, float16 z) { return __air_fma(x, y, z, _AIR_FAST_MATH_); }
METAL_ALWAYS_INLINE float fmax3(float x, float y, float z) { return __air_fmax3(x, y, z, _AIR_FAST_MATH_); }
METAL_ALWAYS_INLINE float2 fmax3(float2 x, float2 y, float2 z) { return __air_fmax3(x, y, z, _AIR_FAST_MATH_); }
METAL_ALWAYS_INLINE float3 fmax3(float3 x, float3 y, float3 z) { return __air_fmax3(x, y, z, _AIR_FAST_MATH_); }
METAL_ALWAYS_INLINE float4 fmax3(float4 x, float4 y, float4 z) { return __air_fmax3(x, y, z, _AIR_FAST_MATH_); }
METAL_ALWAYS_INLINE float8 fmax3(float8 x, float8 y, float8 z) { return __air_fmax3(x, y, z, _AIR_FAST_MATH_); }
METAL_ALWAYS_INLINE float16 fmax3(float16 x, float16 y, float16 z) { return __air_fmax3(x, y, z, _AIR_FAST_MATH_); }
METAL_ALWAYS_INLINE float fmin3(float x, float y, float z) { return __air_fmin3(x, y, z, _AIR_FAST_MATH_); }
METAL_ALWAYS_INLINE float2 fmin3(float2 x, float2 y, float2 z) { return __air_fmin3(x, y, z, _AIR_FAST_MATH_); }
METAL_ALWAYS_INLINE float3 fmin3(float3 x, float3 y, float3 z) { return __air_fmin3(x, y, z, _AIR_FAST_MATH_); }
METAL_ALWAYS_INLINE float4 fmin3(float4 x, float4 y, float4 z) { return __air_fmin3(x, y, z, _AIR_FAST_MATH_); }
METAL_ALWAYS_INLINE float8 fmin3(float8 x, float8 y, float8 z) { return __air_fmin3(x, y, z, _AIR_FAST_MATH_); }
METAL_ALWAYS_INLINE float16 fmin3(float16 x, float16 y, float16 z) { return __air_fmin3(x, y, z, _AIR_FAST_MATH_); }
METAL_ALWAYS_INLINE float fmedian3(float x, float y, float z) { return __air_fmedian3(x, y, z, _AIR_FAST_MATH_); }
METAL_ALWAYS_INLINE float2 fmedian3(float2 x, float2 y, float2 z) { return __air_fmedian3(x, y, z, _AIR_FAST_MATH_); }
METAL_ALWAYS_INLINE float3 fmedian3(float3 x, float3 y, float3 z) { return __air_fmedian3(x, y, z, _AIR_FAST_MATH_); }
METAL_ALWAYS_INLINE float4 fmedian3(float4 x, float4 y, float4 z) { return __air_fmedian3(x, y, z, _AIR_FAST_MATH_); }
METAL_ALWAYS_INLINE float8 fmedian3(float8 x, float8 y, float8 z) { return __air_fmedian3(x, y, z, _AIR_FAST_MATH_); }
METAL_ALWAYS_INLINE float16 fmedian3(float16 x, float16 y, float16 z) { return __air_fmedian3(x, y, z, _AIR_FAST_MATH_); }
METAL_ALWAYS_INLINE float max3(float x, float y, float z) { return __air_max3(x, y, z, _AIR_FAST_MATH_); }
METAL_ALWAYS_INLINE float2 max3(float2 x, float2 y, float2 z) { return __air_max3(x, y, z, _AIR_FAST_MATH_); }
METAL_ALWAYS_INLINE float3 max3(float3 x, float3 y, float3 z) { return __air_max3(x, y, z, _AIR_FAST_MATH_); }
METAL_ALWAYS_INLINE float4 max3(float4 x, float4 y, float4 z) { return __air_max3(x, y, z, _AIR_FAST_MATH_); }
METAL_ALWAYS_INLINE float8 max3(float8 x, float8 y, float8 z) { return __air_max3(x, y, z, _AIR_FAST_MATH_); }
METAL_ALWAYS_INLINE float16 max3(float16 x, float16 y, float16 z) { return __air_max3(x, y, z, _AIR_FAST_MATH_); }
METAL_ALWAYS_INLINE float min3(float x, float y, float z) { return __air_min3(x, y, z, _AIR_FAST_MATH_); }
METAL_ALWAYS_INLINE float2 min3(float2 x, float2 y, float2 z) { return __air_min3(x, y, z, _AIR_FAST_MATH_); }
METAL_ALWAYS_INLINE float3 min3(float3 x, float3 y, float3 z) { return __air_min3(x, y, z, _AIR_FAST_MATH_); }
METAL_ALWAYS_INLINE float4 min3(float4 x, float4 y, float4 z) { return __air_min3(x, y, z, _AIR_FAST_MATH_); }
METAL_ALWAYS_INLINE float8 min3(float8 x, float8 y, float8 z) { return __air_min3(x, y, z, _AIR_FAST_MATH_); }
METAL_ALWAYS_INLINE float16 min3(float16 x, float16 y, float16 z) { return __air_min3(x, y, z, _AIR_FAST_MATH_); }
METAL_ALWAYS_INLINE float median3(float x, float y, float z) { return __air_median3(x, y, z, _AIR_FAST_MATH_); }
METAL_ALWAYS_INLINE float2 median3(float2 x, float2 y, float2 z) { return __air_median3(x, y, z, _AIR_FAST_MATH_); }
METAL_ALWAYS_INLINE float3 median3(float3 x, float3 y, float3 z) { return __air_median3(x, y, z, _AIR_FAST_MATH_); }
METAL_ALWAYS_INLINE float4 median3(float4 x, float4 y, float4 z) { return __air_median3(x, y, z, _AIR_FAST_MATH_); }
METAL_ALWAYS_INLINE float8 median3(float8 x, float8 y, float8 z) { return __air_median3(x, y, z, _AIR_FAST_MATH_); }
METAL_ALWAYS_INLINE float16 median3(float16 x, float16 y, float16 z) { return __air_median3(x, y, z, _AIR_FAST_MATH_); }

METAL_ALWAYS_INLINE float sincos(float x, thread float &cosval) { return __air_sincos(x, &cosval, _AIR_FAST_MATH_); }
METAL_ALWAYS_INLINE float2 sincos(float2 x, thread float2 &cosval) { return __air_sincos(x, &cosval, _AIR_FAST_MATH_); }
METAL_ALWAYS_INLINE float3 sincos(float3 x, thread float3 &cosval) { return __air_sincos(x, &cosval, _AIR_FAST_MATH_); }
METAL_ALWAYS_INLINE float4 sincos(float4 x, thread float4 &cosval) { return __air_sincos(x, &cosval, _AIR_FAST_MATH_); }
METAL_ALWAYS_INLINE float8 sincos(float8 x, thread float8 &cosval) { return __air_sincos(x, &cosval, _AIR_FAST_MATH_); }
METAL_ALWAYS_INLINE float16 sincos(float16 x, thread float16 &cosval) { return __air_sincos(x, &cosval, _AIR_FAST_MATH_); }

METAL_ALWAYS_INLINE float frexp(float x, thread int &exp) { return __air_frexp(x, &exp); }
METAL_ALWAYS_INLINE float2 frexp(float2 x, thread int2 &exp) { return __air_frexp(x, &exp); }
METAL_ALWAYS_INLINE float3 frexp(float3 x, thread int3 &exp) { return __air_frexp(x, &exp); }
METAL_ALWAYS_INLINE float4 frexp(float4 x, thread int4 &exp) { return __air_frexp(x, &exp); }
METAL_ALWAYS_INLINE float8 frexp(float8 x, thread int8 &exp) { return __air_frexp(x, &exp); }
METAL_ALWAYS_INLINE float16 frexp(float16 x, thread int16 &exp) { return __air_frexp(x, &exp); }

METAL_ALWAYS_INLINE float modf(float x, thread float &intval) { return __air_modf(x, &intval, _AIR_FAST_MATH_); }
METAL_ALWAYS_INLINE float2 modf(float2 x, thread float2 &intval) { return __air_modf(x, &intval, _AIR_FAST_MATH_); }
METAL_ALWAYS_INLINE float3 modf(float3 x, thread float3 &intval) { return __air_modf(x, &intval, _AIR_FAST_MATH_); }
METAL_ALWAYS_INLINE float4 modf(float4 x, thread float4 &intval) { return __air_modf(x, &intval, _AIR_FAST_MATH_); }
METAL_ALWAYS_INLINE float8 modf(float8 x, thread float8 &intval) { return __air_modf(x, &intval, _AIR_FAST_MATH_); }
METAL_ALWAYS_INLINE float16 modf(float16 x, thread float16 &intval) { return __air_modf(x, &intval, _AIR_FAST_MATH_); }

METAL_ALWAYS_INLINE float ldexp(float x, int k) { return __air_ldexp(x, k, _AIR_FAST_MATH_); }
METAL_ALWAYS_INLINE float2 ldexp(float2 x, int2 k) { return __air_ldexp(x, k, _AIR_FAST_MATH_); }
METAL_ALWAYS_INLINE float3 ldexp(float3 x, int3 k) { return __air_ldexp(x, k, _AIR_FAST_MATH_); }
METAL_ALWAYS_INLINE float4 ldexp(float4 x, int4 k) { return __air_ldexp(x, k, _AIR_FAST_MATH_); }
METAL_ALWAYS_INLINE float8 ldexp(float8 x, int8 k) { return __air_ldexp(x, k, _AIR_FAST_MATH_); }
METAL_ALWAYS_INLINE float16 ldexp(float16 x, int16 k) { return __air_ldexp(x, k, _AIR_FAST_MATH_); }

METAL_ALWAYS_INLINE float sign(float x) { return __air_sign(x, _AIR_FAST_MATH_); }
METAL_ALWAYS_INLINE float2 sign(float2 x) { return __air_sign(x, _AIR_FAST_MATH_); }
METAL_ALWAYS_INLINE float3 sign(float3 x) { return __air_sign(x, _AIR_FAST_MATH_); }
METAL_ALWAYS_INLINE float4 sign(float4 x) { return __air_sign(x, _AIR_FAST_MATH_); }
METAL_ALWAYS_INLINE float8 sign(float8 x) { return __air_sign(x, _AIR_FAST_MATH_); }
METAL_ALWAYS_INLINE float16 sign(float16 x) { return __air_sign(x, _AIR_FAST_MATH_); }

METAL_ALWAYS_INLINE float fmuladd(float x, float y, float z) { return __air_fma(x, y, z); }
METAL_ALWAYS_INLINE float2 fmuladd(float2 x, float2 y, float2 z) { return __air_fma(x, y, z); }
METAL_ALWAYS_INLINE float3 fmuladd(float3 x, float3 y, float3 z) { return __air_fma(x, y, z); }
METAL_ALWAYS_INLINE float4 fmuladd(float4 x, float4 y, float4 z) { return __air_fma(x, y, z); }
METAL_ALWAYS_INLINE float8 fmuladd(float8 x, float8 y, float8 z) { return __air_fma(x, y, z); }
METAL_ALWAYS_INLINE float16 fmuladd(float16 x, float16 y, float16 z) { return __air_fma(x, y, z); }
METAL_ALWAYS_INLINE float mad(float x, float y, float z) { return __air_fma(x, y, z); }
METAL_ALWAYS_INLINE float2 mad(float2 x, float2 y, float2 z) { return __air_fma(x, y, z); }
METAL_ALWAYS_INLINE float3 mad(float3 x, float3 y, float3 z) { return __air_fma(x, y, z); }
METAL_ALWAYS_INLINE float4 mad(float4 x, float4 y, float4 z) { return __air_fma(x, y, z); }
METAL_ALWAYS_INLINE float8 mad(float8 x, float8 y, float8 z) { return __air_fma(x, y, z); }
METAL_ALWAYS_INLINE float16 mad(float16 x, float16 y, float16 z) { return __air_fma(x, y, z); }

METAL_ALWAYS_INLINE float pown(float x, int y) { return __air_pown(x, y, _AIR_FAST_MATH_); }
METAL_ALWAYS_INLINE float rootn(float x, int y) { return __air_rootn(x, y, _AIR_FAST_MATH_); }
METAL_ALWAYS_INLINE float2 pown(float2 x, int2 y) { return __air_pown(x, y, _AIR_FAST_MATH_); }
METAL_ALWAYS_INLINE float2 rootn(float2 x, int2 y) { return __air_rootn(x, y, _AIR_FAST_MATH_); }
METAL_ALWAYS_INLINE float3 pown(float3 x, int3 y) { return __air_pown(x, y, _AIR_FAST_MATH_); }
METAL_ALWAYS_INLINE float3 rootn(float3 x, int3 y) { return __air_rootn(x, y, _AIR_FAST_MATH_); }
METAL_ALWAYS_INLINE float4 pown(float4 x, int4 y) { return __air_pown(x, y, _AIR_FAST_MATH_); }
METAL_ALWAYS_INLINE float4 rootn(float4 x, int4 y) { return __air_rootn(x, y, _AIR_FAST_MATH_); }
METAL_ALWAYS_INLINE float8 pown(float8 x, int8 y) { return __air_pown(x, y, _AIR_FAST_MATH_); }
METAL_ALWAYS_INLINE float8 rootn(float8 x, int8 y) { return __air_rootn(x, y, _AIR_FAST_MATH_); }
METAL_ALWAYS_INLINE float16 pown(float16 x, int16 y) { return __air_pown(x, y, _AIR_FAST_MATH_); }
METAL_ALWAYS_INLINE float16 rootn(float16 x, int16 y) { return __air_rootn(x, y, _AIR_FAST_MATH_); }

} // namespace fast
namespace precise {
METAL_ALWAYS_INLINE float abs(float x) { return __air_abs(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE half abs(half x) { return __air_abs(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE float2 abs(float2 x) { return __air_abs(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE float3 abs(float3 x) { return __air_abs(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE float4 abs(float4 x) { return __air_abs(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE float8 abs(float8 x) { return __air_abs(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE float16 abs(float16 x) { return __air_abs(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE half2 abs(half2 x) { return __air_abs(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE half3 abs(half3 x) { return __air_abs(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE half4 abs(half4 x) { return __air_abs(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE half8 abs(half8 x) { return __air_abs(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE half16 abs(half16 x) { return __air_abs(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE float acos(float x) { return __air_acos(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE half acos(half x) { return __air_acos(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE float2 acos(float2 x) { return __air_acos(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE float3 acos(float3 x) { return __air_acos(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE float4 acos(float4 x) { return __air_acos(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE float8 acos(float8 x) { return __air_acos(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE float16 acos(float16 x) { return __air_acos(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE half2 acos(half2 x) { return __air_acos(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE half3 acos(half3 x) { return __air_acos(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE half4 acos(half4 x) { return __air_acos(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE half8 acos(half8 x) { return __air_acos(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE half16 acos(half16 x) { return __air_acos(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE float acosh(float x) { return __air_acosh(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE half acosh(half x) { return __air_acosh(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE float2 acosh(float2 x) { return __air_acosh(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE float3 acosh(float3 x) { return __air_acosh(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE float4 acosh(float4 x) { return __air_acosh(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE float8 acosh(float8 x) { return __air_acosh(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE float16 acosh(float16 x) { return __air_acosh(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE half2 acosh(half2 x) { return __air_acosh(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE half3 acosh(half3 x) { return __air_acosh(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE half4 acosh(half4 x) { return __air_acosh(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE half8 acosh(half8 x) { return __air_acosh(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE half16 acosh(half16 x) { return __air_acosh(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE float asin(float x) { return __air_asin(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE half asin(half x) { return __air_asin(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE float2 asin(float2 x) { return __air_asin(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE float3 asin(float3 x) { return __air_asin(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE float4 asin(float4 x) { return __air_asin(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE float8 asin(float8 x) { return __air_asin(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE float16 asin(float16 x) { return __air_asin(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE half2 asin(half2 x) { return __air_asin(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE half3 asin(half3 x) { return __air_asin(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE half4 asin(half4 x) { return __air_asin(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE half8 asin(half8 x) { return __air_asin(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE half16 asin(half16 x) { return __air_asin(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE float asinh(float x) { return __air_asinh(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE half asinh(half x) { return __air_asinh(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE float2 asinh(float2 x) { return __air_asinh(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE float3 asinh(float3 x) { return __air_asinh(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE float4 asinh(float4 x) { return __air_asinh(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE float8 asinh(float8 x) { return __air_asinh(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE float16 asinh(float16 x) { return __air_asinh(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE half2 asinh(half2 x) { return __air_asinh(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE half3 asinh(half3 x) { return __air_asinh(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE half4 asinh(half4 x) { return __air_asinh(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE half8 asinh(half8 x) { return __air_asinh(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE half16 asinh(half16 x) { return __air_asinh(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE float atan(float x) { return __air_atan(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE half atan(half x) { return __air_atan(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE float2 atan(float2 x) { return __air_atan(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE float3 atan(float3 x) { return __air_atan(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE float4 atan(float4 x) { return __air_atan(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE float8 atan(float8 x) { return __air_atan(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE float16 atan(float16 x) { return __air_atan(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE half2 atan(half2 x) { return __air_atan(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE half3 atan(half3 x) { return __air_atan(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE half4 atan(half4 x) { return __air_atan(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE half8 atan(half8 x) { return __air_atan(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE half16 atan(half16 x) { return __air_atan(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE float atanh(float x) { return __air_atanh(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE half atanh(half x) { return __air_atanh(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE float2 atanh(float2 x) { return __air_atanh(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE float3 atanh(float3 x) { return __air_atanh(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE float4 atanh(float4 x) { return __air_atanh(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE float8 atanh(float8 x) { return __air_atanh(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE float16 atanh(float16 x) { return __air_atanh(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE half2 atanh(half2 x) { return __air_atanh(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE half3 atanh(half3 x) { return __air_atanh(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE half4 atanh(half4 x) { return __air_atanh(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE half8 atanh(half8 x) { return __air_atanh(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE half16 atanh(half16 x) { return __air_atanh(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE float ceil(float x) { return __air_ceil(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE half ceil(half x) { return __air_ceil(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE float2 ceil(float2 x) { return __air_ceil(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE float3 ceil(float3 x) { return __air_ceil(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE float4 ceil(float4 x) { return __air_ceil(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE float8 ceil(float8 x) { return __air_ceil(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE float16 ceil(float16 x) { return __air_ceil(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE half2 ceil(half2 x) { return __air_ceil(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE half3 ceil(half3 x) { return __air_ceil(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE half4 ceil(half4 x) { return __air_ceil(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE half8 ceil(half8 x) { return __air_ceil(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE half16 ceil(half16 x) { return __air_ceil(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE float cos(float x) { return __air_cos(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE half cos(half x) { return __air_cos(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE float2 cos(float2 x) { return __air_cos(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE float3 cos(float3 x) { return __air_cos(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE float4 cos(float4 x) { return __air_cos(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE float8 cos(float8 x) { return __air_cos(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE float16 cos(float16 x) { return __air_cos(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE half2 cos(half2 x) { return __air_cos(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE half3 cos(half3 x) { return __air_cos(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE half4 cos(half4 x) { return __air_cos(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE half8 cos(half8 x) { return __air_cos(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE half16 cos(half16 x) { return __air_cos(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE float cosh(float x) { return __air_cosh(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE half cosh(half x) { return __air_cosh(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE float2 cosh(float2 x) { return __air_cosh(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE float3 cosh(float3 x) { return __air_cosh(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE float4 cosh(float4 x) { return __air_cosh(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE float8 cosh(float8 x) { return __air_cosh(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE float16 cosh(float16 x) { return __air_cosh(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE half2 cosh(half2 x) { return __air_cosh(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE half3 cosh(half3 x) { return __air_cosh(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE half4 cosh(half4 x) { return __air_cosh(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE half8 cosh(half8 x) { return __air_cosh(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE half16 cosh(half16 x) { return __air_cosh(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE float cospi(float x) { return __air_cospi(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE half cospi(half x) { return __air_cospi(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE float2 cospi(float2 x) { return __air_cospi(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE float3 cospi(float3 x) { return __air_cospi(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE float4 cospi(float4 x) { return __air_cospi(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE float8 cospi(float8 x) { return __air_cospi(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE float16 cospi(float16 x) { return __air_cospi(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE half2 cospi(half2 x) { return __air_cospi(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE half3 cospi(half3 x) { return __air_cospi(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE half4 cospi(half4 x) { return __air_cospi(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE half8 cospi(half8 x) { return __air_cospi(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE half16 cospi(half16 x) { return __air_cospi(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE float divide(float x) { return __air_divide(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE half divide(half x) { return __air_divide(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE float2 divide(float2 x) { return __air_divide(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE float3 divide(float3 x) { return __air_divide(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE float4 divide(float4 x) { return __air_divide(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE float8 divide(float8 x) { return __air_divide(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE float16 divide(float16 x) { return __air_divide(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE half2 divide(half2 x) { return __air_divide(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE half3 divide(half3 x) { return __air_divide(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE half4 divide(half4 x) { return __air_divide(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE half8 divide(half8 x) { return __air_divide(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE half16 divide(half16 x) { return __air_divide(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE float exp(float x) { return __air_exp(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE half exp(half x) { return __air_exp(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE float2 exp(float2 x) { return __air_exp(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE float3 exp(float3 x) { return __air_exp(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE float4 exp(float4 x) { return __air_exp(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE float8 exp(float8 x) { return __air_exp(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE float16 exp(float16 x) { return __air_exp(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE half2 exp(half2 x) { return __air_exp(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE half3 exp(half3 x) { return __air_exp(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE half4 exp(half4 x) { return __air_exp(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE half8 exp(half8 x) { return __air_exp(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE half16 exp(half16 x) { return __air_exp(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE float exp10(float x) { return __air_exp10(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE half exp10(half x) { return __air_exp10(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE float2 exp10(float2 x) { return __air_exp10(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE float3 exp10(float3 x) { return __air_exp10(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE float4 exp10(float4 x) { return __air_exp10(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE float8 exp10(float8 x) { return __air_exp10(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE float16 exp10(float16 x) { return __air_exp10(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE half2 exp10(half2 x) { return __air_exp10(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE half3 exp10(half3 x) { return __air_exp10(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE half4 exp10(half4 x) { return __air_exp10(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE half8 exp10(half8 x) { return __air_exp10(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE half16 exp10(half16 x) { return __air_exp10(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE float exp2(float x) { return __air_exp2(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE half exp2(half x) { return __air_exp2(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE float2 exp2(float2 x) { return __air_exp2(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE float3 exp2(float3 x) { return __air_exp2(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE float4 exp2(float4 x) { return __air_exp2(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE float8 exp2(float8 x) { return __air_exp2(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE float16 exp2(float16 x) { return __air_exp2(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE half2 exp2(half2 x) { return __air_exp2(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE half3 exp2(half3 x) { return __air_exp2(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE half4 exp2(half4 x) { return __air_exp2(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE half8 exp2(half8 x) { return __air_exp2(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE half16 exp2(half16 x) { return __air_exp2(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE float fabs(float x) { return __air_fabs(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE half fabs(half x) { return __air_fabs(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE float2 fabs(float2 x) { return __air_fabs(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE float3 fabs(float3 x) { return __air_fabs(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE float4 fabs(float4 x) { return __air_fabs(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE float8 fabs(float8 x) { return __air_fabs(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE float16 fabs(float16 x) { return __air_fabs(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE half2 fabs(half2 x) { return __air_fabs(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE half3 fabs(half3 x) { return __air_fabs(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE half4 fabs(half4 x) { return __air_fabs(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE half8 fabs(half8 x) { return __air_fabs(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE half16 fabs(half16 x) { return __air_fabs(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE float floor(float x) { return __air_floor(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE half floor(half x) { return __air_floor(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE float2 floor(float2 x) { return __air_floor(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE float3 floor(float3 x) { return __air_floor(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE float4 floor(float4 x) { return __air_floor(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE float8 floor(float8 x) { return __air_floor(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE float16 floor(float16 x) { return __air_floor(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE half2 floor(half2 x) { return __air_floor(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE half3 floor(half3 x) { return __air_floor(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE half4 floor(half4 x) { return __air_floor(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE half8 floor(half8 x) { return __air_floor(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE half16 floor(half16 x) { return __air_floor(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE float fract(float x) { return __air_fract(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE half fract(half x) { return __air_fract(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE float2 fract(float2 x) { return __air_fract(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE float3 fract(float3 x) { return __air_fract(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE float4 fract(float4 x) { return __air_fract(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE float8 fract(float8 x) { return __air_fract(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE float16 fract(float16 x) { return __air_fract(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE half2 fract(half2 x) { return __air_fract(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE half3 fract(half3 x) { return __air_fract(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE half4 fract(half4 x) { return __air_fract(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE half8 fract(half8 x) { return __air_fract(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE half16 fract(half16 x) { return __air_fract(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE float ilogb(float x) { return __air_ilogb(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE half ilogb(half x) { return __air_ilogb(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE float2 ilogb(float2 x) { return __air_ilogb(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE float3 ilogb(float3 x) { return __air_ilogb(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE float4 ilogb(float4 x) { return __air_ilogb(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE float8 ilogb(float8 x) { return __air_ilogb(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE float16 ilogb(float16 x) { return __air_ilogb(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE half2 ilogb(half2 x) { return __air_ilogb(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE half3 ilogb(half3 x) { return __air_ilogb(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE half4 ilogb(half4 x) { return __air_ilogb(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE half8 ilogb(half8 x) { return __air_ilogb(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE half16 ilogb(half16 x) { return __air_ilogb(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE float log(float x) { return __air_log(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE half log(half x) { return __air_log(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE float2 log(float2 x) { return __air_log(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE float3 log(float3 x) { return __air_log(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE float4 log(float4 x) { return __air_log(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE float8 log(float8 x) { return __air_log(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE float16 log(float16 x) { return __air_log(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE half2 log(half2 x) { return __air_log(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE half3 log(half3 x) { return __air_log(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE half4 log(half4 x) { return __air_log(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE half8 log(half8 x) { return __air_log(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE half16 log(half16 x) { return __air_log(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE float log10(float x) { return __air_log10(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE half log10(half x) { return __air_log10(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE float2 log10(float2 x) { return __air_log10(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE float3 log10(float3 x) { return __air_log10(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE float4 log10(float4 x) { return __air_log10(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE float8 log10(float8 x) { return __air_log10(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE float16 log10(float16 x) { return __air_log10(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE half2 log10(half2 x) { return __air_log10(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE half3 log10(half3 x) { return __air_log10(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE half4 log10(half4 x) { return __air_log10(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE half8 log10(half8 x) { return __air_log10(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE half16 log10(half16 x) { return __air_log10(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE float log2(float x) { return __air_log2(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE half log2(half x) { return __air_log2(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE float2 log2(float2 x) { return __air_log2(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE float3 log2(float3 x) { return __air_log2(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE float4 log2(float4 x) { return __air_log2(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE float8 log2(float8 x) { return __air_log2(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE float16 log2(float16 x) { return __air_log2(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE half2 log2(half2 x) { return __air_log2(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE half3 log2(half3 x) { return __air_log2(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE half4 log2(half4 x) { return __air_log2(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE half8 log2(half8 x) { return __air_log2(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE half16 log2(half16 x) { return __air_log2(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE float rint(float x) { return __air_rint(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE half rint(half x) { return __air_rint(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE float2 rint(float2 x) { return __air_rint(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE float3 rint(float3 x) { return __air_rint(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE float4 rint(float4 x) { return __air_rint(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE float8 rint(float8 x) { return __air_rint(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE float16 rint(float16 x) { return __air_rint(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE half2 rint(half2 x) { return __air_rint(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE half3 rint(half3 x) { return __air_rint(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE half4 rint(half4 x) { return __air_rint(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE half8 rint(half8 x) { return __air_rint(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE half16 rint(half16 x) { return __air_rint(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE float round(float x) { return __air_round(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE half round(half x) { return __air_round(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE float2 round(float2 x) { return __air_round(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE float3 round(float3 x) { return __air_round(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE float4 round(float4 x) { return __air_round(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE float8 round(float8 x) { return __air_round(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE float16 round(float16 x) { return __air_round(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE half2 round(half2 x) { return __air_round(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE half3 round(half3 x) { return __air_round(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE half4 round(half4 x) { return __air_round(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE half8 round(half8 x) { return __air_round(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE half16 round(half16 x) { return __air_round(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE float rsqrt(float x) { return __air_rsqrt(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE half rsqrt(half x) { return __air_rsqrt(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE float2 rsqrt(float2 x) { return __air_rsqrt(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE float3 rsqrt(float3 x) { return __air_rsqrt(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE float4 rsqrt(float4 x) { return __air_rsqrt(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE float8 rsqrt(float8 x) { return __air_rsqrt(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE float16 rsqrt(float16 x) { return __air_rsqrt(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE half2 rsqrt(half2 x) { return __air_rsqrt(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE half3 rsqrt(half3 x) { return __air_rsqrt(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE half4 rsqrt(half4 x) { return __air_rsqrt(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE half8 rsqrt(half8 x) { return __air_rsqrt(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE half16 rsqrt(half16 x) { return __air_rsqrt(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE float sin(float x) { return __air_sin(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE half sin(half x) { return __air_sin(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE float2 sin(float2 x) { return __air_sin(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE float3 sin(float3 x) { return __air_sin(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE float4 sin(float4 x) { return __air_sin(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE float8 sin(float8 x) { return __air_sin(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE float16 sin(float16 x) { return __air_sin(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE half2 sin(half2 x) { return __air_sin(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE half3 sin(half3 x) { return __air_sin(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE half4 sin(half4 x) { return __air_sin(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE half8 sin(half8 x) { return __air_sin(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE half16 sin(half16 x) { return __air_sin(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE float sinh(float x) { return __air_sinh(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE half sinh(half x) { return __air_sinh(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE float2 sinh(float2 x) { return __air_sinh(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE float3 sinh(float3 x) { return __air_sinh(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE float4 sinh(float4 x) { return __air_sinh(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE float8 sinh(float8 x) { return __air_sinh(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE float16 sinh(float16 x) { return __air_sinh(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE half2 sinh(half2 x) { return __air_sinh(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE half3 sinh(half3 x) { return __air_sinh(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE half4 sinh(half4 x) { return __air_sinh(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE half8 sinh(half8 x) { return __air_sinh(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE half16 sinh(half16 x) { return __air_sinh(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE float sinpi(float x) { return __air_sinpi(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE half sinpi(half x) { return __air_sinpi(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE float2 sinpi(float2 x) { return __air_sinpi(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE float3 sinpi(float3 x) { return __air_sinpi(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE float4 sinpi(float4 x) { return __air_sinpi(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE float8 sinpi(float8 x) { return __air_sinpi(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE float16 sinpi(float16 x) { return __air_sinpi(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE half2 sinpi(half2 x) { return __air_sinpi(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE half3 sinpi(half3 x) { return __air_sinpi(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE half4 sinpi(half4 x) { return __air_sinpi(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE half8 sinpi(half8 x) { return __air_sinpi(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE half16 sinpi(half16 x) { return __air_sinpi(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE float sqrt(float x) { return __air_sqrt(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE half sqrt(half x) { return __air_sqrt(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE float2 sqrt(float2 x) { return __air_sqrt(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE float3 sqrt(float3 x) { return __air_sqrt(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE float4 sqrt(float4 x) { return __air_sqrt(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE float8 sqrt(float8 x) { return __air_sqrt(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE float16 sqrt(float16 x) { return __air_sqrt(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE half2 sqrt(half2 x) { return __air_sqrt(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE half3 sqrt(half3 x) { return __air_sqrt(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE half4 sqrt(half4 x) { return __air_sqrt(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE half8 sqrt(half8 x) { return __air_sqrt(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE half16 sqrt(half16 x) { return __air_sqrt(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE float tan(float x) { return __air_tan(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE half tan(half x) { return __air_tan(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE float2 tan(float2 x) { return __air_tan(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE float3 tan(float3 x) { return __air_tan(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE float4 tan(float4 x) { return __air_tan(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE float8 tan(float8 x) { return __air_tan(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE float16 tan(float16 x) { return __air_tan(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE half2 tan(half2 x) { return __air_tan(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE half3 tan(half3 x) { return __air_tan(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE half4 tan(half4 x) { return __air_tan(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE half8 tan(half8 x) { return __air_tan(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE half16 tan(half16 x) { return __air_tan(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE float tanh(float x) { return __air_tanh(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE half tanh(half x) { return __air_tanh(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE float2 tanh(float2 x) { return __air_tanh(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE float3 tanh(float3 x) { return __air_tanh(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE float4 tanh(float4 x) { return __air_tanh(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE float8 tanh(float8 x) { return __air_tanh(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE float16 tanh(float16 x) { return __air_tanh(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE half2 tanh(half2 x) { return __air_tanh(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE half3 tanh(half3 x) { return __air_tanh(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE half4 tanh(half4 x) { return __air_tanh(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE half8 tanh(half8 x) { return __air_tanh(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE half16 tanh(half16 x) { return __air_tanh(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE float tanpi(float x) { return __air_tanpi(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE half tanpi(half x) { return __air_tanpi(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE float2 tanpi(float2 x) { return __air_tanpi(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE float3 tanpi(float3 x) { return __air_tanpi(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE float4 tanpi(float4 x) { return __air_tanpi(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE float8 tanpi(float8 x) { return __air_tanpi(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE float16 tanpi(float16 x) { return __air_tanpi(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE half2 tanpi(half2 x) { return __air_tanpi(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE half3 tanpi(half3 x) { return __air_tanpi(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE half4 tanpi(half4 x) { return __air_tanpi(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE half8 tanpi(half8 x) { return __air_tanpi(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE half16 tanpi(half16 x) { return __air_tanpi(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE float trunc(float x) { return __air_trunc(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE half trunc(half x) { return __air_trunc(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE float2 trunc(float2 x) { return __air_trunc(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE float3 trunc(float3 x) { return __air_trunc(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE float4 trunc(float4 x) { return __air_trunc(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE float8 trunc(float8 x) { return __air_trunc(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE float16 trunc(float16 x) { return __air_trunc(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE half2 trunc(half2 x) { return __air_trunc(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE half3 trunc(half3 x) { return __air_trunc(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE half4 trunc(half4 x) { return __air_trunc(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE half8 trunc(half8 x) { return __air_trunc(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE half16 trunc(half16 x) { return __air_trunc(x, _AIR_PRECISE_MATH_); }

METAL_ALWAYS_INLINE float atan2(float x, float y) { return __air_atan2(x, y, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE half atan2(half x, half y) { return __air_atan2(x, y, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE float2 atan2(float2 x, float2 y) { return __air_atan2(x, y, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE float3 atan2(float3 x, float3 y) { return __air_atan2(x, y, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE float4 atan2(float4 x, float4 y) { return __air_atan2(x, y, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE float8 atan2(float8 x, float8 y) { return __air_atan2(x, y, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE float16 atan2(float16 x, float16 y) { return __air_atan2(x, y, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE half2 atan2(half2 x, half2 y) { return __air_atan2(x, y, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE half3 atan2(half3 x, half3 y) { return __air_atan2(x, y, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE half4 atan2(half4 x, half4 y) { return __air_atan2(x, y, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE half8 atan2(half8 x, half8 y) { return __air_atan2(x, y, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE half16 atan2(half16 x, half16 y) { return __air_atan2(x, y, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE float copysign(float x, float y) { return __air_copysign(x, y, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE half copysign(half x, half y) { return __air_copysign(x, y, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE float2 copysign(float2 x, float2 y) { return __air_copysign(x, y, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE float3 copysign(float3 x, float3 y) { return __air_copysign(x, y, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE float4 copysign(float4 x, float4 y) { return __air_copysign(x, y, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE float8 copysign(float8 x, float8 y) { return __air_copysign(x, y, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE float16 copysign(float16 x, float16 y) { return __air_copysign(x, y, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE half2 copysign(half2 x, half2 y) { return __air_copysign(x, y, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE half3 copysign(half3 x, half3 y) { return __air_copysign(x, y, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE half4 copysign(half4 x, half4 y) { return __air_copysign(x, y, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE half8 copysign(half8 x, half8 y) { return __air_copysign(x, y, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE half16 copysign(half16 x, half16 y) { return __air_copysign(x, y, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE float fdim(float x, float y) { return __air_fdim(x, y, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE half fdim(half x, half y) { return __air_fdim(x, y, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE float2 fdim(float2 x, float2 y) { return __air_fdim(x, y, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE float3 fdim(float3 x, float3 y) { return __air_fdim(x, y, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE float4 fdim(float4 x, float4 y) { return __air_fdim(x, y, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE float8 fdim(float8 x, float8 y) { return __air_fdim(x, y, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE float16 fdim(float16 x, float16 y) { return __air_fdim(x, y, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE half2 fdim(half2 x, half2 y) { return __air_fdim(x, y, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE half3 fdim(half3 x, half3 y) { return __air_fdim(x, y, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE half4 fdim(half4 x, half4 y) { return __air_fdim(x, y, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE half8 fdim(half8 x, half8 y) { return __air_fdim(x, y, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE half16 fdim(half16 x, half16 y) { return __air_fdim(x, y, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE float fmax(float x, float y) { return __air_fmax(x, y, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE half fmax(half x, half y) { return __air_fmax(x, y, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE float2 fmax(float2 x, float2 y) { return __air_fmax(x, y, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE float3 fmax(float3 x, float3 y) { return __air_fmax(x, y, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE float4 fmax(float4 x, float4 y) { return __air_fmax(x, y, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE float8 fmax(float8 x, float8 y) { return __air_fmax(x, y, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE float16 fmax(float16 x, float16 y) { return __air_fmax(x, y, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE half2 fmax(half2 x, half2 y) { return __air_fmax(x, y, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE half3 fmax(half3 x, half3 y) { return __air_fmax(x, y, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE half4 fmax(half4 x, half4 y) { return __air_fmax(x, y, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE half8 fmax(half8 x, half8 y) { return __air_fmax(x, y, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE half16 fmax(half16 x, half16 y) { return __air_fmax(x, y, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE float fmin(float x, float y) { return __air_fmin(x, y, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE half fmin(half x, half y) { return __air_fmin(x, y, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE float2 fmin(float2 x, float2 y) { return __air_fmin(x, y, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE float3 fmin(float3 x, float3 y) { return __air_fmin(x, y, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE float4 fmin(float4 x, float4 y) { return __air_fmin(x, y, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE float8 fmin(float8 x, float8 y) { return __air_fmin(x, y, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE float16 fmin(float16 x, float16 y) { return __air_fmin(x, y, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE half2 fmin(half2 x, half2 y) { return __air_fmin(x, y, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE half3 fmin(half3 x, half3 y) { return __air_fmin(x, y, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE half4 fmin(half4 x, half4 y) { return __air_fmin(x, y, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE half8 fmin(half8 x, half8 y) { return __air_fmin(x, y, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE half16 fmin(half16 x, half16 y) { return __air_fmin(x, y, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE float fmod(float x, float y) { return __air_fmod(x, y, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE half fmod(half x, half y) { return __air_fmod(x, y, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE float2 fmod(float2 x, float2 y) { return __air_fmod(x, y, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE float3 fmod(float3 x, float3 y) { return __air_fmod(x, y, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE float4 fmod(float4 x, float4 y) { return __air_fmod(x, y, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE float8 fmod(float8 x, float8 y) { return __air_fmod(x, y, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE float16 fmod(float16 x, float16 y) { return __air_fmod(x, y, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE half2 fmod(half2 x, half2 y) { return __air_fmod(x, y, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE half3 fmod(half3 x, half3 y) { return __air_fmod(x, y, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE half4 fmod(half4 x, half4 y) { return __air_fmod(x, y, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE half8 fmod(half8 x, half8 y) { return __air_fmod(x, y, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE half16 fmod(half16 x, half16 y) { return __air_fmod(x, y, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE float max(float x, float y) { return __air_max(x, y, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE half max(half x, half y) { return __air_max(x, y, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE float2 max(float2 x, float2 y) { return __air_max(x, y, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE float3 max(float3 x, float3 y) { return __air_max(x, y, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE float4 max(float4 x, float4 y) { return __air_max(x, y, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE float8 max(float8 x, float8 y) { return __air_max(x, y, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE float16 max(float16 x, float16 y) { return __air_max(x, y, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE half2 max(half2 x, half2 y) { return __air_max(x, y, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE half3 max(half3 x, half3 y) { return __air_max(x, y, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE half4 max(half4 x, half4 y) { return __air_max(x, y, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE half8 max(half8 x, half8 y) { return __air_max(x, y, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE half16 max(half16 x, half16 y) { return __air_max(x, y, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE float min(float x, float y) { return __air_min(x, y, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE half min(half x, half y) { return __air_min(x, y, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE float2 min(float2 x, float2 y) { return __air_min(x, y, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE float3 min(float3 x, float3 y) { return __air_min(x, y, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE float4 min(float4 x, float4 y) { return __air_min(x, y, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE float8 min(float8 x, float8 y) { return __air_min(x, y, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE float16 min(float16 x, float16 y) { return __air_min(x, y, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE half2 min(half2 x, half2 y) { return __air_min(x, y, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE half3 min(half3 x, half3 y) { return __air_min(x, y, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE half4 min(half4 x, half4 y) { return __air_min(x, y, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE half8 min(half8 x, half8 y) { return __air_min(x, y, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE half16 min(half16 x, half16 y) { return __air_min(x, y, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE float nextafter(float x, float y) { return __air_nextafter(x, y, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE half nextafter(half x, half y) { return __air_nextafter(x, y, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE float2 nextafter(float2 x, float2 y) { return __air_nextafter(x, y, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE float3 nextafter(float3 x, float3 y) { return __air_nextafter(x, y, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE float4 nextafter(float4 x, float4 y) { return __air_nextafter(x, y, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE float8 nextafter(float8 x, float8 y) { return __air_nextafter(x, y, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE float16 nextafter(float16 x, float16 y) { return __air_nextafter(x, y, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE half2 nextafter(half2 x, half2 y) { return __air_nextafter(x, y, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE half3 nextafter(half3 x, half3 y) { return __air_nextafter(x, y, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE half4 nextafter(half4 x, half4 y) { return __air_nextafter(x, y, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE half8 nextafter(half8 x, half8 y) { return __air_nextafter(x, y, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE half16 nextafter(half16 x, half16 y) { return __air_nextafter(x, y, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE float pow(float x, float y) { return __air_pow(x, y, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE half pow(half x, half y) { return __air_pow(x, y, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE float2 pow(float2 x, float2 y) { return __air_pow(x, y, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE float3 pow(float3 x, float3 y) { return __air_pow(x, y, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE float4 pow(float4 x, float4 y) { return __air_pow(x, y, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE float8 pow(float8 x, float8 y) { return __air_pow(x, y, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE float16 pow(float16 x, float16 y) { return __air_pow(x, y, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE half2 pow(half2 x, half2 y) { return __air_pow(x, y, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE half3 pow(half3 x, half3 y) { return __air_pow(x, y, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE half4 pow(half4 x, half4 y) { return __air_pow(x, y, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE half8 pow(half8 x, half8 y) { return __air_pow(x, y, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE half16 pow(half16 x, half16 y) { return __air_pow(x, y, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE float powr(float x, float y) { return __air_powr(x, y, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE half powr(half x, half y) { return __air_powr(x, y, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE float2 powr(float2 x, float2 y) { return __air_powr(x, y, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE float3 powr(float3 x, float3 y) { return __air_powr(x, y, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE float4 powr(float4 x, float4 y) { return __air_powr(x, y, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE float8 powr(float8 x, float8 y) { return __air_powr(x, y, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE float16 powr(float16 x, float16 y) { return __air_powr(x, y, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE half2 powr(half2 x, half2 y) { return __air_powr(x, y, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE half3 powr(half3 x, half3 y) { return __air_powr(x, y, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE half4 powr(half4 x, half4 y) { return __air_powr(x, y, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE half8 powr(half8 x, half8 y) { return __air_powr(x, y, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE half16 powr(half16 x, half16 y) { return __air_powr(x, y, _AIR_PRECISE_MATH_); }

METAL_ALWAYS_INLINE float fma(float x, float y, float z) { return __air_fma(x, y, z, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE half fma(half x, half y, half z) { return __air_fma(x, y, z, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE float2 fma(float2 x, float2 y, float2 z) { return __air_fma(x, y, z, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE float3 fma(float3 x, float3 y, float3 z) { return __air_fma(x, y, z, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE float4 fma(float4 x, float4 y, float4 z) { return __air_fma(x, y, z, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE float8 fma(float8 x, float8 y, float8 z) { return __air_fma(x, y, z, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE float16 fma(float16 x, float16 y, float16 z) { return __air_fma(x, y, z, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE half2 fma(half2 x, half2 y, half2 z) { return __air_fma(x, y, z, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE half3 fma(half3 x, half3 y, half3 z) { return __air_fma(x, y, z, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE half4 fma(half4 x, half4 y, half4 z) { return __air_fma(x, y, z, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE half8 fma(half8 x, half8 y, half8 z) { return __air_fma(x, y, z, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE half16 fma(half16 x, half16 y, half16 z) { return __air_fma(x, y, z, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE float fmax3(float x, float y, float z) { return __air_fmax3(x, y, z, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE half fmax3(half x, half y, half z) { return __air_fmax3(x, y, z, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE float2 fmax3(float2 x, float2 y, float2 z) { return __air_fmax3(x, y, z, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE float3 fmax3(float3 x, float3 y, float3 z) { return __air_fmax3(x, y, z, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE float4 fmax3(float4 x, float4 y, float4 z) { return __air_fmax3(x, y, z, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE float8 fmax3(float8 x, float8 y, float8 z) { return __air_fmax3(x, y, z, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE float16 fmax3(float16 x, float16 y, float16 z) { return __air_fmax3(x, y, z, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE half2 fmax3(half2 x, half2 y, half2 z) { return __air_fmax3(x, y, z, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE half3 fmax3(half3 x, half3 y, half3 z) { return __air_fmax3(x, y, z, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE half4 fmax3(half4 x, half4 y, half4 z) { return __air_fmax3(x, y, z, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE half8 fmax3(half8 x, half8 y, half8 z) { return __air_fmax3(x, y, z, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE half16 fmax3(half16 x, half16 y, half16 z) { return __air_fmax3(x, y, z, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE float fmin3(float x, float y, float z) { return __air_fmin3(x, y, z, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE half fmin3(half x, half y, half z) { return __air_fmin3(x, y, z, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE float2 fmin3(float2 x, float2 y, float2 z) { return __air_fmin3(x, y, z, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE float3 fmin3(float3 x, float3 y, float3 z) { return __air_fmin3(x, y, z, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE float4 fmin3(float4 x, float4 y, float4 z) { return __air_fmin3(x, y, z, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE float8 fmin3(float8 x, float8 y, float8 z) { return __air_fmin3(x, y, z, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE float16 fmin3(float16 x, float16 y, float16 z) { return __air_fmin3(x, y, z, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE half2 fmin3(half2 x, half2 y, half2 z) { return __air_fmin3(x, y, z, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE half3 fmin3(half3 x, half3 y, half3 z) { return __air_fmin3(x, y, z, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE half4 fmin3(half4 x, half4 y, half4 z) { return __air_fmin3(x, y, z, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE half8 fmin3(half8 x, half8 y, half8 z) { return __air_fmin3(x, y, z, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE half16 fmin3(half16 x, half16 y, half16 z) { return __air_fmin3(x, y, z, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE float fmedian3(float x, float y, float z) { return __air_fmedian3(x, y, z, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE half fmedian3(half x, half y, half z) { return __air_fmedian3(x, y, z, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE float2 fmedian3(float2 x, float2 y, float2 z) { return __air_fmedian3(x, y, z, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE float3 fmedian3(float3 x, float3 y, float3 z) { return __air_fmedian3(x, y, z, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE float4 fmedian3(float4 x, float4 y, float4 z) { return __air_fmedian3(x, y, z, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE float8 fmedian3(float8 x, float8 y, float8 z) { return __air_fmedian3(x, y, z, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE float16 fmedian3(float16 x, float16 y, float16 z) { return __air_fmedian3(x, y, z, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE half2 fmedian3(half2 x, half2 y, half2 z) { return __air_fmedian3(x, y, z, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE half3 fmedian3(half3 x, half3 y, half3 z) { return __air_fmedian3(x, y, z, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE half4 fmedian3(half4 x, half4 y, half4 z) { return __air_fmedian3(x, y, z, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE half8 fmedian3(half8 x, half8 y, half8 z) { return __air_fmedian3(x, y, z, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE half16 fmedian3(half16 x, half16 y, half16 z) { return __air_fmedian3(x, y, z, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE float max3(float x, float y, float z) { return __air_max3(x, y, z, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE half max3(half x, half y, half z) { return __air_max3(x, y, z, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE float2 max3(float2 x, float2 y, float2 z) { return __air_max3(x, y, z, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE float3 max3(float3 x, float3 y, float3 z) { return __air_max3(x, y, z, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE float4 max3(float4 x, float4 y, float4 z) { return __air_max3(x, y, z, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE float8 max3(float8 x, float8 y, float8 z) { return __air_max3(x, y, z, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE float16 max3(float16 x, float16 y, float16 z) { return __air_max3(x, y, z, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE half2 max3(half2 x, half2 y, half2 z) { return __air_max3(x, y, z, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE half3 max3(half3 x, half3 y, half3 z) { return __air_max3(x, y, z, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE half4 max3(half4 x, half4 y, half4 z) { return __air_max3(x, y, z, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE half8 max3(half8 x, half8 y, half8 z) { return __air_max3(x, y, z, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE half16 max3(half16 x, half16 y, half16 z) { return __air_max3(x, y, z, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE float min3(float x, float y, float z) { return __air_min3(x, y, z, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE half min3(half x, half y, half z) { return __air_min3(x, y, z, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE float2 min3(float2 x, float2 y, float2 z) { return __air_min3(x, y, z, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE float3 min3(float3 x, float3 y, float3 z) { return __air_min3(x, y, z, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE float4 min3(float4 x, float4 y, float4 z) { return __air_min3(x, y, z, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE float8 min3(float8 x, float8 y, float8 z) { return __air_min3(x, y, z, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE float16 min3(float16 x, float16 y, float16 z) { return __air_min3(x, y, z, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE half2 min3(half2 x, half2 y, half2 z) { return __air_min3(x, y, z, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE half3 min3(half3 x, half3 y, half3 z) { return __air_min3(x, y, z, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE half4 min3(half4 x, half4 y, half4 z) { return __air_min3(x, y, z, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE half8 min3(half8 x, half8 y, half8 z) { return __air_min3(x, y, z, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE half16 min3(half16 x, half16 y, half16 z) { return __air_min3(x, y, z, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE float median3(float x, float y, float z) { return __air_median3(x, y, z, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE half median3(half x, half y, half z) { return __air_median3(x, y, z, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE float2 median3(float2 x, float2 y, float2 z) { return __air_median3(x, y, z, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE float3 median3(float3 x, float3 y, float3 z) { return __air_median3(x, y, z, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE float4 median3(float4 x, float4 y, float4 z) { return __air_median3(x, y, z, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE float8 median3(float8 x, float8 y, float8 z) { return __air_median3(x, y, z, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE float16 median3(float16 x, float16 y, float16 z) { return __air_median3(x, y, z, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE half2 median3(half2 x, half2 y, half2 z) { return __air_median3(x, y, z, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE half3 median3(half3 x, half3 y, half3 z) { return __air_median3(x, y, z, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE half4 median3(half4 x, half4 y, half4 z) { return __air_median3(x, y, z, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE half8 median3(half8 x, half8 y, half8 z) { return __air_median3(x, y, z, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE half16 median3(half16 x, half16 y, half16 z) { return __air_median3(x, y, z, _AIR_PRECISE_MATH_); }

METAL_ALWAYS_INLINE float sincos(float x, thread float &cosval) { return __air_sincos(x, &cosval, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE half sincos(half x, thread half &cosval) { return __air_sincos(x, &cosval, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE float2 sincos(float2 x, thread float2 &cosval) { return __air_sincos(x, &cosval, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE float3 sincos(float3 x, thread float3 &cosval) { return __air_sincos(x, &cosval, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE float4 sincos(float4 x, thread float4 &cosval) { return __air_sincos(x, &cosval, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE float8 sincos(float8 x, thread float8 &cosval) { return __air_sincos(x, &cosval, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE float16 sincos(float16 x, thread float16 &cosval) { return __air_sincos(x, &cosval, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE half2 sincos(half2 x, thread half2 &cosval) { return __air_sincos(x, &cosval, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE half3 sincos(half3 x, thread half3 &cosval) { return __air_sincos(x, &cosval, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE half4 sincos(half4 x, thread half4 &cosval) { return __air_sincos(x, &cosval, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE half8 sincos(half8 x, thread half8 &cosval) { return __air_sincos(x, &cosval, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE half16 sincos(half16 x, thread half16 &cosval) { return __air_sincos(x, &cosval, _AIR_PRECISE_MATH_); }

METAL_ALWAYS_INLINE float frexp(float x, thread int &exp) { return __air_frexp(x, &exp); }
METAL_ALWAYS_INLINE half frexp(half x, thread int &exp) { return __air_frexp(x, &exp); }
METAL_ALWAYS_INLINE float2 frexp(float2 x, thread int2 &exp) { return __air_frexp(x, &exp); }
METAL_ALWAYS_INLINE float3 frexp(float3 x, thread int3 &exp) { return __air_frexp(x, &exp); }
METAL_ALWAYS_INLINE float4 frexp(float4 x, thread int4 &exp) { return __air_frexp(x, &exp); }
METAL_ALWAYS_INLINE float8 frexp(float8 x, thread int8 &exp) { return __air_frexp(x, &exp); }
METAL_ALWAYS_INLINE float16 frexp(float16 x, thread int16 &exp) { return __air_frexp(x, &exp); }
METAL_ALWAYS_INLINE half2 frexp(half2 x, thread int2 &exp) { return __air_frexp(x, &exp); }
METAL_ALWAYS_INLINE half3 frexp(half3 x, thread int3 &exp) { return __air_frexp(x, &exp); }
METAL_ALWAYS_INLINE half4 frexp(half4 x, thread int4 &exp) { return __air_frexp(x, &exp); }
METAL_ALWAYS_INLINE half8 frexp(half8 x, thread int8 &exp) { return __air_frexp(x, &exp); }
METAL_ALWAYS_INLINE half16 frexp(half16 x, thread int16 &exp) { return __air_frexp(x, &exp); }

METAL_ALWAYS_INLINE float modf(float x, thread float &intval) { return __air_modf(x, &intval, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE half modf(half x, thread half &intval) { return __air_modf(x, &intval, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE float2 modf(float2 x, thread float2 &intval) { return __air_modf(x, &intval, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE float3 modf(float3 x, thread float3 &intval) { return __air_modf(x, &intval, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE float4 modf(float4 x, thread float4 &intval) { return __air_modf(x, &intval, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE float8 modf(float8 x, thread float8 &intval) { return __air_modf(x, &intval, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE float16 modf(float16 x, thread float16 &intval) { return __air_modf(x, &intval, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE half2 modf(half2 x, thread half2 &intval) { return __air_modf(x, &intval, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE half3 modf(half3 x, thread half3 &intval) { return __air_modf(x, &intval, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE half4 modf(half4 x, thread half4 &intval) { return __air_modf(x, &intval, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE half8 modf(half8 x, thread half8 &intval) { return __air_modf(x, &intval, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE half16 modf(half16 x, thread half16 &intval) { return __air_modf(x, &intval, _AIR_PRECISE_MATH_); }

METAL_ALWAYS_INLINE float ldexp(float x, int k) { return __air_ldexp(x, k, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE half ldexp(half x, int k) { return __air_ldexp(x, k, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE float2 ldexp(float2 x, int2 k) { return __air_ldexp(x, k, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE float3 ldexp(float3 x, int3 k) { return __air_ldexp(x, k, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE float4 ldexp(float4 x, int4 k) { return __air_ldexp(x, k, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE float8 ldexp(float8 x, int8 k) { return __air_ldexp(x, k, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE float16 ldexp(float16 x, int16 k) { return __air_ldexp(x, k, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE half2 ldexp(half2 x, int2 k) { return __air_ldexp(x, k, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE half3 ldexp(half3 x, int3 k) { return __air_ldexp(x, k, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE half4 ldexp(half4 x, int4 k) { return __air_ldexp(x, k, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE half8 ldexp(half8 x, int8 k) { return __air_ldexp(x, k, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE half16 ldexp(half16 x, int16 k) { return __air_ldexp(x, k, _AIR_PRECISE_MATH_); }

METAL_ALWAYS_INLINE float sign(float x) { return __air_sign(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE half sign(half x) { return __air_sign(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE float2 sign(float2 x) { return __air_sign(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE float3 sign(float3 x) { return __air_sign(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE float4 sign(float4 x) { return __air_sign(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE float8 sign(float8 x) { return __air_sign(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE float16 sign(float16 x) { return __air_sign(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE half2 sign(half2 x) { return __air_sign(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE half3 sign(half3 x) { return __air_sign(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE half4 sign(half4 x) { return __air_sign(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE half8 sign(half8 x) { return __air_sign(x, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE half16 sign(half16 x) { return __air_sign(x, _AIR_PRECISE_MATH_); }

METAL_ALWAYS_INLINE float fmuladd(float x, float y, float z) { return __air_fma(x, y, z); }
METAL_ALWAYS_INLINE half fmuladd(half x, half y, half z) { return __air_fma(x, y, z); }
METAL_ALWAYS_INLINE float2 fmuladd(float2 x, float2 y, float2 z) { return __air_fma(x, y, z); }
METAL_ALWAYS_INLINE float3 fmuladd(float3 x, float3 y, float3 z) { return __air_fma(x, y, z); }
METAL_ALWAYS_INLINE float4 fmuladd(float4 x, float4 y, float4 z) { return __air_fma(x, y, z); }
METAL_ALWAYS_INLINE float8 fmuladd(float8 x, float8 y, float8 z) { return __air_fma(x, y, z); }
METAL_ALWAYS_INLINE float16 fmuladd(float16 x, float16 y, float16 z) { return __air_fma(x, y, z); }
METAL_ALWAYS_INLINE half2 fmuladd(half2 x, half2 y, half2 z) { return __air_fma(x, y, z); }
METAL_ALWAYS_INLINE half3 fmuladd(half3 x, half3 y, half3 z) { return __air_fma(x, y, z); }
METAL_ALWAYS_INLINE half4 fmuladd(half4 x, half4 y, half4 z) { return __air_fma(x, y, z); }
METAL_ALWAYS_INLINE half8 fmuladd(half8 x, half8 y, half8 z) { return __air_fma(x, y, z); }
METAL_ALWAYS_INLINE half16 fmuladd(half16 x, half16 y, half16 z) { return __air_fma(x, y, z); }
METAL_ALWAYS_INLINE float mad(float x, float y, float z) { return __air_fma(x, y, z); }
METAL_ALWAYS_INLINE half mad(half x, half y, half z) { return __air_fma(x, y, z); }
METAL_ALWAYS_INLINE float2 mad(float2 x, float2 y, float2 z) { return __air_fma(x, y, z); }
METAL_ALWAYS_INLINE float3 mad(float3 x, float3 y, float3 z) { return __air_fma(x, y, z); }
METAL_ALWAYS_INLINE float4 mad(float4 x, float4 y, float4 z) { return __air_fma(x, y, z); }
METAL_ALWAYS_INLINE float8 mad(float8 x, float8 y, float8 z) { return __air_fma(x, y, z); }
METAL_ALWAYS_INLINE float16 mad(float16 x, float16 y, float16 z) { return __air_fma(x, y, z); }
METAL_ALWAYS_INLINE half2 mad(half2 x, half2 y, half2 z) { return __air_fma(x, y, z); }
METAL_ALWAYS_INLINE half3 mad(half3 x, half3 y, half3 z) { return __air_fma(x, y, z); }
METAL_ALWAYS_INLINE half4 mad(half4 x, half4 y, half4 z) { return __air_fma(x, y, z); }
METAL_ALWAYS_INLINE half8 mad(half8 x, half8 y, half8 z) { return __air_fma(x, y, z); }
METAL_ALWAYS_INLINE half16 mad(half16 x, half16 y, half16 z) { return __air_fma(x, y, z); }

METAL_ALWAYS_INLINE float pown(float x, int y) { return __air_pown(x, y, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE float rootn(float x, int y) { return __air_rootn(x, y, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE half pown(half x, int y) { return __air_pown(x, y, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE half rootn(half x, int y) { return __air_rootn(x, y, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE float2 pown(float2 x, int2 y) { return __air_pown(x, y, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE float2 rootn(float2 x, int2 y) { return __air_rootn(x, y, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE float3 pown(float3 x, int3 y) { return __air_pown(x, y, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE float3 rootn(float3 x, int3 y) { return __air_rootn(x, y, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE float4 pown(float4 x, int4 y) { return __air_pown(x, y, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE float4 rootn(float4 x, int4 y) { return __air_rootn(x, y, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE float8 pown(float8 x, int8 y) { return __air_pown(x, y, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE float8 rootn(float8 x, int8 y) { return __air_rootn(x, y, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE float16 pown(float16 x, int16 y) { return __air_pown(x, y, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE float16 rootn(float16 x, int16 y) { return __air_rootn(x, y, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE half2 pown(half2 x, int2 y) { return __air_pown(x, y, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE half2 rootn(half2 x, int2 y) { return __air_rootn(x, y, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE half3 pown(half3 x, int3 y) { return __air_pown(x, y, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE half3 rootn(half3 x, int3 y) { return __air_rootn(x, y, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE half4 pown(half4 x, int4 y) { return __air_pown(x, y, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE half4 rootn(half4 x, int4 y) { return __air_rootn(x, y, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE half8 pown(half8 x, int8 y) { return __air_pown(x, y, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE half8 rootn(half8 x, int8 y) { return __air_rootn(x, y, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE half16 pown(half16 x, int16 y) { return __air_pown(x, y, _AIR_PRECISE_MATH_); }
METAL_ALWAYS_INLINE half16 rootn(half16 x, int16 y) { return __air_rootn(x, y, _AIR_PRECISE_MATH_); }

} // namespace precise
METAL_ALWAYS_INLINE float abs(float x) { return __air_abs(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE half abs(half x) { return __air_abs(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE float2 abs(float2 x) { return __air_abs(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE float3 abs(float3 x) { return __air_abs(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE float4 abs(float4 x) { return __air_abs(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE float8 abs(float8 x) { return __air_abs(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE float16 abs(float16 x) { return __air_abs(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE half2 abs(half2 x) { return __air_abs(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE half3 abs(half3 x) { return __air_abs(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE half4 abs(half4 x) { return __air_abs(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE half8 abs(half8 x) { return __air_abs(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE half16 abs(half16 x) { return __air_abs(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE float acos(float x) { return __air_acos(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE half acos(half x) { return __air_acos(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE float2 acos(float2 x) { return __air_acos(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE float3 acos(float3 x) { return __air_acos(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE float4 acos(float4 x) { return __air_acos(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE float8 acos(float8 x) { return __air_acos(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE float16 acos(float16 x) { return __air_acos(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE half2 acos(half2 x) { return __air_acos(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE half3 acos(half3 x) { return __air_acos(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE half4 acos(half4 x) { return __air_acos(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE half8 acos(half8 x) { return __air_acos(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE half16 acos(half16 x) { return __air_acos(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE float acosh(float x) { return __air_acosh(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE half acosh(half x) { return __air_acosh(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE float2 acosh(float2 x) { return __air_acosh(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE float3 acosh(float3 x) { return __air_acosh(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE float4 acosh(float4 x) { return __air_acosh(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE float8 acosh(float8 x) { return __air_acosh(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE float16 acosh(float16 x) { return __air_acosh(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE half2 acosh(half2 x) { return __air_acosh(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE half3 acosh(half3 x) { return __air_acosh(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE half4 acosh(half4 x) { return __air_acosh(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE half8 acosh(half8 x) { return __air_acosh(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE half16 acosh(half16 x) { return __air_acosh(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE float asin(float x) { return __air_asin(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE half asin(half x) { return __air_asin(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE float2 asin(float2 x) { return __air_asin(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE float3 asin(float3 x) { return __air_asin(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE float4 asin(float4 x) { return __air_asin(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE float8 asin(float8 x) { return __air_asin(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE float16 asin(float16 x) { return __air_asin(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE half2 asin(half2 x) { return __air_asin(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE half3 asin(half3 x) { return __air_asin(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE half4 asin(half4 x) { return __air_asin(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE half8 asin(half8 x) { return __air_asin(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE half16 asin(half16 x) { return __air_asin(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE float asinh(float x) { return __air_asinh(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE half asinh(half x) { return __air_asinh(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE float2 asinh(float2 x) { return __air_asinh(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE float3 asinh(float3 x) { return __air_asinh(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE float4 asinh(float4 x) { return __air_asinh(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE float8 asinh(float8 x) { return __air_asinh(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE float16 asinh(float16 x) { return __air_asinh(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE half2 asinh(half2 x) { return __air_asinh(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE half3 asinh(half3 x) { return __air_asinh(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE half4 asinh(half4 x) { return __air_asinh(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE half8 asinh(half8 x) { return __air_asinh(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE half16 asinh(half16 x) { return __air_asinh(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE float atan(float x) { return __air_atan(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE half atan(half x) { return __air_atan(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE float2 atan(float2 x) { return __air_atan(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE float3 atan(float3 x) { return __air_atan(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE float4 atan(float4 x) { return __air_atan(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE float8 atan(float8 x) { return __air_atan(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE float16 atan(float16 x) { return __air_atan(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE half2 atan(half2 x) { return __air_atan(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE half3 atan(half3 x) { return __air_atan(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE half4 atan(half4 x) { return __air_atan(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE half8 atan(half8 x) { return __air_atan(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE half16 atan(half16 x) { return __air_atan(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE float atanh(float x) { return __air_atanh(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE half atanh(half x) { return __air_atanh(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE float2 atanh(float2 x) { return __air_atanh(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE float3 atanh(float3 x) { return __air_atanh(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE float4 atanh(float4 x) { return __air_atanh(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE float8 atanh(float8 x) { return __air_atanh(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE float16 atanh(float16 x) { return __air_atanh(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE half2 atanh(half2 x) { return __air_atanh(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE half3 atanh(half3 x) { return __air_atanh(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE half4 atanh(half4 x) { return __air_atanh(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE half8 atanh(half8 x) { return __air_atanh(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE half16 atanh(half16 x) { return __air_atanh(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE float ceil(float x) { return __air_ceil(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE half ceil(half x) { return __air_ceil(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE float2 ceil(float2 x) { return __air_ceil(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE float3 ceil(float3 x) { return __air_ceil(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE float4 ceil(float4 x) { return __air_ceil(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE float8 ceil(float8 x) { return __air_ceil(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE float16 ceil(float16 x) { return __air_ceil(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE half2 ceil(half2 x) { return __air_ceil(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE half3 ceil(half3 x) { return __air_ceil(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE half4 ceil(half4 x) { return __air_ceil(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE half8 ceil(half8 x) { return __air_ceil(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE half16 ceil(half16 x) { return __air_ceil(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE float cos(float x) { return __air_cos(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE half cos(half x) { return __air_cos(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE float2 cos(float2 x) { return __air_cos(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE float3 cos(float3 x) { return __air_cos(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE float4 cos(float4 x) { return __air_cos(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE float8 cos(float8 x) { return __air_cos(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE float16 cos(float16 x) { return __air_cos(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE half2 cos(half2 x) { return __air_cos(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE half3 cos(half3 x) { return __air_cos(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE half4 cos(half4 x) { return __air_cos(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE half8 cos(half8 x) { return __air_cos(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE half16 cos(half16 x) { return __air_cos(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE float cosh(float x) { return __air_cosh(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE half cosh(half x) { return __air_cosh(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE float2 cosh(float2 x) { return __air_cosh(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE float3 cosh(float3 x) { return __air_cosh(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE float4 cosh(float4 x) { return __air_cosh(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE float8 cosh(float8 x) { return __air_cosh(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE float16 cosh(float16 x) { return __air_cosh(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE half2 cosh(half2 x) { return __air_cosh(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE half3 cosh(half3 x) { return __air_cosh(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE half4 cosh(half4 x) { return __air_cosh(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE half8 cosh(half8 x) { return __air_cosh(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE half16 cosh(half16 x) { return __air_cosh(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE float cospi(float x) { return __air_cospi(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE half cospi(half x) { return __air_cospi(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE float2 cospi(float2 x) { return __air_cospi(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE float3 cospi(float3 x) { return __air_cospi(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE float4 cospi(float4 x) { return __air_cospi(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE float8 cospi(float8 x) { return __air_cospi(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE float16 cospi(float16 x) { return __air_cospi(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE half2 cospi(half2 x) { return __air_cospi(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE half3 cospi(half3 x) { return __air_cospi(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE half4 cospi(half4 x) { return __air_cospi(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE half8 cospi(half8 x) { return __air_cospi(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE half16 cospi(half16 x) { return __air_cospi(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE float divide(float x) { return __air_divide(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE half divide(half x) { return __air_divide(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE float2 divide(float2 x) { return __air_divide(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE float3 divide(float3 x) { return __air_divide(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE float4 divide(float4 x) { return __air_divide(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE float8 divide(float8 x) { return __air_divide(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE float16 divide(float16 x) { return __air_divide(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE half2 divide(half2 x) { return __air_divide(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE half3 divide(half3 x) { return __air_divide(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE half4 divide(half4 x) { return __air_divide(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE half8 divide(half8 x) { return __air_divide(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE half16 divide(half16 x) { return __air_divide(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE float exp(float x) { return __air_exp(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE half exp(half x) { return __air_exp(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE float2 exp(float2 x) { return __air_exp(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE float3 exp(float3 x) { return __air_exp(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE float4 exp(float4 x) { return __air_exp(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE float8 exp(float8 x) { return __air_exp(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE float16 exp(float16 x) { return __air_exp(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE half2 exp(half2 x) { return __air_exp(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE half3 exp(half3 x) { return __air_exp(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE half4 exp(half4 x) { return __air_exp(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE half8 exp(half8 x) { return __air_exp(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE half16 exp(half16 x) { return __air_exp(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE float exp10(float x) { return __air_exp10(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE half exp10(half x) { return __air_exp10(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE float2 exp10(float2 x) { return __air_exp10(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE float3 exp10(float3 x) { return __air_exp10(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE float4 exp10(float4 x) { return __air_exp10(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE float8 exp10(float8 x) { return __air_exp10(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE float16 exp10(float16 x) { return __air_exp10(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE half2 exp10(half2 x) { return __air_exp10(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE half3 exp10(half3 x) { return __air_exp10(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE half4 exp10(half4 x) { return __air_exp10(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE half8 exp10(half8 x) { return __air_exp10(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE half16 exp10(half16 x) { return __air_exp10(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE float exp2(float x) { return __air_exp2(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE half exp2(half x) { return __air_exp2(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE float2 exp2(float2 x) { return __air_exp2(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE float3 exp2(float3 x) { return __air_exp2(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE float4 exp2(float4 x) { return __air_exp2(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE float8 exp2(float8 x) { return __air_exp2(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE float16 exp2(float16 x) { return __air_exp2(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE half2 exp2(half2 x) { return __air_exp2(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE half3 exp2(half3 x) { return __air_exp2(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE half4 exp2(half4 x) { return __air_exp2(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE half8 exp2(half8 x) { return __air_exp2(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE half16 exp2(half16 x) { return __air_exp2(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE float fabs(float x) { return __air_fabs(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE half fabs(half x) { return __air_fabs(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE float2 fabs(float2 x) { return __air_fabs(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE float3 fabs(float3 x) { return __air_fabs(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE float4 fabs(float4 x) { return __air_fabs(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE float8 fabs(float8 x) { return __air_fabs(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE float16 fabs(float16 x) { return __air_fabs(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE half2 fabs(half2 x) { return __air_fabs(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE half3 fabs(half3 x) { return __air_fabs(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE half4 fabs(half4 x) { return __air_fabs(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE half8 fabs(half8 x) { return __air_fabs(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE half16 fabs(half16 x) { return __air_fabs(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE float floor(float x) { return __air_floor(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE half floor(half x) { return __air_floor(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE float2 floor(float2 x) { return __air_floor(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE float3 floor(float3 x) { return __air_floor(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE float4 floor(float4 x) { return __air_floor(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE float8 floor(float8 x) { return __air_floor(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE float16 floor(float16 x) { return __air_floor(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE half2 floor(half2 x) { return __air_floor(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE half3 floor(half3 x) { return __air_floor(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE half4 floor(half4 x) { return __air_floor(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE half8 floor(half8 x) { return __air_floor(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE half16 floor(half16 x) { return __air_floor(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE float fract(float x) { return __air_fract(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE half fract(half x) { return __air_fract(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE float2 fract(float2 x) { return __air_fract(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE float3 fract(float3 x) { return __air_fract(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE float4 fract(float4 x) { return __air_fract(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE float8 fract(float8 x) { return __air_fract(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE float16 fract(float16 x) { return __air_fract(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE half2 fract(half2 x) { return __air_fract(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE half3 fract(half3 x) { return __air_fract(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE half4 fract(half4 x) { return __air_fract(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE half8 fract(half8 x) { return __air_fract(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE half16 fract(half16 x) { return __air_fract(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE float ilogb(float x) { return __air_ilogb(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE half ilogb(half x) { return __air_ilogb(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE float2 ilogb(float2 x) { return __air_ilogb(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE float3 ilogb(float3 x) { return __air_ilogb(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE float4 ilogb(float4 x) { return __air_ilogb(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE float8 ilogb(float8 x) { return __air_ilogb(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE float16 ilogb(float16 x) { return __air_ilogb(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE half2 ilogb(half2 x) { return __air_ilogb(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE half3 ilogb(half3 x) { return __air_ilogb(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE half4 ilogb(half4 x) { return __air_ilogb(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE half8 ilogb(half8 x) { return __air_ilogb(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE half16 ilogb(half16 x) { return __air_ilogb(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE float log(float x) { return __air_log(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE half log(half x) { return __air_log(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE float2 log(float2 x) { return __air_log(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE float3 log(float3 x) { return __air_log(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE float4 log(float4 x) { return __air_log(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE float8 log(float8 x) { return __air_log(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE float16 log(float16 x) { return __air_log(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE half2 log(half2 x) { return __air_log(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE half3 log(half3 x) { return __air_log(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE half4 log(half4 x) { return __air_log(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE half8 log(half8 x) { return __air_log(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE half16 log(half16 x) { return __air_log(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE float log10(float x) { return __air_log10(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE half log10(half x) { return __air_log10(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE float2 log10(float2 x) { return __air_log10(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE float3 log10(float3 x) { return __air_log10(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE float4 log10(float4 x) { return __air_log10(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE float8 log10(float8 x) { return __air_log10(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE float16 log10(float16 x) { return __air_log10(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE half2 log10(half2 x) { return __air_log10(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE half3 log10(half3 x) { return __air_log10(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE half4 log10(half4 x) { return __air_log10(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE half8 log10(half8 x) { return __air_log10(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE half16 log10(half16 x) { return __air_log10(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE float log2(float x) { return __air_log2(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE half log2(half x) { return __air_log2(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE float2 log2(float2 x) { return __air_log2(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE float3 log2(float3 x) { return __air_log2(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE float4 log2(float4 x) { return __air_log2(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE float8 log2(float8 x) { return __air_log2(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE float16 log2(float16 x) { return __air_log2(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE half2 log2(half2 x) { return __air_log2(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE half3 log2(half3 x) { return __air_log2(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE half4 log2(half4 x) { return __air_log2(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE half8 log2(half8 x) { return __air_log2(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE half16 log2(half16 x) { return __air_log2(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE float rint(float x) { return __air_rint(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE half rint(half x) { return __air_rint(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE float2 rint(float2 x) { return __air_rint(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE float3 rint(float3 x) { return __air_rint(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE float4 rint(float4 x) { return __air_rint(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE float8 rint(float8 x) { return __air_rint(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE float16 rint(float16 x) { return __air_rint(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE half2 rint(half2 x) { return __air_rint(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE half3 rint(half3 x) { return __air_rint(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE half4 rint(half4 x) { return __air_rint(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE half8 rint(half8 x) { return __air_rint(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE half16 rint(half16 x) { return __air_rint(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE float round(float x) { return __air_round(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE half round(half x) { return __air_round(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE float2 round(float2 x) { return __air_round(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE float3 round(float3 x) { return __air_round(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE float4 round(float4 x) { return __air_round(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE float8 round(float8 x) { return __air_round(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE float16 round(float16 x) { return __air_round(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE half2 round(half2 x) { return __air_round(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE half3 round(half3 x) { return __air_round(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE half4 round(half4 x) { return __air_round(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE half8 round(half8 x) { return __air_round(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE half16 round(half16 x) { return __air_round(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE float rsqrt(float x) { return __air_rsqrt(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE half rsqrt(half x) { return __air_rsqrt(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE float2 rsqrt(float2 x) { return __air_rsqrt(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE float3 rsqrt(float3 x) { return __air_rsqrt(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE float4 rsqrt(float4 x) { return __air_rsqrt(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE float8 rsqrt(float8 x) { return __air_rsqrt(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE float16 rsqrt(float16 x) { return __air_rsqrt(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE half2 rsqrt(half2 x) { return __air_rsqrt(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE half3 rsqrt(half3 x) { return __air_rsqrt(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE half4 rsqrt(half4 x) { return __air_rsqrt(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE half8 rsqrt(half8 x) { return __air_rsqrt(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE half16 rsqrt(half16 x) { return __air_rsqrt(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE float sin(float x) { return __air_sin(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE half sin(half x) { return __air_sin(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE float2 sin(float2 x) { return __air_sin(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE float3 sin(float3 x) { return __air_sin(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE float4 sin(float4 x) { return __air_sin(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE float8 sin(float8 x) { return __air_sin(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE float16 sin(float16 x) { return __air_sin(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE half2 sin(half2 x) { return __air_sin(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE half3 sin(half3 x) { return __air_sin(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE half4 sin(half4 x) { return __air_sin(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE half8 sin(half8 x) { return __air_sin(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE half16 sin(half16 x) { return __air_sin(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE float sinh(float x) { return __air_sinh(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE half sinh(half x) { return __air_sinh(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE float2 sinh(float2 x) { return __air_sinh(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE float3 sinh(float3 x) { return __air_sinh(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE float4 sinh(float4 x) { return __air_sinh(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE float8 sinh(float8 x) { return __air_sinh(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE float16 sinh(float16 x) { return __air_sinh(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE half2 sinh(half2 x) { return __air_sinh(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE half3 sinh(half3 x) { return __air_sinh(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE half4 sinh(half4 x) { return __air_sinh(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE half8 sinh(half8 x) { return __air_sinh(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE half16 sinh(half16 x) { return __air_sinh(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE float sinpi(float x) { return __air_sinpi(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE half sinpi(half x) { return __air_sinpi(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE float2 sinpi(float2 x) { return __air_sinpi(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE float3 sinpi(float3 x) { return __air_sinpi(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE float4 sinpi(float4 x) { return __air_sinpi(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE float8 sinpi(float8 x) { return __air_sinpi(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE float16 sinpi(float16 x) { return __air_sinpi(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE half2 sinpi(half2 x) { return __air_sinpi(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE half3 sinpi(half3 x) { return __air_sinpi(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE half4 sinpi(half4 x) { return __air_sinpi(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE half8 sinpi(half8 x) { return __air_sinpi(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE half16 sinpi(half16 x) { return __air_sinpi(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE float sqrt(float x) { return __air_sqrt(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE half sqrt(half x) { return __air_sqrt(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE float2 sqrt(float2 x) { return __air_sqrt(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE float3 sqrt(float3 x) { return __air_sqrt(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE float4 sqrt(float4 x) { return __air_sqrt(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE float8 sqrt(float8 x) { return __air_sqrt(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE float16 sqrt(float16 x) { return __air_sqrt(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE half2 sqrt(half2 x) { return __air_sqrt(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE half3 sqrt(half3 x) { return __air_sqrt(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE half4 sqrt(half4 x) { return __air_sqrt(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE half8 sqrt(half8 x) { return __air_sqrt(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE half16 sqrt(half16 x) { return __air_sqrt(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE float tan(float x) { return __air_tan(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE half tan(half x) { return __air_tan(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE float2 tan(float2 x) { return __air_tan(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE float3 tan(float3 x) { return __air_tan(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE float4 tan(float4 x) { return __air_tan(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE float8 tan(float8 x) { return __air_tan(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE float16 tan(float16 x) { return __air_tan(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE half2 tan(half2 x) { return __air_tan(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE half3 tan(half3 x) { return __air_tan(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE half4 tan(half4 x) { return __air_tan(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE half8 tan(half8 x) { return __air_tan(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE half16 tan(half16 x) { return __air_tan(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE float tanh(float x) { return __air_tanh(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE half tanh(half x) { return __air_tanh(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE float2 tanh(float2 x) { return __air_tanh(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE float3 tanh(float3 x) { return __air_tanh(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE float4 tanh(float4 x) { return __air_tanh(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE float8 tanh(float8 x) { return __air_tanh(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE float16 tanh(float16 x) { return __air_tanh(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE half2 tanh(half2 x) { return __air_tanh(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE half3 tanh(half3 x) { return __air_tanh(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE half4 tanh(half4 x) { return __air_tanh(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE half8 tanh(half8 x) { return __air_tanh(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE half16 tanh(half16 x) { return __air_tanh(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE float tanpi(float x) { return __air_tanpi(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE half tanpi(half x) { return __air_tanpi(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE float2 tanpi(float2 x) { return __air_tanpi(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE float3 tanpi(float3 x) { return __air_tanpi(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE float4 tanpi(float4 x) { return __air_tanpi(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE float8 tanpi(float8 x) { return __air_tanpi(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE float16 tanpi(float16 x) { return __air_tanpi(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE half2 tanpi(half2 x) { return __air_tanpi(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE half3 tanpi(half3 x) { return __air_tanpi(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE half4 tanpi(half4 x) { return __air_tanpi(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE half8 tanpi(half8 x) { return __air_tanpi(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE half16 tanpi(half16 x) { return __air_tanpi(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE float trunc(float x) { return __air_trunc(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE half trunc(half x) { return __air_trunc(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE float2 trunc(float2 x) { return __air_trunc(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE float3 trunc(float3 x) { return __air_trunc(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE float4 trunc(float4 x) { return __air_trunc(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE float8 trunc(float8 x) { return __air_trunc(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE float16 trunc(float16 x) { return __air_trunc(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE half2 trunc(half2 x) { return __air_trunc(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE half3 trunc(half3 x) { return __air_trunc(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE half4 trunc(half4 x) { return __air_trunc(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE half8 trunc(half8 x) { return __air_trunc(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE half16 trunc(half16 x) { return __air_trunc(x, _AIR_ACTIVE_MATH_); }

METAL_ALWAYS_INLINE float atan2(float x, float y) { return __air_atan2(x, y, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE half atan2(half x, half y) { return __air_atan2(x, y, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE float2 atan2(float2 x, float2 y) { return __air_atan2(x, y, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE float3 atan2(float3 x, float3 y) { return __air_atan2(x, y, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE float4 atan2(float4 x, float4 y) { return __air_atan2(x, y, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE float8 atan2(float8 x, float8 y) { return __air_atan2(x, y, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE float16 atan2(float16 x, float16 y) { return __air_atan2(x, y, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE half2 atan2(half2 x, half2 y) { return __air_atan2(x, y, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE half3 atan2(half3 x, half3 y) { return __air_atan2(x, y, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE half4 atan2(half4 x, half4 y) { return __air_atan2(x, y, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE half8 atan2(half8 x, half8 y) { return __air_atan2(x, y, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE half16 atan2(half16 x, half16 y) { return __air_atan2(x, y, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE float copysign(float x, float y) { return __air_copysign(x, y, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE half copysign(half x, half y) { return __air_copysign(x, y, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE float2 copysign(float2 x, float2 y) { return __air_copysign(x, y, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE float3 copysign(float3 x, float3 y) { return __air_copysign(x, y, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE float4 copysign(float4 x, float4 y) { return __air_copysign(x, y, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE float8 copysign(float8 x, float8 y) { return __air_copysign(x, y, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE float16 copysign(float16 x, float16 y) { return __air_copysign(x, y, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE half2 copysign(half2 x, half2 y) { return __air_copysign(x, y, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE half3 copysign(half3 x, half3 y) { return __air_copysign(x, y, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE half4 copysign(half4 x, half4 y) { return __air_copysign(x, y, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE half8 copysign(half8 x, half8 y) { return __air_copysign(x, y, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE half16 copysign(half16 x, half16 y) { return __air_copysign(x, y, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE float fdim(float x, float y) { return __air_fdim(x, y, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE half fdim(half x, half y) { return __air_fdim(x, y, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE float2 fdim(float2 x, float2 y) { return __air_fdim(x, y, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE float3 fdim(float3 x, float3 y) { return __air_fdim(x, y, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE float4 fdim(float4 x, float4 y) { return __air_fdim(x, y, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE float8 fdim(float8 x, float8 y) { return __air_fdim(x, y, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE float16 fdim(float16 x, float16 y) { return __air_fdim(x, y, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE half2 fdim(half2 x, half2 y) { return __air_fdim(x, y, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE half3 fdim(half3 x, half3 y) { return __air_fdim(x, y, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE half4 fdim(half4 x, half4 y) { return __air_fdim(x, y, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE half8 fdim(half8 x, half8 y) { return __air_fdim(x, y, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE half16 fdim(half16 x, half16 y) { return __air_fdim(x, y, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE float fmax(float x, float y) { return __air_fmax(x, y, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE half fmax(half x, half y) { return __air_fmax(x, y, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE float2 fmax(float2 x, float2 y) { return __air_fmax(x, y, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE float3 fmax(float3 x, float3 y) { return __air_fmax(x, y, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE float4 fmax(float4 x, float4 y) { return __air_fmax(x, y, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE float8 fmax(float8 x, float8 y) { return __air_fmax(x, y, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE float16 fmax(float16 x, float16 y) { return __air_fmax(x, y, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE half2 fmax(half2 x, half2 y) { return __air_fmax(x, y, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE half3 fmax(half3 x, half3 y) { return __air_fmax(x, y, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE half4 fmax(half4 x, half4 y) { return __air_fmax(x, y, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE half8 fmax(half8 x, half8 y) { return __air_fmax(x, y, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE half16 fmax(half16 x, half16 y) { return __air_fmax(x, y, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE float fmin(float x, float y) { return __air_fmin(x, y, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE half fmin(half x, half y) { return __air_fmin(x, y, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE float2 fmin(float2 x, float2 y) { return __air_fmin(x, y, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE float3 fmin(float3 x, float3 y) { return __air_fmin(x, y, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE float4 fmin(float4 x, float4 y) { return __air_fmin(x, y, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE float8 fmin(float8 x, float8 y) { return __air_fmin(x, y, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE float16 fmin(float16 x, float16 y) { return __air_fmin(x, y, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE half2 fmin(half2 x, half2 y) { return __air_fmin(x, y, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE half3 fmin(half3 x, half3 y) { return __air_fmin(x, y, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE half4 fmin(half4 x, half4 y) { return __air_fmin(x, y, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE half8 fmin(half8 x, half8 y) { return __air_fmin(x, y, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE half16 fmin(half16 x, half16 y) { return __air_fmin(x, y, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE float fmod(float x, float y) { return __air_fmod(x, y, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE half fmod(half x, half y) { return __air_fmod(x, y, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE float2 fmod(float2 x, float2 y) { return __air_fmod(x, y, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE float3 fmod(float3 x, float3 y) { return __air_fmod(x, y, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE float4 fmod(float4 x, float4 y) { return __air_fmod(x, y, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE float8 fmod(float8 x, float8 y) { return __air_fmod(x, y, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE float16 fmod(float16 x, float16 y) { return __air_fmod(x, y, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE half2 fmod(half2 x, half2 y) { return __air_fmod(x, y, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE half3 fmod(half3 x, half3 y) { return __air_fmod(x, y, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE half4 fmod(half4 x, half4 y) { return __air_fmod(x, y, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE half8 fmod(half8 x, half8 y) { return __air_fmod(x, y, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE half16 fmod(half16 x, half16 y) { return __air_fmod(x, y, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE float max(float x, float y) { return __air_max(x, y, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE half max(half x, half y) { return __air_max(x, y, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE float2 max(float2 x, float2 y) { return __air_max(x, y, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE float3 max(float3 x, float3 y) { return __air_max(x, y, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE float4 max(float4 x, float4 y) { return __air_max(x, y, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE float8 max(float8 x, float8 y) { return __air_max(x, y, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE float16 max(float16 x, float16 y) { return __air_max(x, y, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE half2 max(half2 x, half2 y) { return __air_max(x, y, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE half3 max(half3 x, half3 y) { return __air_max(x, y, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE half4 max(half4 x, half4 y) { return __air_max(x, y, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE half8 max(half8 x, half8 y) { return __air_max(x, y, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE half16 max(half16 x, half16 y) { return __air_max(x, y, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE float min(float x, float y) { return __air_min(x, y, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE half min(half x, half y) { return __air_min(x, y, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE float2 min(float2 x, float2 y) { return __air_min(x, y, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE float3 min(float3 x, float3 y) { return __air_min(x, y, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE float4 min(float4 x, float4 y) { return __air_min(x, y, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE float8 min(float8 x, float8 y) { return __air_min(x, y, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE float16 min(float16 x, float16 y) { return __air_min(x, y, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE half2 min(half2 x, half2 y) { return __air_min(x, y, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE half3 min(half3 x, half3 y) { return __air_min(x, y, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE half4 min(half4 x, half4 y) { return __air_min(x, y, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE half8 min(half8 x, half8 y) { return __air_min(x, y, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE half16 min(half16 x, half16 y) { return __air_min(x, y, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE float nextafter(float x, float y) { return __air_nextafter(x, y, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE half nextafter(half x, half y) { return __air_nextafter(x, y, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE float2 nextafter(float2 x, float2 y) { return __air_nextafter(x, y, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE float3 nextafter(float3 x, float3 y) { return __air_nextafter(x, y, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE float4 nextafter(float4 x, float4 y) { return __air_nextafter(x, y, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE float8 nextafter(float8 x, float8 y) { return __air_nextafter(x, y, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE float16 nextafter(float16 x, float16 y) { return __air_nextafter(x, y, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE half2 nextafter(half2 x, half2 y) { return __air_nextafter(x, y, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE half3 nextafter(half3 x, half3 y) { return __air_nextafter(x, y, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE half4 nextafter(half4 x, half4 y) { return __air_nextafter(x, y, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE half8 nextafter(half8 x, half8 y) { return __air_nextafter(x, y, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE half16 nextafter(half16 x, half16 y) { return __air_nextafter(x, y, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE float pow(float x, float y) { return __air_pow(x, y, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE half pow(half x, half y) { return __air_pow(x, y, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE float2 pow(float2 x, float2 y) { return __air_pow(x, y, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE float3 pow(float3 x, float3 y) { return __air_pow(x, y, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE float4 pow(float4 x, float4 y) { return __air_pow(x, y, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE float8 pow(float8 x, float8 y) { return __air_pow(x, y, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE float16 pow(float16 x, float16 y) { return __air_pow(x, y, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE half2 pow(half2 x, half2 y) { return __air_pow(x, y, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE half3 pow(half3 x, half3 y) { return __air_pow(x, y, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE half4 pow(half4 x, half4 y) { return __air_pow(x, y, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE half8 pow(half8 x, half8 y) { return __air_pow(x, y, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE half16 pow(half16 x, half16 y) { return __air_pow(x, y, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE float powr(float x, float y) { return __air_powr(x, y, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE half powr(half x, half y) { return __air_powr(x, y, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE float2 powr(float2 x, float2 y) { return __air_powr(x, y, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE float3 powr(float3 x, float3 y) { return __air_powr(x, y, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE float4 powr(float4 x, float4 y) { return __air_powr(x, y, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE float8 powr(float8 x, float8 y) { return __air_powr(x, y, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE float16 powr(float16 x, float16 y) { return __air_powr(x, y, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE half2 powr(half2 x, half2 y) { return __air_powr(x, y, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE half3 powr(half3 x, half3 y) { return __air_powr(x, y, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE half4 powr(half4 x, half4 y) { return __air_powr(x, y, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE half8 powr(half8 x, half8 y) { return __air_powr(x, y, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE half16 powr(half16 x, half16 y) { return __air_powr(x, y, _AIR_ACTIVE_MATH_); }

METAL_ALWAYS_INLINE float fma(float x, float y, float z) { return __air_fma(x, y, z, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE half fma(half x, half y, half z) { return __air_fma(x, y, z, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE float2 fma(float2 x, float2 y, float2 z) { return __air_fma(x, y, z, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE float3 fma(float3 x, float3 y, float3 z) { return __air_fma(x, y, z, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE float4 fma(float4 x, float4 y, float4 z) { return __air_fma(x, y, z, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE float8 fma(float8 x, float8 y, float8 z) { return __air_fma(x, y, z, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE float16 fma(float16 x, float16 y, float16 z) { return __air_fma(x, y, z, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE half2 fma(half2 x, half2 y, half2 z) { return __air_fma(x, y, z, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE half3 fma(half3 x, half3 y, half3 z) { return __air_fma(x, y, z, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE half4 fma(half4 x, half4 y, half4 z) { return __air_fma(x, y, z, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE half8 fma(half8 x, half8 y, half8 z) { return __air_fma(x, y, z, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE half16 fma(half16 x, half16 y, half16 z) { return __air_fma(x, y, z, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE float fmax3(float x, float y, float z) { return __air_fmax3(x, y, z, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE half fmax3(half x, half y, half z) { return __air_fmax3(x, y, z, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE float2 fmax3(float2 x, float2 y, float2 z) { return __air_fmax3(x, y, z, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE float3 fmax3(float3 x, float3 y, float3 z) { return __air_fmax3(x, y, z, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE float4 fmax3(float4 x, float4 y, float4 z) { return __air_fmax3(x, y, z, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE float8 fmax3(float8 x, float8 y, float8 z) { return __air_fmax3(x, y, z, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE float16 fmax3(float16 x, float16 y, float16 z) { return __air_fmax3(x, y, z, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE half2 fmax3(half2 x, half2 y, half2 z) { return __air_fmax3(x, y, z, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE half3 fmax3(half3 x, half3 y, half3 z) { return __air_fmax3(x, y, z, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE half4 fmax3(half4 x, half4 y, half4 z) { return __air_fmax3(x, y, z, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE half8 fmax3(half8 x, half8 y, half8 z) { return __air_fmax3(x, y, z, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE half16 fmax3(half16 x, half16 y, half16 z) { return __air_fmax3(x, y, z, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE float fmin3(float x, float y, float z) { return __air_fmin3(x, y, z, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE half fmin3(half x, half y, half z) { return __air_fmin3(x, y, z, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE float2 fmin3(float2 x, float2 y, float2 z) { return __air_fmin3(x, y, z, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE float3 fmin3(float3 x, float3 y, float3 z) { return __air_fmin3(x, y, z, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE float4 fmin3(float4 x, float4 y, float4 z) { return __air_fmin3(x, y, z, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE float8 fmin3(float8 x, float8 y, float8 z) { return __air_fmin3(x, y, z, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE float16 fmin3(float16 x, float16 y, float16 z) { return __air_fmin3(x, y, z, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE half2 fmin3(half2 x, half2 y, half2 z) { return __air_fmin3(x, y, z, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE half3 fmin3(half3 x, half3 y, half3 z) { return __air_fmin3(x, y, z, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE half4 fmin3(half4 x, half4 y, half4 z) { return __air_fmin3(x, y, z, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE half8 fmin3(half8 x, half8 y, half8 z) { return __air_fmin3(x, y, z, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE half16 fmin3(half16 x, half16 y, half16 z) { return __air_fmin3(x, y, z, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE float fmedian3(float x, float y, float z) { return __air_fmedian3(x, y, z, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE half fmedian3(half x, half y, half z) { return __air_fmedian3(x, y, z, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE float2 fmedian3(float2 x, float2 y, float2 z) { return __air_fmedian3(x, y, z, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE float3 fmedian3(float3 x, float3 y, float3 z) { return __air_fmedian3(x, y, z, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE float4 fmedian3(float4 x, float4 y, float4 z) { return __air_fmedian3(x, y, z, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE float8 fmedian3(float8 x, float8 y, float8 z) { return __air_fmedian3(x, y, z, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE float16 fmedian3(float16 x, float16 y, float16 z) { return __air_fmedian3(x, y, z, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE half2 fmedian3(half2 x, half2 y, half2 z) { return __air_fmedian3(x, y, z, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE half3 fmedian3(half3 x, half3 y, half3 z) { return __air_fmedian3(x, y, z, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE half4 fmedian3(half4 x, half4 y, half4 z) { return __air_fmedian3(x, y, z, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE half8 fmedian3(half8 x, half8 y, half8 z) { return __air_fmedian3(x, y, z, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE half16 fmedian3(half16 x, half16 y, half16 z) { return __air_fmedian3(x, y, z, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE float max3(float x, float y, float z) { return __air_max3(x, y, z, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE half max3(half x, half y, half z) { return __air_max3(x, y, z, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE float2 max3(float2 x, float2 y, float2 z) { return __air_max3(x, y, z, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE float3 max3(float3 x, float3 y, float3 z) { return __air_max3(x, y, z, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE float4 max3(float4 x, float4 y, float4 z) { return __air_max3(x, y, z, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE float8 max3(float8 x, float8 y, float8 z) { return __air_max3(x, y, z, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE float16 max3(float16 x, float16 y, float16 z) { return __air_max3(x, y, z, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE half2 max3(half2 x, half2 y, half2 z) { return __air_max3(x, y, z, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE half3 max3(half3 x, half3 y, half3 z) { return __air_max3(x, y, z, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE half4 max3(half4 x, half4 y, half4 z) { return __air_max3(x, y, z, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE half8 max3(half8 x, half8 y, half8 z) { return __air_max3(x, y, z, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE half16 max3(half16 x, half16 y, half16 z) { return __air_max3(x, y, z, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE float min3(float x, float y, float z) { return __air_min3(x, y, z, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE half min3(half x, half y, half z) { return __air_min3(x, y, z, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE float2 min3(float2 x, float2 y, float2 z) { return __air_min3(x, y, z, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE float3 min3(float3 x, float3 y, float3 z) { return __air_min3(x, y, z, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE float4 min3(float4 x, float4 y, float4 z) { return __air_min3(x, y, z, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE float8 min3(float8 x, float8 y, float8 z) { return __air_min3(x, y, z, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE float16 min3(float16 x, float16 y, float16 z) { return __air_min3(x, y, z, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE half2 min3(half2 x, half2 y, half2 z) { return __air_min3(x, y, z, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE half3 min3(half3 x, half3 y, half3 z) { return __air_min3(x, y, z, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE half4 min3(half4 x, half4 y, half4 z) { return __air_min3(x, y, z, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE half8 min3(half8 x, half8 y, half8 z) { return __air_min3(x, y, z, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE half16 min3(half16 x, half16 y, half16 z) { return __air_min3(x, y, z, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE float median3(float x, float y, float z) { return __air_median3(x, y, z, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE half median3(half x, half y, half z) { return __air_median3(x, y, z, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE float2 median3(float2 x, float2 y, float2 z) { return __air_median3(x, y, z, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE float3 median3(float3 x, float3 y, float3 z) { return __air_median3(x, y, z, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE float4 median3(float4 x, float4 y, float4 z) { return __air_median3(x, y, z, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE float8 median3(float8 x, float8 y, float8 z) { return __air_median3(x, y, z, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE float16 median3(float16 x, float16 y, float16 z) { return __air_median3(x, y, z, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE half2 median3(half2 x, half2 y, half2 z) { return __air_median3(x, y, z, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE half3 median3(half3 x, half3 y, half3 z) { return __air_median3(x, y, z, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE half4 median3(half4 x, half4 y, half4 z) { return __air_median3(x, y, z, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE half8 median3(half8 x, half8 y, half8 z) { return __air_median3(x, y, z, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE half16 median3(half16 x, half16 y, half16 z) { return __air_median3(x, y, z, _AIR_ACTIVE_MATH_); }

METAL_ALWAYS_INLINE float sincos(float x, thread float &cosval) { return __air_sincos(x, &cosval, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE half sincos(half x, thread half &cosval) { return __air_sincos(x, &cosval, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE float2 sincos(float2 x, thread float2 &cosval) { return __air_sincos(x, &cosval, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE float3 sincos(float3 x, thread float3 &cosval) { return __air_sincos(x, &cosval, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE float4 sincos(float4 x, thread float4 &cosval) { return __air_sincos(x, &cosval, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE float8 sincos(float8 x, thread float8 &cosval) { return __air_sincos(x, &cosval, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE float16 sincos(float16 x, thread float16 &cosval) { return __air_sincos(x, &cosval, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE half2 sincos(half2 x, thread half2 &cosval) { return __air_sincos(x, &cosval, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE half3 sincos(half3 x, thread half3 &cosval) { return __air_sincos(x, &cosval, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE half4 sincos(half4 x, thread half4 &cosval) { return __air_sincos(x, &cosval, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE half8 sincos(half8 x, thread half8 &cosval) { return __air_sincos(x, &cosval, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE half16 sincos(half16 x, thread half16 &cosval) { return __air_sincos(x, &cosval, _AIR_ACTIVE_MATH_); }

METAL_ALWAYS_INLINE float frexp(float x, thread int &exp) { return __air_frexp(x, &exp); }
METAL_ALWAYS_INLINE half frexp(half x, thread int &exp) { return __air_frexp(x, &exp); }
METAL_ALWAYS_INLINE float2 frexp(float2 x, thread int2 &exp) { return __air_frexp(x, &exp); }
METAL_ALWAYS_INLINE float3 frexp(float3 x, thread int3 &exp) { return __air_frexp(x, &exp); }
METAL_ALWAYS_INLINE float4 frexp(float4 x, thread int4 &exp) { return __air_frexp(x, &exp); }
METAL_ALWAYS_INLINE float8 frexp(float8 x, thread int8 &exp) { return __air_frexp(x, &exp); }
METAL_ALWAYS_INLINE float16 frexp(float16 x, thread int16 &exp) { return __air_frexp(x, &exp); }
METAL_ALWAYS_INLINE half2 frexp(half2 x, thread int2 &exp) { return __air_frexp(x, &exp); }
METAL_ALWAYS_INLINE half3 frexp(half3 x, thread int3 &exp) { return __air_frexp(x, &exp); }
METAL_ALWAYS_INLINE half4 frexp(half4 x, thread int4 &exp) { return __air_frexp(x, &exp); }
METAL_ALWAYS_INLINE half8 frexp(half8 x, thread int8 &exp) { return __air_frexp(x, &exp); }
METAL_ALWAYS_INLINE half16 frexp(half16 x, thread int16 &exp) { return __air_frexp(x, &exp); }

METAL_ALWAYS_INLINE float modf(float x, thread float &intval) { return __air_modf(x, &intval, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE half modf(half x, thread half &intval) { return __air_modf(x, &intval, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE float2 modf(float2 x, thread float2 &intval) { return __air_modf(x, &intval, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE float3 modf(float3 x, thread float3 &intval) { return __air_modf(x, &intval, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE float4 modf(float4 x, thread float4 &intval) { return __air_modf(x, &intval, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE float8 modf(float8 x, thread float8 &intval) { return __air_modf(x, &intval, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE float16 modf(float16 x, thread float16 &intval) { return __air_modf(x, &intval, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE half2 modf(half2 x, thread half2 &intval) { return __air_modf(x, &intval, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE half3 modf(half3 x, thread half3 &intval) { return __air_modf(x, &intval, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE half4 modf(half4 x, thread half4 &intval) { return __air_modf(x, &intval, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE half8 modf(half8 x, thread half8 &intval) { return __air_modf(x, &intval, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE half16 modf(half16 x, thread half16 &intval) { return __air_modf(x, &intval, _AIR_ACTIVE_MATH_); }

METAL_ALWAYS_INLINE float ldexp(float x, int k) { return __air_ldexp(x, k, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE half ldexp(half x, int k) { return __air_ldexp(x, k, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE float2 ldexp(float2 x, int2 k) { return __air_ldexp(x, k, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE float3 ldexp(float3 x, int3 k) { return __air_ldexp(x, k, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE float4 ldexp(float4 x, int4 k) { return __air_ldexp(x, k, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE float8 ldexp(float8 x, int8 k) { return __air_ldexp(x, k, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE float16 ldexp(float16 x, int16 k) { return __air_ldexp(x, k, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE half2 ldexp(half2 x, int2 k) { return __air_ldexp(x, k, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE half3 ldexp(half3 x, int3 k) { return __air_ldexp(x, k, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE half4 ldexp(half4 x, int4 k) { return __air_ldexp(x, k, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE half8 ldexp(half8 x, int8 k) { return __air_ldexp(x, k, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE half16 ldexp(half16 x, int16 k) { return __air_ldexp(x, k, _AIR_ACTIVE_MATH_); }

METAL_ALWAYS_INLINE float sign(float x) { return __air_sign(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE half sign(half x) { return __air_sign(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE float2 sign(float2 x) { return __air_sign(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE float3 sign(float3 x) { return __air_sign(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE float4 sign(float4 x) { return __air_sign(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE float8 sign(float8 x) { return __air_sign(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE float16 sign(float16 x) { return __air_sign(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE half2 sign(half2 x) { return __air_sign(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE half3 sign(half3 x) { return __air_sign(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE half4 sign(half4 x) { return __air_sign(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE half8 sign(half8 x) { return __air_sign(x, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE half16 sign(half16 x) { return __air_sign(x, _AIR_ACTIVE_MATH_); }

METAL_ALWAYS_INLINE float fmuladd(float x, float y, float z) { return __air_fma(x, y, z); }
METAL_ALWAYS_INLINE half fmuladd(half x, half y, half z) { return __air_fma(x, y, z); }
METAL_ALWAYS_INLINE float2 fmuladd(float2 x, float2 y, float2 z) { return __air_fma(x, y, z); }
METAL_ALWAYS_INLINE float3 fmuladd(float3 x, float3 y, float3 z) { return __air_fma(x, y, z); }
METAL_ALWAYS_INLINE float4 fmuladd(float4 x, float4 y, float4 z) { return __air_fma(x, y, z); }
METAL_ALWAYS_INLINE float8 fmuladd(float8 x, float8 y, float8 z) { return __air_fma(x, y, z); }
METAL_ALWAYS_INLINE float16 fmuladd(float16 x, float16 y, float16 z) { return __air_fma(x, y, z); }
METAL_ALWAYS_INLINE half2 fmuladd(half2 x, half2 y, half2 z) { return __air_fma(x, y, z); }
METAL_ALWAYS_INLINE half3 fmuladd(half3 x, half3 y, half3 z) { return __air_fma(x, y, z); }
METAL_ALWAYS_INLINE half4 fmuladd(half4 x, half4 y, half4 z) { return __air_fma(x, y, z); }
METAL_ALWAYS_INLINE half8 fmuladd(half8 x, half8 y, half8 z) { return __air_fma(x, y, z); }
METAL_ALWAYS_INLINE half16 fmuladd(half16 x, half16 y, half16 z) { return __air_fma(x, y, z); }
METAL_ALWAYS_INLINE float mad(float x, float y, float z) { return __air_fma(x, y, z); }
METAL_ALWAYS_INLINE half mad(half x, half y, half z) { return __air_fma(x, y, z); }
METAL_ALWAYS_INLINE float2 mad(float2 x, float2 y, float2 z) { return __air_fma(x, y, z); }
METAL_ALWAYS_INLINE float3 mad(float3 x, float3 y, float3 z) { return __air_fma(x, y, z); }
METAL_ALWAYS_INLINE float4 mad(float4 x, float4 y, float4 z) { return __air_fma(x, y, z); }
METAL_ALWAYS_INLINE float8 mad(float8 x, float8 y, float8 z) { return __air_fma(x, y, z); }
METAL_ALWAYS_INLINE float16 mad(float16 x, float16 y, float16 z) { return __air_fma(x, y, z); }
METAL_ALWAYS_INLINE half2 mad(half2 x, half2 y, half2 z) { return __air_fma(x, y, z); }
METAL_ALWAYS_INLINE half3 mad(half3 x, half3 y, half3 z) { return __air_fma(x, y, z); }
METAL_ALWAYS_INLINE half4 mad(half4 x, half4 y, half4 z) { return __air_fma(x, y, z); }
METAL_ALWAYS_INLINE half8 mad(half8 x, half8 y, half8 z) { return __air_fma(x, y, z); }
METAL_ALWAYS_INLINE half16 mad(half16 x, half16 y, half16 z) { return __air_fma(x, y, z); }

METAL_ALWAYS_INLINE float pown(float x, int y) { return __air_pown(x, y, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE float rootn(float x, int y) { return __air_rootn(x, y, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE half pown(half x, int y) { return __air_pown(x, y, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE half rootn(half x, int y) { return __air_rootn(x, y, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE float2 pown(float2 x, int2 y) { return __air_pown(x, y, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE float2 rootn(float2 x, int2 y) { return __air_rootn(x, y, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE float3 pown(float3 x, int3 y) { return __air_pown(x, y, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE float3 rootn(float3 x, int3 y) { return __air_rootn(x, y, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE float4 pown(float4 x, int4 y) { return __air_pown(x, y, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE float4 rootn(float4 x, int4 y) { return __air_rootn(x, y, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE float8 pown(float8 x, int8 y) { return __air_pown(x, y, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE float8 rootn(float8 x, int8 y) { return __air_rootn(x, y, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE float16 pown(float16 x, int16 y) { return __air_pown(x, y, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE float16 rootn(float16 x, int16 y) { return __air_rootn(x, y, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE half2 pown(half2 x, int2 y) { return __air_pown(x, y, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE half2 rootn(half2 x, int2 y) { return __air_rootn(x, y, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE half3 pown(half3 x, int3 y) { return __air_pown(x, y, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE half3 rootn(half3 x, int3 y) { return __air_rootn(x, y, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE half4 pown(half4 x, int4 y) { return __air_pown(x, y, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE half4 rootn(half4 x, int4 y) { return __air_rootn(x, y, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE half8 pown(half8 x, int8 y) { return __air_pown(x, y, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE half8 rootn(half8 x, int8 y) { return __air_rootn(x, y, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE half16 pown(half16 x, int16 y) { return __air_pown(x, y, _AIR_ACTIVE_MATH_); }
METAL_ALWAYS_INLINE half16 rootn(half16 x, int16 y) { return __air_rootn(x, y, _AIR_ACTIVE_MATH_); }

} // namespace metal
#endif // _METAL_MATH_H_
