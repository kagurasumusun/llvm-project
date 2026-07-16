// metal_bfloat.cpp - Metal BFloat16 Operations (Metal 4.0+)

extern "C" {

// BFloat16 is a 16-bit brain floating point format
// Upper 16 bits of a 32-bit float

typedef unsigned short bfloat16_raw;

static float bfloat_to_float(bfloat16_raw b) {
    unsigned int f = (unsigned int)b << 16;
    float result; __builtin_memcpy(&result, &f, 4); return result;
}

static bfloat16_raw float_to_bfloat(float val) {
    unsigned int f; __builtin_memcpy(&f, &val, 4);
    return (bfloat16_raw)(f >> 16);
}

float ___metal_bfloat_add(float a, float b) { return a + b; }
float ___metal_bfloat_sub(float a, float b) { return a - b; }
float ___metal_bfloat_mul(float a, float b) { return a * b; }
float ___metal_bfloat_div(float a, float b) { return a / b; }
float ___metal_bfloat_mad(float a, float b, float c) { return a * b + c; }
float ___metal_bfloat_sqrt(float x) { return __builtin_sqrtf(x); }

} // extern C
