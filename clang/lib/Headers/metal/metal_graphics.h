// metal_graphics — MSL graphics functions (auto-generated, full)
#ifndef _METAL_GRAPHICS_H_
#define _METAL_GRAPHICS_H_
#include <metal/metal_common>
namespace metal {

METAL_FUNC float dfdx(float x) { return __metal_dfdx(x); }
METAL_FUNC float2 dfdx(float2 x) { return __metal_dfdx(x); }
METAL_FUNC float3 dfdx(float3 x) { return __metal_dfdx(x); }
METAL_FUNC float4 dfdx(float4 x) { return __metal_dfdx(x); }
METAL_FUNC half dfdx(half x) { return __metal_dfdx(x); }
METAL_FUNC half2 dfdx(half2 x) { return __metal_dfdx(x); }
METAL_FUNC half3 dfdx(half3 x) { return __metal_dfdx(x); }
METAL_FUNC half4 dfdx(half4 x) { return __metal_dfdx(x); }
METAL_FUNC float dfdx(float x) { return __metal_dfdx(x); }
METAL_FUNC float2 dfdx(float2 x) { return __metal_dfdx(x); }
METAL_FUNC float3 dfdx(float3 x) { return __metal_dfdx(x); }
METAL_FUNC float4 dfdx(float4 x) { return __metal_dfdx(x); }
METAL_FUNC half dfdx(half x) { return __metal_dfdx(x); }
METAL_FUNC half2 dfdx(half2 x) { return __metal_dfdx(x); }
METAL_FUNC half3 dfdx(half3 x) { return __metal_dfdx(x); }
METAL_FUNC half4 dfdx(half4 x) { return __metal_dfdx(x); }
METAL_FUNC float dfdy(float x) { return __metal_dfdy(x); }
METAL_FUNC float2 dfdy(float2 x) { return __metal_dfdy(x); }
METAL_FUNC float3 dfdy(float3 x) { return __metal_dfdy(x); }
METAL_FUNC float4 dfdy(float4 x) { return __metal_dfdy(x); }
METAL_FUNC half dfdy(half x) { return __metal_dfdy(x); }
METAL_FUNC half2 dfdy(half2 x) { return __metal_dfdy(x); }
METAL_FUNC half3 dfdy(half3 x) { return __metal_dfdy(x); }
METAL_FUNC half4 dfdy(half4 x) { return __metal_dfdy(x); }
METAL_FUNC float dfdy(float x) { return __metal_dfdy(x); }
METAL_FUNC float2 dfdy(float2 x) { return __metal_dfdy(x); }
METAL_FUNC float3 dfdy(float3 x) { return __metal_dfdy(x); }
METAL_FUNC float4 dfdy(float4 x) { return __metal_dfdy(x); }
METAL_FUNC half dfdy(half x) { return __metal_dfdy(x); }
METAL_FUNC half2 dfdy(half2 x) { return __metal_dfdy(x); }
METAL_FUNC half3 dfdy(half3 x) { return __metal_dfdy(x); }
METAL_FUNC half4 dfdy(half4 x) { return __metal_dfdy(x); }
METAL_FUNC float fwidth(float x) { return __metal_fwidth(x); }
METAL_FUNC float2 fwidth(float2 x) { return __metal_fwidth(x); }
METAL_FUNC float3 fwidth(float3 x) { return __metal_fwidth(x); }
METAL_FUNC float4 fwidth(float4 x) { return __metal_fwidth(x); }
METAL_FUNC half fwidth(half x) { return __metal_fwidth(x); }
METAL_FUNC half2 fwidth(half2 x) { return __metal_fwidth(x); }
METAL_FUNC half3 fwidth(half3 x) { return __metal_fwidth(x); }
METAL_FUNC half4 fwidth(half4 x) { return __metal_fwidth(x); }
METAL_FUNC float fwidth(float x) { return __metal_fwidth(x); }
METAL_FUNC float2 fwidth(float2 x) { return __metal_fwidth(x); }
METAL_FUNC float3 fwidth(float3 x) { return __metal_fwidth(x); }
METAL_FUNC float4 fwidth(float4 x) { return __metal_fwidth(x); }
METAL_FUNC half fwidth(half x) { return __metal_fwidth(x); }
METAL_FUNC half2 fwidth(half2 x) { return __metal_fwidth(x); }
METAL_FUNC half3 fwidth(half3 x) { return __metal_fwidth(x); }
METAL_FUNC half4 fwidth(half4 x) { return __metal_fwidth(x); }

METAL_FUNC float2 map_physical_to_screen_coordinates(float2 coord, device void *map) { return coord; }
METAL_FUNC float2 map_screen_to_physical_coordinates(float2 coord, device void *map) { return coord; }
METAL_FUNC uint get_num_samples() { return 1; }
METAL_FUNC float2 get_sample_position(uint s) { return float2(0); }
METAL_FUNC void discard_fragment() {}

struct rasterization_rate_map_decoder_t {
  METAL_FUNC float2 map_screen_to_physical_coordinates(float2 coord) const { return coord; }
  METAL_FUNC float2 map_physical_to_screen_coordinates(float2 coord) const { return coord; }
};

} // namespace metal
#endif