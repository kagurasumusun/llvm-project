// metal_pixel — MSL pixel operations (cleanroom)
#ifndef _METAL_PIXEL_H_
#define _METAL_PIXEL_H_
#include <metal/metal_common>
namespace metal {

template <>
struct color_attachment<float> {
  METAL_FUNC float read() const { return float(); }
  METAL_FUNC float read(uint sample) const { return float(); }
  METAL_FUNC void write(float val) const {}
  METAL_FUNC void write(float val, uint sample) const {}
  METAL_FUNC void write(float val, bool retain) const {}
  METAL_FUNC void write(float val, uint sample, bool retain) const {}
};

template <>
struct color_attachment<float2> {
  METAL_FUNC float2 read() const { return float2(); }
  METAL_FUNC float2 read(uint sample) const { return float2(); }
  METAL_FUNC void write(float2 val) const {}
  METAL_FUNC void write(float2 val, uint sample) const {}
  METAL_FUNC void write(float2 val, bool retain) const {}
  METAL_FUNC void write(float2 val, uint sample, bool retain) const {}
};

template <>
struct color_attachment<float3> {
  METAL_FUNC float3 read() const { return float3(); }
  METAL_FUNC float3 read(uint sample) const { return float3(); }
  METAL_FUNC void write(float3 val) const {}
  METAL_FUNC void write(float3 val, uint sample) const {}
  METAL_FUNC void write(float3 val, bool retain) const {}
  METAL_FUNC void write(float3 val, uint sample, bool retain) const {}
};

template <>
struct color_attachment<float4> {
  METAL_FUNC float4 read() const { return float4(); }
  METAL_FUNC float4 read(uint sample) const { return float4(); }
  METAL_FUNC void write(float4 val) const {}
  METAL_FUNC void write(float4 val, uint sample) const {}
  METAL_FUNC void write(float4 val, bool retain) const {}
  METAL_FUNC void write(float4 val, uint sample, bool retain) const {}
};

template <>
struct color_attachment<float8> {
  METAL_FUNC float8 read() const { return float8(); }
  METAL_FUNC float8 read(uint sample) const { return float8(); }
  METAL_FUNC void write(float8 val) const {}
  METAL_FUNC void write(float8 val, uint sample) const {}
  METAL_FUNC void write(float8 val, bool retain) const {}
  METAL_FUNC void write(float8 val, uint sample, bool retain) const {}
};

template <>
struct color_attachment<float16> {
  METAL_FUNC float16 read() const { return float16(); }
  METAL_FUNC float16 read(uint sample) const { return float16(); }
  METAL_FUNC void write(float16 val) const {}
  METAL_FUNC void write(float16 val, uint sample) const {}
  METAL_FUNC void write(float16 val, bool retain) const {}
  METAL_FUNC void write(float16 val, uint sample, bool retain) const {}
};

template <>
struct color_attachment<half> {
  METAL_FUNC half read() const { return half(); }
  METAL_FUNC half read(uint sample) const { return half(); }
  METAL_FUNC void write(half val) const {}
  METAL_FUNC void write(half val, uint sample) const {}
  METAL_FUNC void write(half val, bool retain) const {}
  METAL_FUNC void write(half val, uint sample, bool retain) const {}
};

template <>
struct color_attachment<half2> {
  METAL_FUNC half2 read() const { return half2(); }
  METAL_FUNC half2 read(uint sample) const { return half2(); }
  METAL_FUNC void write(half2 val) const {}
  METAL_FUNC void write(half2 val, uint sample) const {}
  METAL_FUNC void write(half2 val, bool retain) const {}
  METAL_FUNC void write(half2 val, uint sample, bool retain) const {}
};

template <>
struct color_attachment<half3> {
  METAL_FUNC half3 read() const { return half3(); }
  METAL_FUNC half3 read(uint sample) const { return half3(); }
  METAL_FUNC void write(half3 val) const {}
  METAL_FUNC void write(half3 val, uint sample) const {}
  METAL_FUNC void write(half3 val, bool retain) const {}
  METAL_FUNC void write(half3 val, uint sample, bool retain) const {}
};

template <>
struct color_attachment<half4> {
  METAL_FUNC half4 read() const { return half4(); }
  METAL_FUNC half4 read(uint sample) const { return half4(); }
  METAL_FUNC void write(half4 val) const {}
  METAL_FUNC void write(half4 val, uint sample) const {}
  METAL_FUNC void write(half4 val, bool retain) const {}
  METAL_FUNC void write(half4 val, uint sample, bool retain) const {}
};

template <>
struct color_attachment<half8> {
  METAL_FUNC half8 read() const { return half8(); }
  METAL_FUNC half8 read(uint sample) const { return half8(); }
  METAL_FUNC void write(half8 val) const {}
  METAL_FUNC void write(half8 val, uint sample) const {}
  METAL_FUNC void write(half8 val, bool retain) const {}
  METAL_FUNC void write(half8 val, uint sample, bool retain) const {}
};

template <>
struct color_attachment<half16> {
  METAL_FUNC half16 read() const { return half16(); }
  METAL_FUNC half16 read(uint sample) const { return half16(); }
  METAL_FUNC void write(half16 val) const {}
  METAL_FUNC void write(half16 val, uint sample) const {}
  METAL_FUNC void write(half16 val, bool retain) const {}
  METAL_FUNC void write(half16 val, uint sample, bool retain) const {}
};

template <>
struct color_attachment<int> {
  METAL_FUNC int read() const { return int(); }
  METAL_FUNC int read(uint sample) const { return int(); }
  METAL_FUNC void write(int val) const {}
  METAL_FUNC void write(int val, uint sample) const {}
  METAL_FUNC void write(int val, bool retain) const {}
  METAL_FUNC void write(int val, uint sample, bool retain) const {}
};

template <>
struct color_attachment<int2> {
  METAL_FUNC int2 read() const { return int2(); }
  METAL_FUNC int2 read(uint sample) const { return int2(); }
  METAL_FUNC void write(int2 val) const {}
  METAL_FUNC void write(int2 val, uint sample) const {}
  METAL_FUNC void write(int2 val, bool retain) const {}
  METAL_FUNC void write(int2 val, uint sample, bool retain) const {}
};

template <>
struct color_attachment<int3> {
  METAL_FUNC int3 read() const { return int3(); }
  METAL_FUNC int3 read(uint sample) const { return int3(); }
  METAL_FUNC void write(int3 val) const {}
  METAL_FUNC void write(int3 val, uint sample) const {}
  METAL_FUNC void write(int3 val, bool retain) const {}
  METAL_FUNC void write(int3 val, uint sample, bool retain) const {}
};

template <>
struct color_attachment<int4> {
  METAL_FUNC int4 read() const { return int4(); }
  METAL_FUNC int4 read(uint sample) const { return int4(); }
  METAL_FUNC void write(int4 val) const {}
  METAL_FUNC void write(int4 val, uint sample) const {}
  METAL_FUNC void write(int4 val, bool retain) const {}
  METAL_FUNC void write(int4 val, uint sample, bool retain) const {}
};

template <>
struct color_attachment<int8> {
  METAL_FUNC int8 read() const { return int8(); }
  METAL_FUNC int8 read(uint sample) const { return int8(); }
  METAL_FUNC void write(int8 val) const {}
  METAL_FUNC void write(int8 val, uint sample) const {}
  METAL_FUNC void write(int8 val, bool retain) const {}
  METAL_FUNC void write(int8 val, uint sample, bool retain) const {}
};

template <>
struct color_attachment<int16> {
  METAL_FUNC int16 read() const { return int16(); }
  METAL_FUNC int16 read(uint sample) const { return int16(); }
  METAL_FUNC void write(int16 val) const {}
  METAL_FUNC void write(int16 val, uint sample) const {}
  METAL_FUNC void write(int16 val, bool retain) const {}
  METAL_FUNC void write(int16 val, uint sample, bool retain) const {}
};

template <>
struct color_attachment<uint> {
  METAL_FUNC uint read() const { return uint(); }
  METAL_FUNC uint read(uint sample) const { return uint(); }
  METAL_FUNC void write(uint val) const {}
  METAL_FUNC void write(uint val, uint sample) const {}
  METAL_FUNC void write(uint val, bool retain) const {}
  METAL_FUNC void write(uint val, uint sample, bool retain) const {}
};

template <>
struct color_attachment<uint2> {
  METAL_FUNC uint2 read() const { return uint2(); }
  METAL_FUNC uint2 read(uint sample) const { return uint2(); }
  METAL_FUNC void write(uint2 val) const {}
  METAL_FUNC void write(uint2 val, uint sample) const {}
  METAL_FUNC void write(uint2 val, bool retain) const {}
  METAL_FUNC void write(uint2 val, uint sample, bool retain) const {}
};

template <>
struct color_attachment<uint3> {
  METAL_FUNC uint3 read() const { return uint3(); }
  METAL_FUNC uint3 read(uint sample) const { return uint3(); }
  METAL_FUNC void write(uint3 val) const {}
  METAL_FUNC void write(uint3 val, uint sample) const {}
  METAL_FUNC void write(uint3 val, bool retain) const {}
  METAL_FUNC void write(uint3 val, uint sample, bool retain) const {}
};

template <>
struct color_attachment<uint4> {
  METAL_FUNC uint4 read() const { return uint4(); }
  METAL_FUNC uint4 read(uint sample) const { return uint4(); }
  METAL_FUNC void write(uint4 val) const {}
  METAL_FUNC void write(uint4 val, uint sample) const {}
  METAL_FUNC void write(uint4 val, bool retain) const {}
  METAL_FUNC void write(uint4 val, uint sample, bool retain) const {}
};

template <>
struct color_attachment<uint8> {
  METAL_FUNC uint8 read() const { return uint8(); }
  METAL_FUNC uint8 read(uint sample) const { return uint8(); }
  METAL_FUNC void write(uint8 val) const {}
  METAL_FUNC void write(uint8 val, uint sample) const {}
  METAL_FUNC void write(uint8 val, bool retain) const {}
  METAL_FUNC void write(uint8 val, uint sample, bool retain) const {}
};

template <>
struct color_attachment<uint16> {
  METAL_FUNC uint16 read() const { return uint16(); }
  METAL_FUNC uint16 read(uint sample) const { return uint16(); }
  METAL_FUNC void write(uint16 val) const {}
  METAL_FUNC void write(uint16 val, uint sample) const {}
  METAL_FUNC void write(uint16 val, bool retain) const {}
  METAL_FUNC void write(uint16 val, uint sample, bool retain) const {}
};

template <>
struct color_attachment<short> {
  METAL_FUNC short read() const { return short(); }
  METAL_FUNC short read(uint sample) const { return short(); }
  METAL_FUNC void write(short val) const {}
  METAL_FUNC void write(short val, uint sample) const {}
  METAL_FUNC void write(short val, bool retain) const {}
  METAL_FUNC void write(short val, uint sample, bool retain) const {}
};

template <>
struct color_attachment<short2> {
  METAL_FUNC short2 read() const { return short2(); }
  METAL_FUNC short2 read(uint sample) const { return short2(); }
  METAL_FUNC void write(short2 val) const {}
  METAL_FUNC void write(short2 val, uint sample) const {}
  METAL_FUNC void write(short2 val, bool retain) const {}
  METAL_FUNC void write(short2 val, uint sample, bool retain) const {}
};

template <>
struct color_attachment<short3> {
  METAL_FUNC short3 read() const { return short3(); }
  METAL_FUNC short3 read(uint sample) const { return short3(); }
  METAL_FUNC void write(short3 val) const {}
  METAL_FUNC void write(short3 val, uint sample) const {}
  METAL_FUNC void write(short3 val, bool retain) const {}
  METAL_FUNC void write(short3 val, uint sample, bool retain) const {}
};

template <>
struct color_attachment<short4> {
  METAL_FUNC short4 read() const { return short4(); }
  METAL_FUNC short4 read(uint sample) const { return short4(); }
  METAL_FUNC void write(short4 val) const {}
  METAL_FUNC void write(short4 val, uint sample) const {}
  METAL_FUNC void write(short4 val, bool retain) const {}
  METAL_FUNC void write(short4 val, uint sample, bool retain) const {}
};

template <>
struct color_attachment<short8> {
  METAL_FUNC short8 read() const { return short8(); }
  METAL_FUNC short8 read(uint sample) const { return short8(); }
  METAL_FUNC void write(short8 val) const {}
  METAL_FUNC void write(short8 val, uint sample) const {}
  METAL_FUNC void write(short8 val, bool retain) const {}
  METAL_FUNC void write(short8 val, uint sample, bool retain) const {}
};

template <>
struct color_attachment<short16> {
  METAL_FUNC short16 read() const { return short16(); }
  METAL_FUNC short16 read(uint sample) const { return short16(); }
  METAL_FUNC void write(short16 val) const {}
  METAL_FUNC void write(short16 val, uint sample) const {}
  METAL_FUNC void write(short16 val, bool retain) const {}
  METAL_FUNC void write(short16 val, uint sample, bool retain) const {}
};

template <>
struct color_attachment<ushort> {
  METAL_FUNC ushort read() const { return ushort(); }
  METAL_FUNC ushort read(uint sample) const { return ushort(); }
  METAL_FUNC void write(ushort val) const {}
  METAL_FUNC void write(ushort val, uint sample) const {}
  METAL_FUNC void write(ushort val, bool retain) const {}
  METAL_FUNC void write(ushort val, uint sample, bool retain) const {}
};

template <>
struct color_attachment<ushort2> {
  METAL_FUNC ushort2 read() const { return ushort2(); }
  METAL_FUNC ushort2 read(uint sample) const { return ushort2(); }
  METAL_FUNC void write(ushort2 val) const {}
  METAL_FUNC void write(ushort2 val, uint sample) const {}
  METAL_FUNC void write(ushort2 val, bool retain) const {}
  METAL_FUNC void write(ushort2 val, uint sample, bool retain) const {}
};

template <>
struct color_attachment<ushort3> {
  METAL_FUNC ushort3 read() const { return ushort3(); }
  METAL_FUNC ushort3 read(uint sample) const { return ushort3(); }
  METAL_FUNC void write(ushort3 val) const {}
  METAL_FUNC void write(ushort3 val, uint sample) const {}
  METAL_FUNC void write(ushort3 val, bool retain) const {}
  METAL_FUNC void write(ushort3 val, uint sample, bool retain) const {}
};

template <>
struct color_attachment<ushort4> {
  METAL_FUNC ushort4 read() const { return ushort4(); }
  METAL_FUNC ushort4 read(uint sample) const { return ushort4(); }
  METAL_FUNC void write(ushort4 val) const {}
  METAL_FUNC void write(ushort4 val, uint sample) const {}
  METAL_FUNC void write(ushort4 val, bool retain) const {}
  METAL_FUNC void write(ushort4 val, uint sample, bool retain) const {}
};

template <>
struct color_attachment<ushort8> {
  METAL_FUNC ushort8 read() const { return ushort8(); }
  METAL_FUNC ushort8 read(uint sample) const { return ushort8(); }
  METAL_FUNC void write(ushort8 val) const {}
  METAL_FUNC void write(ushort8 val, uint sample) const {}
  METAL_FUNC void write(ushort8 val, bool retain) const {}
  METAL_FUNC void write(ushort8 val, uint sample, bool retain) const {}
};

template <>
struct color_attachment<ushort16> {
  METAL_FUNC ushort16 read() const { return ushort16(); }
  METAL_FUNC ushort16 read(uint sample) const { return ushort16(); }
  METAL_FUNC void write(ushort16 val) const {}
  METAL_FUNC void write(ushort16 val, uint sample) const {}
  METAL_FUNC void write(ushort16 val, bool retain) const {}
  METAL_FUNC void write(ushort16 val, uint sample, bool retain) const {}
};

template <>
struct color_attachment<char> {
  METAL_FUNC char read() const { return char(); }
  METAL_FUNC char read(uint sample) const { return char(); }
  METAL_FUNC void write(char val) const {}
  METAL_FUNC void write(char val, uint sample) const {}
  METAL_FUNC void write(char val, bool retain) const {}
  METAL_FUNC void write(char val, uint sample, bool retain) const {}
};

template <>
struct color_attachment<char2> {
  METAL_FUNC char2 read() const { return char2(); }
  METAL_FUNC char2 read(uint sample) const { return char2(); }
  METAL_FUNC void write(char2 val) const {}
  METAL_FUNC void write(char2 val, uint sample) const {}
  METAL_FUNC void write(char2 val, bool retain) const {}
  METAL_FUNC void write(char2 val, uint sample, bool retain) const {}
};

template <>
struct color_attachment<char3> {
  METAL_FUNC char3 read() const { return char3(); }
  METAL_FUNC char3 read(uint sample) const { return char3(); }
  METAL_FUNC void write(char3 val) const {}
  METAL_FUNC void write(char3 val, uint sample) const {}
  METAL_FUNC void write(char3 val, bool retain) const {}
  METAL_FUNC void write(char3 val, uint sample, bool retain) const {}
};

template <>
struct color_attachment<char4> {
  METAL_FUNC char4 read() const { return char4(); }
  METAL_FUNC char4 read(uint sample) const { return char4(); }
  METAL_FUNC void write(char4 val) const {}
  METAL_FUNC void write(char4 val, uint sample) const {}
  METAL_FUNC void write(char4 val, bool retain) const {}
  METAL_FUNC void write(char4 val, uint sample, bool retain) const {}
};

template <>
struct color_attachment<char8> {
  METAL_FUNC char8 read() const { return char8(); }
  METAL_FUNC char8 read(uint sample) const { return char8(); }
  METAL_FUNC void write(char8 val) const {}
  METAL_FUNC void write(char8 val, uint sample) const {}
  METAL_FUNC void write(char8 val, bool retain) const {}
  METAL_FUNC void write(char8 val, uint sample, bool retain) const {}
};

template <>
struct color_attachment<char16> {
  METAL_FUNC char16 read() const { return char16(); }
  METAL_FUNC char16 read(uint sample) const { return char16(); }
  METAL_FUNC void write(char16 val) const {}
  METAL_FUNC void write(char16 val, uint sample) const {}
  METAL_FUNC void write(char16 val, bool retain) const {}
  METAL_FUNC void write(char16 val, uint sample, bool retain) const {}
};

template <>
struct color_attachment<uchar> {
  METAL_FUNC uchar read() const { return uchar(); }
  METAL_FUNC uchar read(uint sample) const { return uchar(); }
  METAL_FUNC void write(uchar val) const {}
  METAL_FUNC void write(uchar val, uint sample) const {}
  METAL_FUNC void write(uchar val, bool retain) const {}
  METAL_FUNC void write(uchar val, uint sample, bool retain) const {}
};

template <>
struct color_attachment<uchar2> {
  METAL_FUNC uchar2 read() const { return uchar2(); }
  METAL_FUNC uchar2 read(uint sample) const { return uchar2(); }
  METAL_FUNC void write(uchar2 val) const {}
  METAL_FUNC void write(uchar2 val, uint sample) const {}
  METAL_FUNC void write(uchar2 val, bool retain) const {}
  METAL_FUNC void write(uchar2 val, uint sample, bool retain) const {}
};

template <>
struct color_attachment<uchar3> {
  METAL_FUNC uchar3 read() const { return uchar3(); }
  METAL_FUNC uchar3 read(uint sample) const { return uchar3(); }
  METAL_FUNC void write(uchar3 val) const {}
  METAL_FUNC void write(uchar3 val, uint sample) const {}
  METAL_FUNC void write(uchar3 val, bool retain) const {}
  METAL_FUNC void write(uchar3 val, uint sample, bool retain) const {}
};

template <>
struct color_attachment<uchar4> {
  METAL_FUNC uchar4 read() const { return uchar4(); }
  METAL_FUNC uchar4 read(uint sample) const { return uchar4(); }
  METAL_FUNC void write(uchar4 val) const {}
  METAL_FUNC void write(uchar4 val, uint sample) const {}
  METAL_FUNC void write(uchar4 val, bool retain) const {}
  METAL_FUNC void write(uchar4 val, uint sample, bool retain) const {}
};

template <>
struct color_attachment<uchar8> {
  METAL_FUNC uchar8 read() const { return uchar8(); }
  METAL_FUNC uchar8 read(uint sample) const { return uchar8(); }
  METAL_FUNC void write(uchar8 val) const {}
  METAL_FUNC void write(uchar8 val, uint sample) const {}
  METAL_FUNC void write(uchar8 val, bool retain) const {}
  METAL_FUNC void write(uchar8 val, uint sample, bool retain) const {}
};

template <>
struct color_attachment<uchar16> {
  METAL_FUNC uchar16 read() const { return uchar16(); }
  METAL_FUNC uchar16 read(uint sample) const { return uchar16(); }
  METAL_FUNC void write(uchar16 val) const {}
  METAL_FUNC void write(uchar16 val, uint sample) const {}
  METAL_FUNC void write(uchar16 val, bool retain) const {}
  METAL_FUNC void write(uchar16 val, uint sample, bool retain) const {}
};

template <>
struct color_attachment<long> {
  METAL_FUNC long read() const { return long(); }
  METAL_FUNC long read(uint sample) const { return long(); }
  METAL_FUNC void write(long val) const {}
  METAL_FUNC void write(long val, uint sample) const {}
  METAL_FUNC void write(long val, bool retain) const {}
  METAL_FUNC void write(long val, uint sample, bool retain) const {}
};

template <>
struct color_attachment<long2> {
  METAL_FUNC long2 read() const { return long2(); }
  METAL_FUNC long2 read(uint sample) const { return long2(); }
  METAL_FUNC void write(long2 val) const {}
  METAL_FUNC void write(long2 val, uint sample) const {}
  METAL_FUNC void write(long2 val, bool retain) const {}
  METAL_FUNC void write(long2 val, uint sample, bool retain) const {}
};

template <>
struct color_attachment<long3> {
  METAL_FUNC long3 read() const { return long3(); }
  METAL_FUNC long3 read(uint sample) const { return long3(); }
  METAL_FUNC void write(long3 val) const {}
  METAL_FUNC void write(long3 val, uint sample) const {}
  METAL_FUNC void write(long3 val, bool retain) const {}
  METAL_FUNC void write(long3 val, uint sample, bool retain) const {}
};

template <>
struct color_attachment<long4> {
  METAL_FUNC long4 read() const { return long4(); }
  METAL_FUNC long4 read(uint sample) const { return long4(); }
  METAL_FUNC void write(long4 val) const {}
  METAL_FUNC void write(long4 val, uint sample) const {}
  METAL_FUNC void write(long4 val, bool retain) const {}
  METAL_FUNC void write(long4 val, uint sample, bool retain) const {}
};

template <>
struct color_attachment<long8> {
  METAL_FUNC long8 read() const { return long8(); }
  METAL_FUNC long8 read(uint sample) const { return long8(); }
  METAL_FUNC void write(long8 val) const {}
  METAL_FUNC void write(long8 val, uint sample) const {}
  METAL_FUNC void write(long8 val, bool retain) const {}
  METAL_FUNC void write(long8 val, uint sample, bool retain) const {}
};

template <>
struct color_attachment<long16> {
  METAL_FUNC long16 read() const { return long16(); }
  METAL_FUNC long16 read(uint sample) const { return long16(); }
  METAL_FUNC void write(long16 val) const {}
  METAL_FUNC void write(long16 val, uint sample) const {}
  METAL_FUNC void write(long16 val, bool retain) const {}
  METAL_FUNC void write(long16 val, uint sample, bool retain) const {}
};

template <>
struct color_attachment<ulong> {
  METAL_FUNC ulong read() const { return ulong(); }
  METAL_FUNC ulong read(uint sample) const { return ulong(); }
  METAL_FUNC void write(ulong val) const {}
  METAL_FUNC void write(ulong val, uint sample) const {}
  METAL_FUNC void write(ulong val, bool retain) const {}
  METAL_FUNC void write(ulong val, uint sample, bool retain) const {}
};

template <>
struct color_attachment<ulong2> {
  METAL_FUNC ulong2 read() const { return ulong2(); }
  METAL_FUNC ulong2 read(uint sample) const { return ulong2(); }
  METAL_FUNC void write(ulong2 val) const {}
  METAL_FUNC void write(ulong2 val, uint sample) const {}
  METAL_FUNC void write(ulong2 val, bool retain) const {}
  METAL_FUNC void write(ulong2 val, uint sample, bool retain) const {}
};

template <>
struct color_attachment<ulong3> {
  METAL_FUNC ulong3 read() const { return ulong3(); }
  METAL_FUNC ulong3 read(uint sample) const { return ulong3(); }
  METAL_FUNC void write(ulong3 val) const {}
  METAL_FUNC void write(ulong3 val, uint sample) const {}
  METAL_FUNC void write(ulong3 val, bool retain) const {}
  METAL_FUNC void write(ulong3 val, uint sample, bool retain) const {}
};

template <>
struct color_attachment<ulong4> {
  METAL_FUNC ulong4 read() const { return ulong4(); }
  METAL_FUNC ulong4 read(uint sample) const { return ulong4(); }
  METAL_FUNC void write(ulong4 val) const {}
  METAL_FUNC void write(ulong4 val, uint sample) const {}
  METAL_FUNC void write(ulong4 val, bool retain) const {}
  METAL_FUNC void write(ulong4 val, uint sample, bool retain) const {}
};

template <>
struct color_attachment<ulong8> {
  METAL_FUNC ulong8 read() const { return ulong8(); }
  METAL_FUNC ulong8 read(uint sample) const { return ulong8(); }
  METAL_FUNC void write(ulong8 val) const {}
  METAL_FUNC void write(ulong8 val, uint sample) const {}
  METAL_FUNC void write(ulong8 val, bool retain) const {}
  METAL_FUNC void write(ulong8 val, uint sample, bool retain) const {}
};

template <>
struct color_attachment<ulong16> {
  METAL_FUNC ulong16 read() const { return ulong16(); }
  METAL_FUNC ulong16 read(uint sample) const { return ulong16(); }
  METAL_FUNC void write(ulong16 val) const {}
  METAL_FUNC void write(ulong16 val, uint sample) const {}
  METAL_FUNC void write(ulong16 val, bool retain) const {}
  METAL_FUNC void write(ulong16 val, uint sample, bool retain) const {}
};

template <typename T>
struct depth_attachment {
  METAL_FUNC T read() const { return T(); }
  METAL_FUNC T read(uint2 coord) const { return T(); }
  METAL_FUNC T read(uint2 coord, uint sample) const { return T(); }
  METAL_FUNC void write(T val) const {}
  METAL_FUNC void write(T val, uint2 coord) const {}
  METAL_FUNC void write(T val, uint2 coord, uint sample) const {}
};

template <typename T>
struct stencil_attachment {
  METAL_FUNC T read() const { return T(); }
  METAL_FUNC T read(uint2 coord) const { return T(); }
  METAL_FUNC void write(T val) const {}
  METAL_FUNC void write(T val, uint2 coord) const {}
};

} // namespace metal
#endif