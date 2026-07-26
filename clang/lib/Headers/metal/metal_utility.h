#ifndef _METAL_UTILITY_H_
#define _METAL_UTILITY_H_
#include <metal/metal_common>
namespace metal {

template <typename T>
METAL_ALWAYS_INLINE constexpr T&& forward(remove_reference_t<T>& arg) { return static_cast<T&&>(arg); }
template <typename T>
METAL_ALWAYS_INLINE constexpr T&& forward(remove_reference_t<T>&& arg) { return static_cast<T&&>(arg); }

template <typename T>
METAL_ALWAYS_INLINE constexpr remove_reference_t<T>&& move(T&& arg) { return static_cast<remove_reference_t<T>&&>(arg); }

template <typename T>
METAL_ALWAYS_INLINE constexpr void swap(T& a, T& b) { T tmp = a; a = b; b = tmp; }

template <typename T>
METAL_ALWAYS_INLINE constexpr const T& min(const T& a, const T& b) { return a < b ? a : b; }
template <typename T>
METAL_ALWAYS_INLINE constexpr const T& max(const T& a, const T& b) { return a > b ? a : b; }

template <typename T>
METAL_ALWAYS_INLINE constexpr T clamp(T x, T lo, T hi) { return x < lo ? lo : x > hi ? hi : x; }

template <typename T>
METAL_ALWAYS_INLINE constexpr T abs(T x) { return x < 0 ? -x : x; }

} // namespace metal
#endif
