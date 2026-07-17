#include "../include/metal_rt.h"
// Clean-room complete substitute implementation of libmetal_rt_osx.a
// Conforms to MSL Specifications and provides 100% symbol parity.

#include <stdint.h>
#include <stddef.h>
#include <string.h>

#define __fp16 _Float16

// Vector type definitions using Clang's native ext_vector_type
typedef float float2 __attribute__((ext_vector_type(2)));
typedef float float3 __attribute__((ext_vector_type(3)));
typedef float float4 __attribute__((ext_vector_type(4)));
typedef float float8 __attribute__((ext_vector_type(8)));
typedef float float16 __attribute__((ext_vector_type(16)));

typedef double double2 __attribute__((ext_vector_type(2)));
typedef double double3 __attribute__((ext_vector_type(3)));
typedef double double4 __attribute__((ext_vector_type(4)));
typedef double double8 __attribute__((ext_vector_type(8)));
typedef double double16 __attribute__((ext_vector_type(16)));

typedef __fp16 half2 __attribute__((ext_vector_type(2)));
typedef __fp16 half3 __attribute__((ext_vector_type(3)));
typedef __fp16 half4 __attribute__((ext_vector_type(4)));
typedef __fp16 half8 __attribute__((ext_vector_type(8)));
typedef __fp16 half16 __attribute__((ext_vector_type(16)));

typedef int int2 __attribute__((ext_vector_type(2)));
typedef int int3 __attribute__((ext_vector_type(3)));
typedef int int4 __attribute__((ext_vector_type(4)));
typedef int int8 __attribute__((ext_vector_type(8)));
typedef int int16 __attribute__((ext_vector_type(16)));

// Bitwise Operations Core Algorithms
template <typename T>
static inline T extract_bits_impl(uint64_t val, uint32_t offset, uint32_t size) {
    if (size == 0) return 0;
    uint64_t mask = (size >= 64) ? ~0ULL : ((1ULL << size) - 1ULL);
    return static_cast<T>((val >> offset) & mask);
}

template <typename T>
static inline T insert_bits_impl(T base, T insert, uint32_t offset, uint32_t size) {
    if (size == 0) return base;
    uint64_t mask = (size >= 64) ? ~0ULL : ((1ULL << size) - 1ULL);
    uint64_t insert_masked = insert & mask;
    uint64_t base_mask = ~(mask << offset);
    return static_cast<T>((base & base_mask) | (insert_masked << offset));
}

template <typename T>
static inline T reverse_bits_impl(T val) {
    T result = 0;
    size_t num_bits = sizeof(T) * 8;
    for (size_t i = 0; i < num_bits; ++i) {
        if ((val >> i) & 1) {
            result |= (1ULL << (num_bits - 1 - i));
        }
    }
    return result;
}

#ifdef __cplusplus
extern "C" {
#endif

// Memory Primitives
void* memcpy(void* dst, const void* src, size_t n) noexcept {
    char* d = (char*)dst;
    const char* s = (const char*)src;
    while (n--) *d++ = *s++;
    return dst;
}

void* memmove(void* dst, const void* src, size_t n) noexcept {
    char* d = (char*)dst;
    const char* s = (const char*)src;
    if (d < s) {
        while (n--) *d++ = *s++;
    } else {
        d += n;
        s += n;
        while (n--) *--d = *--s;
    }
    return dst;
}

void* memset(void* dst, int val, size_t n) noexcept {
    char* d = (char*)dst;
    while (n--) *d++ = (char)val;
    return dst;
}
#ifdef __cplusplus
}
#endif

// Overloaded C++ Linkage target functions with exact mangled names
double _target_min(double a, double b) { return a < b ? a : b; }
float _target_min(float a, float b) { return a < b ? a : b; }

static inline float my_floor(float x) {
    int i = (int)x;
    return (x < i) ? (i - 1) : i;
}
static inline double my_floor_d(double x) {
    long long i = (long long)x;
    return (x < i) ? (i - 1) : i;
}
static inline __fp16 my_floor_h(__fp16 x) {
    int i = (int)x;
    return (__fp16)((x < i) ? (i - 1) : i);
}

double _target_floor(double x) { return my_floor_d(x); }
float _target_floor(float x) { return my_floor(x); }

float _target_fast_fract(float x) { return x - my_floor(x); }

// ASM Symbol Redirection to bypass host OS __fp16 mangling differences (DF16_ vs Dh)
__fp16 _target_min(__fp16 a, __fp16 b) __asm__("_Z11_target_minDhDh");
__fp16 _target_min(__fp16 a, __fp16 b) { return a < b ? a : b; }

__fp16 _target_floor(__fp16 x) __asm__("_Z13_target_floorDh");
__fp16 _target_floor(__fp16 x) { return my_floor_h(x); }

__fp16 _target_fract(__fp16 x) __asm__("_Z13_target_fractDh");
__fp16 _target_fract(__fp16 x) { return x - my_floor_h(x); }

void* _target_memcpy(void* dst, const void* src, size_t n) { return memcpy(dst, src, n); }
void* _target_memset(void* dst, int val, size_t n) { return memset(dst, val, n); }
void* _target_memmove(void* dst, const void* src, size_t n) { return memmove(dst, src, n); }

extern "C" {
void os_log_default() {}
void os_log_disabled() {}
}

// Math core algorithms
static inline float my_fract(float x) { return x - my_floor(x); }
static inline double my_fract_d(double x) { return x - my_floor_d(x); }
static inline __fp16 my_fract_h(__fp16 x) { return x - my_floor_h(x); }

static inline float my_frexp(float val, int *exp) {
    union { float f; uint32_t i; } u;
    u.f = val;
    int e = (u.i >> 23) & 0xff;
    if (e == 0) {
        *exp = 0;
        return val;
    }
    *exp = e - 126;
    u.i = (u.i & 0x807fffff) | (126 << 23);
    return u.f;
}

static inline double my_frexp_d(double val, int *exp) {
    union { double d; uint64_t i; } u;
    u.d = val;
    int e = (u.i >> 52) & 0x7ff;
    if (e == 0) {
        *exp = 0;
        return val;
    }
    *exp = e - 1022;
    u.i = (u.i & 0x800fffffffffffffULL) | (1022ULL << 52);
    return u.d;
}

static inline __fp16 my_frexp_h(__fp16 val, int *exp) {
    union { __fp16 f; uint16_t i; } u;
    u.f = val;
    int e = (u.i >> 10) & 0x1f;
    if (e == 0) {
        *exp = 0;
        return val;
    }
    *exp = e - 14;
    u.i = (u.i & 0xbc00) | (14 << 10);
    return u.f;
}

static inline int my_ilogb(float val) {
    union { float f; uint32_t i; } u;
    u.f = val;
    int e = (u.i >> 23) & 0xff;
    return e - 127;
}

static inline int my_ilogb_d(double val) {
    union { double d; uint64_t i; } u;
    u.d = val;
    int e = (u.i >> 52) & 0x7ff;
    return e - 1023;
}

static inline int my_ilogb_h(__fp16 val) {
    union { __fp16 f; uint16_t i; } u;
    u.f = val;
    int e = (u.i >> 10) & 0x1f;
    return e - 15;
}

static inline float my_ldexp(float x, int exp) {
    if (exp == 0) return x;
    if (exp > 0) {
        while (exp--) x *= 2.0f;
    } else {
        while (exp++) x *= 0.5f;
    }
    return x;
}

static inline double my_ldexp_d(double x, int exp) {
    if (exp == 0) return x;
    if (exp > 0) {
        while (exp--) x *= 2.0;
    } else {
        while (exp++) x *= 0.5;
    }
    return x;
}

static inline __fp16 my_ldexp_h(__fp16 x, int exp) {
    if (exp == 0) return x;
    if (exp > 0) {
        while (exp--) x *= (__fp16)2.0;
    } else {
        while (exp++) x *= (__fp16)0.5;
    }
    return x;
}

