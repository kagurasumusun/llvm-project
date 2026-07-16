// metal_math.h - Metal Math Function Declarations
// Clean-room implementation

#ifndef __METAL_MATH_H__
#define __METAL_MATH_H__

#include "metal_types.h"

namespace metal_math {

// ============================================================================
// Trigonometric
// ============================================================================
template<typename T> T sin(T x);
template<typename T> T cos(T x);
template<typename T> T tan(T x);
template<typename T> T asin(T x);
template<typename T> T acos(T x);
template<typename T> T atan(T x);
template<typename T> T atan2(T y, T x);
template<typename T> T sinh(T x);
template<typename T> T cosh(T x);
template<typename T> T tanh(T x);
template<typename T> T asinh(T x);
template<typename T> T acosh(T x);
template<typename T> T atanh(T x);
template<typename T> T sincos(T x, T* cosval);

// ============================================================================
// Exponential & Logarithmic
// ============================================================================
template<typename T> T exp(T x);
template<typename T> T exp2(T x);
template<typename T> T exp10(T x);
template<typename T> T log(T x);
template<typename T> T log2(T x);
template<typename T> T log10(T x);
template<typename T> T pow(T x, T y);
template<typename T> T powr(T x, T y);
template<typename T> T sqrt(T x);
template<typename T> T rsqrt(T x);
template<typename T> T cbrt(T x);
template<typename T> T hypot(T x, T y);

// ============================================================================
// Rounding
// ============================================================================
template<typename T> T ceil(T x);
template<typename T> T floor(T x);
template<typename T> T trunc(T x);
template<typename T> T round(T x);
template<typename T> T rint(T x);
template<typename T> T fract(T x);

// ============================================================================
// Comparison
// ============================================================================
template<typename T> T fmin(T x, T y);
template<typename T> T fmax(T x, T y);
template<typename T> T fmod(T x, T y);
template<typename T> T fma(T a, T b, T c);
template<typename T> T fabs(T x);
template<typename T> T fdim(T x, T y);
template<typename T> T copysign(T mag, T sign);

// ============================================================================
// Special
// ============================================================================
template<typename T> T modf(T x, T* iptr);
template<typename T> T frexp(T x, int* exp);
template<typename T> int ilogb(T x);
template<typename T> T ldexp(T x, int exp);
template<typename T> T nextafter(T x, T y);
template<typename T> T remainder(T x, T y);
template<typename T> T remquo(T x, T y, int* quo);

} // namespace metal_math

#endif // __METAL_MATH_H__
