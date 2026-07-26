//===----------------------------------------------------------------------===//
// metal_texture — MSL §7.7 texture types
// Opaque types matching Apple's _texture_*_t = type opaque in addrspace(1)
//===----------------------------------------------------------------------===//

#ifndef _METAL_TEXTURE_H_
#define _METAL_TEXTURE_H_

#include <metal/metal_common>

// ---- Texture opaque types (match Apple's bitcode layout) ----
struct _texture_1d_t;
struct _texture_1d_array_t;
struct _texture_2d_t;
struct _texture_2d_array_t;
struct _texture_2d_ms_t;
struct _texture_2d_ms_array_t;
struct _texture_3d_t;
struct _texture_cube_t;
struct _texture_cube_array_t;
struct _texture_buffer_1d_t;

// ---- Depth texture opaque types ----
struct _depth_2d_t;
struct _depth_2d_array_t;
struct _depth_2d_ms_t;
struct _depth_2d_ms_array_t;
struct _depth_cube_t;
struct _depth_cube_array_t;

// ---- Sampler opaque type ----
struct _sampler_t;

namespace metal {

// Texture access modes
enum access { sample = 0, read = 1, write = 2, read_write = 3 };

// ---- Texture read functions (§7.7.2) ----
// These call __metal_read_texture_2d_t etc. which lower to air.read_texture_2d.v4f32

template <typename T>
struct texture2d {
  typedef device _texture_2d_t *handle_type;

  METAL_ALWAYS_INLINE T read(uint2 coord, uint lod = 0) const;
  METAL_ALWAYS_INLINE T read(uint2 coord, uint2 offset, uint lod = 0) const;
  METAL_ALWAYS_INLINE T sample(float2 coord) const;
  METAL_ALWAYS_INLINE T sample(float2 coord, float2 offset) const;
  METAL_ALWAYS_INLINE T sample(float2 coord, float2 offset, float lod) const;
  METAL_ALWAYS_INLINE uint get_width() const;
  METAL_ALWAYS_INLINE uint get_height() const;
  METAL_ALWAYS_INLINE uint get_num_mip_levels() const;
};

template <typename T>
struct texture2d_array {
  METAL_ALWAYS_INLINE T read(uint2 coord, uint array_index, uint lod = 0) const;
  METAL_ALWAYS_INLINE T sample(float2 coord, uint array_index) const;
  METAL_ALWAYS_INLINE uint get_width() const;
  METAL_ALWAYS_INLINE uint get_height() const;
  METAL_ALWAYS_INLINE uint get_array_size() const;
};

template <typename T>
struct texture3d {
  METAL_ALWAYS_INLINE T read(uint3 coord, uint lod = 0) const;
  METAL_ALWAYS_INLINE T sample(float3 coord) const;
  METAL_ALWAYS_INLINE uint get_width() const;
  METAL_ALWAYS_INLINE uint get_height() const;
  METAL_ALWAYS_INLINE uint get_depth() const;
};

template <typename T>
struct texturecube {
  METAL_ALWAYS_INLINE T sample(float3 coord) const;
  METAL_ALWAYS_INLINE uint get_width() const;
  METAL_ALWAYS_INLINE uint get_height() const;
};

template <typename T>
struct depth2d {
  METAL_ALWAYS_INLINE float read(uint2 coord, uint lod = 0) const;
  METAL_ALWAYS_INLINE float sample(float2 coord) const;
  METAL_ALWAYS_INLINE float sample_compare(float2 coord, float compare_value) const;
  METAL_ALWAYS_INLINE uint get_width() const;
  METAL_ALWAYS_INLINE uint get_height() const;
};

// Convenience typedefs
typedef texture2d<float>      texture2d_f;
typedef texture2d<half>       texture2d_h;
typedef texture2d<int>        texture2d_i;
typedef texture2d<uint>       texture2d_u;
typedef texture3d<float>      texture3d_f;
typedef texturecube<float>    texturecube_f;
typedef depth2d<float>        depth2d_f;
typedef depth2d<half>         depth2d_h;

} // namespace metal

#endif // _METAL_TEXTURE_H_
