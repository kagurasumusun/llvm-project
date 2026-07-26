#ifndef _METAL_PACKED_VECTOR_H_
#define _METAL_PACKED_VECTOR_H_
#include <metal/metal_common>
namespace metal {
struct packed_float2 { float x, y; };
struct packed_float3 { float x, y, z; };
struct packed_float4 { float x, y, z, w; };
struct packed_half2  { half x, y; };
struct packed_half3  { half x, y, z; };
struct packed_half4  { half x, y, z, w; };
struct packed_char2  { char x, y; };
struct packed_char3  { char x, y, z; };
struct packed_uchar2 { uchar x, y; };
struct packed_uchar3 { uchar x, y, z; };
struct packed_short2 { short x, y; };
struct packed_short3 { short x, y, z; };
struct packed_ushort2 { ushort x, y; };
struct packed_ushort3 { ushort x, y, z; };
struct packed_int2   { int x, y; };
struct packed_int3   { int x, y, z; };
struct packed_uint2  { uint x, y; };
struct packed_uint3  { uint x, y, z; };
} // namespace metal
#endif
