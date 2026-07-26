#ifndef _METAL_FUNCTIONAL_H_
#define _METAL_FUNCTIONAL_H_
#include <metal/metal_common>
namespace metal {
template <typename T> struct plus  { T operator()(T a, T b) const { return a + b; } };
template <typename T> struct minus { T operator()(T a, T b) const { return a - b; } };
template <typename T> struct multiplies { T operator()(T a, T b) const { return a * b; } };
template <typename T> struct divides { T operator()(T a, T b) const { return a / b; } };
template <typename T> struct modulus { T operator()(T a, T b) const { return a % b; } };
template <typename T> struct negate { T operator()(T a) const { return -a; } };
template <typename T> struct equal_to { bool operator()(T a, T b) const { return a == b; } };
template <typename T> struct not_equal_to { bool operator()(T a, T b) const { return a != b; } };
template <typename T> struct greater { bool operator()(T a, T b) const { return a > b; } };
template <typename T> struct less { bool operator()(T a, T b) const { return a < b; } };
template <typename T> struct greater_equal { bool operator()(T a, T b) const { return a >= b; } };
template <typename T> struct less_equal { bool operator()(T a, T b) const { return a <= b; } };
template <typename T> struct bit_and { T operator()(T a, T b) const { return a & b; } };
template <typename T> struct bit_or  { T operator()(T a, T b) const { return a | b; } };
template <typename T> struct bit_xor { T operator()(T a, T b) const { return a ^ b; } };
template <typename T> struct logical_and { bool operator()(T a, T b) const { return a && b; } };
template <typename T> struct logical_or  { bool operator()(T a, T b) const { return a || b; } };
template <typename T> struct logical_not { bool operator()(T a) const { return !a; } };
template <typename T> struct identity { T operator()(T a) const { return a; } };
} // namespace metal
#endif
