#ifndef _METAL_TYPE_TRAITS_H_
#define _METAL_TYPE_TRAITS_H_
#include <metal/metal_common>
namespace metal {

template <typename T, typename U> struct is_same { static constexpr bool value = false; };
template <typename T> struct is_same<T,T> { static constexpr bool value = true; };
template <typename T, typename U> constexpr bool is_same_v = is_same<T,U>::value;

template <typename T> struct remove_const { typedef T type; };
template <typename T> struct remove_const<const T> { typedef T type; };
template <typename T> using remove_const_t = typename remove_const<T>::type;

template <typename T> struct remove_volatile { typedef T type; };
template <typename T> struct remove_volatile<volatile T> { typedef T type; };
template <typename T> using remove_volatile_t = typename remove_volatile<T>::type;

template <typename T> struct remove_cv { typedef remove_volatile_t<remove_const_t<T>> type; };
template <typename T> using remove_cv_t = typename remove_cv<T>::type;

template <typename T> struct remove_reference { typedef T type; };
template <typename T> struct remove_reference<T&> { typedef T type; };
template <typename T> using remove_reference_t = typename remove_reference<T>::type;

template <typename T> struct decay { typedef remove_cv_t<remove_reference_t<T>> type; };
template <typename T> using decay_t = typename decay<T>::type;

template <bool B, typename T = void> struct enable_if {};
template <typename T> struct enable_if<true, T> { typedef T type; };
template <bool B, typename T = void> using enable_if_t = typename enable_if<B,T>::type;

template <typename T> struct is_integral { static constexpr bool value = false; };
template <> struct is_integral<bool> { static constexpr bool value = true; };
template <> struct is_integral<char> { static constexpr bool value = true; };
template <> struct is_integral<short> { static constexpr bool value = true; };
template <> struct is_integral<int> { static constexpr bool value = true; };
template <> struct is_integral<long> { static constexpr bool value = true; };
template <> struct is_integral<uchar> { static constexpr bool value = true; };
template <> struct is_integral<ushort> { static constexpr bool value = true; };
template <> struct is_integral<uint> { static constexpr bool value = true; };
template <> struct is_integral<ulong> { static constexpr bool value = true; };
template <typename T> constexpr bool is_integral_v = is_integral<T>::value;

template <typename T> struct is_floating_point { static constexpr bool value = false; };
template <> struct is_floating_point<float> { static constexpr bool value = true; };
template <> struct is_floating_point<half> { static constexpr bool value = true; };
template <> struct is_floating_point<double> { static constexpr bool value = true; };
template <typename T> constexpr bool is_floating_point_v = is_floating_point<T>::value;

template <typename T> struct is_signed { static constexpr bool value = is_floating_point_v<T>; };
template <> struct is_signed<short> { static constexpr bool value = true; };
template <> struct is_signed<int> { static constexpr bool value = true; };
template <> struct is_signed<long> { static constexpr bool value = true; };
template <typename T> constexpr bool is_signed_v = is_signed<T>::value;

template <typename T> struct is_unsigned { static constexpr bool value = is_integral_v<T> && !is_signed_v<T>; };
template <typename T> constexpr bool is_unsigned_v = is_unsigned<T>::value;

template <typename T> struct make_unsigned;
template <> struct make_unsigned<char> { typedef uchar type; };
template <> struct make_unsigned<short> { typedef ushort type; };
template <> struct make_unsigned<int> { typedef uint type; };
template <> struct make_unsigned<long> { typedef ulong type; };
template <typename T> using make_unsigned_t = typename make_unsigned<T>::type;

template <typename T> struct make_signed;
template <> struct make_signed<uchar> { typedef char type; };
template <> struct make_signed<ushort> { typedef short type; };
template <> struct make_signed<uint> { typedef int type; };
template <> struct make_signed<ulong> { typedef long type; };
template <typename T> using make_signed_t = typename make_signed<T>::type;

template <typename T> struct is_arithmetic { static constexpr bool value = is_integral_v<T> || is_floating_point_v<T>; };
template <typename T> constexpr bool is_arithmetic_v = is_arithmetic<T>::value;

template <typename T> struct is_scalar { static constexpr bool value = is_arithmetic_v<T>; };
template <typename T> constexpr bool is_scalar_v = is_scalar<T>::value;

template <typename T> struct is_vector { static constexpr bool value = false; };
template <> struct is_vector<float2> { static constexpr bool value = true; };
template <> struct is_vector<float3> { static constexpr bool value = true; };
template <> struct is_vector<float4> { static constexpr bool value = true; };
template <> struct is_vector<half2> { static constexpr bool value = true; };
template <> struct is_vector<half3> { static constexpr bool value = true; };
template <> struct is_vector<half4> { static constexpr bool value = true; };
template <> struct is_vector<int2> { static constexpr bool value = true; };
template <> struct is_vector<int3> { static constexpr bool value = true; };
template <> struct is_vector<int4> { static constexpr bool value = true; };
template <> struct is_vector<uint2> { static constexpr bool value = true; };
template <> struct is_vector<uint3> { static constexpr bool value = true; };
template <> struct is_vector<uint4> { static constexpr bool value = true; };
template <typename T> constexpr bool is_vector_v = is_vector<T>::value;

// Conditional
template <bool B, typename T, typename F> struct conditional { typedef T type; };
template <typename T, typename F> struct conditional<false, T, F> { typedef F type; };
template <bool B, typename T, typename F> using conditional_t = typename conditional<B,T,F>::type;

} // namespace metal
#endif
