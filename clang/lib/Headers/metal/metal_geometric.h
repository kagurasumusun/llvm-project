// metal_geometric — MSL geometric functions (cleanroom)
#ifndef _METAL_GEOMETRIC_H_
#define _METAL_GEOMETRIC_H_
#include <metal/metal_common>
#include <metal/metal_math>
namespace metal {

METAL_FUNC half dot(half2 x, half2 y) { return __air_dot(x, y, _AIR_PRECISE_MATH_); }
METAL_FUNC half dot(half3 x, half3 y) { return __air_dot(x, y, _AIR_PRECISE_MATH_); }
METAL_FUNC half dot(half4 x, half4 y) { return __air_dot(x, y, _AIR_PRECISE_MATH_); }
METAL_FUNC half dot(half8 x, half8 y) { return __air_dot(x, y, _AIR_PRECISE_MATH_); }
METAL_FUNC half dot(half16 x, half16 y) { return __air_dot(x, y, _AIR_PRECISE_MATH_); }
METAL_FUNC float dot(float2 x, float2 y) { return __air_dot(x, y, _AIR_PRECISE_MATH_); }
METAL_FUNC float dot(float3 x, float3 y) { return __air_dot(x, y, _AIR_PRECISE_MATH_); }
METAL_FUNC float dot(float4 x, float4 y) { return __air_dot(x, y, _AIR_PRECISE_MATH_); }
METAL_FUNC float dot(float8 x, float8 y) { return __air_dot(x, y, _AIR_PRECISE_MATH_); }
METAL_FUNC float dot(float16 x, float16 y) { return __air_dot(x, y, _AIR_PRECISE_MATH_); }
METAL_FUNC double dot(double2 x, double2 y) { return __air_dot(x, y, _AIR_PRECISE_MATH_); }
METAL_FUNC double dot(double3 x, double3 y) { return __air_dot(x, y, _AIR_PRECISE_MATH_); }
METAL_FUNC double dot(double4 x, double4 y) { return __air_dot(x, y, _AIR_PRECISE_MATH_); }
METAL_FUNC double dot(double8 x, double8 y) { return __air_dot(x, y, _AIR_PRECISE_MATH_); }
METAL_FUNC double dot(double16 x, double16 y) { return __air_dot(x, y, _AIR_PRECISE_MATH_); }

METAL_FUNC half3 cross(half3 x, half3 y) { return __air_cross(x, y, _AIR_PRECISE_MATH_); }
METAL_FUNC float3 cross(float3 x, float3 y) { return __air_cross(x, y, _AIR_PRECISE_MATH_); }
METAL_FUNC double3 cross(double3 x, double3 y) { return __air_cross(x, y, _AIR_PRECISE_MATH_); }

METAL_FUNC float length(float2 x) { return __air_length(x, _AIR_PRECISE_MATH_); }
METAL_FUNC float length(float3 x) { return __air_length(x, _AIR_PRECISE_MATH_); }
METAL_FUNC float length(float4 x) { return __air_length(x, _AIR_PRECISE_MATH_); }
METAL_FUNC float length(float8 x) { return __air_length(x, _AIR_PRECISE_MATH_); }
METAL_FUNC float length(float16 x) { return __air_length(x, _AIR_PRECISE_MATH_); }
METAL_FUNC float length(float2 x) { return __air_length(x, _AIR_PRECISE_MATH_); }
METAL_FUNC float length(float3 x) { return __air_length(x, _AIR_PRECISE_MATH_); }
METAL_FUNC float length(float4 x) { return __air_length(x, _AIR_PRECISE_MATH_); }
METAL_FUNC float length(float8 x) { return __air_length(x, _AIR_PRECISE_MATH_); }
METAL_FUNC float length(float16 x) { return __air_length(x, _AIR_PRECISE_MATH_); }
METAL_FUNC half length(half2 x) { return __air_length(x, _AIR_PRECISE_MATH_); }
METAL_FUNC half length(half3 x) { return __air_length(x, _AIR_PRECISE_MATH_); }
METAL_FUNC half length(half4 x) { return __air_length(x, _AIR_PRECISE_MATH_); }
METAL_FUNC half length(half8 x) { return __air_length(x, _AIR_PRECISE_MATH_); }
METAL_FUNC half length(half16 x) { return __air_length(x, _AIR_PRECISE_MATH_); }
METAL_FUNC float length(float2 x) { return __air_length(x, _AIR_PRECISE_MATH_); }
METAL_FUNC float length(float3 x) { return __air_length(x, _AIR_PRECISE_MATH_); }
METAL_FUNC float length(float4 x) { return __air_length(x, _AIR_PRECISE_MATH_); }
METAL_FUNC float length(float8 x) { return __air_length(x, _AIR_PRECISE_MATH_); }
METAL_FUNC float length(float16 x) { return __air_length(x, _AIR_PRECISE_MATH_); }
METAL_FUNC double length(double2 x) { return __air_length(x, _AIR_PRECISE_MATH_); }
METAL_FUNC double length(double3 x) { return __air_length(x, _AIR_PRECISE_MATH_); }
METAL_FUNC double length(double4 x) { return __air_length(x, _AIR_PRECISE_MATH_); }
METAL_FUNC double length(double8 x) { return __air_length(x, _AIR_PRECISE_MATH_); }
METAL_FUNC double length(double16 x) { return __air_length(x, _AIR_PRECISE_MATH_); }

METAL_FUNC half length_squared(half2 x) { return __air_length_squared(x, _AIR_PRECISE_MATH_); }
METAL_FUNC half length_squared(half3 x) { return __air_length_squared(x, _AIR_PRECISE_MATH_); }
METAL_FUNC half length_squared(half4 x) { return __air_length_squared(x, _AIR_PRECISE_MATH_); }
METAL_FUNC half length_squared(half8 x) { return __air_length_squared(x, _AIR_PRECISE_MATH_); }
METAL_FUNC half length_squared(half16 x) { return __air_length_squared(x, _AIR_PRECISE_MATH_); }
METAL_FUNC float length_squared(float2 x) { return __air_length_squared(x, _AIR_PRECISE_MATH_); }
METAL_FUNC float length_squared(float3 x) { return __air_length_squared(x, _AIR_PRECISE_MATH_); }
METAL_FUNC float length_squared(float4 x) { return __air_length_squared(x, _AIR_PRECISE_MATH_); }
METAL_FUNC float length_squared(float8 x) { return __air_length_squared(x, _AIR_PRECISE_MATH_); }
METAL_FUNC float length_squared(float16 x) { return __air_length_squared(x, _AIR_PRECISE_MATH_); }
METAL_FUNC double length_squared(double2 x) { return __air_length_squared(x, _AIR_PRECISE_MATH_); }
METAL_FUNC double length_squared(double3 x) { return __air_length_squared(x, _AIR_PRECISE_MATH_); }
METAL_FUNC double length_squared(double4 x) { return __air_length_squared(x, _AIR_PRECISE_MATH_); }
METAL_FUNC double length_squared(double8 x) { return __air_length_squared(x, _AIR_PRECISE_MATH_); }
METAL_FUNC double length_squared(double16 x) { return __air_length_squared(x, _AIR_PRECISE_MATH_); }

METAL_FUNC float distance(float2 x, float2 y) { return __air_distance(x, y, _AIR_PRECISE_MATH_); }
METAL_FUNC float distance(float3 x, float3 y) { return __air_distance(x, y, _AIR_PRECISE_MATH_); }
METAL_FUNC float distance(float4 x, float4 y) { return __air_distance(x, y, _AIR_PRECISE_MATH_); }
METAL_FUNC float distance(float8 x, float8 y) { return __air_distance(x, y, _AIR_PRECISE_MATH_); }
METAL_FUNC float distance(float16 x, float16 y) { return __air_distance(x, y, _AIR_PRECISE_MATH_); }
METAL_FUNC float distance(float2 x, float2 y) { return __air_distance(x, y, _AIR_PRECISE_MATH_); }
METAL_FUNC float distance(float3 x, float3 y) { return __air_distance(x, y, _AIR_PRECISE_MATH_); }
METAL_FUNC float distance(float4 x, float4 y) { return __air_distance(x, y, _AIR_PRECISE_MATH_); }
METAL_FUNC float distance(float8 x, float8 y) { return __air_distance(x, y, _AIR_PRECISE_MATH_); }
METAL_FUNC float distance(float16 x, float16 y) { return __air_distance(x, y, _AIR_PRECISE_MATH_); }
METAL_FUNC half distance(half2 x, half2 y) { return __air_distance(x, y, _AIR_PRECISE_MATH_); }
METAL_FUNC half distance(half3 x, half3 y) { return __air_distance(x, y, _AIR_PRECISE_MATH_); }
METAL_FUNC half distance(half4 x, half4 y) { return __air_distance(x, y, _AIR_PRECISE_MATH_); }
METAL_FUNC half distance(half8 x, half8 y) { return __air_distance(x, y, _AIR_PRECISE_MATH_); }
METAL_FUNC half distance(half16 x, half16 y) { return __air_distance(x, y, _AIR_PRECISE_MATH_); }
METAL_FUNC float distance(float2 x, float2 y) { return __air_distance(x, y, _AIR_PRECISE_MATH_); }
METAL_FUNC float distance(float3 x, float3 y) { return __air_distance(x, y, _AIR_PRECISE_MATH_); }
METAL_FUNC float distance(float4 x, float4 y) { return __air_distance(x, y, _AIR_PRECISE_MATH_); }
METAL_FUNC float distance(float8 x, float8 y) { return __air_distance(x, y, _AIR_PRECISE_MATH_); }
METAL_FUNC float distance(float16 x, float16 y) { return __air_distance(x, y, _AIR_PRECISE_MATH_); }
METAL_FUNC double distance(double2 x, double2 y) { return __air_distance(x, y, _AIR_PRECISE_MATH_); }
METAL_FUNC double distance(double3 x, double3 y) { return __air_distance(x, y, _AIR_PRECISE_MATH_); }
METAL_FUNC double distance(double4 x, double4 y) { return __air_distance(x, y, _AIR_PRECISE_MATH_); }
METAL_FUNC double distance(double8 x, double8 y) { return __air_distance(x, y, _AIR_PRECISE_MATH_); }
METAL_FUNC double distance(double16 x, double16 y) { return __air_distance(x, y, _AIR_PRECISE_MATH_); }

METAL_FUNC half distance_squared(half2 x, half2 y) { return __air_distance_squared(x, y, _AIR_PRECISE_MATH_); }
METAL_FUNC half distance_squared(half3 x, half3 y) { return __air_distance_squared(x, y, _AIR_PRECISE_MATH_); }
METAL_FUNC half distance_squared(half4 x, half4 y) { return __air_distance_squared(x, y, _AIR_PRECISE_MATH_); }
METAL_FUNC half distance_squared(half8 x, half8 y) { return __air_distance_squared(x, y, _AIR_PRECISE_MATH_); }
METAL_FUNC half distance_squared(half16 x, half16 y) { return __air_distance_squared(x, y, _AIR_PRECISE_MATH_); }
METAL_FUNC float distance_squared(float2 x, float2 y) { return __air_distance_squared(x, y, _AIR_PRECISE_MATH_); }
METAL_FUNC float distance_squared(float3 x, float3 y) { return __air_distance_squared(x, y, _AIR_PRECISE_MATH_); }
METAL_FUNC float distance_squared(float4 x, float4 y) { return __air_distance_squared(x, y, _AIR_PRECISE_MATH_); }
METAL_FUNC float distance_squared(float8 x, float8 y) { return __air_distance_squared(x, y, _AIR_PRECISE_MATH_); }
METAL_FUNC float distance_squared(float16 x, float16 y) { return __air_distance_squared(x, y, _AIR_PRECISE_MATH_); }
METAL_FUNC double distance_squared(double2 x, double2 y) { return __air_distance_squared(x, y, _AIR_PRECISE_MATH_); }
METAL_FUNC double distance_squared(double3 x, double3 y) { return __air_distance_squared(x, y, _AIR_PRECISE_MATH_); }
METAL_FUNC double distance_squared(double4 x, double4 y) { return __air_distance_squared(x, y, _AIR_PRECISE_MATH_); }
METAL_FUNC double distance_squared(double8 x, double8 y) { return __air_distance_squared(x, y, _AIR_PRECISE_MATH_); }
METAL_FUNC double distance_squared(double16 x, double16 y) { return __air_distance_squared(x, y, _AIR_PRECISE_MATH_); }

METAL_FUNC float2 normalize(float2 x) { return __air_normalize(x, _AIR_PRECISE_MATH_); }
METAL_FUNC float3 normalize(float3 x) { return __air_normalize(x, _AIR_PRECISE_MATH_); }
METAL_FUNC float4 normalize(float4 x) { return __air_normalize(x, _AIR_PRECISE_MATH_); }
METAL_FUNC float8 normalize(float8 x) { return __air_normalize(x, _AIR_PRECISE_MATH_); }
METAL_FUNC float16 normalize(float16 x) { return __air_normalize(x, _AIR_PRECISE_MATH_); }
METAL_FUNC float2 normalize(float2 x) { return __air_normalize(x, _AIR_PRECISE_MATH_); }
METAL_FUNC float3 normalize(float3 x) { return __air_normalize(x, _AIR_PRECISE_MATH_); }
METAL_FUNC float4 normalize(float4 x) { return __air_normalize(x, _AIR_PRECISE_MATH_); }
METAL_FUNC float8 normalize(float8 x) { return __air_normalize(x, _AIR_PRECISE_MATH_); }
METAL_FUNC float16 normalize(float16 x) { return __air_normalize(x, _AIR_PRECISE_MATH_); }
METAL_FUNC half2 normalize(half2 x) { return __air_normalize(x, _AIR_PRECISE_MATH_); }
METAL_FUNC half3 normalize(half3 x) { return __air_normalize(x, _AIR_PRECISE_MATH_); }
METAL_FUNC half4 normalize(half4 x) { return __air_normalize(x, _AIR_PRECISE_MATH_); }
METAL_FUNC half8 normalize(half8 x) { return __air_normalize(x, _AIR_PRECISE_MATH_); }
METAL_FUNC half16 normalize(half16 x) { return __air_normalize(x, _AIR_PRECISE_MATH_); }
METAL_FUNC float2 normalize(float2 x) { return __air_normalize(x, _AIR_PRECISE_MATH_); }
METAL_FUNC float3 normalize(float3 x) { return __air_normalize(x, _AIR_PRECISE_MATH_); }
METAL_FUNC float4 normalize(float4 x) { return __air_normalize(x, _AIR_PRECISE_MATH_); }
METAL_FUNC float8 normalize(float8 x) { return __air_normalize(x, _AIR_PRECISE_MATH_); }
METAL_FUNC float16 normalize(float16 x) { return __air_normalize(x, _AIR_PRECISE_MATH_); }
METAL_FUNC double2 normalize(double2 x) { return __air_normalize(x, _AIR_PRECISE_MATH_); }
METAL_FUNC double3 normalize(double3 x) { return __air_normalize(x, _AIR_PRECISE_MATH_); }
METAL_FUNC double4 normalize(double4 x) { return __air_normalize(x, _AIR_PRECISE_MATH_); }
METAL_FUNC double8 normalize(double8 x) { return __air_normalize(x, _AIR_PRECISE_MATH_); }
METAL_FUNC double16 normalize(double16 x) { return __air_normalize(x, _AIR_PRECISE_MATH_); }

METAL_FUNC half2 faceforward(half2 n, half2 i, half2 nref) { return __air_faceforward(n, i, nref, _AIR_PRECISE_MATH_); }
METAL_FUNC half3 faceforward(half3 n, half3 i, half3 nref) { return __air_faceforward(n, i, nref, _AIR_PRECISE_MATH_); }
METAL_FUNC half4 faceforward(half4 n, half4 i, half4 nref) { return __air_faceforward(n, i, nref, _AIR_PRECISE_MATH_); }
METAL_FUNC half8 faceforward(half8 n, half8 i, half8 nref) { return __air_faceforward(n, i, nref, _AIR_PRECISE_MATH_); }
METAL_FUNC half16 faceforward(half16 n, half16 i, half16 nref) { return __air_faceforward(n, i, nref, _AIR_PRECISE_MATH_); }
METAL_FUNC float2 faceforward(float2 n, float2 i, float2 nref) { return __air_faceforward(n, i, nref, _AIR_PRECISE_MATH_); }
METAL_FUNC float3 faceforward(float3 n, float3 i, float3 nref) { return __air_faceforward(n, i, nref, _AIR_PRECISE_MATH_); }
METAL_FUNC float4 faceforward(float4 n, float4 i, float4 nref) { return __air_faceforward(n, i, nref, _AIR_PRECISE_MATH_); }
METAL_FUNC float8 faceforward(float8 n, float8 i, float8 nref) { return __air_faceforward(n, i, nref, _AIR_PRECISE_MATH_); }
METAL_FUNC float16 faceforward(float16 n, float16 i, float16 nref) { return __air_faceforward(n, i, nref, _AIR_PRECISE_MATH_); }
METAL_FUNC double2 faceforward(double2 n, double2 i, double2 nref) { return __air_faceforward(n, i, nref, _AIR_PRECISE_MATH_); }
METAL_FUNC double3 faceforward(double3 n, double3 i, double3 nref) { return __air_faceforward(n, i, nref, _AIR_PRECISE_MATH_); }
METAL_FUNC double4 faceforward(double4 n, double4 i, double4 nref) { return __air_faceforward(n, i, nref, _AIR_PRECISE_MATH_); }
METAL_FUNC double8 faceforward(double8 n, double8 i, double8 nref) { return __air_faceforward(n, i, nref, _AIR_PRECISE_MATH_); }
METAL_FUNC double16 faceforward(double16 n, double16 i, double16 nref) { return __air_faceforward(n, i, nref, _AIR_PRECISE_MATH_); }

METAL_FUNC half2 reflect(half2 i, half2 n) { return __air_reflect(i, n, _AIR_PRECISE_MATH_); }
METAL_FUNC half3 reflect(half3 i, half3 n) { return __air_reflect(i, n, _AIR_PRECISE_MATH_); }
METAL_FUNC half4 reflect(half4 i, half4 n) { return __air_reflect(i, n, _AIR_PRECISE_MATH_); }
METAL_FUNC half8 reflect(half8 i, half8 n) { return __air_reflect(i, n, _AIR_PRECISE_MATH_); }
METAL_FUNC half16 reflect(half16 i, half16 n) { return __air_reflect(i, n, _AIR_PRECISE_MATH_); }
METAL_FUNC float2 reflect(float2 i, float2 n) { return __air_reflect(i, n, _AIR_PRECISE_MATH_); }
METAL_FUNC float3 reflect(float3 i, float3 n) { return __air_reflect(i, n, _AIR_PRECISE_MATH_); }
METAL_FUNC float4 reflect(float4 i, float4 n) { return __air_reflect(i, n, _AIR_PRECISE_MATH_); }
METAL_FUNC float8 reflect(float8 i, float8 n) { return __air_reflect(i, n, _AIR_PRECISE_MATH_); }
METAL_FUNC float16 reflect(float16 i, float16 n) { return __air_reflect(i, n, _AIR_PRECISE_MATH_); }
METAL_FUNC double2 reflect(double2 i, double2 n) { return __air_reflect(i, n, _AIR_PRECISE_MATH_); }
METAL_FUNC double3 reflect(double3 i, double3 n) { return __air_reflect(i, n, _AIR_PRECISE_MATH_); }
METAL_FUNC double4 reflect(double4 i, double4 n) { return __air_reflect(i, n, _AIR_PRECISE_MATH_); }
METAL_FUNC double8 reflect(double8 i, double8 n) { return __air_reflect(i, n, _AIR_PRECISE_MATH_); }
METAL_FUNC double16 reflect(double16 i, double16 n) { return __air_reflect(i, n, _AIR_PRECISE_MATH_); }

METAL_FUNC half2 refract(half2 i, half2 n, half eta) { return __air_refract(i, n, eta, _AIR_PRECISE_MATH_); }
METAL_FUNC half3 refract(half3 i, half3 n, half eta) { return __air_refract(i, n, eta, _AIR_PRECISE_MATH_); }
METAL_FUNC half4 refract(half4 i, half4 n, half eta) { return __air_refract(i, n, eta, _AIR_PRECISE_MATH_); }
METAL_FUNC half8 refract(half8 i, half8 n, half eta) { return __air_refract(i, n, eta, _AIR_PRECISE_MATH_); }
METAL_FUNC half16 refract(half16 i, half16 n, half eta) { return __air_refract(i, n, eta, _AIR_PRECISE_MATH_); }
METAL_FUNC float2 refract(float2 i, float2 n, float eta) { return __air_refract(i, n, eta, _AIR_PRECISE_MATH_); }
METAL_FUNC float3 refract(float3 i, float3 n, float eta) { return __air_refract(i, n, eta, _AIR_PRECISE_MATH_); }
METAL_FUNC float4 refract(float4 i, float4 n, float eta) { return __air_refract(i, n, eta, _AIR_PRECISE_MATH_); }
METAL_FUNC float8 refract(float8 i, float8 n, float eta) { return __air_refract(i, n, eta, _AIR_PRECISE_MATH_); }
METAL_FUNC float16 refract(float16 i, float16 n, float eta) { return __air_refract(i, n, eta, _AIR_PRECISE_MATH_); }
METAL_FUNC double2 refract(double2 i, double2 n, double eta) { return __air_refract(i, n, eta, _AIR_PRECISE_MATH_); }
METAL_FUNC double3 refract(double3 i, double3 n, double eta) { return __air_refract(i, n, eta, _AIR_PRECISE_MATH_); }
METAL_FUNC double4 refract(double4 i, double4 n, double eta) { return __air_refract(i, n, eta, _AIR_PRECISE_MATH_); }
METAL_FUNC double8 refract(double8 i, double8 n, double eta) { return __air_refract(i, n, eta, _AIR_PRECISE_MATH_); }
METAL_FUNC double16 refract(double16 i, double16 n, double eta) { return __air_refract(i, n, eta, _AIR_PRECISE_MATH_); }

} // namespace metal
#endif