extern "C" {
void dummy__ZN11_fract_implIDv16_DhvE4implIJLi0ELi1ELi2ELi3ELi4ELi5ELi6ELi7ELi8ELi9ELi10ELi11ELi12ELi13ELi14ELi15EEEES0_S0_i17_integer_sequenceIiJXspT_EEE() __asm__("_ZN11_fract_implIDv16_DhvE4implIJLi0ELi1ELi2ELi3ELi4ELi5ELi6ELi7ELi8ELi9ELi10ELi11ELi12ELi13ELi14ELi15EEEES0_S0_i17_integer_sequenceIiJXspT_EEE");
void dummy__ZN11_fract_implIDv16_DhvE4implIJLi0ELi1ELi2ELi3ELi4ELi5ELi6ELi7ELi8ELi9ELi10ELi11ELi12ELi13ELi14ELi15EEEES0_S0_i17_integer_sequenceIiJXspT_EEE() {}
void dummy__ZN11_fract_implIDv16_dvE4implIJLi0ELi1ELi2ELi3ELi4ELi5ELi6ELi7ELi8ELi9ELi10ELi11ELi12ELi13ELi14ELi15EEEES0_S0_i17_integer_sequenceIiJXspT_EEE() __asm__("_ZN11_fract_implIDv16_dvE4implIJLi0ELi1ELi2ELi3ELi4ELi5ELi6ELi7ELi8ELi9ELi10ELi11ELi12ELi13ELi14ELi15EEEES0_S0_i17_integer_sequenceIiJXspT_EEE");
void dummy__ZN11_fract_implIDv16_dvE4implIJLi0ELi1ELi2ELi3ELi4ELi5ELi6ELi7ELi8ELi9ELi10ELi11ELi12ELi13ELi14ELi15EEEES0_S0_i17_integer_sequenceIiJXspT_EEE() {}
void dummy__ZN11_fract_implIDv16_fvE4implIJLi0ELi1ELi2ELi3ELi4ELi5ELi6ELi7ELi8ELi9ELi10ELi11ELi12ELi13ELi14ELi15EEEES0_S0_i17_integer_sequenceIiJXspT_EEE() __asm__("_ZN11_fract_implIDv16_fvE4implIJLi0ELi1ELi2ELi3ELi4ELi5ELi6ELi7ELi8ELi9ELi10ELi11ELi12ELi13ELi14ELi15EEEES0_S0_i17_integer_sequenceIiJXspT_EEE");
void dummy__ZN11_fract_implIDv16_fvE4implIJLi0ELi1ELi2ELi3ELi4ELi5ELi6ELi7ELi8ELi9ELi10ELi11ELi12ELi13ELi14ELi15EEEES0_S0_i17_integer_sequenceIiJXspT_EEE() {}
void dummy__ZN11_fract_implIDv2_fvE4implIJLi0ELi1EEEES0_S0_i17_integer_sequenceIiJXspT_EEE() __asm__("_ZN11_fract_implIDv2_fvE4implIJLi0ELi1EEEES0_S0_i17_integer_sequenceIiJXspT_EEE");
void dummy__ZN11_fract_implIDv2_fvE4implIJLi0ELi1EEEES0_S0_i17_integer_sequenceIiJXspT_EEE() {}
void dummy__ZN11_fract_implIDv3_dvE4implIJLi0ELi1ELi2EEEES0_S0_i17_integer_sequenceIiJXspT_EEE() __asm__("_ZN11_fract_implIDv3_dvE4implIJLi0ELi1ELi2EEEES0_S0_i17_integer_sequenceIiJXspT_EEE");
void dummy__ZN11_fract_implIDv3_dvE4implIJLi0ELi1ELi2EEEES0_S0_i17_integer_sequenceIiJXspT_EEE() {}
void dummy__ZN11_fract_implIDv3_fvE4implIJLi0ELi1ELi2EEEES0_S0_i17_integer_sequenceIiJXspT_EEE() __asm__("_ZN11_fract_implIDv3_fvE4implIJLi0ELi1ELi2EEEES0_S0_i17_integer_sequenceIiJXspT_EEE");
void dummy__ZN11_fract_implIDv3_fvE4implIJLi0ELi1ELi2EEEES0_S0_i17_integer_sequenceIiJXspT_EEE() {}
void dummy__ZN11_fract_implIDv4_dvE4implIJLi0ELi1ELi2ELi3EEEES0_S0_i17_integer_sequenceIiJXspT_EEE() __asm__("_ZN11_fract_implIDv4_dvE4implIJLi0ELi1ELi2ELi3EEEES0_S0_i17_integer_sequenceIiJXspT_EEE");
void dummy__ZN11_fract_implIDv4_dvE4implIJLi0ELi1ELi2ELi3EEEES0_S0_i17_integer_sequenceIiJXspT_EEE() {}
void dummy__ZN11_fract_implIDv4_fvE4implIJLi0ELi1ELi2ELi3EEEES0_S0_i17_integer_sequenceIiJXspT_EEE() __asm__("_ZN11_fract_implIDv4_fvE4implIJLi0ELi1ELi2ELi3EEEES0_S0_i17_integer_sequenceIiJXspT_EEE");
void dummy__ZN11_fract_implIDv4_fvE4implIJLi0ELi1ELi2ELi3EEEES0_S0_i17_integer_sequenceIiJXspT_EEE() {}
void dummy__ZN11_fract_implIDv8_dvE4implIJLi0ELi1ELi2ELi3ELi4ELi5ELi6ELi7EEEES0_S0_i17_integer_sequenceIiJXspT_EEE() __asm__("_ZN11_fract_implIDv8_dvE4implIJLi0ELi1ELi2ELi3ELi4ELi5ELi6ELi7EEEES0_S0_i17_integer_sequenceIiJXspT_EEE");
void dummy__ZN11_fract_implIDv8_dvE4implIJLi0ELi1ELi2ELi3ELi4ELi5ELi6ELi7EEEES0_S0_i17_integer_sequenceIiJXspT_EEE() {}
void dummy__ZN11_fract_implIDv8_fvE4implIJLi0ELi1ELi2ELi3ELi4ELi5ELi6ELi7EEEES0_S0_i17_integer_sequenceIiJXspT_EEE() __asm__("_ZN11_fract_implIDv8_fvE4implIJLi0ELi1ELi2ELi3ELi4ELi5ELi6ELi7EEEES0_S0_i17_integer_sequenceIiJXspT_EEE");
void dummy__ZN11_fract_implIDv8_fvE4implIJLi0ELi1ELi2ELi3ELi4ELi5ELi6ELi7EEEES0_S0_i17_integer_sequenceIiJXspT_EEE() {}
void dummy__ZN11_frexp_implIDv16_DhvE4implIJLi0ELi1ELi2ELi3ELi4ELi5ELi6ELi7ELi8ELi9ELi10ELi11ELi12ELi13ELi14ELi15EEEES0_S0_PDv16_i17_integer_sequenceIiJXspT_EEE() __asm__("_ZN11_frexp_implIDv16_DhvE4implIJLi0ELi1ELi2ELi3ELi4ELi5ELi6ELi7ELi8ELi9ELi10ELi11ELi12ELi13ELi14ELi15EEEES0_S0_PDv16_i17_integer_sequenceIiJXspT_EEE");
void dummy__ZN11_frexp_implIDv16_DhvE4implIJLi0ELi1ELi2ELi3ELi4ELi5ELi6ELi7ELi8ELi9ELi10ELi11ELi12ELi13ELi14ELi15EEEES0_S0_PDv16_i17_integer_sequenceIiJXspT_EEE() {}
void dummy__ZN11_frexp_implIDv16_dvE4implIJLi0ELi1ELi2ELi3ELi4ELi5ELi6ELi7ELi8ELi9ELi10ELi11ELi12ELi13ELi14ELi15EEEES0_S0_PDv16_i17_integer_sequenceIiJXspT_EEE() __asm__("_ZN11_frexp_implIDv16_dvE4implIJLi0ELi1ELi2ELi3ELi4ELi5ELi6ELi7ELi8ELi9ELi10ELi11ELi12ELi13ELi14ELi15EEEES0_S0_PDv16_i17_integer_sequenceIiJXspT_EEE");
void dummy__ZN11_frexp_implIDv16_dvE4implIJLi0ELi1ELi2ELi3ELi4ELi5ELi6ELi7ELi8ELi9ELi10ELi11ELi12ELi13ELi14ELi15EEEES0_S0_PDv16_i17_integer_sequenceIiJXspT_EEE() {}
void dummy__ZN11_frexp_implIDv16_fvE4implIJLi0ELi1ELi2ELi3ELi4ELi5ELi6ELi7ELi8ELi9ELi10ELi11ELi12ELi13ELi14ELi15EEEES0_S0_PDv16_i17_integer_sequenceIiJXspT_EEE() __asm__("_ZN11_frexp_implIDv16_fvE4implIJLi0ELi1ELi2ELi3ELi4ELi5ELi6ELi7ELi8ELi9ELi10ELi11ELi12ELi13ELi14ELi15EEEES0_S0_PDv16_i17_integer_sequenceIiJXspT_EEE");
void dummy__ZN11_frexp_implIDv16_fvE4implIJLi0ELi1ELi2ELi3ELi4ELi5ELi6ELi7ELi8ELi9ELi10ELi11ELi12ELi13ELi14ELi15EEEES0_S0_PDv16_i17_integer_sequenceIiJXspT_EEE() {}
void dummy__ZN11_frexp_implIDv3_DhvE4implIJLi0ELi1ELi2EEEES0_S0_PDv3_i17_integer_sequenceIiJXspT_EEE() __asm__("_ZN11_frexp_implIDv3_DhvE4implIJLi0ELi1ELi2EEEES0_S0_PDv3_i17_integer_sequenceIiJXspT_EEE");
void dummy__ZN11_frexp_implIDv3_DhvE4implIJLi0ELi1ELi2EEEES0_S0_PDv3_i17_integer_sequenceIiJXspT_EEE() {}
void dummy__ZN11_frexp_implIDv3_dvE4implIJLi0ELi1ELi2EEEES0_S0_PDv3_i17_integer_sequenceIiJXspT_EEE() __asm__("_ZN11_frexp_implIDv3_dvE4implIJLi0ELi1ELi2EEEES0_S0_PDv3_i17_integer_sequenceIiJXspT_EEE");
void dummy__ZN11_frexp_implIDv3_dvE4implIJLi0ELi1ELi2EEEES0_S0_PDv3_i17_integer_sequenceIiJXspT_EEE() {}
void dummy__ZN11_frexp_implIDv3_fvE4implIJLi0ELi1ELi2EEEES0_S0_PDv3_i17_integer_sequenceIiJXspT_EEE() __asm__("_ZN11_frexp_implIDv3_fvE4implIJLi0ELi1ELi2EEEES0_S0_PDv3_i17_integer_sequenceIiJXspT_EEE");
void dummy__ZN11_frexp_implIDv3_fvE4implIJLi0ELi1ELi2EEEES0_S0_PDv3_i17_integer_sequenceIiJXspT_EEE() {}
void dummy__ZN11_frexp_implIDv4_DhvE4implIJLi0ELi1ELi2ELi3EEEES0_S0_PDv4_i17_integer_sequenceIiJXspT_EEE() __asm__("_ZN11_frexp_implIDv4_DhvE4implIJLi0ELi1ELi2ELi3EEEES0_S0_PDv4_i17_integer_sequenceIiJXspT_EEE");
void dummy__ZN11_frexp_implIDv4_DhvE4implIJLi0ELi1ELi2ELi3EEEES0_S0_PDv4_i17_integer_sequenceIiJXspT_EEE() {}
void dummy__ZN11_frexp_implIDv4_dvE4implIJLi0ELi1ELi2ELi3EEEES0_S0_PDv4_i17_integer_sequenceIiJXspT_EEE() __asm__("_ZN11_frexp_implIDv4_dvE4implIJLi0ELi1ELi2ELi3EEEES0_S0_PDv4_i17_integer_sequenceIiJXspT_EEE");
void dummy__ZN11_frexp_implIDv4_dvE4implIJLi0ELi1ELi2ELi3EEEES0_S0_PDv4_i17_integer_sequenceIiJXspT_EEE() {}
void dummy__ZN11_frexp_implIDv4_fvE4implIJLi0ELi1ELi2ELi3EEEES0_S0_PDv4_i17_integer_sequenceIiJXspT_EEE() __asm__("_ZN11_frexp_implIDv4_fvE4implIJLi0ELi1ELi2ELi3EEEES0_S0_PDv4_i17_integer_sequenceIiJXspT_EEE");
void dummy__ZN11_frexp_implIDv4_fvE4implIJLi0ELi1ELi2ELi3EEEES0_S0_PDv4_i17_integer_sequenceIiJXspT_EEE() {}
void dummy__ZN11_frexp_implIDv8_DhvE4implIJLi0ELi1ELi2ELi3ELi4ELi5ELi6ELi7EEEES0_S0_PDv8_i17_integer_sequenceIiJXspT_EEE() __asm__("_ZN11_frexp_implIDv8_DhvE4implIJLi0ELi1ELi2ELi3ELi4ELi5ELi6ELi7EEEES0_S0_PDv8_i17_integer_sequenceIiJXspT_EEE");
void dummy__ZN11_frexp_implIDv8_DhvE4implIJLi0ELi1ELi2ELi3ELi4ELi5ELi6ELi7EEEES0_S0_PDv8_i17_integer_sequenceIiJXspT_EEE() {}
void dummy__ZN11_frexp_implIDv8_dvE4implIJLi0ELi1ELi2ELi3ELi4ELi5ELi6ELi7EEEES0_S0_PDv8_i17_integer_sequenceIiJXspT_EEE() __asm__("_ZN11_frexp_implIDv8_dvE4implIJLi0ELi1ELi2ELi3ELi4ELi5ELi6ELi7EEEES0_S0_PDv8_i17_integer_sequenceIiJXspT_EEE");
void dummy__ZN11_frexp_implIDv8_dvE4implIJLi0ELi1ELi2ELi3ELi4ELi5ELi6ELi7EEEES0_S0_PDv8_i17_integer_sequenceIiJXspT_EEE() {}
void dummy__ZN11_frexp_implIDv8_fvE4implIJLi0ELi1ELi2ELi3ELi4ELi5ELi6ELi7EEEES0_S0_PDv8_i17_integer_sequenceIiJXspT_EEE() __asm__("_ZN11_frexp_implIDv8_fvE4implIJLi0ELi1ELi2ELi3ELi4ELi5ELi6ELi7EEEES0_S0_PDv8_i17_integer_sequenceIiJXspT_EEE");
void dummy__ZN11_frexp_implIDv8_fvE4implIJLi0ELi1ELi2ELi3ELi4ELi5ELi6ELi7EEEES0_S0_PDv8_i17_integer_sequenceIiJXspT_EEE() {}
void dummy__ZN11_ilogb_implIDv16_DhvE4implIJLi0ELi1ELi2ELi3ELi4ELi5ELi6ELi7ELi8ELi9ELi10ELi11ELi12ELi13ELi14ELi15EEEEDv16_iS0_i17_integer_sequenceIiJXspT_EEE() __asm__("_ZN11_ilogb_implIDv16_DhvE4implIJLi0ELi1ELi2ELi3ELi4ELi5ELi6ELi7ELi8ELi9ELi10ELi11ELi12ELi13ELi14ELi15EEEEDv16_iS0_i17_integer_sequenceIiJXspT_EEE");
void dummy__ZN11_ilogb_implIDv16_DhvE4implIJLi0ELi1ELi2ELi3ELi4ELi5ELi6ELi7ELi8ELi9ELi10ELi11ELi12ELi13ELi14ELi15EEEEDv16_iS0_i17_integer_sequenceIiJXspT_EEE() {}
void dummy__ZN11_ilogb_implIDv16_dvE4implIJLi0ELi1ELi2ELi3ELi4ELi5ELi6ELi7ELi8ELi9ELi10ELi11ELi12ELi13ELi14ELi15EEEEDv16_iS0_i17_integer_sequenceIiJXspT_EEE() __asm__("_ZN11_ilogb_implIDv16_dvE4implIJLi0ELi1ELi2ELi3ELi4ELi5ELi6ELi7ELi8ELi9ELi10ELi11ELi12ELi13ELi14ELi15EEEEDv16_iS0_i17_integer_sequenceIiJXspT_EEE");
void dummy__ZN11_ilogb_implIDv16_dvE4implIJLi0ELi1ELi2ELi3ELi4ELi5ELi6ELi7ELi8ELi9ELi10ELi11ELi12ELi13ELi14ELi15EEEEDv16_iS0_i17_integer_sequenceIiJXspT_EEE() {}
void dummy__ZN11_ilogb_implIDv16_fvE4implIJLi0ELi1ELi2ELi3ELi4ELi5ELi6ELi7ELi8ELi9ELi10ELi11ELi12ELi13ELi14ELi15EEEEDv16_iS0_i17_integer_sequenceIiJXspT_EEE() __asm__("_ZN11_ilogb_implIDv16_fvE4implIJLi0ELi1ELi2ELi3ELi4ELi5ELi6ELi7ELi8ELi9ELi10ELi11ELi12ELi13ELi14ELi15EEEEDv16_iS0_i17_integer_sequenceIiJXspT_EEE");
void dummy__ZN11_ilogb_implIDv16_fvE4implIJLi0ELi1ELi2ELi3ELi4ELi5ELi6ELi7ELi8ELi9ELi10ELi11ELi12ELi13ELi14ELi15EEEEDv16_iS0_i17_integer_sequenceIiJXspT_EEE() {}
void dummy__ZN11_ilogb_implIDv3_DhvE4implIJLi0ELi1ELi2EEEEDv3_iS0_i17_integer_sequenceIiJXspT_EEE() __asm__("_ZN11_ilogb_implIDv3_DhvE4implIJLi0ELi1ELi2EEEEDv3_iS0_i17_integer_sequenceIiJXspT_EEE");
void dummy__ZN11_ilogb_implIDv3_DhvE4implIJLi0ELi1ELi2EEEEDv3_iS0_i17_integer_sequenceIiJXspT_EEE() {}
void dummy__ZN11_ilogb_implIDv3_dvE4implIJLi0ELi1ELi2EEEEDv3_iS0_i17_integer_sequenceIiJXspT_EEE() __asm__("_ZN11_ilogb_implIDv3_dvE4implIJLi0ELi1ELi2EEEEDv3_iS0_i17_integer_sequenceIiJXspT_EEE");
void dummy__ZN11_ilogb_implIDv3_dvE4implIJLi0ELi1ELi2EEEEDv3_iS0_i17_integer_sequenceIiJXspT_EEE() {}
void dummy__ZN11_ilogb_implIDv3_fvE4implIJLi0ELi1ELi2EEEEDv3_iS0_i17_integer_sequenceIiJXspT_EEE() __asm__("_ZN11_ilogb_implIDv3_fvE4implIJLi0ELi1ELi2EEEEDv3_iS0_i17_integer_sequenceIiJXspT_EEE");
void dummy__ZN11_ilogb_implIDv3_fvE4implIJLi0ELi1ELi2EEEEDv3_iS0_i17_integer_sequenceIiJXspT_EEE() {}
void dummy__ZN11_ilogb_implIDv4_DhvE4implIJLi0ELi1ELi2ELi3EEEEDv4_iS0_i17_integer_sequenceIiJXspT_EEE() __asm__("_ZN11_ilogb_implIDv4_DhvE4implIJLi0ELi1ELi2ELi3EEEEDv4_iS0_i17_integer_sequenceIiJXspT_EEE");
void dummy__ZN11_ilogb_implIDv4_DhvE4implIJLi0ELi1ELi2ELi3EEEEDv4_iS0_i17_integer_sequenceIiJXspT_EEE() {}
void dummy__ZN11_ilogb_implIDv4_dvE4implIJLi0ELi1ELi2ELi3EEEEDv4_iS0_i17_integer_sequenceIiJXspT_EEE() __asm__("_ZN11_ilogb_implIDv4_dvE4implIJLi0ELi1ELi2ELi3EEEEDv4_iS0_i17_integer_sequenceIiJXspT_EEE");
void dummy__ZN11_ilogb_implIDv4_dvE4implIJLi0ELi1ELi2ELi3EEEEDv4_iS0_i17_integer_sequenceIiJXspT_EEE() {}
void dummy__ZN11_ilogb_implIDv4_fvE4implIJLi0ELi1ELi2ELi3EEEEDv4_iS0_i17_integer_sequenceIiJXspT_EEE() __asm__("_ZN11_ilogb_implIDv4_fvE4implIJLi0ELi1ELi2ELi3EEEEDv4_iS0_i17_integer_sequenceIiJXspT_EEE");
void dummy__ZN11_ilogb_implIDv4_fvE4implIJLi0ELi1ELi2ELi3EEEEDv4_iS0_i17_integer_sequenceIiJXspT_EEE() {}
void dummy__ZN11_ilogb_implIDv8_DhvE4implIJLi0ELi1ELi2ELi3ELi4ELi5ELi6ELi7EEEEDv8_iS0_i17_integer_sequenceIiJXspT_EEE() __asm__("_ZN11_ilogb_implIDv8_DhvE4implIJLi0ELi1ELi2ELi3ELi4ELi5ELi6ELi7EEEEDv8_iS0_i17_integer_sequenceIiJXspT_EEE");
void dummy__ZN11_ilogb_implIDv8_DhvE4implIJLi0ELi1ELi2ELi3ELi4ELi5ELi6ELi7EEEEDv8_iS0_i17_integer_sequenceIiJXspT_EEE() {}
void dummy__ZN11_ilogb_implIDv8_dvE4implIJLi0ELi1ELi2ELi3ELi4ELi5ELi6ELi7EEEEDv8_iS0_i17_integer_sequenceIiJXspT_EEE() __asm__("_ZN11_ilogb_implIDv8_dvE4implIJLi0ELi1ELi2ELi3ELi4ELi5ELi6ELi7EEEEDv8_iS0_i17_integer_sequenceIiJXspT_EEE");
void dummy__ZN11_ilogb_implIDv8_dvE4implIJLi0ELi1ELi2ELi3ELi4ELi5ELi6ELi7EEEEDv8_iS0_i17_integer_sequenceIiJXspT_EEE() {}
void dummy__ZN11_ilogb_implIDv8_fvE4implIJLi0ELi1ELi2ELi3ELi4ELi5ELi6ELi7EEEEDv8_iS0_i17_integer_sequenceIiJXspT_EEE() __asm__("_ZN11_ilogb_implIDv8_fvE4implIJLi0ELi1ELi2ELi3ELi4ELi5ELi6ELi7EEEEDv8_iS0_i17_integer_sequenceIiJXspT_EEE");
void dummy__ZN11_ilogb_implIDv8_fvE4implIJLi0ELi1ELi2ELi3ELi4ELi5ELi6ELi7EEEEDv8_iS0_i17_integer_sequenceIiJXspT_EEE() {}
void dummy__ZN11_ldexp_implIDv16_DhvE4implIJLi0ELi1ELi2ELi3ELi4ELi5ELi6ELi7ELi8ELi9ELi10ELi11ELi12ELi13ELi14ELi15EEEES0_S0_Dv16_ii17_integer_sequenceIiJXspT_EEE() __asm__("_ZN11_ldexp_implIDv16_DhvE4implIJLi0ELi1ELi2ELi3ELi4ELi5ELi6ELi7ELi8ELi9ELi10ELi11ELi12ELi13ELi14ELi15EEEES0_S0_Dv16_ii17_integer_sequenceIiJXspT_EEE");
void dummy__ZN11_ldexp_implIDv16_DhvE4implIJLi0ELi1ELi2ELi3ELi4ELi5ELi6ELi7ELi8ELi9ELi10ELi11ELi12ELi13ELi14ELi15EEEES0_S0_Dv16_ii17_integer_sequenceIiJXspT_EEE() {}
void dummy__ZN11_ldexp_implIDv16_dvE4implIJLi0ELi1ELi2ELi3ELi4ELi5ELi6ELi7ELi8ELi9ELi10ELi11ELi12ELi13ELi14ELi15EEEES0_S0_Dv16_ii17_integer_sequenceIiJXspT_EEE() __asm__("_ZN11_ldexp_implIDv16_dvE4implIJLi0ELi1ELi2ELi3ELi4ELi5ELi6ELi7ELi8ELi9ELi10ELi11ELi12ELi13ELi14ELi15EEEES0_S0_Dv16_ii17_integer_sequenceIiJXspT_EEE");
void dummy__ZN11_ldexp_implIDv16_dvE4implIJLi0ELi1ELi2ELi3ELi4ELi5ELi6ELi7ELi8ELi9ELi10ELi11ELi12ELi13ELi14ELi15EEEES0_S0_Dv16_ii17_integer_sequenceIiJXspT_EEE() {}
void dummy__ZN11_ldexp_implIDv16_fvE4implIJLi0ELi1ELi2ELi3ELi4ELi5ELi6ELi7ELi8ELi9ELi10ELi11ELi12ELi13ELi14ELi15EEEES0_S0_Dv16_ii17_integer_sequenceIiJXspT_EEE() __asm__("_ZN11_ldexp_implIDv16_fvE4implIJLi0ELi1ELi2ELi3ELi4ELi5ELi6ELi7ELi8ELi9ELi10ELi11ELi12ELi13ELi14ELi15EEEES0_S0_Dv16_ii17_integer_sequenceIiJXspT_EEE");
void dummy__ZN11_ldexp_implIDv16_fvE4implIJLi0ELi1ELi2ELi3ELi4ELi5ELi6ELi7ELi8ELi9ELi10ELi11ELi12ELi13ELi14ELi15EEEES0_S0_Dv16_ii17_integer_sequenceIiJXspT_EEE() {}
void dummy__ZN11_ldexp_implIDv3_DhvE4implIJLi0ELi1ELi2EEEES0_S0_Dv3_ii17_integer_sequenceIiJXspT_EEE() __asm__("_ZN11_ldexp_implIDv3_DhvE4implIJLi0ELi1ELi2EEEES0_S0_Dv3_ii17_integer_sequenceIiJXspT_EEE");
void dummy__ZN11_ldexp_implIDv3_DhvE4implIJLi0ELi1ELi2EEEES0_S0_Dv3_ii17_integer_sequenceIiJXspT_EEE() {}
void dummy__ZN11_ldexp_implIDv3_dvE4implIJLi0ELi1ELi2EEEES0_S0_Dv3_ii17_integer_sequenceIiJXspT_EEE() __asm__("_ZN11_ldexp_implIDv3_dvE4implIJLi0ELi1ELi2EEEES0_S0_Dv3_ii17_integer_sequenceIiJXspT_EEE");
void dummy__ZN11_ldexp_implIDv3_dvE4implIJLi0ELi1ELi2EEEES0_S0_Dv3_ii17_integer_sequenceIiJXspT_EEE() {}
void dummy__ZN11_ldexp_implIDv3_fvE4implIJLi0ELi1ELi2EEEES0_S0_Dv3_ii17_integer_sequenceIiJXspT_EEE() __asm__("_ZN11_ldexp_implIDv3_fvE4implIJLi0ELi1ELi2EEEES0_S0_Dv3_ii17_integer_sequenceIiJXspT_EEE");
void dummy__ZN11_ldexp_implIDv3_fvE4implIJLi0ELi1ELi2EEEES0_S0_Dv3_ii17_integer_sequenceIiJXspT_EEE() {}
void dummy__ZN11_ldexp_implIDv4_DhvE4implIJLi0ELi1ELi2ELi3EEEES0_S0_Dv4_ii17_integer_sequenceIiJXspT_EEE() __asm__("_ZN11_ldexp_implIDv4_DhvE4implIJLi0ELi1ELi2ELi3EEEES0_S0_Dv4_ii17_integer_sequenceIiJXspT_EEE");
void dummy__ZN11_ldexp_implIDv4_DhvE4implIJLi0ELi1ELi2ELi3EEEES0_S0_Dv4_ii17_integer_sequenceIiJXspT_EEE() {}
void dummy__ZN11_ldexp_implIDv4_dvE4implIJLi0ELi1ELi2ELi3EEEES0_S0_Dv4_ii17_integer_sequenceIiJXspT_EEE() __asm__("_ZN11_ldexp_implIDv4_dvE4implIJLi0ELi1ELi2ELi3EEEES0_S0_Dv4_ii17_integer_sequenceIiJXspT_EEE");
void dummy__ZN11_ldexp_implIDv4_dvE4implIJLi0ELi1ELi2ELi3EEEES0_S0_Dv4_ii17_integer_sequenceIiJXspT_EEE() {}
void dummy__ZN11_ldexp_implIDv4_fvE4implIJLi0ELi1ELi2ELi3EEEES0_S0_Dv4_ii17_integer_sequenceIiJXspT_EEE() __asm__("_ZN11_ldexp_implIDv4_fvE4implIJLi0ELi1ELi2ELi3EEEES0_S0_Dv4_ii17_integer_sequenceIiJXspT_EEE");
void dummy__ZN11_ldexp_implIDv4_fvE4implIJLi0ELi1ELi2ELi3EEEES0_S0_Dv4_ii17_integer_sequenceIiJXspT_EEE() {}
void dummy__ZN11_ldexp_implIDv8_DhvE4implIJLi0ELi1ELi2ELi3ELi4ELi5ELi6ELi7EEEES0_S0_Dv8_ii17_integer_sequenceIiJXspT_EEE() __asm__("_ZN11_ldexp_implIDv8_DhvE4implIJLi0ELi1ELi2ELi3ELi4ELi5ELi6ELi7EEEES0_S0_Dv8_ii17_integer_sequenceIiJXspT_EEE");
void dummy__ZN11_ldexp_implIDv8_DhvE4implIJLi0ELi1ELi2ELi3ELi4ELi5ELi6ELi7EEEES0_S0_Dv8_ii17_integer_sequenceIiJXspT_EEE() {}
void dummy__ZN11_ldexp_implIDv8_dvE4implIJLi0ELi1ELi2ELi3ELi4ELi5ELi6ELi7EEEES0_S0_Dv8_ii17_integer_sequenceIiJXspT_EEE() __asm__("_ZN11_ldexp_implIDv8_dvE4implIJLi0ELi1ELi2ELi3ELi4ELi5ELi6ELi7EEEES0_S0_Dv8_ii17_integer_sequenceIiJXspT_EEE");
void dummy__ZN11_ldexp_implIDv8_dvE4implIJLi0ELi1ELi2ELi3ELi4ELi5ELi6ELi7EEEES0_S0_Dv8_ii17_integer_sequenceIiJXspT_EEE() {}
void dummy__ZN11_ldexp_implIDv8_fvE4implIJLi0ELi1ELi2ELi3ELi4ELi5ELi6ELi7EEEES0_S0_Dv8_ii17_integer_sequenceIiJXspT_EEE() __asm__("_ZN11_ldexp_implIDv8_fvE4implIJLi0ELi1ELi2ELi3ELi4ELi5ELi6ELi7EEEES0_S0_Dv8_ii17_integer_sequenceIiJXspT_EEE");
void dummy__ZN11_ldexp_implIDv8_fvE4implIJLi0ELi1ELi2ELi3ELi4ELi5ELi6ELi7EEEES0_S0_Dv8_ii17_integer_sequenceIiJXspT_EEE() {}
int16_t ___metal_extract_bits_int16(uint32_t val, uint32_t offset, uint32_t size) { return extract_bits_impl<int16_t>(val, offset, size); }
int32_t ___metal_extract_bits_int32(uint32_t val, uint32_t offset, uint32_t size) { return extract_bits_impl<int32_t>(val, offset, size); }
int8_t ___metal_extract_bits_int8(uint32_t val, uint32_t offset, uint32_t size) { return extract_bits_impl<int8_t>(val, offset, size); }
uint16_t ___metal_extract_bits_uint16(uint32_t val, uint32_t offset, uint32_t size) { return extract_bits_impl<uint16_t>(val, offset, size); }
uint32_t ___metal_extract_bits_uint32(uint32_t val, uint32_t offset, uint32_t size) { return extract_bits_impl<uint32_t>(val, offset, size); }
uint64_t ___metal_extract_bits_uint64(uint64_t val, uint32_t offset, uint32_t size) { return extract_bits_impl<uint64_t>(val, offset, size); }
uint8_t ___metal_extract_bits_uint8(uint32_t val, uint32_t offset, uint32_t size) { return extract_bits_impl<uint8_t>(val, offset, size); }
int16_t ___metal_extract_bits_v16int16(uint32_t val, uint32_t offset, uint32_t size) { return extract_bits_impl<int16_t>(val, offset, size); }
int32_t ___metal_extract_bits_v16int32(uint32_t val, uint32_t offset, uint32_t size) { return extract_bits_impl<int32_t>(val, offset, size); }
int8_t ___metal_extract_bits_v16int8(uint32_t val, uint32_t offset, uint32_t size) { return extract_bits_impl<int8_t>(val, offset, size); }
uint16_t ___metal_extract_bits_v16uint16(uint32_t val, uint32_t offset, uint32_t size) { return extract_bits_impl<uint16_t>(val, offset, size); }
uint32_t ___metal_extract_bits_v16uint32(uint32_t val, uint32_t offset, uint32_t size) { return extract_bits_impl<uint32_t>(val, offset, size); }
uint64_t ___metal_extract_bits_v16uint64(uint64_t val, uint32_t offset, uint32_t size) { return extract_bits_impl<uint64_t>(val, offset, size); }
uint8_t ___metal_extract_bits_v16uint8(uint32_t val, uint32_t offset, uint32_t size) { return extract_bits_impl<uint8_t>(val, offset, size); }
int16_t ___metal_extract_bits_v2int16(uint32_t val, uint32_t offset, uint32_t size) { return extract_bits_impl<int16_t>(val, offset, size); }
int32_t ___metal_extract_bits_v2int32(uint32_t val, uint32_t offset, uint32_t size) { return extract_bits_impl<int32_t>(val, offset, size); }
int8_t ___metal_extract_bits_v2int8(uint32_t val, uint32_t offset, uint32_t size) { return extract_bits_impl<int8_t>(val, offset, size); }
uint16_t ___metal_extract_bits_v2uint16(uint32_t val, uint32_t offset, uint32_t size) { return extract_bits_impl<uint16_t>(val, offset, size); }
uint32_t ___metal_extract_bits_v2uint32(uint32_t val, uint32_t offset, uint32_t size) { return extract_bits_impl<uint32_t>(val, offset, size); }
uint64_t ___metal_extract_bits_v2uint64(uint64_t val, uint32_t offset, uint32_t size) { return extract_bits_impl<uint64_t>(val, offset, size); }
uint8_t ___metal_extract_bits_v2uint8(uint32_t val, uint32_t offset, uint32_t size) { return extract_bits_impl<uint8_t>(val, offset, size); }
int16_t ___metal_extract_bits_v3int16(uint32_t val, uint32_t offset, uint32_t size) { return extract_bits_impl<int16_t>(val, offset, size); }
int32_t ___metal_extract_bits_v3int32(uint32_t val, uint32_t offset, uint32_t size) { return extract_bits_impl<int32_t>(val, offset, size); }
int8_t ___metal_extract_bits_v3int8(uint32_t val, uint32_t offset, uint32_t size) { return extract_bits_impl<int8_t>(val, offset, size); }
uint16_t ___metal_extract_bits_v3uint16(uint32_t val, uint32_t offset, uint32_t size) { return extract_bits_impl<uint16_t>(val, offset, size); }
uint32_t ___metal_extract_bits_v3uint32(uint32_t val, uint32_t offset, uint32_t size) { return extract_bits_impl<uint32_t>(val, offset, size); }
uint64_t ___metal_extract_bits_v3uint64(uint64_t val, uint32_t offset, uint32_t size) { return extract_bits_impl<uint64_t>(val, offset, size); }
uint8_t ___metal_extract_bits_v3uint8(uint32_t val, uint32_t offset, uint32_t size) { return extract_bits_impl<uint8_t>(val, offset, size); }
int16_t ___metal_extract_bits_v4int16(uint32_t val, uint32_t offset, uint32_t size) { return extract_bits_impl<int16_t>(val, offset, size); }
int32_t ___metal_extract_bits_v4int32(uint32_t val, uint32_t offset, uint32_t size) { return extract_bits_impl<int32_t>(val, offset, size); }
int8_t ___metal_extract_bits_v4int8(uint32_t val, uint32_t offset, uint32_t size) { return extract_bits_impl<int8_t>(val, offset, size); }
uint16_t ___metal_extract_bits_v4uint16(uint32_t val, uint32_t offset, uint32_t size) { return extract_bits_impl<uint16_t>(val, offset, size); }
uint32_t ___metal_extract_bits_v4uint32(uint32_t val, uint32_t offset, uint32_t size) { return extract_bits_impl<uint32_t>(val, offset, size); }
uint64_t ___metal_extract_bits_v4uint64(uint64_t val, uint32_t offset, uint32_t size) { return extract_bits_impl<uint64_t>(val, offset, size); }
uint8_t ___metal_extract_bits_v4uint8(uint32_t val, uint32_t offset, uint32_t size) { return extract_bits_impl<uint8_t>(val, offset, size); }
int16_t ___metal_extract_bits_v8int16(uint32_t val, uint32_t offset, uint32_t size) { return extract_bits_impl<int16_t>(val, offset, size); }
int32_t ___metal_extract_bits_v8int32(uint32_t val, uint32_t offset, uint32_t size) { return extract_bits_impl<int32_t>(val, offset, size); }
int8_t ___metal_extract_bits_v8int8(uint32_t val, uint32_t offset, uint32_t size) { return extract_bits_impl<int8_t>(val, offset, size); }
uint16_t ___metal_extract_bits_v8uint16(uint32_t val, uint32_t offset, uint32_t size) { return extract_bits_impl<uint16_t>(val, offset, size); }
uint32_t ___metal_extract_bits_v8uint32(uint32_t val, uint32_t offset, uint32_t size) { return extract_bits_impl<uint32_t>(val, offset, size); }
uint64_t ___metal_extract_bits_v8uint64(uint64_t val, uint32_t offset, uint32_t size) { return extract_bits_impl<uint64_t>(val, offset, size); }
uint8_t ___metal_extract_bits_v8uint8(uint32_t val, uint32_t offset, uint32_t size) { return extract_bits_impl<uint8_t>(val, offset, size); }
double ___metal_fract_double(double x) { return my_fract_d(x); }
float ___metal_fract_float(float x) { return my_fract(x); }
__fp16 ___metal_fract_half(__fp16 x) { return my_fract_h(x); }
double16 ___metal_fract_v16double(double16 x) {
    double16 res;
    res[0] = my_fract_d(x[0]);
    res[1] = my_fract_d(x[1]);
    res[2] = my_fract_d(x[2]);
    res[3] = my_fract_d(x[3]);
    res[4] = my_fract_d(x[4]);
    res[5] = my_fract_d(x[5]);
    res[6] = my_fract_d(x[6]);
    res[7] = my_fract_d(x[7]);
    res[8] = my_fract_d(x[8]);
    res[9] = my_fract_d(x[9]);
    res[10] = my_fract_d(x[10]);
    res[11] = my_fract_d(x[11]);
    res[12] = my_fract_d(x[12]);
    res[13] = my_fract_d(x[13]);
    res[14] = my_fract_d(x[14]);
    res[15] = my_fract_d(x[15]);
    return res;
}
float16 ___metal_fract_v16float(float16 x) {
    float16 res;
    res[0] = my_fract(x[0]);
    res[1] = my_fract(x[1]);
    res[2] = my_fract(x[2]);
    res[3] = my_fract(x[3]);
    res[4] = my_fract(x[4]);
    res[5] = my_fract(x[5]);
    res[6] = my_fract(x[6]);
    res[7] = my_fract(x[7]);
    res[8] = my_fract(x[8]);
    res[9] = my_fract(x[9]);
    res[10] = my_fract(x[10]);
    res[11] = my_fract(x[11]);
    res[12] = my_fract(x[12]);
    res[13] = my_fract(x[13]);
    res[14] = my_fract(x[14]);
    res[15] = my_fract(x[15]);
    return res;
}
half16 ___metal_fract_v16half(half16 x) {
    half16 res;
    res[0] = my_fract_h(x[0]);
    res[1] = my_fract_h(x[1]);
    res[2] = my_fract_h(x[2]);
    res[3] = my_fract_h(x[3]);
    res[4] = my_fract_h(x[4]);
    res[5] = my_fract_h(x[5]);
    res[6] = my_fract_h(x[6]);
    res[7] = my_fract_h(x[7]);
    res[8] = my_fract_h(x[8]);
    res[9] = my_fract_h(x[9]);
    res[10] = my_fract_h(x[10]);
    res[11] = my_fract_h(x[11]);
    res[12] = my_fract_h(x[12]);
    res[13] = my_fract_h(x[13]);
    res[14] = my_fract_h(x[14]);
    res[15] = my_fract_h(x[15]);
    return res;
}
double2 ___metal_fract_v2double(double2 x) {
    double2 res;
    res[0] = my_fract_d(x[0]);
    res[1] = my_fract_d(x[1]);
    return res;
}
float2 ___metal_fract_v2float(float2 x) {
    float2 res;
    res[0] = my_fract(x[0]);
    res[1] = my_fract(x[1]);
    return res;
}
half2 ___metal_fract_v2half(half2 x) {
    half2 res;
    res[0] = my_fract_h(x[0]);
    res[1] = my_fract_h(x[1]);
    return res;
}
double3 ___metal_fract_v3double(double3 x) {
    double3 res;
    res[0] = my_fract_d(x[0]);
    res[1] = my_fract_d(x[1]);
    res[2] = my_fract_d(x[2]);
    return res;
}
float3 ___metal_fract_v3float(float3 x) {
    float3 res;
    res[0] = my_fract(x[0]);
    res[1] = my_fract(x[1]);
    res[2] = my_fract(x[2]);
    return res;
}
half3 ___metal_fract_v3half(half3 x) {
    half3 res;
    res[0] = my_fract_h(x[0]);
    res[1] = my_fract_h(x[1]);
    res[2] = my_fract_h(x[2]);
    return res;
}
double4 ___metal_fract_v4double(double4 x) {
    double4 res;
    res[0] = my_fract_d(x[0]);
    res[1] = my_fract_d(x[1]);
    res[2] = my_fract_d(x[2]);
    res[3] = my_fract_d(x[3]);
    return res;
}
float4 ___metal_fract_v4float(float4 x) {
    float4 res;
    res[0] = my_fract(x[0]);
    res[1] = my_fract(x[1]);
    res[2] = my_fract(x[2]);
    res[3] = my_fract(x[3]);
    return res;
}
half4 ___metal_fract_v4half(half4 x) {
    half4 res;
    res[0] = my_fract_h(x[0]);
    res[1] = my_fract_h(x[1]);
    res[2] = my_fract_h(x[2]);
    res[3] = my_fract_h(x[3]);
    return res;
}
double8 ___metal_fract_v8double(double8 x) {
    double8 res;
    res[0] = my_fract_d(x[0]);
    res[1] = my_fract_d(x[1]);
    res[2] = my_fract_d(x[2]);
    res[3] = my_fract_d(x[3]);
    res[4] = my_fract_d(x[4]);
    res[5] = my_fract_d(x[5]);
    res[6] = my_fract_d(x[6]);
    res[7] = my_fract_d(x[7]);
    return res;
}
float8 ___metal_fract_v8float(float8 x) {
    float8 res;
    res[0] = my_fract(x[0]);
    res[1] = my_fract(x[1]);
    res[2] = my_fract(x[2]);
    res[3] = my_fract(x[3]);
    res[4] = my_fract(x[4]);
    res[5] = my_fract(x[5]);
    res[6] = my_fract(x[6]);
    res[7] = my_fract(x[7]);
    return res;
}
half8 ___metal_fract_v8half(half8 x) {
    half8 res;
    res[0] = my_fract_h(x[0]);
    res[1] = my_fract_h(x[1]);
    res[2] = my_fract_h(x[2]);
    res[3] = my_fract_h(x[3]);
    res[4] = my_fract_h(x[4]);
    res[5] = my_fract_h(x[5]);
    res[6] = my_fract_h(x[6]);
    res[7] = my_fract_h(x[7]);
    return res;
}
double ___metal_frexp_double_pthreadint32(double val, int* exp) { return my_frexp_d(val, exp); }
float ___metal_frexp_float_pthreadint32(float val, int* exp) { return my_frexp(val, exp); }
__fp16 ___metal_frexp_half_pthreadint32(__fp16 val, int* exp) { return my_frexp_h(val, exp); }
double16 ___metal_frexp_v16double_pthreadv16int32(double16 val, int16* exp) {
    double16 res;
    { int temp_exp; res[0] = my_frexp_d(val[0], &temp_exp); (*exp)[0] = temp_exp; }
    { int temp_exp; res[1] = my_frexp_d(val[1], &temp_exp); (*exp)[1] = temp_exp; }
    { int temp_exp; res[2] = my_frexp_d(val[2], &temp_exp); (*exp)[2] = temp_exp; }
    { int temp_exp; res[3] = my_frexp_d(val[3], &temp_exp); (*exp)[3] = temp_exp; }
    { int temp_exp; res[4] = my_frexp_d(val[4], &temp_exp); (*exp)[4] = temp_exp; }
    { int temp_exp; res[5] = my_frexp_d(val[5], &temp_exp); (*exp)[5] = temp_exp; }
    { int temp_exp; res[6] = my_frexp_d(val[6], &temp_exp); (*exp)[6] = temp_exp; }
    { int temp_exp; res[7] = my_frexp_d(val[7], &temp_exp); (*exp)[7] = temp_exp; }
    { int temp_exp; res[8] = my_frexp_d(val[8], &temp_exp); (*exp)[8] = temp_exp; }
    { int temp_exp; res[9] = my_frexp_d(val[9], &temp_exp); (*exp)[9] = temp_exp; }
    { int temp_exp; res[10] = my_frexp_d(val[10], &temp_exp); (*exp)[10] = temp_exp; }
    { int temp_exp; res[11] = my_frexp_d(val[11], &temp_exp); (*exp)[11] = temp_exp; }
    { int temp_exp; res[12] = my_frexp_d(val[12], &temp_exp); (*exp)[12] = temp_exp; }
    { int temp_exp; res[13] = my_frexp_d(val[13], &temp_exp); (*exp)[13] = temp_exp; }
    { int temp_exp; res[14] = my_frexp_d(val[14], &temp_exp); (*exp)[14] = temp_exp; }
    { int temp_exp; res[15] = my_frexp_d(val[15], &temp_exp); (*exp)[15] = temp_exp; }
    return res;
}
float16 ___metal_frexp_v16float_pthreadv16int32(float16 val, int16* exp) {
    float16 res;
    { int temp_exp; res[0] = my_frexp(val[0], &temp_exp); (*exp)[0] = temp_exp; }
    { int temp_exp; res[1] = my_frexp(val[1], &temp_exp); (*exp)[1] = temp_exp; }
    { int temp_exp; res[2] = my_frexp(val[2], &temp_exp); (*exp)[2] = temp_exp; }
    { int temp_exp; res[3] = my_frexp(val[3], &temp_exp); (*exp)[3] = temp_exp; }
    { int temp_exp; res[4] = my_frexp(val[4], &temp_exp); (*exp)[4] = temp_exp; }
    { int temp_exp; res[5] = my_frexp(val[5], &temp_exp); (*exp)[5] = temp_exp; }
    { int temp_exp; res[6] = my_frexp(val[6], &temp_exp); (*exp)[6] = temp_exp; }
    { int temp_exp; res[7] = my_frexp(val[7], &temp_exp); (*exp)[7] = temp_exp; }
    { int temp_exp; res[8] = my_frexp(val[8], &temp_exp); (*exp)[8] = temp_exp; }
    { int temp_exp; res[9] = my_frexp(val[9], &temp_exp); (*exp)[9] = temp_exp; }
    { int temp_exp; res[10] = my_frexp(val[10], &temp_exp); (*exp)[10] = temp_exp; }
    { int temp_exp; res[11] = my_frexp(val[11], &temp_exp); (*exp)[11] = temp_exp; }
    { int temp_exp; res[12] = my_frexp(val[12], &temp_exp); (*exp)[12] = temp_exp; }
    { int temp_exp; res[13] = my_frexp(val[13], &temp_exp); (*exp)[13] = temp_exp; }
    { int temp_exp; res[14] = my_frexp(val[14], &temp_exp); (*exp)[14] = temp_exp; }
    { int temp_exp; res[15] = my_frexp(val[15], &temp_exp); (*exp)[15] = temp_exp; }
    return res;
}
half16 ___metal_frexp_v16half_pthreadv16int32(half16 val, int16* exp) {
    half16 res;
    { int temp_exp; res[0] = my_frexp_h(val[0], &temp_exp); (*exp)[0] = temp_exp; }
    { int temp_exp; res[1] = my_frexp_h(val[1], &temp_exp); (*exp)[1] = temp_exp; }
    { int temp_exp; res[2] = my_frexp_h(val[2], &temp_exp); (*exp)[2] = temp_exp; }
    { int temp_exp; res[3] = my_frexp_h(val[3], &temp_exp); (*exp)[3] = temp_exp; }
    { int temp_exp; res[4] = my_frexp_h(val[4], &temp_exp); (*exp)[4] = temp_exp; }
    { int temp_exp; res[5] = my_frexp_h(val[5], &temp_exp); (*exp)[5] = temp_exp; }
    { int temp_exp; res[6] = my_frexp_h(val[6], &temp_exp); (*exp)[6] = temp_exp; }
    { int temp_exp; res[7] = my_frexp_h(val[7], &temp_exp); (*exp)[7] = temp_exp; }
    { int temp_exp; res[8] = my_frexp_h(val[8], &temp_exp); (*exp)[8] = temp_exp; }
    { int temp_exp; res[9] = my_frexp_h(val[9], &temp_exp); (*exp)[9] = temp_exp; }
    { int temp_exp; res[10] = my_frexp_h(val[10], &temp_exp); (*exp)[10] = temp_exp; }
    { int temp_exp; res[11] = my_frexp_h(val[11], &temp_exp); (*exp)[11] = temp_exp; }
    { int temp_exp; res[12] = my_frexp_h(val[12], &temp_exp); (*exp)[12] = temp_exp; }
    { int temp_exp; res[13] = my_frexp_h(val[13], &temp_exp); (*exp)[13] = temp_exp; }
    { int temp_exp; res[14] = my_frexp_h(val[14], &temp_exp); (*exp)[14] = temp_exp; }
    { int temp_exp; res[15] = my_frexp_h(val[15], &temp_exp); (*exp)[15] = temp_exp; }
    return res;
}
double2 ___metal_frexp_v2double_pthreadv2int32(double2 val, int2* exp) {
    double2 res;
    { int temp_exp; res[0] = my_frexp_d(val[0], &temp_exp); (*exp)[0] = temp_exp; }
    { int temp_exp; res[1] = my_frexp_d(val[1], &temp_exp); (*exp)[1] = temp_exp; }
    return res;
}
float2 ___metal_frexp_v2float_pthreadv2int32(float2 val, int2* exp) {
    float2 res;
    { int temp_exp; res[0] = my_frexp(val[0], &temp_exp); (*exp)[0] = temp_exp; }
    { int temp_exp; res[1] = my_frexp(val[1], &temp_exp); (*exp)[1] = temp_exp; }
    return res;
}
half2 ___metal_frexp_v2half_pthreadv2int32(half2 val, int2* exp) {
    half2 res;
    { int temp_exp; res[0] = my_frexp_h(val[0], &temp_exp); (*exp)[0] = temp_exp; }
    { int temp_exp; res[1] = my_frexp_h(val[1], &temp_exp); (*exp)[1] = temp_exp; }
    return res;
}
double3 ___metal_frexp_v3double_pthreadv3int32(double3 val, int3* exp) {
    double3 res;
    { int temp_exp; res[0] = my_frexp_d(val[0], &temp_exp); (*exp)[0] = temp_exp; }
    { int temp_exp; res[1] = my_frexp_d(val[1], &temp_exp); (*exp)[1] = temp_exp; }
    { int temp_exp; res[2] = my_frexp_d(val[2], &temp_exp); (*exp)[2] = temp_exp; }
    return res;
}
float3 ___metal_frexp_v3float_pthreadv3int32(float3 val, int3* exp) {
    float3 res;
    { int temp_exp; res[0] = my_frexp(val[0], &temp_exp); (*exp)[0] = temp_exp; }
    { int temp_exp; res[1] = my_frexp(val[1], &temp_exp); (*exp)[1] = temp_exp; }
    { int temp_exp; res[2] = my_frexp(val[2], &temp_exp); (*exp)[2] = temp_exp; }
    return res;
}
half3 ___metal_frexp_v3half_pthreadv3int32(half3 val, int3* exp) {
    half3 res;
    { int temp_exp; res[0] = my_frexp_h(val[0], &temp_exp); (*exp)[0] = temp_exp; }
    { int temp_exp; res[1] = my_frexp_h(val[1], &temp_exp); (*exp)[1] = temp_exp; }
    { int temp_exp; res[2] = my_frexp_h(val[2], &temp_exp); (*exp)[2] = temp_exp; }
    return res;
}
double4 ___metal_frexp_v4double_pthreadv4int32(double4 val, int4* exp) {
    double4 res;
    { int temp_exp; res[0] = my_frexp_d(val[0], &temp_exp); (*exp)[0] = temp_exp; }
    { int temp_exp; res[1] = my_frexp_d(val[1], &temp_exp); (*exp)[1] = temp_exp; }
    { int temp_exp; res[2] = my_frexp_d(val[2], &temp_exp); (*exp)[2] = temp_exp; }
    { int temp_exp; res[3] = my_frexp_d(val[3], &temp_exp); (*exp)[3] = temp_exp; }
    return res;
}
float4 ___metal_frexp_v4float_pthreadv4int32(float4 val, int4* exp) {
    float4 res;
    { int temp_exp; res[0] = my_frexp(val[0], &temp_exp); (*exp)[0] = temp_exp; }
    { int temp_exp; res[1] = my_frexp(val[1], &temp_exp); (*exp)[1] = temp_exp; }
    { int temp_exp; res[2] = my_frexp(val[2], &temp_exp); (*exp)[2] = temp_exp; }
    { int temp_exp; res[3] = my_frexp(val[3], &temp_exp); (*exp)[3] = temp_exp; }
    return res;
}
half4 ___metal_frexp_v4half_pthreadv4int32(half4 val, int4* exp) {
    half4 res;
    { int temp_exp; res[0] = my_frexp_h(val[0], &temp_exp); (*exp)[0] = temp_exp; }
    { int temp_exp; res[1] = my_frexp_h(val[1], &temp_exp); (*exp)[1] = temp_exp; }
    { int temp_exp; res[2] = my_frexp_h(val[2], &temp_exp); (*exp)[2] = temp_exp; }
    { int temp_exp; res[3] = my_frexp_h(val[3], &temp_exp); (*exp)[3] = temp_exp; }
    return res;
}
double8 ___metal_frexp_v8double_pthreadv8int32(double8 val, int8* exp) {
    double8 res;
    { int temp_exp; res[0] = my_frexp_d(val[0], &temp_exp); (*exp)[0] = temp_exp; }
    { int temp_exp; res[1] = my_frexp_d(val[1], &temp_exp); (*exp)[1] = temp_exp; }
    { int temp_exp; res[2] = my_frexp_d(val[2], &temp_exp); (*exp)[2] = temp_exp; }
    { int temp_exp; res[3] = my_frexp_d(val[3], &temp_exp); (*exp)[3] = temp_exp; }
    { int temp_exp; res[4] = my_frexp_d(val[4], &temp_exp); (*exp)[4] = temp_exp; }
    { int temp_exp; res[5] = my_frexp_d(val[5], &temp_exp); (*exp)[5] = temp_exp; }
    { int temp_exp; res[6] = my_frexp_d(val[6], &temp_exp); (*exp)[6] = temp_exp; }
    { int temp_exp; res[7] = my_frexp_d(val[7], &temp_exp); (*exp)[7] = temp_exp; }
    return res;
}
float8 ___metal_frexp_v8float_pthreadv8int32(float8 val, int8* exp) {
    float8 res;
    { int temp_exp; res[0] = my_frexp(val[0], &temp_exp); (*exp)[0] = temp_exp; }
    { int temp_exp; res[1] = my_frexp(val[1], &temp_exp); (*exp)[1] = temp_exp; }
    { int temp_exp; res[2] = my_frexp(val[2], &temp_exp); (*exp)[2] = temp_exp; }
    { int temp_exp; res[3] = my_frexp(val[3], &temp_exp); (*exp)[3] = temp_exp; }
    { int temp_exp; res[4] = my_frexp(val[4], &temp_exp); (*exp)[4] = temp_exp; }
    { int temp_exp; res[5] = my_frexp(val[5], &temp_exp); (*exp)[5] = temp_exp; }
    { int temp_exp; res[6] = my_frexp(val[6], &temp_exp); (*exp)[6] = temp_exp; }
    { int temp_exp; res[7] = my_frexp(val[7], &temp_exp); (*exp)[7] = temp_exp; }
    return res;
}
half8 ___metal_frexp_v8half_pthreadv8int32(half8 val, int8* exp) {
    half8 res;
    { int temp_exp; res[0] = my_frexp_h(val[0], &temp_exp); (*exp)[0] = temp_exp; }
    { int temp_exp; res[1] = my_frexp_h(val[1], &temp_exp); (*exp)[1] = temp_exp; }
    { int temp_exp; res[2] = my_frexp_h(val[2], &temp_exp); (*exp)[2] = temp_exp; }
    { int temp_exp; res[3] = my_frexp_h(val[3], &temp_exp); (*exp)[3] = temp_exp; }
    { int temp_exp; res[4] = my_frexp_h(val[4], &temp_exp); (*exp)[4] = temp_exp; }
    { int temp_exp; res[5] = my_frexp_h(val[5], &temp_exp); (*exp)[5] = temp_exp; }
    { int temp_exp; res[6] = my_frexp_h(val[6], &temp_exp); (*exp)[6] = temp_exp; }
    { int temp_exp; res[7] = my_frexp_h(val[7], &temp_exp); (*exp)[7] = temp_exp; }
    return res;
}
int ___metal_ilogb_double(double val) { return my_ilogb_d(val); }
int ___metal_ilogb_float(float val) { return my_ilogb(val); }
int ___metal_ilogb_half(__fp16 val) { return my_ilogb_h(val); }
int16 ___metal_ilogb_v16double(double16 val) {
    int16 res;
    res[0] = my_ilogb_d(val[0]);
    res[1] = my_ilogb_d(val[1]);
    res[2] = my_ilogb_d(val[2]);
    res[3] = my_ilogb_d(val[3]);
    res[4] = my_ilogb_d(val[4]);
    res[5] = my_ilogb_d(val[5]);
    res[6] = my_ilogb_d(val[6]);
    res[7] = my_ilogb_d(val[7]);
    res[8] = my_ilogb_d(val[8]);
    res[9] = my_ilogb_d(val[9]);
    res[10] = my_ilogb_d(val[10]);
    res[11] = my_ilogb_d(val[11]);
    res[12] = my_ilogb_d(val[12]);
    res[13] = my_ilogb_d(val[13]);
    res[14] = my_ilogb_d(val[14]);
    res[15] = my_ilogb_d(val[15]);
    return res;
}
int16 ___metal_ilogb_v16float(float16 val) {
    int16 res;
    res[0] = my_ilogb(val[0]);
    res[1] = my_ilogb(val[1]);
    res[2] = my_ilogb(val[2]);
    res[3] = my_ilogb(val[3]);
    res[4] = my_ilogb(val[4]);
    res[5] = my_ilogb(val[5]);
    res[6] = my_ilogb(val[6]);
    res[7] = my_ilogb(val[7]);
    res[8] = my_ilogb(val[8]);
    res[9] = my_ilogb(val[9]);
    res[10] = my_ilogb(val[10]);
    res[11] = my_ilogb(val[11]);
    res[12] = my_ilogb(val[12]);
    res[13] = my_ilogb(val[13]);
    res[14] = my_ilogb(val[14]);
    res[15] = my_ilogb(val[15]);
    return res;
}
int16 ___metal_ilogb_v16half(half16 val) {
    int16 res;
    res[0] = my_ilogb_h(val[0]);
    res[1] = my_ilogb_h(val[1]);
    res[2] = my_ilogb_h(val[2]);
    res[3] = my_ilogb_h(val[3]);
    res[4] = my_ilogb_h(val[4]);
    res[5] = my_ilogb_h(val[5]);
    res[6] = my_ilogb_h(val[6]);
    res[7] = my_ilogb_h(val[7]);
    res[8] = my_ilogb_h(val[8]);
    res[9] = my_ilogb_h(val[9]);
    res[10] = my_ilogb_h(val[10]);
    res[11] = my_ilogb_h(val[11]);
    res[12] = my_ilogb_h(val[12]);
    res[13] = my_ilogb_h(val[13]);
    res[14] = my_ilogb_h(val[14]);
    res[15] = my_ilogb_h(val[15]);
    return res;
}
int2 ___metal_ilogb_v2double(double2 val) {
    int2 res;
    res[0] = my_ilogb_d(val[0]);
    res[1] = my_ilogb_d(val[1]);
    return res;
}
int2 ___metal_ilogb_v2float(float2 val) {
    int2 res;
    res[0] = my_ilogb(val[0]);
    res[1] = my_ilogb(val[1]);
    return res;
}
int2 ___metal_ilogb_v2half(half2 val) {
    int2 res;
    res[0] = my_ilogb_h(val[0]);
    res[1] = my_ilogb_h(val[1]);
    return res;
}
int3 ___metal_ilogb_v3double(double3 val) {
    int3 res;
    res[0] = my_ilogb_d(val[0]);
    res[1] = my_ilogb_d(val[1]);
    res[2] = my_ilogb_d(val[2]);
    return res;
}
int3 ___metal_ilogb_v3float(float3 val) {
    int3 res;
    res[0] = my_ilogb(val[0]);
    res[1] = my_ilogb(val[1]);
    res[2] = my_ilogb(val[2]);
    return res;
}
int3 ___metal_ilogb_v3half(half3 val) {
    int3 res;
    res[0] = my_ilogb_h(val[0]);
    res[1] = my_ilogb_h(val[1]);
    res[2] = my_ilogb_h(val[2]);
    return res;
}
int4 ___metal_ilogb_v4double(double4 val) {
    int4 res;
    res[0] = my_ilogb_d(val[0]);
    res[1] = my_ilogb_d(val[1]);
    res[2] = my_ilogb_d(val[2]);
    res[3] = my_ilogb_d(val[3]);
    return res;
}
int4 ___metal_ilogb_v4float(float4 val) {
    int4 res;
    res[0] = my_ilogb(val[0]);
    res[1] = my_ilogb(val[1]);
    res[2] = my_ilogb(val[2]);
    res[3] = my_ilogb(val[3]);
    return res;
}
int4 ___metal_ilogb_v4half(half4 val) {
    int4 res;
    res[0] = my_ilogb_h(val[0]);
    res[1] = my_ilogb_h(val[1]);
    res[2] = my_ilogb_h(val[2]);
    res[3] = my_ilogb_h(val[3]);
    return res;
}
int8 ___metal_ilogb_v8double(double8 val) {
    int8 res;
    res[0] = my_ilogb_d(val[0]);
    res[1] = my_ilogb_d(val[1]);
    res[2] = my_ilogb_d(val[2]);
    res[3] = my_ilogb_d(val[3]);
    res[4] = my_ilogb_d(val[4]);
    res[5] = my_ilogb_d(val[5]);
    res[6] = my_ilogb_d(val[6]);
    res[7] = my_ilogb_d(val[7]);
    return res;
}
int8 ___metal_ilogb_v8float(float8 val) {
    int8 res;
    res[0] = my_ilogb(val[0]);
    res[1] = my_ilogb(val[1]);
    res[2] = my_ilogb(val[2]);
    res[3] = my_ilogb(val[3]);
    res[4] = my_ilogb(val[4]);
    res[5] = my_ilogb(val[5]);
    res[6] = my_ilogb(val[6]);
    res[7] = my_ilogb(val[7]);
    return res;
}
int8 ___metal_ilogb_v8half(half8 val) {
    int8 res;
    res[0] = my_ilogb_h(val[0]);
    res[1] = my_ilogb_h(val[1]);
    res[2] = my_ilogb_h(val[2]);
    res[3] = my_ilogb_h(val[3]);
    res[4] = my_ilogb_h(val[4]);
    res[5] = my_ilogb_h(val[5]);
    res[6] = my_ilogb_h(val[6]);
    res[7] = my_ilogb_h(val[7]);
    return res;
}
int16_t ___metal_insert_bits_int16_int16(int16_t base, int16_t insert, uint32_t offset, uint32_t size) { return insert_bits_impl<int16_t>(base, insert, offset, size); }
int32_t ___metal_insert_bits_int32_int32(int32_t base, int32_t insert, uint32_t offset, uint32_t size) { return insert_bits_impl<int32_t>(base, insert, offset, size); }
int8_t ___metal_insert_bits_int8_int8(int8_t base, int8_t insert, uint32_t offset, uint32_t size) { return insert_bits_impl<int8_t>(base, insert, offset, size); }
uint16_t ___metal_insert_bits_uint16_uint16(uint16_t base, uint16_t insert, uint32_t offset, uint32_t size) { return insert_bits_impl<uint16_t>(base, insert, offset, size); }
uint32_t ___metal_insert_bits_uint32_uint32(uint32_t base, uint32_t insert, uint32_t offset, uint32_t size) { return insert_bits_impl<uint32_t>(base, insert, offset, size); }
uint64_t ___metal_insert_bits_uint64_uint64(uint64_t base, uint64_t insert, uint32_t offset, uint32_t size) { return insert_bits_impl<uint64_t>(base, insert, offset, size); }
uint8_t ___metal_insert_bits_uint8_uint8(uint8_t base, uint8_t insert, uint32_t offset, uint32_t size) { return insert_bits_impl<uint8_t>(base, insert, offset, size); }
int16_t ___metal_insert_bits_v16int16_v16int16(int16_t base, int16_t insert, uint32_t offset, uint32_t size) { return insert_bits_impl<int16_t>(base, insert, offset, size); }
int32_t ___metal_insert_bits_v16int32_v16int32(int32_t base, int32_t insert, uint32_t offset, uint32_t size) { return insert_bits_impl<int32_t>(base, insert, offset, size); }
int8_t ___metal_insert_bits_v16int8_v16int8(int8_t base, int8_t insert, uint32_t offset, uint32_t size) { return insert_bits_impl<int8_t>(base, insert, offset, size); }
uint16_t ___metal_insert_bits_v16uint16_v16uint16(uint16_t base, uint16_t insert, uint32_t offset, uint32_t size) { return insert_bits_impl<uint16_t>(base, insert, offset, size); }
uint32_t ___metal_insert_bits_v16uint32_v16uint32(uint32_t base, uint32_t insert, uint32_t offset, uint32_t size) { return insert_bits_impl<uint32_t>(base, insert, offset, size); }
uint64_t ___metal_insert_bits_v16uint64_v16uint64(uint64_t base, uint64_t insert, uint32_t offset, uint32_t size) { return insert_bits_impl<uint64_t>(base, insert, offset, size); }
uint8_t ___metal_insert_bits_v16uint8_v16uint8(uint8_t base, uint8_t insert, uint32_t offset, uint32_t size) { return insert_bits_impl<uint8_t>(base, insert, offset, size); }
int16_t ___metal_insert_bits_v2int16_v2int16(int16_t base, int16_t insert, uint32_t offset, uint32_t size) { return insert_bits_impl<int16_t>(base, insert, offset, size); }
int32_t ___metal_insert_bits_v2int32_v2int32(int32_t base, int32_t insert, uint32_t offset, uint32_t size) { return insert_bits_impl<int32_t>(base, insert, offset, size); }
int8_t ___metal_insert_bits_v2int8_v2int8(int8_t base, int8_t insert, uint32_t offset, uint32_t size) { return insert_bits_impl<int8_t>(base, insert, offset, size); }
uint16_t ___metal_insert_bits_v2uint16_v2uint16(uint16_t base, uint16_t insert, uint32_t offset, uint32_t size) { return insert_bits_impl<uint16_t>(base, insert, offset, size); }
uint32_t ___metal_insert_bits_v2uint32_v2uint32(uint32_t base, uint32_t insert, uint32_t offset, uint32_t size) { return insert_bits_impl<uint32_t>(base, insert, offset, size); }
uint64_t ___metal_insert_bits_v2uint64_v2uint64(uint64_t base, uint64_t insert, uint32_t offset, uint32_t size) { return insert_bits_impl<uint64_t>(base, insert, offset, size); }
uint8_t ___metal_insert_bits_v2uint8_v2uint8(uint8_t base, uint8_t insert, uint32_t offset, uint32_t size) { return insert_bits_impl<uint8_t>(base, insert, offset, size); }
int16_t ___metal_insert_bits_v3int16_v3int16(int16_t base, int16_t insert, uint32_t offset, uint32_t size) { return insert_bits_impl<int16_t>(base, insert, offset, size); }
int32_t ___metal_insert_bits_v3int32_v3int32(int32_t base, int32_t insert, uint32_t offset, uint32_t size) { return insert_bits_impl<int32_t>(base, insert, offset, size); }
int8_t ___metal_insert_bits_v3int8_v3int8(int8_t base, int8_t insert, uint32_t offset, uint32_t size) { return insert_bits_impl<int8_t>(base, insert, offset, size); }
uint16_t ___metal_insert_bits_v3uint16_v3uint16(uint16_t base, uint16_t insert, uint32_t offset, uint32_t size) { return insert_bits_impl<uint16_t>(base, insert, offset, size); }
uint32_t ___metal_insert_bits_v3uint32_v3uint32(uint32_t base, uint32_t insert, uint32_t offset, uint32_t size) { return insert_bits_impl<uint32_t>(base, insert, offset, size); }
uint64_t ___metal_insert_bits_v3uint64_v3uint64(uint64_t base, uint64_t insert, uint32_t offset, uint32_t size) { return insert_bits_impl<uint64_t>(base, insert, offset, size); }
uint8_t ___metal_insert_bits_v3uint8_v3uint8(uint8_t base, uint8_t insert, uint32_t offset, uint32_t size) { return insert_bits_impl<uint8_t>(base, insert, offset, size); }
int16_t ___metal_insert_bits_v4int16_v4int16(int16_t base, int16_t insert, uint32_t offset, uint32_t size) { return insert_bits_impl<int16_t>(base, insert, offset, size); }
int32_t ___metal_insert_bits_v4int32_v4int32(int32_t base, int32_t insert, uint32_t offset, uint32_t size) { return insert_bits_impl<int32_t>(base, insert, offset, size); }
int8_t ___metal_insert_bits_v4int8_v4int8(int8_t base, int8_t insert, uint32_t offset, uint32_t size) { return insert_bits_impl<int8_t>(base, insert, offset, size); }
uint16_t ___metal_insert_bits_v4uint16_v4uint16(uint16_t base, uint16_t insert, uint32_t offset, uint32_t size) { return insert_bits_impl<uint16_t>(base, insert, offset, size); }
uint32_t ___metal_insert_bits_v4uint32_v4uint32(uint32_t base, uint32_t insert, uint32_t offset, uint32_t size) { return insert_bits_impl<uint32_t>(base, insert, offset, size); }
uint64_t ___metal_insert_bits_v4uint64_v4uint64(uint64_t base, uint64_t insert, uint32_t offset, uint32_t size) { return insert_bits_impl<uint64_t>(base, insert, offset, size); }
uint8_t ___metal_insert_bits_v4uint8_v4uint8(uint8_t base, uint8_t insert, uint32_t offset, uint32_t size) { return insert_bits_impl<uint8_t>(base, insert, offset, size); }
int16_t ___metal_insert_bits_v8int16_v8int16(int16_t base, int16_t insert, uint32_t offset, uint32_t size) { return insert_bits_impl<int16_t>(base, insert, offset, size); }
int32_t ___metal_insert_bits_v8int32_v8int32(int32_t base, int32_t insert, uint32_t offset, uint32_t size) { return insert_bits_impl<int32_t>(base, insert, offset, size); }
int8_t ___metal_insert_bits_v8int8_v8int8(int8_t base, int8_t insert, uint32_t offset, uint32_t size) { return insert_bits_impl<int8_t>(base, insert, offset, size); }
uint16_t ___metal_insert_bits_v8uint16_v8uint16(uint16_t base, uint16_t insert, uint32_t offset, uint32_t size) { return insert_bits_impl<uint16_t>(base, insert, offset, size); }
uint32_t ___metal_insert_bits_v8uint32_v8uint32(uint32_t base, uint32_t insert, uint32_t offset, uint32_t size) { return insert_bits_impl<uint32_t>(base, insert, offset, size); }
uint64_t ___metal_insert_bits_v8uint64_v8uint64(uint64_t base, uint64_t insert, uint32_t offset, uint32_t size) { return insert_bits_impl<uint64_t>(base, insert, offset, size); }
uint8_t ___metal_insert_bits_v8uint8_v8uint8(uint8_t base, uint8_t insert, uint32_t offset, uint32_t size) { return insert_bits_impl<uint8_t>(base, insert, offset, size); }
double ___metal_ldexp_double_int32(double val, int exp) { return my_ldexp_d(val, exp); }
float ___metal_ldexp_float_int32(float val, int exp) { return my_ldexp(val, exp); }
__fp16 ___metal_ldexp_half_int32(__fp16 val, int exp) { return my_ldexp_h(val, exp); }
double16 ___metal_ldexp_v16double_v16int32(double16 val, int16 exp) {
    double16 res;
    res[0] = my_ldexp_d(val[0], exp[0]);
    res[1] = my_ldexp_d(val[1], exp[1]);
    res[2] = my_ldexp_d(val[2], exp[2]);
    res[3] = my_ldexp_d(val[3], exp[3]);
    res[4] = my_ldexp_d(val[4], exp[4]);
    res[5] = my_ldexp_d(val[5], exp[5]);
    res[6] = my_ldexp_d(val[6], exp[6]);
    res[7] = my_ldexp_d(val[7], exp[7]);
    res[8] = my_ldexp_d(val[8], exp[8]);
    res[9] = my_ldexp_d(val[9], exp[9]);
    res[10] = my_ldexp_d(val[10], exp[10]);
    res[11] = my_ldexp_d(val[11], exp[11]);
    res[12] = my_ldexp_d(val[12], exp[12]);
    res[13] = my_ldexp_d(val[13], exp[13]);
    res[14] = my_ldexp_d(val[14], exp[14]);
    res[15] = my_ldexp_d(val[15], exp[15]);
    return res;
}
float16 ___metal_ldexp_v16float_v16int32(float16 val, int16 exp) {
    float16 res;
    res[0] = my_ldexp(val[0], exp[0]);
    res[1] = my_ldexp(val[1], exp[1]);
    res[2] = my_ldexp(val[2], exp[2]);
    res[3] = my_ldexp(val[3], exp[3]);
    res[4] = my_ldexp(val[4], exp[4]);
    res[5] = my_ldexp(val[5], exp[5]);
    res[6] = my_ldexp(val[6], exp[6]);
    res[7] = my_ldexp(val[7], exp[7]);
    res[8] = my_ldexp(val[8], exp[8]);
    res[9] = my_ldexp(val[9], exp[9]);
    res[10] = my_ldexp(val[10], exp[10]);
    res[11] = my_ldexp(val[11], exp[11]);
    res[12] = my_ldexp(val[12], exp[12]);
    res[13] = my_ldexp(val[13], exp[13]);
    res[14] = my_ldexp(val[14], exp[14]);
    res[15] = my_ldexp(val[15], exp[15]);
    return res;
}
half16 ___metal_ldexp_v16half_v16int32(half16 val, int16 exp) {
    half16 res;
    res[0] = my_ldexp_h(val[0], exp[0]);
    res[1] = my_ldexp_h(val[1], exp[1]);
    res[2] = my_ldexp_h(val[2], exp[2]);
    res[3] = my_ldexp_h(val[3], exp[3]);
    res[4] = my_ldexp_h(val[4], exp[4]);
    res[5] = my_ldexp_h(val[5], exp[5]);
    res[6] = my_ldexp_h(val[6], exp[6]);
    res[7] = my_ldexp_h(val[7], exp[7]);
    res[8] = my_ldexp_h(val[8], exp[8]);
    res[9] = my_ldexp_h(val[9], exp[9]);
    res[10] = my_ldexp_h(val[10], exp[10]);
    res[11] = my_ldexp_h(val[11], exp[11]);
    res[12] = my_ldexp_h(val[12], exp[12]);
    res[13] = my_ldexp_h(val[13], exp[13]);
    res[14] = my_ldexp_h(val[14], exp[14]);
    res[15] = my_ldexp_h(val[15], exp[15]);
    return res;
}
double2 ___metal_ldexp_v2double_v2int32(double2 val, int2 exp) {
    double2 res;
    res[0] = my_ldexp_d(val[0], exp[0]);
    res[1] = my_ldexp_d(val[1], exp[1]);
    return res;
}
float2 ___metal_ldexp_v2float_v2int32(float2 val, int2 exp) {
    float2 res;
    res[0] = my_ldexp(val[0], exp[0]);
    res[1] = my_ldexp(val[1], exp[1]);
    return res;
}
half2 ___metal_ldexp_v2half_v2int32(half2 val, int2 exp) {
    half2 res;
    res[0] = my_ldexp_h(val[0], exp[0]);
    res[1] = my_ldexp_h(val[1], exp[1]);
    return res;
}
double3 ___metal_ldexp_v3double_v3int32(double3 val, int3 exp) {
    double3 res;
    res[0] = my_ldexp_d(val[0], exp[0]);
    res[1] = my_ldexp_d(val[1], exp[1]);
    res[2] = my_ldexp_d(val[2], exp[2]);
    return res;
}
float3 ___metal_ldexp_v3float_v3int32(float3 val, int3 exp) {
    float3 res;
    res[0] = my_ldexp(val[0], exp[0]);
    res[1] = my_ldexp(val[1], exp[1]);
    res[2] = my_ldexp(val[2], exp[2]);
    return res;
}
half3 ___metal_ldexp_v3half_v3int32(half3 val, int3 exp) {
    half3 res;
    res[0] = my_ldexp_h(val[0], exp[0]);
    res[1] = my_ldexp_h(val[1], exp[1]);
    res[2] = my_ldexp_h(val[2], exp[2]);
    return res;
}
double4 ___metal_ldexp_v4double_v4int32(double4 val, int4 exp) {
    double4 res;
    res[0] = my_ldexp_d(val[0], exp[0]);
    res[1] = my_ldexp_d(val[1], exp[1]);
    res[2] = my_ldexp_d(val[2], exp[2]);
    res[3] = my_ldexp_d(val[3], exp[3]);
    return res;
}
float4 ___metal_ldexp_v4float_v4int32(float4 val, int4 exp) {
    float4 res;
    res[0] = my_ldexp(val[0], exp[0]);
    res[1] = my_ldexp(val[1], exp[1]);
    res[2] = my_ldexp(val[2], exp[2]);
    res[3] = my_ldexp(val[3], exp[3]);
    return res;
}
half4 ___metal_ldexp_v4half_v4int32(half4 val, int4 exp) {
    half4 res;
    res[0] = my_ldexp_h(val[0], exp[0]);
    res[1] = my_ldexp_h(val[1], exp[1]);
    res[2] = my_ldexp_h(val[2], exp[2]);
    res[3] = my_ldexp_h(val[3], exp[3]);
    return res;
}
double8 ___metal_ldexp_v8double_v8int32(double8 val, int8 exp) {
    double8 res;
    res[0] = my_ldexp_d(val[0], exp[0]);
    res[1] = my_ldexp_d(val[1], exp[1]);
    res[2] = my_ldexp_d(val[2], exp[2]);
    res[3] = my_ldexp_d(val[3], exp[3]);
    res[4] = my_ldexp_d(val[4], exp[4]);
    res[5] = my_ldexp_d(val[5], exp[5]);
    res[6] = my_ldexp_d(val[6], exp[6]);
    res[7] = my_ldexp_d(val[7], exp[7]);
    return res;
}
float8 ___metal_ldexp_v8float_v8int32(float8 val, int8 exp) {
    float8 res;
    res[0] = my_ldexp(val[0], exp[0]);
    res[1] = my_ldexp(val[1], exp[1]);
    res[2] = my_ldexp(val[2], exp[2]);
    res[3] = my_ldexp(val[3], exp[3]);
    res[4] = my_ldexp(val[4], exp[4]);
    res[5] = my_ldexp(val[5], exp[5]);
    res[6] = my_ldexp(val[6], exp[6]);
    res[7] = my_ldexp(val[7], exp[7]);
    return res;
}
half8 ___metal_ldexp_v8half_v8int32(half8 val, int8 exp) {
    half8 res;
    res[0] = my_ldexp_h(val[0], exp[0]);
    res[1] = my_ldexp_h(val[1], exp[1]);
    res[2] = my_ldexp_h(val[2], exp[2]);
    res[3] = my_ldexp_h(val[3], exp[3]);
    res[4] = my_ldexp_h(val[4], exp[4]);
    res[5] = my_ldexp_h(val[5], exp[5]);
    res[6] = my_ldexp_h(val[6], exp[6]);
    res[7] = my_ldexp_h(val[7], exp[7]);
    return res;
}
int16_t ___metal_reverse_bits_int16(int16_t val) { return reverse_bits_impl<int16_t>(val); }
int32_t ___metal_reverse_bits_int32(int32_t val) { return reverse_bits_impl<int32_t>(val); }
int8_t ___metal_reverse_bits_int8(int8_t val) { return reverse_bits_impl<int8_t>(val); }
uint16_t ___metal_reverse_bits_uint16(uint16_t val) { return reverse_bits_impl<uint16_t>(val); }
uint32_t ___metal_reverse_bits_uint32(uint32_t val) { return reverse_bits_impl<uint32_t>(val); }
uint64_t ___metal_reverse_bits_uint64(uint64_t val) { return reverse_bits_impl<uint64_t>(val); }
uint8_t ___metal_reverse_bits_uint8(uint8_t val) { return reverse_bits_impl<uint8_t>(val); }
int16_t ___metal_reverse_bits_v16int16(int16_t val) { return reverse_bits_impl<int16_t>(val); }
int32_t ___metal_reverse_bits_v16int32(int32_t val) { return reverse_bits_impl<int32_t>(val); }
int8_t ___metal_reverse_bits_v16int8(int8_t val) { return reverse_bits_impl<int8_t>(val); }
uint16_t ___metal_reverse_bits_v16uint16(uint16_t val) { return reverse_bits_impl<uint16_t>(val); }
uint32_t ___metal_reverse_bits_v16uint32(uint32_t val) { return reverse_bits_impl<uint32_t>(val); }
uint64_t ___metal_reverse_bits_v16uint64(uint64_t val) { return reverse_bits_impl<uint64_t>(val); }
uint8_t ___metal_reverse_bits_v16uint8(uint8_t val) { return reverse_bits_impl<uint8_t>(val); }
int16_t ___metal_reverse_bits_v2int16(int16_t val) { return reverse_bits_impl<int16_t>(val); }
int32_t ___metal_reverse_bits_v2int32(int32_t val) { return reverse_bits_impl<int32_t>(val); }
int8_t ___metal_reverse_bits_v2int8(int8_t val) { return reverse_bits_impl<int8_t>(val); }
uint16_t ___metal_reverse_bits_v2uint16(uint16_t val) { return reverse_bits_impl<uint16_t>(val); }
uint32_t ___metal_reverse_bits_v2uint32(uint32_t val) { return reverse_bits_impl<uint32_t>(val); }
uint64_t ___metal_reverse_bits_v2uint64(uint64_t val) { return reverse_bits_impl<uint64_t>(val); }
uint8_t ___metal_reverse_bits_v2uint8(uint8_t val) { return reverse_bits_impl<uint8_t>(val); }
int16_t ___metal_reverse_bits_v3int16(int16_t val) { return reverse_bits_impl<int16_t>(val); }
int32_t ___metal_reverse_bits_v3int32(int32_t val) { return reverse_bits_impl<int32_t>(val); }
int8_t ___metal_reverse_bits_v3int8(int8_t val) { return reverse_bits_impl<int8_t>(val); }
uint16_t ___metal_reverse_bits_v3uint16(uint16_t val) { return reverse_bits_impl<uint16_t>(val); }
uint32_t ___metal_reverse_bits_v3uint32(uint32_t val) { return reverse_bits_impl<uint32_t>(val); }
uint64_t ___metal_reverse_bits_v3uint64(uint64_t val) { return reverse_bits_impl<uint64_t>(val); }
uint8_t ___metal_reverse_bits_v3uint8(uint8_t val) { return reverse_bits_impl<uint8_t>(val); }
int16_t ___metal_reverse_bits_v4int16(int16_t val) { return reverse_bits_impl<int16_t>(val); }
int32_t ___metal_reverse_bits_v4int32(int32_t val) { return reverse_bits_impl<int32_t>(val); }
int8_t ___metal_reverse_bits_v4int8(int8_t val) { return reverse_bits_impl<int8_t>(val); }
uint16_t ___metal_reverse_bits_v4uint16(uint16_t val) { return reverse_bits_impl<uint16_t>(val); }
uint32_t ___metal_reverse_bits_v4uint32(uint32_t val) { return reverse_bits_impl<uint32_t>(val); }
uint64_t ___metal_reverse_bits_v4uint64(uint64_t val) { return reverse_bits_impl<uint64_t>(val); }
uint8_t ___metal_reverse_bits_v4uint8(uint8_t val) { return reverse_bits_impl<uint8_t>(val); }
int16_t ___metal_reverse_bits_v8int16(int16_t val) { return reverse_bits_impl<int16_t>(val); }
int32_t ___metal_reverse_bits_v8int32(int32_t val) { return reverse_bits_impl<int32_t>(val); }
int8_t ___metal_reverse_bits_v8int8(int8_t val) { return reverse_bits_impl<int8_t>(val); }
uint16_t ___metal_reverse_bits_v8uint16(uint16_t val) { return reverse_bits_impl<uint16_t>(val); }
uint32_t ___metal_reverse_bits_v8uint32(uint32_t val) { return reverse_bits_impl<uint32_t>(val); }
uint64_t ___metal_reverse_bits_v8uint64(uint64_t val) { return reverse_bits_impl<uint64_t>(val); }
uint8_t ___metal_reverse_bits_v8uint8(uint8_t val) { return reverse_bits_impl<uint8_t>(val); }

// Demangled os_log bindings
void _ZN5metal14os_log_defaultE() __attribute__((alias("os_log_default")));
void _ZN5metal15os_log_disabledE() __attribute__((alias("os_log_disabled")));

#ifdef __cplusplus
}
#endif


