#ifndef __METAL_TYPES_H__
#define __METAL_TYPES_H__

#ifdef __METAL__

// Address space definitions
#define device __metal_device
#define constant __metal_constant
#define threadgroup __metal_threadgroup
#define thread __metal_thread
#define ray_data __metal_ray_data
#define object_data __metal_object_data
#define threadgroup_imageblock __metal_threadgroup_imageblock

namespace metal {

typedef _Float16 half;
typedef float float32_t;
typedef signed char int8_t;
typedef unsigned char uint8_t;
typedef short int16_t;
typedef unsigned short uint16_t;
typedef int int32_t;
typedef unsigned int uint32_t;
typedef unsigned int uint;
typedef unsigned short ushort;
typedef unsigned char uchar;
typedef unsigned long size_t;
typedef long ptrdiff_t;

// Vector types via Clang's ext_vector_type
typedef float float2 __attribute__((ext_vector_type(2)));
typedef float float3 __attribute__((ext_vector_type(3)));
typedef float float4 __attribute__((ext_vector_type(4)));

typedef half half2 __attribute__((ext_vector_type(2)));
typedef half half3 __attribute__((ext_vector_type(3)));
typedef half half4 __attribute__((ext_vector_type(4)));

typedef int int2 __attribute__((ext_vector_type(2)));
typedef int int3 __attribute__((ext_vector_type(3)));
typedef int int4 __attribute__((ext_vector_type(4)));

typedef uint uint2 __attribute__((ext_vector_type(2)));
typedef uint uint3 __attribute__((ext_vector_type(3)));
typedef uint uint4 __attribute__((ext_vector_type(4)));

typedef short short2 __attribute__((ext_vector_type(2)));
typedef short short3 __attribute__((ext_vector_type(3)));
typedef short short4 __attribute__((ext_vector_type(4)));

typedef ushort ushort2 __attribute__((ext_vector_type(2)));
typedef ushort ushort3 __attribute__((ext_vector_type(3)));
typedef ushort ushort4 __attribute__((ext_vector_type(4)));

typedef bool bool2 __attribute__((ext_vector_type(2)));
typedef bool bool3 __attribute__((ext_vector_type(3)));
typedef bool bool4 __attribute__((ext_vector_type(4)));

// Packed vectors
struct packed_float3 { float x, y, z; };
struct packed_half3 { half x, y, z; };

// Matrix types
template<typename T, int Cols, int Rows>
struct matrix {
    T cols[Cols] __attribute__((ext_vector_type(Rows)));
    matrix() {}
};

typedef matrix<float, 2, 2> float2x2;
typedef matrix<float, 3, 3> float3x3;
typedef matrix<float, 4, 4> float4x4;
typedef matrix<half, 2, 2> half2x2;
typedef matrix<half, 3, 3> half3x3;
typedef matrix<half, 4, 4> half4x4;

// Array template
template<typename T, size_t N>
struct array {
    T elements[N];
    T& operator[](size_t idx) { return elements[idx]; }
    const T& operator[](size_t idx) const { return elements[idx]; }
};

} // namespace metal

// Bring types into global scope or allow using namespace metal
using metal::half;
using metal::uint;
using metal::ushort;
using metal::uchar;
using metal::float2;
using metal::float3;
using metal::float4;
using metal::half2;
using metal::half3;
using metal::half4;
using metal::int2;
using metal::int3;
using metal::int4;
using metal::uint2;
using metal::uint3;
using metal::uint4;
using metal::float4x4;
using metal::half4x4;

#endif // __METAL__
#endif // __METAL_TYPES_H__
