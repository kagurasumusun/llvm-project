// metal_pack — MSL pack/unpack (cleanroom)
#ifndef _METAL_PACK_H_
#define _METAL_PACK_H_
#include <metal/metal_common>
namespace metal {

METAL_FUNC uint pack_float_to_snorm4x8(float4 v) { return __air_pack_float_to_snorm4x8(v); }
METAL_FUNC uint pack_float_to_unorm4x8(float4 v) { return __air_pack_float_to_unorm4x8(v); }
METAL_FUNC uint pack_float_to_snorm2x16(float2 v) { return __air_pack_float_to_snorm2x16(v); }
METAL_FUNC uint pack_float_to_unorm2x16(float2 v) { return __air_pack_float_to_unorm2x16(v); }
METAL_FUNC uint pack_float_to_snorm10a2(float4 v) { return __air_pack_float_to_snorm10a2(v); }
METAL_FUNC uint pack_float_to_unorm10a2(float4 v) { return __air_pack_float_to_unorm10a2(v); }
METAL_FUNC uint pack_float_to_unorm565(float3 v) { return __air_pack_float_to_unorm565(v); }
METAL_FUNC uint pack_float_to_srgb_unorm4x8(float4 v) { return __air_pack_float_to_srgb_unorm4x8(v); }
METAL_FUNC uint pack_half_to_snorm4x8(half4 v) { return __air_pack_half_to_snorm4x8(v); }
METAL_FUNC uint pack_half_to_unorm4x8(half4 v) { return __air_pack_half_to_unorm4x8(v); }
METAL_FUNC uint pack_half_to_snorm2x16(half2 v) { return __air_pack_half_to_snorm2x16(v); }
METAL_FUNC uint pack_half_to_unorm2x16(half2 v) { return __air_pack_half_to_unorm2x16(v); }
METAL_FUNC uint pack_half_to_snorm10a2(half4 v) { return __air_pack_half_to_snorm10a2(v); }
METAL_FUNC uint pack_half_to_unorm10a2(half4 v) { return __air_pack_half_to_unorm10a2(v); }
METAL_FUNC uint pack_half_to_unorm565(half3 v) { return __air_pack_half_to_unorm565(v); }
METAL_FUNC uint pack_half_to_srgb_unorm4x8(half4 v) { return __air_pack_half_to_srgb_unorm4x8(v); }

METAL_FUNC float4 unpack_snorm4x8_to_float(uint v) { return __air_unpack_snorm4x8_to_float(v); }
METAL_FUNC float4 unpack_unorm4x8_to_float(uint v) { return __air_unpack_unorm4x8_to_float(v); }
METAL_FUNC float2 unpack_snorm2x16_to_float(uint v) { return __air_unpack_snorm2x16_to_float(v); }
METAL_FUNC float2 unpack_unorm2x16_to_float(uint v) { return __air_unpack_unorm2x16_to_float(v); }
METAL_FUNC float4 unpack_snorm10a2_to_float(uint v) { return __air_unpack_snorm10a2_to_float(v); }
METAL_FUNC float4 unpack_unorm10a2_to_float(uint v) { return __air_unpack_unorm10a2_to_float(v); }
METAL_FUNC float3 unpack_unorm565_to_float(uint v) { return __air_unpack_unorm565_to_float(v); }
METAL_FUNC float4 unpack_unorm4x8_srgb_to_float(uint v) { return __air_unpack_unorm4x8_srgb_to_float(v); }
METAL_FUNC half4 unpack_snorm4x8_to_half(uint v) { return __air_unpack_snorm4x8_to_half(v); }
METAL_FUNC half4 unpack_unorm4x8_to_half(uint v) { return __air_unpack_unorm4x8_to_half(v); }
METAL_FUNC half2 unpack_snorm2x16_to_half(uint v) { return __air_unpack_snorm2x16_to_half(v); }
METAL_FUNC half2 unpack_unorm2x16_to_half(uint v) { return __air_unpack_unorm2x16_to_half(v); }
METAL_FUNC half4 unpack_snorm10a2_to_half(uint v) { return __air_unpack_snorm10a2_to_half(v); }
METAL_FUNC half4 unpack_unorm10a2_to_half(uint v) { return __air_unpack_unorm10a2_to_half(v); }
METAL_FUNC half3 unpack_unorm565_to_half(uint v) { return __air_unpack_unorm565_to_half(v); }
METAL_FUNC half4 unpack_unorm4x8_srgb_to_half(uint v) { return __air_unpack_unorm4x8_srgb_to_half(v); }

} // namespace metal
#endif