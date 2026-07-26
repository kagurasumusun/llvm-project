#ifndef _METAL_VERTEX_VALUE_H_
#define _METAL_VERTEX_VALUE_H_
#include <metal/metal_common>
namespace metal {
struct vertex_value_t {
  float4 position;
  float4 color;
  float2 texcoord;
  float3 normal;
};
} // namespace metal
#endif
