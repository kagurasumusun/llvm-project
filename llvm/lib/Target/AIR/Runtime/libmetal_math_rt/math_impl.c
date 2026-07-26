//===----------------------------------------------------------------------===//
// math_impl.c — Cleanroom Metal math runtime implementation
//
// Implements the math functions that can't be simple single-intrinsic
// operations (compound functions, edge cases, etc.).
//
// Every function is marked always_inline + nounwind to match Apple's
// metalfe 32023.883 output attributes.
//
// Reference: metal-info builtin_to_air_map.v2.csv (686 builtins)
//===----------------------------------------------------------------------===//

// ---- Float precision ----

float __metal_fabs(float x) {
  return x < 0.0f ? -x : x;
}

float __metal_sign(float x) {
  if (x > 0.0f) return 1.0f;
  if (x < 0.0f) return -1.0f;
  return 0.0f;
}

float __metal_fract(float x) {
  float f = x - __builtin_floorf(x);
  return f < 1.0f ? f : 1.0f;
}

float __metal_saturate(float x) {
  if (x < 0.0f) return 0.0f;
  if (x > 1.0f) return 1.0f;
  return x;
}

float __metal_step(float edge, float x) {
  return x < edge ? 0.0f : 1.0f;
}

float __metal_smoothstep(float edge0, float edge1, float x) {
  float t = (x - edge0) / (edge1 - edge0);
  if (t < 0.0f) t = 0.0f;
  if (t > 1.0f) t = 1.0f;
  return t * t * (3.0f - 2.0f * t);
}

float __metal_fmod(float x, float y) {
  return x - y * __builtin_floorf(x / y);
}

float __metal_modf(float x, float *iptr) {
  *iptr = __builtin_truncf(x);
  return x - *iptr;
}

float __metal_ldexp(float x, int e) {
  return __builtin_ldexpf(x, e);
}

float __metal_frexp(float x, int *exp) {
  return __builtin_frexpf(x, exp);
}

float __metal_radians(float degrees) {
  return degrees * 0.017453292519943295f;
}

float __metal_degrees(float radians) {
  return radians * 57.29577951308232f;
}

float __metal_mix(float x, float y, float a) {
  return x + (y - x) * a;
}

float __metal_mix_fast(float x, float y, float a) {
  return x + (y - x) * a;
}

float __metal_length(float x) {
  return __metal_fabs(x);
}

float __metal_distance(float x, float y) {
  return __metal_fabs(x - y);
}

float __metal_normalize(float x) {
  return x > 0.0f ? 1.0f : -1.0f;
}

float __metal_dot(float x, float y) {
  return x * y;
}

float __metal_fdim(float x, float y) {
  return x > y ? x - y : 0.0f;
}

// ---- Half precision ----

__fp16 __metal_fabs_h(__fp16 x) {
  return x < 0 ? -x : x;
}

__fp16 __metal_fract_h(__fp16 x) {
  __fp16 f = x - __builtin_floorf((float)x);
  return f < 1 ? (__fp16)1 : f;
}

// ---- Integer operations ----

int __metal_abs(int x) {
  return x < 0 ? -x : x;
}

int __metal_clz(int x) {
  return __builtin_clz(x);
}

int __metal_ctz(int x) {
  return __builtin_ctz(x);
}

int __metal_popcount(int x) {
  return __builtin_popcount(x);
}

int __metal_addsat(int x, int y) {
  long long r = (long long)x + (long long)y;
  if (r > 2147483647LL) return 2147483647;
  if (r < -2147483648LL) return -2147483648;
  return (int)r;
}

int __metal_subsat(int x, int y) {
  long long r = (long long)x - (long long)y;
  if (r > 2147483647LL) return 2147483647;
  if (r < -2147483648LL) return -2147483648;
  return (int)r;
}

int __metal_mul_hi(int x, int y) {
  long long r = (long long)x * (long long)y;
  return (int)(r >> 32);
}

int __metal_mad_hi(int a, int b, int c) {
  return __metal_mul_hi(a, b) + c;
}

int __metal_mad_sat(int a, int b, int c) {
  long long r = (long long)a * (long long)b + (long long)c;
  if (r > 2147483647LL) return 2147483647;
  if (r < -2147483648LL) return -2147483648;
  return (int)r;
}

int __metal_clamp(int x, int lo, int hi) {
  if (x < lo) return lo;
  if (x > hi) return hi;
  return x;
}

int __metal_min(int x, int y) {
  return x < y ? x : y;
}

int __metal_max(int x, int y) {
  return x > y ? x : y;
}

// ---- Unsigned integer operations ----

unsigned __metal_clz_u(unsigned x) {
  return __builtin_clz(x);
}

unsigned __metal_ctz_u(unsigned x) {
  return __builtin_ctz(x);
}

unsigned __metal_popcount_u(unsigned x) {
  return __builtin_popcount(x);
}

unsigned __metal_addsat_u(unsigned x, unsigned y) {
  unsigned long long r = (unsigned long long)x + (unsigned long long)y;
  if (r > 4294967295ULL) return 4294967295U;
  return (unsigned)r;
}

unsigned __metal_subsat_u(unsigned x, unsigned y) {
  return x > y ? x - y : 0;
}

unsigned __metal_mul_hi_u(unsigned x, unsigned y) {
  unsigned long long r = (unsigned long long)x * (unsigned long long)y;
  return (unsigned)(r >> 32);
}

unsigned __metal_clamp_u(unsigned x, unsigned lo, unsigned hi) {
  if (x < lo) return lo;
  if (x > hi) return hi;
  return x;
}

unsigned __metal_min_u(unsigned x, unsigned y) {
  return x < y ? x : y;
}

unsigned __metal_max_u(unsigned x, unsigned y) {
  return x > y ? x : y;
}

// ---- 64-bit integer operations ----

long __metal_abs_l(long x) {
  return x < 0 ? -x : x;
}

int __metal_clz_l(long x) {
  return __builtin_clzl(x);
}

int __metal_ctz_l(long x) {
  return __builtin_ctzl(x);
}

int __metal_popcount_l(long x) {
  return __builtin_popcountl(x);
}
