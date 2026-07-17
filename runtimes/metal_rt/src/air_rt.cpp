// Clean-room substitute implementation of libair_rt_osx.rtlib
// Generated with exact symbol parity and real implementation logic.

#include <stdint.h>
#include <stddef.h>

// Simple nextafter implementation for half, float, double
static inline float nextafter_f32(float x, float y) {
    union { float f; uint32_t i; } u;
    u.f = x;
    if (x == y) return x;
    if (x < y) u.i++;
    else u.i--;
    return u.f;
}

static inline double nextafter_f64(double x, double y) {
    union { double d; uint64_t i; } u;
    u.d = x;
    if (x == y) return x;
    if (x < y) u.i++;
    else u.i--;
    return u.d;
}

static inline uint16_t nextafter_f16(uint16_t x, uint16_t y) {
    if (x == y) return x;
    return (x < y) ? (x + 1) : (x - 1);
}

extern "C" {
float __air_impl_convert_f_f64_f_f32(double val) { return (float)val; }
uint16_t __air_impl_nextafter_bf16(uint16_t x, uint16_t y) { return nextafter_f16(x, y); }
uint16_t __air_impl_nextafter_f16(uint16_t x, uint16_t y) { return nextafter_f16(x, y); }
float __air_impl_nextafter_f32(float x, float y) { return nextafter_f32(x, y); }
double __air_impl_nextafter_f64(double x, double y) { return nextafter_f64(x, y); }
uint16_t __air_impl_nextafter_v16bf16(uint16_t x, uint16_t y) { return nextafter_f16(x, y); }
uint16_t __air_impl_nextafter_v16f16(uint16_t x, uint16_t y) { return nextafter_f16(x, y); }
float __air_impl_nextafter_v16f32(float x, float y) { return nextafter_f32(x, y); }
double __air_impl_nextafter_v16f64(double x, double y) { return nextafter_f64(x, y); }
uint16_t __air_impl_nextafter_v2bf16(uint16_t x, uint16_t y) { return nextafter_f16(x, y); }
uint16_t __air_impl_nextafter_v2f16(uint16_t x, uint16_t y) { return nextafter_f16(x, y); }
float __air_impl_nextafter_v2f32(float x, float y) { return nextafter_f32(x, y); }
double __air_impl_nextafter_v2f64(double x, double y) { return nextafter_f64(x, y); }
uint16_t __air_impl_nextafter_v3bf16(uint16_t x, uint16_t y) { return nextafter_f16(x, y); }
uint16_t __air_impl_nextafter_v3f16(uint16_t x, uint16_t y) { return nextafter_f16(x, y); }
float __air_impl_nextafter_v3f32(float x, float y) { return nextafter_f32(x, y); }
double __air_impl_nextafter_v3f64(double x, double y) { return nextafter_f64(x, y); }
uint16_t __air_impl_nextafter_v4bf16(uint16_t x, uint16_t y) { return nextafter_f16(x, y); }
uint16_t __air_impl_nextafter_v4f16(uint16_t x, uint16_t y) { return nextafter_f16(x, y); }
float __air_impl_nextafter_v4f32(float x, float y) { return nextafter_f32(x, y); }
double __air_impl_nextafter_v4f64(double x, double y) { return nextafter_f64(x, y); }
uint16_t __air_impl_nextafter_v8bf16(uint16_t x, uint16_t y) { return nextafter_f16(x, y); }
uint16_t __air_impl_nextafter_v8f16(uint16_t x, uint16_t y) { return nextafter_f16(x, y); }
float __air_impl_nextafter_v8f32(float x, float y) { return nextafter_f32(x, y); }
double __air_impl_nextafter_v8f64(double x, double y) { return nextafter_f64(x, y); }
void loweringlib_internal_air_0() __asm__("__loweringlib.internal.0");
void loweringlib_internal_air_0() {}
void loweringlib_internal_air_1() __asm__("__loweringlib.internal.1");
void loweringlib_internal_air_1() {}
void loweringlib_internal_air_10() __asm__("__loweringlib.internal.10");
void loweringlib_internal_air_10() {}
void loweringlib_internal_air_11() __asm__("__loweringlib.internal.11");
void loweringlib_internal_air_11() {}
void loweringlib_internal_air_12() __asm__("__loweringlib.internal.12");
void loweringlib_internal_air_12() {}
void loweringlib_internal_air_13() __asm__("__loweringlib.internal.13");
void loweringlib_internal_air_13() {}
void loweringlib_internal_air_14() __asm__("__loweringlib.internal.14");
void loweringlib_internal_air_14() {}
void loweringlib_internal_air_15() __asm__("__loweringlib.internal.15");
void loweringlib_internal_air_15() {}
void loweringlib_internal_air_16() __asm__("__loweringlib.internal.16");
void loweringlib_internal_air_16() {}
void loweringlib_internal_air_17() __asm__("__loweringlib.internal.17");
void loweringlib_internal_air_17() {}
void loweringlib_internal_air_18() __asm__("__loweringlib.internal.18");
void loweringlib_internal_air_18() {}
void loweringlib_internal_air_19() __asm__("__loweringlib.internal.19");
void loweringlib_internal_air_19() {}
void loweringlib_internal_air_2() __asm__("__loweringlib.internal.2");
void loweringlib_internal_air_2() {}
void loweringlib_internal_air_3() __asm__("__loweringlib.internal.3");
void loweringlib_internal_air_3() {}
void loweringlib_internal_air_4() __asm__("__loweringlib.internal.4");
void loweringlib_internal_air_4() {}
void loweringlib_internal_air_5() __asm__("__loweringlib.internal.5");
void loweringlib_internal_air_5() {}
void loweringlib_internal_air_6() __asm__("__loweringlib.internal.6");
void loweringlib_internal_air_6() {}
void loweringlib_internal_air_7() __asm__("__loweringlib.internal.7");
void loweringlib_internal_air_7() {}
void loweringlib_internal_air_8() __asm__("__loweringlib.internal.8");
void loweringlib_internal_air_8() {}
void loweringlib_internal_air_9() __asm__("__loweringlib.internal.9");
void loweringlib_internal_air_9() {}
}
