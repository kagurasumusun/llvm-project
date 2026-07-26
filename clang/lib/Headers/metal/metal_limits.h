//===----------------------------------------------------------------------===//
// metal_limits — MSL §9 numeric limits
//===----------------------------------------------------------------------===//
#ifndef _METAL_LIMITS_H_
#define _METAL_LIMITS_H_
#include <metal/metal_common>

namespace metal {
namespace limits {

METAL_ALWAYS_INLINE constexpr float  max_float()  { return 3.402823466e+38f; }
METAL_ALWAYS_INLINE constexpr float  min_float()  { return 1.175494351e-38f; }
METAL_ALWAYS_INLINE constexpr float  lowest_float() { return -3.402823466e+38f; }
METAL_ALWAYS_INLINE constexpr double max_double() { return 1.7976931348623157e+308; }
METAL_ALWAYS_INLINE constexpr double min_double() { return 2.2250738585072014e-308; }
METAL_ALWAYS_INLINE constexpr half    max_half()   { return 65504.0h; }
METAL_ALWAYS_INLINE constexpr half    min_half()   { return 6.103515625e-05h; }
METAL_ALWAYS_INLINE constexpr int     max_int()    { return 2147483647; }
METAL_ALWAYS_INLINE constexpr int     min_int()    { return -2147483648; }
METAL_ALWAYS_INLINE constexpr uint    max_uint()   { return 4294967295u; }
METAL_ALWAYS_INLINE constexpr short   max_short()  { return 32767; }
METAL_ALWAYS_INLINE constexpr short   min_short()  { return -32768; }
METAL_ALWAYS_INLINE constexpr ushort  max_ushort() { return 65535; }
METAL_ALWAYS_INLINE constexpr char    max_char()   { return 127; }
METAL_ALWAYS_INLINE constexpr char    min_char()   { return -128; }
METAL_ALWAYS_INLINE constexpr uchar   max_uchar()  { return 255; }
METAL_ALWAYS_INLINE constexpr long    max_long()   { return 9223372036854775807L; }
METAL_ALWAYS_INLINE constexpr long    min_long()   { return -9223372036854775807L - 1; }
METAL_ALWAYS_INLINE constexpr ulong   max_ulong()  { return 18446744073709551615UL; }

// Floating-point constants
METAL_ALWAYS_INLINE constexpr float  epsilon_float()  { return 1.192092896e-07f; }
METAL_ALWAYS_INLINE constexpr double epsilon_double() { return 2.2204460492503131e-16; }
METAL_ALWAYS_INLINE constexpr half   epsilon_half()   { return 9.765625e-04h; }
METAL_ALWAYS_INLINE constexpr float  inf_float()      { return __builtin_huge_valf(); }
METAL_ALWAYS_INLINE constexpr double inf_double()     { return __builtin_huge_val(); }
METAL_ALWAYS_INLINE constexpr float  nan_float()      { return __builtin_nanf(""); }
METAL_ALWAYS_INLINE constexpr double nan_double()     { return __builtin_nan(""); }
METAL_ALWAYS_INLINE constexpr float  m_e_float()      { return 2.718281828f; }
METAL_ALWAYS_INLINE constexpr float  m_log2e_float()  { return 1.442695041f; }
METAL_ALWAYS_INLINE constexpr float  m_log10e_float() { return 0.434294482f; }
METAL_ALWAYS_INLINE constexpr float  m_ln2_float()    { return 0.693147181f; }
METAL_ALWAYS_INLINE constexpr float  m_ln10_float()   { return 2.302585093f; }
METAL_ALWAYS_INLINE constexpr float  m_pi_float()     { return 3.141592654f; }
METAL_ALWAYS_INLINE constexpr float  m_pi_2_float()   { return 1.570796327f; }
METAL_ALWAYS_INLINE constexpr float  m_pi_4_float()   { return 0.785398163f; }
METAL_ALWAYS_INLINE constexpr float  m_1_pi_float()   { return 0.318309886f; }
METAL_ALWAYS_INLINE constexpr float  m_2_pi_float()   { return 0.636619772f; }
METAL_ALWAYS_INLINE constexpr float  m_2_sqrtpi_float() { return 1.128379167f; }
METAL_ALWAYS_INLINE constexpr float  m_sqrt2_float()  { return 1.414213562f; }
METAL_ALWAYS_INLINE constexpr float  m_sqrt1_2_float(){ return 0.707106781f; }

} // namespace limits
} // namespace metal
#endif