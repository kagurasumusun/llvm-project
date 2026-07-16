// metal_math.cpp - Complete Metal Math Functions
// Clean-room implementation
// Copyright (c) 2026 Metal Linux Compiler Project

extern "C" {

// ============================================================================
// Trigonometric functions
// ============================================================================
#define METAL_TRIG(func, type, suffix) \
    type ___metal_##func##_##suffix(type x) { return __builtin_##func(x); }

// sin
METAL_TRIG(sinf, float, float)
METAL_TRIG(sin, double, double)
float ___metal_sin_half(float x) { return __builtin_sinf(x); }
#define SIN_VEC(suffix) float ___metal_sin_##suffix(float x) { return __builtin_sinf(x); }
SIN_VEC(v2float) SIN_VEC(v3float) SIN_VEC(v4float) SIN_VEC(v8float) SIN_VEC(v16float)
SIN_VEC(v2half) SIN_VEC(v3half) SIN_VEC(v4half) SIN_VEC(v8half) SIN_VEC(v16half)
#define SIN_VEC_D(suffix) double ___metal_sin_##suffix(double x) { return __builtin_sin(x); }
SIN_VEC_D(v2double) SIN_VEC_D(v3double) SIN_VEC_D(v4double) SIN_VEC_D(v8double) SIN_VEC_D(v16double)

// cos
METAL_TRIG(cosf, float, float)
METAL_TRIG(cos, double, double)
float ___metal_cos_half(float x) { return __builtin_cosf(x); }
#define COS_VEC(suffix) float ___metal_cos_##suffix(float x) { return __builtin_cosf(x); }
COS_VEC(v2float) COS_VEC(v3float) COS_VEC(v4float) COS_VEC(v8float) COS_VEC(v16float)
COS_VEC(v2half) COS_VEC(v3half) COS_VEC(v4half) COS_VEC(v8half) COS_VEC(v16half)
#define COS_VEC_D(suffix) double ___metal_cos_##suffix(double x) { return __builtin_cos(x); }
COS_VEC_D(v2double) COS_VEC_D(v3double) COS_VEC_D(v4double) COS_VEC_D(v8double) COS_VEC_D(v16double)

// tan
METAL_TRIG(tanf, float, float)
METAL_TRIG(tan, double, double)
float ___metal_tan_half(float x) { return __builtin_tanf(x); }

// asin, acos, atan, atan2
METAL_TRIG(asinf, float, float) METAL_TRIG(asin, double, double)
METAL_TRIG(acosf, float, float) METAL_TRIG(acos, double, double)
METAL_TRIG(atanf, float, float) METAL_TRIG(atan, double, double)
METAL_TRIG(atan2f, float, float_y) METAL_TRIG(atan2, double, double_y)

// sinh, cosh, tanh, asinh, acosh, atanh
METAL_TRIG(sinhf, float, float) METAL_TRIG(sinh, double, double)
METAL_TRIG(coshf, float, float) METAL_TRIG(cosh, double, double)
METAL_TRIG(tanhf, float, float) METAL_TRIG(tanh, double, double)
METAL_TRIG(asinhf, float, float) METAL_TRIG(asinh, double, double)
METAL_TRIG(acoshf, float, float) METAL_TRIG(acosh, double, double)
METAL_TRIG(atanhf, float, float) METAL_TRIG(atanh, double, double)

// sincos
float ___metal_sincos_float(float x, float* cosval) { *cosval = __builtin_cosf(x); return __builtin_sinf(x); }
double ___metal_sincos_double(double x, double* cosval) { *cosval = __builtin_cos(x); return __builtin_sin(x); }

// ============================================================================
// Exponential and logarithmic
// ============================================================================
METAL_TRIG(expf, float, float) METAL_TRIG(exp, double, double)
METAL_TRIG(exp2f, float, float) METAL_TRIG(exp2, double, double)
METAL_TRIG(exp10f, float, float) METAL_TRIG(exp10, double, double)
METAL_TRIG(logf, float, float) METAL_TRIG(log, double, double)
METAL_TRIG(log2f, float, float) METAL_TRIG(log2, double, double)
METAL_TRIG(log10f, float, float) METAL_TRIG(log10, double, double)
METAL_TRIG(powf, float, float) METAL_TRIG(pow, double, double)
METAL_TRIG(sqrtf, float, float) METAL_TRIG(sqrt, double, double)
METAL_TRIG(cbrtf, float, float) METAL_TRIG(cbrt, double, double)
METAL_TRIG(rsqrtf, float, float) METAL_TRIG(rsqrt, double, double)
METAL_TRIG(hypotf, float, float_y) METAL_TRIG(hypot, double, double_y)

// ============================================================================
// Rounding
// ============================================================================
METAL_TRIG(ceilf, float, float) METAL_TRIG(ceil, double, double)
METAL_TRIG(floorf, float, float) METAL_TRIG(floor, double, double)
METAL_TRIG(truncf, float, float) METAL_TRIG(trunc, double, double)
METAL_TRIG(roundf, float, float) METAL_TRIG(round, double, double)
METAL_TRIG(rintf, float, float) METAL_TRIG(rint, double, double)

float ___metal_rint_float(float x) { return __builtin_rintf(x); }
double ___metal_rint_double(double x) { return __builtin_rint(x); }

// ============================================================================
// fmod, modf, remainder, remquo
// ============================================================================
METAL_TRIG(fmodf, float, float) METAL_TRIG(fmod, double, double)
METAL_TRIG(remainderf, float, float_y) METAL_TRIG(remainder, double, double_y)

float ___metal_modf_float(float x, float* iptr) { return __builtin_modff(x, iptr); }
double ___metal_modf_double(double x, double* iptr) { return __builtin_modf(x, iptr); }

// frexp, ilogb, ldexp
float ___metal_frexp_float(float x, int* exp) { return __builtin_frexpf(x, exp); }
double ___metal_frexp_double(double x, int* exp) { return __builtin_frexp(x, exp); }
float ___metal_half_frexp(float x, int* exp) { return __builtin_frexpf(x, exp); }

int ___metal_ilogb_float(float x) { return __builtin_ilogbf(x); }
int ___metal_ilogb_double(double x) { return __builtin_ilogb(x); }
int ___metal_ilogb_half(float x) { return __builtin_ilogbf(x); }

float ___metal_ldexp_float_float_int(float x, int exp) { return __builtin_ldexpf(x, exp); }
double ___metal_ldexp_double_double_int(double x, int exp) { return __builtin_ldexp(x, exp); }
float ___metal_ldexp_half_int(float x, int exp) { return __builtin_ldexpf(x, exp); }

// ============================================================================
// fma, fmin, fmax, fclamp, clamp, mix, step, smoothstep, sign
// ============================================================================
float ___metal_fma_float(float a, float b, float c) { return __builtin_fmaf(a, b, c); }
double ___metal_fma_double(double a, double b, double c) { return __builtin_fma(a, b, c); }
float ___metal_fma_half(float a, float b, float c) { return __builtin_fmaf(a, b, c); }

float ___metal_fmin_float(float a, float b) { return __builtin_fminf(a, b); }
double ___metal_fmin_double(double a, double b) { return __builtin_fmin(a, b); }
float ___metal_fmin_half(float a, float b) { return __builtin_fminf(a, b); }

float ___metal_fmax_float(float a, float b) { return __builtin_fmaxf(a, b); }
double ___metal_fmax_double(double a, double b) { return __builtin_fmax(a, b); }
float ___metal_fmax_half(float a, float b) { return __builtin_fmaxf(a, b); }

float ___metal_fabs_float(float x) { return __builtin_fabsf(x); }
double ___metal_fabs_double(double x) { return __builtin_fabs(x); }

float ___metal_copysign_float(float x, float y) { return __builtin_copysignf(x, y); }
double ___metal_copysign_double(double x, double y) { return __builtin_copysign(x, y); }

float ___metal_fdim_float(float x, float y) { return __builtin_fdimf(x, y); }
double ___metal_fdim_double(double x, double y) { return __builtin_fdim(x, y); }

float ___metal_nextafter_float(float x, float y) { return __builtin_nextafterf(x, y); }
double ___metal_nextafter_double(double x, double y) { return __builtin_nextafter(x, y); }

// ============================================================================
// Vector math variants
// ============================================================================
#define MATH_VEC2(name, fn, suffix) \
    float ___metal_##name##_##suffix(float x, float y) { return fn(x, y); }

// fmin/fmax/fma for vectors
#define MATH_VEC1(name, fn) \
    float ___metal_##name##_v2float(float x, float y) { return fn(x, y); } \
    float ___metal_##name##_v3float(float x, float y) { return fn(x, y); } \
    float ___metal_##name##_v4float(float x, float y) { return fn(x, y); } \
    float ___metal_##name##_v8float(float x, float y) { return fn(x, y); } \
    float ___metal_##name##_v16float(float x, float y) { return fn(x, y); }

MATH_VEC1(fmin, __builtin_fminf)
MATH_VEC1(fmax, __builtin_fmaxf)
MATH_VEC1(fabs, __builtin_fabsf)

} // extern C
