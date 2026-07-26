// metal_relational — MSL relational functions (cleanroom, exact Apple match)
#ifndef _METAL_RELATIONAL_H_
#define _METAL_RELATIONAL_H_
#include <metal/metal_common>
namespace metal {

METAL_FUNC bool all(bool x) { return __air_all(x); }
METAL_FUNC bool any(bool x) { return __air_any(x); }
METAL_FUNC bool all(bool2 x) { return __air_all(x); }
METAL_FUNC bool any(bool2 x) { return __air_any(x); }
METAL_FUNC bool all(bool3 x) { return __air_all(x); }
METAL_FUNC bool any(bool3 x) { return __air_any(x); }
METAL_FUNC bool all(bool4 x) { return __air_all(x); }
METAL_FUNC bool any(bool4 x) { return __air_any(x); }
METAL_FUNC bool all(bool8 x) { return __air_all(x); }
METAL_FUNC bool any(bool8 x) { return __air_any(x); }
METAL_FUNC bool all(bool16 x) { return __air_all(x); }
METAL_FUNC bool any(bool16 x) { return __air_any(x); }

METAL_FUNC bool isnan(float x) { return __air_isnan(x); }
METAL_FUNC bool2 isnan(float2 x) { return __air_isnan(x); }
METAL_FUNC bool3 isnan(float3 x) { return __air_isnan(x); }
METAL_FUNC bool4 isnan(float4 x) { return __air_isnan(x); }
METAL_FUNC bool8 isnan(float8 x) { return __air_isnan(x); }
METAL_FUNC bool16 isnan(float16 x) { return __air_isnan(x); }
METAL_FUNC bool isnan(half x) { return __air_isnan(x); }
METAL_FUNC bool2 isnan(half2 x) { return __air_isnan(x); }
METAL_FUNC bool3 isnan(half3 x) { return __air_isnan(x); }
METAL_FUNC bool4 isnan(half4 x) { return __air_isnan(x); }
METAL_FUNC bool8 isnan(half8 x) { return __air_isnan(x); }
METAL_FUNC bool16 isnan(half16 x) { return __air_isnan(x); }
METAL_FUNC bool isnan(double x) { return __air_isnan(x); }
METAL_FUNC bool2 isnan(double2 x) { return __air_isnan(x); }
METAL_FUNC bool3 isnan(double3 x) { return __air_isnan(x); }
METAL_FUNC bool4 isnan(double4 x) { return __air_isnan(x); }
METAL_FUNC bool8 isnan(double8 x) { return __air_isnan(x); }
METAL_FUNC bool16 isnan(double16 x) { return __air_isnan(x); }
METAL_FUNC bool isnan(bfloat x) { return __air_isnan(x); }
METAL_FUNC bool2 isnan(bfloat2 x) { return __air_isnan(x); }
METAL_FUNC bool3 isnan(bfloat3 x) { return __air_isnan(x); }
METAL_FUNC bool4 isnan(bfloat4 x) { return __air_isnan(x); }
METAL_FUNC bool8 isnan(bfloat8 x) { return __air_isnan(x); }
METAL_FUNC bool16 isnan(bfloat16 x) { return __air_isnan(x); }

METAL_FUNC bool isinf(float x) { return __air_isinf(x); }
METAL_FUNC bool2 isinf(float2 x) { return __air_isinf(x); }
METAL_FUNC bool3 isinf(float3 x) { return __air_isinf(x); }
METAL_FUNC bool4 isinf(float4 x) { return __air_isinf(x); }
METAL_FUNC bool8 isinf(float8 x) { return __air_isinf(x); }
METAL_FUNC bool16 isinf(float16 x) { return __air_isinf(x); }
METAL_FUNC bool isinf(half x) { return __air_isinf(x); }
METAL_FUNC bool2 isinf(half2 x) { return __air_isinf(x); }
METAL_FUNC bool3 isinf(half3 x) { return __air_isinf(x); }
METAL_FUNC bool4 isinf(half4 x) { return __air_isinf(x); }
METAL_FUNC bool8 isinf(half8 x) { return __air_isinf(x); }
METAL_FUNC bool16 isinf(half16 x) { return __air_isinf(x); }
METAL_FUNC bool isinf(double x) { return __air_isinf(x); }
METAL_FUNC bool2 isinf(double2 x) { return __air_isinf(x); }
METAL_FUNC bool3 isinf(double3 x) { return __air_isinf(x); }
METAL_FUNC bool4 isinf(double4 x) { return __air_isinf(x); }
METAL_FUNC bool8 isinf(double8 x) { return __air_isinf(x); }
METAL_FUNC bool16 isinf(double16 x) { return __air_isinf(x); }
METAL_FUNC bool isinf(bfloat x) { return __air_isinf(x); }
METAL_FUNC bool2 isinf(bfloat2 x) { return __air_isinf(x); }
METAL_FUNC bool3 isinf(bfloat3 x) { return __air_isinf(x); }
METAL_FUNC bool4 isinf(bfloat4 x) { return __air_isinf(x); }
METAL_FUNC bool8 isinf(bfloat8 x) { return __air_isinf(x); }
METAL_FUNC bool16 isinf(bfloat16 x) { return __air_isinf(x); }

METAL_FUNC bool isfinite(float x) { return __air_isfinite(x); }
METAL_FUNC bool2 isfinite(float2 x) { return __air_isfinite(x); }
METAL_FUNC bool3 isfinite(float3 x) { return __air_isfinite(x); }
METAL_FUNC bool4 isfinite(float4 x) { return __air_isfinite(x); }
METAL_FUNC bool8 isfinite(float8 x) { return __air_isfinite(x); }
METAL_FUNC bool16 isfinite(float16 x) { return __air_isfinite(x); }
METAL_FUNC bool isfinite(half x) { return __air_isfinite(x); }
METAL_FUNC bool2 isfinite(half2 x) { return __air_isfinite(x); }
METAL_FUNC bool3 isfinite(half3 x) { return __air_isfinite(x); }
METAL_FUNC bool4 isfinite(half4 x) { return __air_isfinite(x); }
METAL_FUNC bool8 isfinite(half8 x) { return __air_isfinite(x); }
METAL_FUNC bool16 isfinite(half16 x) { return __air_isfinite(x); }
METAL_FUNC bool isfinite(double x) { return __air_isfinite(x); }
METAL_FUNC bool2 isfinite(double2 x) { return __air_isfinite(x); }
METAL_FUNC bool3 isfinite(double3 x) { return __air_isfinite(x); }
METAL_FUNC bool4 isfinite(double4 x) { return __air_isfinite(x); }
METAL_FUNC bool8 isfinite(double8 x) { return __air_isfinite(x); }
METAL_FUNC bool16 isfinite(double16 x) { return __air_isfinite(x); }
METAL_FUNC bool isfinite(bfloat x) { return __air_isfinite(x); }
METAL_FUNC bool2 isfinite(bfloat2 x) { return __air_isfinite(x); }
METAL_FUNC bool3 isfinite(bfloat3 x) { return __air_isfinite(x); }
METAL_FUNC bool4 isfinite(bfloat4 x) { return __air_isfinite(x); }
METAL_FUNC bool8 isfinite(bfloat8 x) { return __air_isfinite(x); }
METAL_FUNC bool16 isfinite(bfloat16 x) { return __air_isfinite(x); }

METAL_FUNC bool isnormal(float x) { return __air_isnormal(x); }
METAL_FUNC bool2 isnormal(float2 x) { return __air_isnormal(x); }
METAL_FUNC bool3 isnormal(float3 x) { return __air_isnormal(x); }
METAL_FUNC bool4 isnormal(float4 x) { return __air_isnormal(x); }
METAL_FUNC bool8 isnormal(float8 x) { return __air_isnormal(x); }
METAL_FUNC bool16 isnormal(float16 x) { return __air_isnormal(x); }
METAL_FUNC bool isnormal(half x) { return __air_isnormal(x); }
METAL_FUNC bool2 isnormal(half2 x) { return __air_isnormal(x); }
METAL_FUNC bool3 isnormal(half3 x) { return __air_isnormal(x); }
METAL_FUNC bool4 isnormal(half4 x) { return __air_isnormal(x); }
METAL_FUNC bool8 isnormal(half8 x) { return __air_isnormal(x); }
METAL_FUNC bool16 isnormal(half16 x) { return __air_isnormal(x); }
METAL_FUNC bool isnormal(double x) { return __air_isnormal(x); }
METAL_FUNC bool2 isnormal(double2 x) { return __air_isnormal(x); }
METAL_FUNC bool3 isnormal(double3 x) { return __air_isnormal(x); }
METAL_FUNC bool4 isnormal(double4 x) { return __air_isnormal(x); }
METAL_FUNC bool8 isnormal(double8 x) { return __air_isnormal(x); }
METAL_FUNC bool16 isnormal(double16 x) { return __air_isnormal(x); }
METAL_FUNC bool isnormal(bfloat x) { return __air_isnormal(x); }
METAL_FUNC bool2 isnormal(bfloat2 x) { return __air_isnormal(x); }
METAL_FUNC bool3 isnormal(bfloat3 x) { return __air_isnormal(x); }
METAL_FUNC bool4 isnormal(bfloat4 x) { return __air_isnormal(x); }
METAL_FUNC bool8 isnormal(bfloat8 x) { return __air_isnormal(x); }
METAL_FUNC bool16 isnormal(bfloat16 x) { return __air_isnormal(x); }

METAL_FUNC bool signbit(float x) { return __air_signbit(x); }
METAL_FUNC bool2 signbit(float2 x) { return __air_signbit(x); }
METAL_FUNC bool3 signbit(float3 x) { return __air_signbit(x); }
METAL_FUNC bool4 signbit(float4 x) { return __air_signbit(x); }
METAL_FUNC bool8 signbit(float8 x) { return __air_signbit(x); }
METAL_FUNC bool16 signbit(float16 x) { return __air_signbit(x); }
METAL_FUNC bool signbit(half x) { return __air_signbit(x); }
METAL_FUNC bool2 signbit(half2 x) { return __air_signbit(x); }
METAL_FUNC bool3 signbit(half3 x) { return __air_signbit(x); }
METAL_FUNC bool4 signbit(half4 x) { return __air_signbit(x); }
METAL_FUNC bool8 signbit(half8 x) { return __air_signbit(x); }
METAL_FUNC bool16 signbit(half16 x) { return __air_signbit(x); }
METAL_FUNC bool signbit(double x) { return __air_signbit(x); }
METAL_FUNC bool2 signbit(double2 x) { return __air_signbit(x); }
METAL_FUNC bool3 signbit(double3 x) { return __air_signbit(x); }
METAL_FUNC bool4 signbit(double4 x) { return __air_signbit(x); }
METAL_FUNC bool8 signbit(double8 x) { return __air_signbit(x); }
METAL_FUNC bool16 signbit(double16 x) { return __air_signbit(x); }
METAL_FUNC bool signbit(bfloat x) { return __air_signbit(x); }
METAL_FUNC bool2 signbit(bfloat2 x) { return __air_signbit(x); }
METAL_FUNC bool3 signbit(bfloat3 x) { return __air_signbit(x); }
METAL_FUNC bool4 signbit(bfloat4 x) { return __air_signbit(x); }
METAL_FUNC bool8 signbit(bfloat8 x) { return __air_signbit(x); }
METAL_FUNC bool16 signbit(bfloat16 x) { return __air_signbit(x); }

METAL_FUNC bool isordered(float x, float y) { return __air_isordered(x, y); }
METAL_FUNC bool2 isordered(float2 x, float2 y) { return __air_isordered(x, y); }
METAL_FUNC bool3 isordered(float3 x, float3 y) { return __air_isordered(x, y); }
METAL_FUNC bool4 isordered(float4 x, float4 y) { return __air_isordered(x, y); }
METAL_FUNC bool8 isordered(float8 x, float8 y) { return __air_isordered(x, y); }
METAL_FUNC bool16 isordered(float16 x, float16 y) { return __air_isordered(x, y); }
METAL_FUNC bool isordered(half x, half y) { return __air_isordered(x, y); }
METAL_FUNC bool2 isordered(half2 x, half2 y) { return __air_isordered(x, y); }
METAL_FUNC bool3 isordered(half3 x, half3 y) { return __air_isordered(x, y); }
METAL_FUNC bool4 isordered(half4 x, half4 y) { return __air_isordered(x, y); }
METAL_FUNC bool8 isordered(half8 x, half8 y) { return __air_isordered(x, y); }
METAL_FUNC bool16 isordered(half16 x, half16 y) { return __air_isordered(x, y); }
METAL_FUNC bool isordered(double x, double y) { return __air_isordered(x, y); }
METAL_FUNC bool2 isordered(double2 x, double2 y) { return __air_isordered(x, y); }
METAL_FUNC bool3 isordered(double3 x, double3 y) { return __air_isordered(x, y); }
METAL_FUNC bool4 isordered(double4 x, double4 y) { return __air_isordered(x, y); }
METAL_FUNC bool8 isordered(double8 x, double8 y) { return __air_isordered(x, y); }
METAL_FUNC bool16 isordered(double16 x, double16 y) { return __air_isordered(x, y); }
METAL_FUNC bool isordered(bfloat x, bfloat y) { return __air_isordered(x, y); }
METAL_FUNC bool2 isordered(bfloat2 x, bfloat2 y) { return __air_isordered(x, y); }
METAL_FUNC bool3 isordered(bfloat3 x, bfloat3 y) { return __air_isordered(x, y); }
METAL_FUNC bool4 isordered(bfloat4 x, bfloat4 y) { return __air_isordered(x, y); }
METAL_FUNC bool8 isordered(bfloat8 x, bfloat8 y) { return __air_isordered(x, y); }
METAL_FUNC bool16 isordered(bfloat16 x, bfloat16 y) { return __air_isordered(x, y); }

METAL_FUNC bool isunordered(float x, float y) { return __air_isunordered(x, y); }
METAL_FUNC bool2 isunordered(float2 x, float2 y) { return __air_isunordered(x, y); }
METAL_FUNC bool3 isunordered(float3 x, float3 y) { return __air_isunordered(x, y); }
METAL_FUNC bool4 isunordered(float4 x, float4 y) { return __air_isunordered(x, y); }
METAL_FUNC bool8 isunordered(float8 x, float8 y) { return __air_isunordered(x, y); }
METAL_FUNC bool16 isunordered(float16 x, float16 y) { return __air_isunordered(x, y); }
METAL_FUNC bool isunordered(half x, half y) { return __air_isunordered(x, y); }
METAL_FUNC bool2 isunordered(half2 x, half2 y) { return __air_isunordered(x, y); }
METAL_FUNC bool3 isunordered(half3 x, half3 y) { return __air_isunordered(x, y); }
METAL_FUNC bool4 isunordered(half4 x, half4 y) { return __air_isunordered(x, y); }
METAL_FUNC bool8 isunordered(half8 x, half8 y) { return __air_isunordered(x, y); }
METAL_FUNC bool16 isunordered(half16 x, half16 y) { return __air_isunordered(x, y); }
METAL_FUNC bool isunordered(double x, double y) { return __air_isunordered(x, y); }
METAL_FUNC bool2 isunordered(double2 x, double2 y) { return __air_isunordered(x, y); }
METAL_FUNC bool3 isunordered(double3 x, double3 y) { return __air_isunordered(x, y); }
METAL_FUNC bool4 isunordered(double4 x, double4 y) { return __air_isunordered(x, y); }
METAL_FUNC bool8 isunordered(double8 x, double8 y) { return __air_isunordered(x, y); }
METAL_FUNC bool16 isunordered(double16 x, double16 y) { return __air_isunordered(x, y); }
METAL_FUNC bool isunordered(bfloat x, bfloat y) { return __air_isunordered(x, y); }
METAL_FUNC bool2 isunordered(bfloat2 x, bfloat2 y) { return __air_isunordered(x, y); }
METAL_FUNC bool3 isunordered(bfloat3 x, bfloat3 y) { return __air_isunordered(x, y); }
METAL_FUNC bool4 isunordered(bfloat4 x, bfloat4 y) { return __air_isunordered(x, y); }
METAL_FUNC bool8 isunordered(bfloat8 x, bfloat8 y) { return __air_isunordered(x, y); }
METAL_FUNC bool16 isunordered(bfloat16 x, bfloat16 y) { return __air_isunordered(x, y); }

METAL_FUNC float select(float a, float b, bool c) { return c ? b : a; }
METAL_FUNC float2 select(float2 a, float2 b, bool2 c) { return c ? b : a; }
METAL_FUNC float3 select(float3 a, float3 b, bool3 c) { return c ? b : a; }
METAL_FUNC float4 select(float4 a, float4 b, bool4 c) { return c ? b : a; }
METAL_FUNC float8 select(float8 a, float8 b, bool8 c) { return c ? b : a; }
METAL_FUNC float16 select(float16 a, float16 b, bool16 c) { return c ? b : a; }
METAL_FUNC half select(half a, half b, bool c) { return c ? b : a; }
METAL_FUNC half2 select(half2 a, half2 b, bool2 c) { return c ? b : a; }
METAL_FUNC half3 select(half3 a, half3 b, bool3 c) { return c ? b : a; }
METAL_FUNC half4 select(half4 a, half4 b, bool4 c) { return c ? b : a; }
METAL_FUNC half8 select(half8 a, half8 b, bool8 c) { return c ? b : a; }
METAL_FUNC half16 select(half16 a, half16 b, bool16 c) { return c ? b : a; }
METAL_FUNC double select(double a, double b, bool c) { return c ? b : a; }
METAL_FUNC double2 select(double2 a, double2 b, bool2 c) { return c ? b : a; }
METAL_FUNC double3 select(double3 a, double3 b, bool3 c) { return c ? b : a; }
METAL_FUNC double4 select(double4 a, double4 b, bool4 c) { return c ? b : a; }
METAL_FUNC double8 select(double8 a, double8 b, bool8 c) { return c ? b : a; }
METAL_FUNC double16 select(double16 a, double16 b, bool16 c) { return c ? b : a; }
METAL_FUNC int select(int a, int b, bool c) { return c ? b : a; }
METAL_FUNC int2 select(int2 a, int2 b, bool2 c) { return c ? b : a; }
METAL_FUNC int3 select(int3 a, int3 b, bool3 c) { return c ? b : a; }
METAL_FUNC int4 select(int4 a, int4 b, bool4 c) { return c ? b : a; }
METAL_FUNC int8 select(int8 a, int8 b, bool8 c) { return c ? b : a; }
METAL_FUNC int16 select(int16 a, int16 b, bool16 c) { return c ? b : a; }
METAL_FUNC uint select(uint a, uint b, bool c) { return c ? b : a; }
METAL_FUNC uint2 select(uint2 a, uint2 b, bool2 c) { return c ? b : a; }
METAL_FUNC uint3 select(uint3 a, uint3 b, bool3 c) { return c ? b : a; }
METAL_FUNC uint4 select(uint4 a, uint4 b, bool4 c) { return c ? b : a; }
METAL_FUNC uint8 select(uint8 a, uint8 b, bool8 c) { return c ? b : a; }
METAL_FUNC uint16 select(uint16 a, uint16 b, bool16 c) { return c ? b : a; }
METAL_FUNC short select(short a, short b, bool c) { return c ? b : a; }
METAL_FUNC short2 select(short2 a, short2 b, bool2 c) { return c ? b : a; }
METAL_FUNC short3 select(short3 a, short3 b, bool3 c) { return c ? b : a; }
METAL_FUNC short4 select(short4 a, short4 b, bool4 c) { return c ? b : a; }
METAL_FUNC short8 select(short8 a, short8 b, bool8 c) { return c ? b : a; }
METAL_FUNC short16 select(short16 a, short16 b, bool16 c) { return c ? b : a; }
METAL_FUNC ushort select(ushort a, ushort b, bool c) { return c ? b : a; }
METAL_FUNC ushort2 select(ushort2 a, ushort2 b, bool2 c) { return c ? b : a; }
METAL_FUNC ushort3 select(ushort3 a, ushort3 b, bool3 c) { return c ? b : a; }
METAL_FUNC ushort4 select(ushort4 a, ushort4 b, bool4 c) { return c ? b : a; }
METAL_FUNC ushort8 select(ushort8 a, ushort8 b, bool8 c) { return c ? b : a; }
METAL_FUNC ushort16 select(ushort16 a, ushort16 b, bool16 c) { return c ? b : a; }
METAL_FUNC char select(char a, char b, bool c) { return c ? b : a; }
METAL_FUNC char2 select(char2 a, char2 b, bool2 c) { return c ? b : a; }
METAL_FUNC char3 select(char3 a, char3 b, bool3 c) { return c ? b : a; }
METAL_FUNC char4 select(char4 a, char4 b, bool4 c) { return c ? b : a; }
METAL_FUNC char8 select(char8 a, char8 b, bool8 c) { return c ? b : a; }
METAL_FUNC char16 select(char16 a, char16 b, bool16 c) { return c ? b : a; }
METAL_FUNC uchar select(uchar a, uchar b, bool c) { return c ? b : a; }
METAL_FUNC uchar2 select(uchar2 a, uchar2 b, bool2 c) { return c ? b : a; }
METAL_FUNC uchar3 select(uchar3 a, uchar3 b, bool3 c) { return c ? b : a; }
METAL_FUNC uchar4 select(uchar4 a, uchar4 b, bool4 c) { return c ? b : a; }
METAL_FUNC uchar8 select(uchar8 a, uchar8 b, bool8 c) { return c ? b : a; }
METAL_FUNC uchar16 select(uchar16 a, uchar16 b, bool16 c) { return c ? b : a; }
METAL_FUNC long select(long a, long b, bool c) { return c ? b : a; }
METAL_FUNC long2 select(long2 a, long2 b, bool2 c) { return c ? b : a; }
METAL_FUNC long3 select(long3 a, long3 b, bool3 c) { return c ? b : a; }
METAL_FUNC long4 select(long4 a, long4 b, bool4 c) { return c ? b : a; }
METAL_FUNC long8 select(long8 a, long8 b, bool8 c) { return c ? b : a; }
METAL_FUNC long16 select(long16 a, long16 b, bool16 c) { return c ? b : a; }
METAL_FUNC ulong select(ulong a, ulong b, bool c) { return c ? b : a; }
METAL_FUNC ulong2 select(ulong2 a, ulong2 b, bool2 c) { return c ? b : a; }
METAL_FUNC ulong3 select(ulong3 a, ulong3 b, bool3 c) { return c ? b : a; }
METAL_FUNC ulong4 select(ulong4 a, ulong4 b, bool4 c) { return c ? b : a; }
METAL_FUNC ulong8 select(ulong8 a, ulong8 b, bool8 c) { return c ? b : a; }
METAL_FUNC ulong16 select(ulong16 a, ulong16 b, bool16 c) { return c ? b : a; }
METAL_FUNC bfloat select(bfloat a, bfloat b, bool c) { return c ? b : a; }
METAL_FUNC bfloat2 select(bfloat2 a, bfloat2 b, bool2 c) { return c ? b : a; }
METAL_FUNC bfloat3 select(bfloat3 a, bfloat3 b, bool3 c) { return c ? b : a; }
METAL_FUNC bfloat4 select(bfloat4 a, bfloat4 b, bool4 c) { return c ? b : a; }
METAL_FUNC bfloat8 select(bfloat8 a, bfloat8 b, bool8 c) { return c ? b : a; }
METAL_FUNC bfloat16 select(bfloat16 a, bfloat16 b, bool16 c) { return c ? b : a; }
METAL_FUNC bool select(bool a, bool b, bool c) { return c ? b : a; }
METAL_FUNC bool2 select(bool2 a, bool2 b, bool2 c) { return c ? b : a; }
METAL_FUNC bool3 select(bool3 a, bool3 b, bool3 c) { return c ? b : a; }
METAL_FUNC bool4 select(bool4 a, bool4 b, bool4 c) { return c ? b : a; }
METAL_FUNC bool8 select(bool8 a, bool8 b, bool8 c) { return c ? b : a; }
METAL_FUNC bool16 select(bool16 a, bool16 b, bool16 c) { return c ? b : a; }


// simd type select overloads
METAL_FUNC float2x2 select(float2x2 a, float2x2 b, bool c) { return c ? b : a; }
METAL_FUNC float3x3 select(float3x3 a, float3x3 b, bool c) { return c ? b : a; }
METAL_FUNC float4x4 select(float4x4 a, float4x4 b, bool c) { return c ? b : a; }
METAL_FUNC half2x2 select(half2x2 a, half2x2 b, bool c) { return c ? b : a; }
METAL_FUNC half3x3 select(half3x3 a, half3x3 b, bool c) { return c ? b : a; }
METAL_FUNC half4x4 select(half4x4 a, half4x4 b, bool c) { return c ? b : a; }
METAL_FUNC double2x2 select(double2x2 a, double2x2 b, bool c) { return c ? b : a; }
METAL_FUNC double3x3 select(double3x3 a, double3x3 b, bool c) { return c ? b : a; }
METAL_FUNC double4x4 select(double4x4 a, double4x4 b, bool c) { return c ? b : a; }
METAL_FUNC bfloat2x2 select(bfloat2x2 a, bfloat2x2 b, bool c) { return c ? b : a; }
METAL_FUNC bfloat3x3 select(bfloat3x3 a, bfloat3x3 b, bool c) { return c ? b : a; }
METAL_FUNC bfloat4x4 select(bfloat4x4 a, bfloat4x4 b, bool c) { return c ? b : a; }

} // namespace metal
#endif