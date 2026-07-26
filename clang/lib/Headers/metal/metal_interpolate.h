// metal_interpolate — MSL interpolation (auto-generated)
#ifndef _METAL_INTERPOLATE_H_
#define _METAL_INTERPOLATE_H_
#include <metal/metal_common>
namespace metal {

template <typename T>
METAL_FUNC T interpolate_center_perspective(T i) { return i; }
template <typename T>
METAL_FUNC T interpolate_center_no_perspective(T i) { return i; }
template <typename T>
METAL_FUNC T interpolate_centroid_perspective(T i) { return i; }
template <typename T>
METAL_FUNC T interpolate_centroid_no_perspective(T i) { return i; }
template <typename T>
METAL_FUNC T interpolate_offset_perspective(T i, float2 offset) { return i; }
template <typename T>
METAL_FUNC T interpolate_offset_no_perspective(T i, float2 offset) { return i; }
template <typename T>
METAL_FUNC T interpolate_sample_perspective(T i, uint s) { return i; }
template <typename T>
METAL_FUNC T interpolate_sample_no_perspective(T i, uint s) { return i; }

} // namespace metal
#endif