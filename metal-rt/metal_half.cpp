// metal_half.cpp - Metal Half-Precision Float Operations

extern "C" {

// Half-precision operations using software emulation
// In Metal, half is a 16-bit floating point (IEEE 754 binary16)

typedef unsigned short half_raw;

static float half_to_float(half_raw h) {
    unsigned int sign = (h >> 15) & 1;
    unsigned int exp = (h >> 10) & 0x1f;
    unsigned int mant = h & 0x3ff;
    unsigned int f;
    if (exp == 0) {
        if (mant == 0) { f = sign << 31; }
        else { exp = 1; while (!(mant & 0x400)) { mant <<= 1; exp--; } mant &= 0x3ff; f = (sign << 31) | ((exp + 127 - 15) << 23) | (mant << 13); }
    } else if (exp == 31) { f = (sign << 31) | 0x7f800000 | (mant << 13); }
    else { f = (sign << 31) | ((exp + 127 - 15) << 23) | (mant << 13); }
    float result; __builtin_memcpy(&result, &f, 4); return result;
}

static half_raw float_to_half(float val) {
    unsigned int f; __builtin_memcpy(&f, &val, 4);
    unsigned int sign = (f >> 31) & 1;
    int exp = ((f >> 23) & 0xff) - 127 + 15;
    unsigned int mant = (f >> 13) & 0x3ff;
    if (exp <= 0) { if (exp < -10) return (half_raw)(sign << 15); mant = (mant | 0x400) >> (1 - exp); return (half_raw)((sign << 15) | mant); }
    if (exp >= 31) return (half_raw)((sign << 15) | 0x7c00);
    return (half_raw)((sign << 15) | (exp << 10) | mant);
}

// Half-precision math operations
float ___metal_half_add(float a, float b) { return a + b; }
float ___metal_half_sub(float a, float b) { return a - b; }
float ___metal_half_mul(float a, float b) { return a * b; }
float ___metal_half_div(float a, float b) { return a / b; }
float ___metal_half_sqrt(float x) { return __builtin_sqrtf(x); }
float ___metal_half_rsqrt(float x) { return 1.0f / __builtin_sqrtf(x); }
float ___metal_half_abs(float x) { return __builtin_fabsf(x); }
float ___metal_half_neg(float x) { return -x; }
float ___metal_half_floor(float x) { return __builtin_floorf(x); }
float ___metal_half_ceil(float x) { return __builtin_ceilf(x); }
float ___metal_half_round(float x) { return __builtin_roundf(x); }
float ___metal_half_trunc(float x) { return __builtin_truncf(x); }
float ___metal_half_fma(float a, float b, float c) { return __builtin_fmaf(a, b, c); }
float ___metal_half_fmin(float a, float b) { return __builtin_fminf(a, b); }
float ___metal_half_fmax(float a, float b) { return __builtin_fmaxf(a, b); }
float ___metal_half_clamp(float x, float lo, float hi) { return x < lo ? lo : (x > hi ? hi : x); }
float ___metal_half_sin(float x) { return __builtin_sinf(x); }
float ___metal_half_cos(float x) { return __builtin_cosf(x); }
float ___metal_half_tan(float x) { return __builtin_tanf(x); }
float ___metal_half_exp(float x) { return __builtin_expf(x); }
float ___metal_half_exp2(float x) { return __builtin_exp2f(x); }
float ___metal_half_log(float x) { return __builtin_logf(x); }
float ___metal_half_log2(float x) { return __builtin_log2f(x); }
float ___metal_half_pow(float x, float y) { return __builtin_powf(x, y); }
float ___metal_half_mad(float a, float b, float c) { return __builtin_fmaf(a, b, c); }

// Conversion functions
float ___metal_half_to_float_convert(half_raw h) { return half_to_float(h); }
half_raw ___metal_float_to_half_convert(float f) { return float_to_half(f); }

} // extern C