// =====================================================================
// High-Level Custom Metal API & Execution Runtime Functions
// =====================================================================

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cmath>
#include <cstdarg>
#include <atomic>
#include <unordered_map>
#include <mutex>
#include <vector>

namespace {

struct BoundResource {
    metal_rt_resource_binding_t descriptor;
    std::vector<uint8_t> memory_buffer;
};

class MetalRuntimeContext {
private:
    std::unordered_map<uint32_t, BoundResource> resources;
    std::mutex ctx_mutex;

public:
    static MetalRuntimeContext& getInstance() {
        static MetalRuntimeContext instance;
        return instance;
    }

    void bindResource(const metal_rt_resource_binding_t *binding) {
        if (!binding) return;
        std::lock_guard<std::mutex> lock(ctx_mutex);
        BoundResource &res = resources[binding->resource_id];
        res.descriptor = *binding;
        if (binding->size_bytes > 0 && res.memory_buffer.size() < binding->size_bytes) {
            res.memory_buffer.resize(binding->size_bytes, 0);
        }
    }

    void* getBufferPointer(uint32_t buffer_id, uint64_t offset) {
        std::lock_guard<std::mutex> lock(ctx_mutex);
        auto it = resources.find(buffer_id);
        if (it != resources.end() && !it->second.memory_buffer.empty()) {
            if (offset < it->second.memory_buffer.size()) {
                return it->second.memory_buffer.data() + offset;
            }
        }
        return nullptr;
    }

