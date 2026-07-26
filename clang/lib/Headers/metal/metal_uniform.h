// metal_uniform — MSL uniform types (cleanroom)
#ifndef _METAL_UNIFORM_H_
#define _METAL_UNIFORM_H_
#include <metal/metal_common>
namespace metal {

template <typename T>
struct uniform {
  T value;
  METAL_FUNC operator T() const { return value; }
  METAL_FUNC T operator*() const { return value; }
  METAL_FUNC T get() const { return value; }
};

template <> struct uniform<float> { float value; METAL_ALWAYS_INLINE operator float() const { return value; } METAL_ALWAYS_INLINE float operator*() const { return value; } METAL_ALWAYS_INLINE float get() const { return value; } };
template <> struct uniform<float2> { float2 value; METAL_ALWAYS_INLINE operator float2() const { return value; } METAL_ALWAYS_INLINE float2 operator*() const { return value; } METAL_ALWAYS_INLINE float2 get() const { return value; } };
template <> struct uniform<float3> { float3 value; METAL_ALWAYS_INLINE operator float3() const { return value; } METAL_ALWAYS_INLINE float3 operator*() const { return value; } METAL_ALWAYS_INLINE float3 get() const { return value; } };
template <> struct uniform<float4> { float4 value; METAL_ALWAYS_INLINE operator float4() const { return value; } METAL_ALWAYS_INLINE float4 operator*() const { return value; } METAL_ALWAYS_INLINE float4 get() const { return value; } };
template <> struct uniform<float8> { float8 value; METAL_ALWAYS_INLINE operator float8() const { return value; } METAL_ALWAYS_INLINE float8 operator*() const { return value; } METAL_ALWAYS_INLINE float8 get() const { return value; } };
template <> struct uniform<float16> { float16 value; METAL_ALWAYS_INLINE operator float16() const { return value; } METAL_ALWAYS_INLINE float16 operator*() const { return value; } METAL_ALWAYS_INLINE float16 get() const { return value; } };
template <> struct uniform<half> { half value; METAL_ALWAYS_INLINE operator half() const { return value; } METAL_ALWAYS_INLINE half operator*() const { return value; } METAL_ALWAYS_INLINE half get() const { return value; } };
template <> struct uniform<half2> { half2 value; METAL_ALWAYS_INLINE operator half2() const { return value; } METAL_ALWAYS_INLINE half2 operator*() const { return value; } METAL_ALWAYS_INLINE half2 get() const { return value; } };
template <> struct uniform<half3> { half3 value; METAL_ALWAYS_INLINE operator half3() const { return value; } METAL_ALWAYS_INLINE half3 operator*() const { return value; } METAL_ALWAYS_INLINE half3 get() const { return value; } };
template <> struct uniform<half4> { half4 value; METAL_ALWAYS_INLINE operator half4() const { return value; } METAL_ALWAYS_INLINE half4 operator*() const { return value; } METAL_ALWAYS_INLINE half4 get() const { return value; } };
template <> struct uniform<half8> { half8 value; METAL_ALWAYS_INLINE operator half8() const { return value; } METAL_ALWAYS_INLINE half8 operator*() const { return value; } METAL_ALWAYS_INLINE half8 get() const { return value; } };
template <> struct uniform<half16> { half16 value; METAL_ALWAYS_INLINE operator half16() const { return value; } METAL_ALWAYS_INLINE half16 operator*() const { return value; } METAL_ALWAYS_INLINE half16 get() const { return value; } };
template <> struct uniform<int> { int value; METAL_ALWAYS_INLINE operator int() const { return value; } METAL_ALWAYS_INLINE int operator*() const { return value; } METAL_ALWAYS_INLINE int get() const { return value; } };
template <> struct uniform<int2> { int2 value; METAL_ALWAYS_INLINE operator int2() const { return value; } METAL_ALWAYS_INLINE int2 operator*() const { return value; } METAL_ALWAYS_INLINE int2 get() const { return value; } };
template <> struct uniform<int3> { int3 value; METAL_ALWAYS_INLINE operator int3() const { return value; } METAL_ALWAYS_INLINE int3 operator*() const { return value; } METAL_ALWAYS_INLINE int3 get() const { return value; } };
template <> struct uniform<int4> { int4 value; METAL_ALWAYS_INLINE operator int4() const { return value; } METAL_ALWAYS_INLINE int4 operator*() const { return value; } METAL_ALWAYS_INLINE int4 get() const { return value; } };
template <> struct uniform<int8> { int8 value; METAL_ALWAYS_INLINE operator int8() const { return value; } METAL_ALWAYS_INLINE int8 operator*() const { return value; } METAL_ALWAYS_INLINE int8 get() const { return value; } };
template <> struct uniform<int16> { int16 value; METAL_ALWAYS_INLINE operator int16() const { return value; } METAL_ALWAYS_INLINE int16 operator*() const { return value; } METAL_ALWAYS_INLINE int16 get() const { return value; } };
template <> struct uniform<uint> { uint value; METAL_ALWAYS_INLINE operator uint() const { return value; } METAL_ALWAYS_INLINE uint operator*() const { return value; } METAL_ALWAYS_INLINE uint get() const { return value; } };
template <> struct uniform<uint2> { uint2 value; METAL_ALWAYS_INLINE operator uint2() const { return value; } METAL_ALWAYS_INLINE uint2 operator*() const { return value; } METAL_ALWAYS_INLINE uint2 get() const { return value; } };
template <> struct uniform<uint3> { uint3 value; METAL_ALWAYS_INLINE operator uint3() const { return value; } METAL_ALWAYS_INLINE uint3 operator*() const { return value; } METAL_ALWAYS_INLINE uint3 get() const { return value; } };
template <> struct uniform<uint4> { uint4 value; METAL_ALWAYS_INLINE operator uint4() const { return value; } METAL_ALWAYS_INLINE uint4 operator*() const { return value; } METAL_ALWAYS_INLINE uint4 get() const { return value; } };
template <> struct uniform<uint8> { uint8 value; METAL_ALWAYS_INLINE operator uint8() const { return value; } METAL_ALWAYS_INLINE uint8 operator*() const { return value; } METAL_ALWAYS_INLINE uint8 get() const { return value; } };
template <> struct uniform<uint16> { uint16 value; METAL_ALWAYS_INLINE operator uint16() const { return value; } METAL_ALWAYS_INLINE uint16 operator*() const { return value; } METAL_ALWAYS_INLINE uint16 get() const { return value; } };
template <> struct uniform<short> { short value; METAL_ALWAYS_INLINE operator short() const { return value; } METAL_ALWAYS_INLINE short operator*() const { return value; } METAL_ALWAYS_INLINE short get() const { return value; } };
template <> struct uniform<short2> { short2 value; METAL_ALWAYS_INLINE operator short2() const { return value; } METAL_ALWAYS_INLINE short2 operator*() const { return value; } METAL_ALWAYS_INLINE short2 get() const { return value; } };
template <> struct uniform<short3> { short3 value; METAL_ALWAYS_INLINE operator short3() const { return value; } METAL_ALWAYS_INLINE short3 operator*() const { return value; } METAL_ALWAYS_INLINE short3 get() const { return value; } };
template <> struct uniform<short4> { short4 value; METAL_ALWAYS_INLINE operator short4() const { return value; } METAL_ALWAYS_INLINE short4 operator*() const { return value; } METAL_ALWAYS_INLINE short4 get() const { return value; } };
template <> struct uniform<short8> { short8 value; METAL_ALWAYS_INLINE operator short8() const { return value; } METAL_ALWAYS_INLINE short8 operator*() const { return value; } METAL_ALWAYS_INLINE short8 get() const { return value; } };
template <> struct uniform<short16> { short16 value; METAL_ALWAYS_INLINE operator short16() const { return value; } METAL_ALWAYS_INLINE short16 operator*() const { return value; } METAL_ALWAYS_INLINE short16 get() const { return value; } };
template <> struct uniform<ushort> { ushort value; METAL_ALWAYS_INLINE operator ushort() const { return value; } METAL_ALWAYS_INLINE ushort operator*() const { return value; } METAL_ALWAYS_INLINE ushort get() const { return value; } };
template <> struct uniform<ushort2> { ushort2 value; METAL_ALWAYS_INLINE operator ushort2() const { return value; } METAL_ALWAYS_INLINE ushort2 operator*() const { return value; } METAL_ALWAYS_INLINE ushort2 get() const { return value; } };
template <> struct uniform<ushort3> { ushort3 value; METAL_ALWAYS_INLINE operator ushort3() const { return value; } METAL_ALWAYS_INLINE ushort3 operator*() const { return value; } METAL_ALWAYS_INLINE ushort3 get() const { return value; } };
template <> struct uniform<ushort4> { ushort4 value; METAL_ALWAYS_INLINE operator ushort4() const { return value; } METAL_ALWAYS_INLINE ushort4 operator*() const { return value; } METAL_ALWAYS_INLINE ushort4 get() const { return value; } };
template <> struct uniform<ushort8> { ushort8 value; METAL_ALWAYS_INLINE operator ushort8() const { return value; } METAL_ALWAYS_INLINE ushort8 operator*() const { return value; } METAL_ALWAYS_INLINE ushort8 get() const { return value; } };
template <> struct uniform<ushort16> { ushort16 value; METAL_ALWAYS_INLINE operator ushort16() const { return value; } METAL_ALWAYS_INLINE ushort16 operator*() const { return value; } METAL_ALWAYS_INLINE ushort16 get() const { return value; } };
template <> struct uniform<char> { char value; METAL_ALWAYS_INLINE operator char() const { return value; } METAL_ALWAYS_INLINE char operator*() const { return value; } METAL_ALWAYS_INLINE char get() const { return value; } };
template <> struct uniform<char2> { char2 value; METAL_ALWAYS_INLINE operator char2() const { return value; } METAL_ALWAYS_INLINE char2 operator*() const { return value; } METAL_ALWAYS_INLINE char2 get() const { return value; } };
template <> struct uniform<char3> { char3 value; METAL_ALWAYS_INLINE operator char3() const { return value; } METAL_ALWAYS_INLINE char3 operator*() const { return value; } METAL_ALWAYS_INLINE char3 get() const { return value; } };
template <> struct uniform<char4> { char4 value; METAL_ALWAYS_INLINE operator char4() const { return value; } METAL_ALWAYS_INLINE char4 operator*() const { return value; } METAL_ALWAYS_INLINE char4 get() const { return value; } };
template <> struct uniform<char8> { char8 value; METAL_ALWAYS_INLINE operator char8() const { return value; } METAL_ALWAYS_INLINE char8 operator*() const { return value; } METAL_ALWAYS_INLINE char8 get() const { return value; } };
template <> struct uniform<char16> { char16 value; METAL_ALWAYS_INLINE operator char16() const { return value; } METAL_ALWAYS_INLINE char16 operator*() const { return value; } METAL_ALWAYS_INLINE char16 get() const { return value; } };
template <> struct uniform<uchar> { uchar value; METAL_ALWAYS_INLINE operator uchar() const { return value; } METAL_ALWAYS_INLINE uchar operator*() const { return value; } METAL_ALWAYS_INLINE uchar get() const { return value; } };
template <> struct uniform<uchar2> { uchar2 value; METAL_ALWAYS_INLINE operator uchar2() const { return value; } METAL_ALWAYS_INLINE uchar2 operator*() const { return value; } METAL_ALWAYS_INLINE uchar2 get() const { return value; } };
template <> struct uniform<uchar3> { uchar3 value; METAL_ALWAYS_INLINE operator uchar3() const { return value; } METAL_ALWAYS_INLINE uchar3 operator*() const { return value; } METAL_ALWAYS_INLINE uchar3 get() const { return value; } };
template <> struct uniform<uchar4> { uchar4 value; METAL_ALWAYS_INLINE operator uchar4() const { return value; } METAL_ALWAYS_INLINE uchar4 operator*() const { return value; } METAL_ALWAYS_INLINE uchar4 get() const { return value; } };
template <> struct uniform<uchar8> { uchar8 value; METAL_ALWAYS_INLINE operator uchar8() const { return value; } METAL_ALWAYS_INLINE uchar8 operator*() const { return value; } METAL_ALWAYS_INLINE uchar8 get() const { return value; } };
template <> struct uniform<uchar16> { uchar16 value; METAL_ALWAYS_INLINE operator uchar16() const { return value; } METAL_ALWAYS_INLINE uchar16 operator*() const { return value; } METAL_ALWAYS_INLINE uchar16 get() const { return value; } };
template <> struct uniform<long> { long value; METAL_ALWAYS_INLINE operator long() const { return value; } METAL_ALWAYS_INLINE long operator*() const { return value; } METAL_ALWAYS_INLINE long get() const { return value; } };
template <> struct uniform<long2> { long2 value; METAL_ALWAYS_INLINE operator long2() const { return value; } METAL_ALWAYS_INLINE long2 operator*() const { return value; } METAL_ALWAYS_INLINE long2 get() const { return value; } };
template <> struct uniform<long3> { long3 value; METAL_ALWAYS_INLINE operator long3() const { return value; } METAL_ALWAYS_INLINE long3 operator*() const { return value; } METAL_ALWAYS_INLINE long3 get() const { return value; } };
template <> struct uniform<long4> { long4 value; METAL_ALWAYS_INLINE operator long4() const { return value; } METAL_ALWAYS_INLINE long4 operator*() const { return value; } METAL_ALWAYS_INLINE long4 get() const { return value; } };
template <> struct uniform<long8> { long8 value; METAL_ALWAYS_INLINE operator long8() const { return value; } METAL_ALWAYS_INLINE long8 operator*() const { return value; } METAL_ALWAYS_INLINE long8 get() const { return value; } };
template <> struct uniform<long16> { long16 value; METAL_ALWAYS_INLINE operator long16() const { return value; } METAL_ALWAYS_INLINE long16 operator*() const { return value; } METAL_ALWAYS_INLINE long16 get() const { return value; } };
template <> struct uniform<ulong> { ulong value; METAL_ALWAYS_INLINE operator ulong() const { return value; } METAL_ALWAYS_INLINE ulong operator*() const { return value; } METAL_ALWAYS_INLINE ulong get() const { return value; } };
template <> struct uniform<ulong2> { ulong2 value; METAL_ALWAYS_INLINE operator ulong2() const { return value; } METAL_ALWAYS_INLINE ulong2 operator*() const { return value; } METAL_ALWAYS_INLINE ulong2 get() const { return value; } };
template <> struct uniform<ulong3> { ulong3 value; METAL_ALWAYS_INLINE operator ulong3() const { return value; } METAL_ALWAYS_INLINE ulong3 operator*() const { return value; } METAL_ALWAYS_INLINE ulong3 get() const { return value; } };
template <> struct uniform<ulong4> { ulong4 value; METAL_ALWAYS_INLINE operator ulong4() const { return value; } METAL_ALWAYS_INLINE ulong4 operator*() const { return value; } METAL_ALWAYS_INLINE ulong4 get() const { return value; } };
template <> struct uniform<ulong8> { ulong8 value; METAL_ALWAYS_INLINE operator ulong8() const { return value; } METAL_ALWAYS_INLINE ulong8 operator*() const { return value; } METAL_ALWAYS_INLINE ulong8 get() const { return value; } };
template <> struct uniform<ulong16> { ulong16 value; METAL_ALWAYS_INLINE operator ulong16() const { return value; } METAL_ALWAYS_INLINE ulong16 operator*() const { return value; } METAL_ALWAYS_INLINE ulong16 get() const { return value; } };
template <> struct uniform<bfloat> { bfloat value; METAL_ALWAYS_INLINE operator bfloat() const { return value; } METAL_ALWAYS_INLINE bfloat operator*() const { return value; } METAL_ALWAYS_INLINE bfloat get() const { return value; } };
template <> struct uniform<bfloat2> { bfloat2 value; METAL_ALWAYS_INLINE operator bfloat2() const { return value; } METAL_ALWAYS_INLINE bfloat2 operator*() const { return value; } METAL_ALWAYS_INLINE bfloat2 get() const { return value; } };
template <> struct uniform<bfloat3> { bfloat3 value; METAL_ALWAYS_INLINE operator bfloat3() const { return value; } METAL_ALWAYS_INLINE bfloat3 operator*() const { return value; } METAL_ALWAYS_INLINE bfloat3 get() const { return value; } };
template <> struct uniform<bfloat4> { bfloat4 value; METAL_ALWAYS_INLINE operator bfloat4() const { return value; } METAL_ALWAYS_INLINE bfloat4 operator*() const { return value; } METAL_ALWAYS_INLINE bfloat4 get() const { return value; } };
template <> struct uniform<bfloat8> { bfloat8 value; METAL_ALWAYS_INLINE operator bfloat8() const { return value; } METAL_ALWAYS_INLINE bfloat8 operator*() const { return value; } METAL_ALWAYS_INLINE bfloat8 get() const { return value; } };
template <> struct uniform<bfloat16> { bfloat16 value; METAL_ALWAYS_INLINE operator bfloat16() const { return value; } METAL_ALWAYS_INLINE bfloat16 operator*() const { return value; } METAL_ALWAYS_INLINE bfloat16 get() const { return value; } };
template <> struct uniform<bool> { bool value; METAL_ALWAYS_INLINE operator bool() const { return value; } METAL_ALWAYS_INLINE bool operator*() const { return value; } METAL_ALWAYS_INLINE bool get() const { return value; } };
template <> struct uniform<bool2> { bool2 value; METAL_ALWAYS_INLINE operator bool2() const { return value; } METAL_ALWAYS_INLINE bool2 operator*() const { return value; } METAL_ALWAYS_INLINE bool2 get() const { return value; } };
template <> struct uniform<bool3> { bool3 value; METAL_ALWAYS_INLINE operator bool3() const { return value; } METAL_ALWAYS_INLINE bool3 operator*() const { return value; } METAL_ALWAYS_INLINE bool3 get() const { return value; } };
template <> struct uniform<bool4> { bool4 value; METAL_ALWAYS_INLINE operator bool4() const { return value; } METAL_ALWAYS_INLINE bool4 operator*() const { return value; } METAL_ALWAYS_INLINE bool4 get() const { return value; } };
template <> struct uniform<bool8> { bool8 value; METAL_ALWAYS_INLINE operator bool8() const { return value; } METAL_ALWAYS_INLINE bool8 operator*() const { return value; } METAL_ALWAYS_INLINE bool8 get() const { return value; } };
template <> struct uniform<bool16> { bool16 value; METAL_ALWAYS_INLINE operator bool16() const { return value; } METAL_ALWAYS_INLINE bool16 operator*() const { return value; } METAL_ALWAYS_INLINE bool16 get() const { return value; } };

template <typename T>
struct interpolant {
  T value;
  METAL_FUNC T interpolate_center() const { return value; }
  METAL_FUNC T interpolate_centroid() const { return value; }
  METAL_FUNC T interpolate_sample(uint s) const { return value; }
  METAL_FUNC T interpolate_offset(float2 off) const { return value; }
  METAL_FUNC operator T() const { return value; }
};

template <> struct interpolant<float> { float value; METAL_ALWAYS_INLINE float interpolate_center() const { return value; } METAL_ALWAYS_INLINE float interpolate_centroid() const { return value; } METAL_ALWAYS_INLINE float interpolate_sample(uint s) const { return value; } METAL_ALWAYS_INLINE float interpolate_offset(float2 off) const { return value; } METAL_ALWAYS_INLINE operator float() const { return value; } };
template <> struct interpolant<float2> { float2 value; METAL_ALWAYS_INLINE float2 interpolate_center() const { return value; } METAL_ALWAYS_INLINE float2 interpolate_centroid() const { return value; } METAL_ALWAYS_INLINE float2 interpolate_sample(uint s) const { return value; } METAL_ALWAYS_INLINE float2 interpolate_offset(float2 off) const { return value; } METAL_ALWAYS_INLINE operator float2() const { return value; } };
template <> struct interpolant<float3> { float3 value; METAL_ALWAYS_INLINE float3 interpolate_center() const { return value; } METAL_ALWAYS_INLINE float3 interpolate_centroid() const { return value; } METAL_ALWAYS_INLINE float3 interpolate_sample(uint s) const { return value; } METAL_ALWAYS_INLINE float3 interpolate_offset(float2 off) const { return value; } METAL_ALWAYS_INLINE operator float3() const { return value; } };
template <> struct interpolant<float4> { float4 value; METAL_ALWAYS_INLINE float4 interpolate_center() const { return value; } METAL_ALWAYS_INLINE float4 interpolate_centroid() const { return value; } METAL_ALWAYS_INLINE float4 interpolate_sample(uint s) const { return value; } METAL_ALWAYS_INLINE float4 interpolate_offset(float2 off) const { return value; } METAL_ALWAYS_INLINE operator float4() const { return value; } };
template <> struct interpolant<half> { half value; METAL_ALWAYS_INLINE half interpolate_center() const { return value; } METAL_ALWAYS_INLINE half interpolate_centroid() const { return value; } METAL_ALWAYS_INLINE half interpolate_sample(uint s) const { return value; } METAL_ALWAYS_INLINE half interpolate_offset(float2 off) const { return value; } METAL_ALWAYS_INLINE operator half() const { return value; } };
template <> struct interpolant<half2> { half2 value; METAL_ALWAYS_INLINE half2 interpolate_center() const { return value; } METAL_ALWAYS_INLINE half2 interpolate_centroid() const { return value; } METAL_ALWAYS_INLINE half2 interpolate_sample(uint s) const { return value; } METAL_ALWAYS_INLINE half2 interpolate_offset(float2 off) const { return value; } METAL_ALWAYS_INLINE operator half2() const { return value; } };
template <> struct interpolant<half3> { half3 value; METAL_ALWAYS_INLINE half3 interpolate_center() const { return value; } METAL_ALWAYS_INLINE half3 interpolate_centroid() const { return value; } METAL_ALWAYS_INLINE half3 interpolate_sample(uint s) const { return value; } METAL_ALWAYS_INLINE half3 interpolate_offset(float2 off) const { return value; } METAL_ALWAYS_INLINE operator half3() const { return value; } };
template <> struct interpolant<half4> { half4 value; METAL_ALWAYS_INLINE half4 interpolate_center() const { return value; } METAL_ALWAYS_INLINE half4 interpolate_centroid() const { return value; } METAL_ALWAYS_INLINE half4 interpolate_sample(uint s) const { return value; } METAL_ALWAYS_INLINE half4 interpolate_offset(float2 off) const { return value; } METAL_ALWAYS_INLINE operator half4() const { return value; } };

} // namespace metal
#endif