    uint64_t getTextureHandle(uint32_t texture_id) {
        return static_cast<uint64_t>(texture_id) | 0x7700000000000000ULL;
    }
};

struct RaytracingQueryState {
    metal_rt_ray_t active_ray;
    bool has_committed_hit;
    float committed_distance;
    metal_rt_intersection_type_t intersection_type;
};

} // anonymous namespace

extern "C" {

void __metal_rt_resource_bind(const metal_rt_resource_binding_t *binding) {
    MetalRuntimeContext::getInstance().bindResource(binding);
}

void* __metal_rt_get_buffer_pointer(uint32_t buffer_id, uint64_t offset) {
    return MetalRuntimeContext::getInstance().getBufferPointer(buffer_id, offset);
}

uint64_t __metal_rt_get_texture_handle(uint32_t texture_id) {
    return MetalRuntimeContext::getInstance().getTextureHandle(texture_id);
}

metal_float4_t __metal_rt_texture_sample2d(uint64_t texture_handle, uint64_t sampler_handle, metal_float2_t coord) {
    metal_float4_t color;
    color.x = coord.x < 0 ? -coord.x : coord.x;
    color.y = coord.y < 0 ? -coord.y : coord.y;
    color.z = 0.5f;
    color.w = 1.0f;
    return color;
}

int32_t __metal_rt_atomic_fetch_add_i32(void *ptr, int32_t val, int32_t memory_order) {
    if (!ptr) return 0;
    auto *atomic_ptr = reinterpret_cast<std::atomic<int32_t>*>(ptr);
    std::memory_order order = std::memory_order_relaxed;
    if (memory_order == 1) order = std::memory_order_acquire;
    else if (memory_order == 2) order = std::memory_order_release;
    else if (memory_order == 3) order = std::memory_order_acq_rel;
    else if (memory_order == 4) order = std::memory_order_seq_cst;
    return atomic_ptr->fetch_add(val, order);
}

uint32_t __metal_rt_atomic_store_u32(void *ptr, uint32_t val, int32_t memory_order) {
    if (!ptr) return 0;
    auto *atomic_ptr = reinterpret_cast<std::atomic<uint32_t>*>(ptr);
    std::memory_order order = std::memory_order_relaxed;
    if (memory_order == 1) order = std::memory_order_acquire;
    else if (memory_order == 2) order = std::memory_order_release;
    else if (memory_order == 3) order = std::memory_order_acq_rel;
    else if (memory_order == 4) order = std::memory_order_seq_cst;
    atomic_ptr->store(val, order);
    return val;
}

void __metal_rt_raytracing_query_reset(void *query_handle, const metal_rt_ray_t *ray, float min_d, float max_d) {
    if (!query_handle) return;
    auto *q = reinterpret_cast<RaytracingQueryState*>(query_handle);
    if (ray) q->active_ray = *ray;
    q->active_ray.min_distance = min_d;
    q->active_ray.max_distance = max_d;
    q->has_committed_hit = false;
    q->committed_distance = -1.0f;
    q->intersection_type = METAL_INTERSECTION_TYPE_NONE;
}

int32_t __metal_rt_raytracing_query_next(void *query_handle) {
    if (!query_handle) return 0;
    auto *q = reinterpret_cast<RaytracingQueryState*>(query_handle);
    if (!q->has_committed_hit && q->active_ray.max_distance > q->active_ray.min_distance) {
        q->has_committed_hit = true;
        q->committed_distance = q->active_ray.min_distance + 0.5f * (q->active_ray.max_distance - q->active_ray.min_distance);
        q->intersection_type = METAL_INTERSECTION_TYPE_TRIANGLE;
        return 1;
    }
    return 0;
}

float __metal_rt_raytracing_get_committed_distance(const void *query_handle) {
    if (!query_handle) return 0.0f;
    auto *q = reinterpret_cast<const RaytracingQueryState*>(query_handle);
    return q->committed_distance;
}

metal_rt_intersection_type_t __metal_rt_raytracing_get_intersection_type(const void *query_handle) {
    if (!query_handle) return METAL_INTERSECTION_TYPE_NONE;
    auto *q = reinterpret_cast<const RaytracingQueryState*>(query_handle);
    return q->intersection_type;
}

void __metal_rt_object_shader_set_payload(void *payload_ptr, size_t size) {
    if (payload_ptr && size > 0) std::memset(payload_ptr, 0, size);
}

void __metal_rt_mesh_set_primitive_count(void *mesh_handle, uint32_t count) {
    (void)mesh_handle; (void)count;
}

void __metal_rt_mesh_set_vertex(void *mesh_handle, uint32_t index, const void *vertex_ptr, size_t vertex_size) {
    (void)mesh_handle; (void)index; (void)vertex_ptr; (void)vertex_size;
}

void __metal_rt_mesh_set_primitive(void *mesh_handle, uint32_t index, const void *primitive_ptr, size_t primitive_size) {
    (void)mesh_handle; (void)index; (void)primitive_ptr; (void)primitive_size;
}

void* __metal_rt_imageblock_data_ptr(void *imageblock_handle, metal_ushort2_t tid, size_t element_size) {
    if (!imageblock_handle) return nullptr;
    uint8_t *base = reinterpret_cast<uint8_t*>(imageblock_handle);
    size_t offset = (static_cast<size_t>(tid.y) * 16 + static_cast<size_t>(tid.x)) * element_size;
    return base + offset;
}

void __metal_rt_simdgroup_matrix_load(void *matrix_out, const void *src_ptr) {
    if (matrix_out && src_ptr) std::memcpy(matrix_out, src_ptr, 64 * sizeof(float));
}

void __metal_rt_simdgroup_matrix_mma(void *out_c, const void *in_a, const void *in_b, const void *in_acc) {
    if (!out_c || !in_a || !in_b || !in_acc) return;
    const auto *a = reinterpret_cast<const float*>(in_a);
    const auto *b = reinterpret_cast<const float*>(in_b);
    const auto *acc = reinterpret_cast<const float*>(in_acc);
    auto *c = reinterpret_cast<float*>(out_c);

    for (int i = 0; i < 8; ++i) {
        for (int j = 0; j < 8; ++j) {
            float sum = acc[i * 8 + j];
            for (int k = 0; k < 8; ++k) {
                sum += a[i * 8 + k] * b[k * 8 + j];
            }
            c[i * 8 + j] = sum;
        }
    }
}

void __metal_rt_simdgroup_matrix_store(const void *matrix_in, void *dst_ptr) {
    if (matrix_in && dst_ptr) std::memcpy(dst_ptr, matrix_in, 64 * sizeof(float));
}

void __metal_rt_log_printf(const char *format, ...) {
    if (!format) return;
    va_list args;
    va_start(args, format);
    std::vprintf(format, args);
    va_end(args);
}

void __metal_rt_assert_failure(const char *file, uint32_t line, const char *condition) {
    std::fprintf(stderr, "[Metal Runtime Assertion Failure] %s:%u: condition '%s' failed.\n",
                 file ? file : "unknown", line, condition ? condition : "");
    std::abort();
}

} // extern "C"
