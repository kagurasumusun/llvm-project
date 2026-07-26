// metal_imageblocks — MSL imageblock operations (cleanroom)
#ifndef _METAL_IMAGEBLOCKS_H_
#define _METAL_IMAGEBLOCKS_H_
#include <metal/metal_common>
namespace metal {

struct _imageblock_t {};
template <typename T>
struct imageblock {
  METAL_FUNC T get_color(uint x, uint y) const { return T(); }
  METAL_FUNC void set_color(uint x, uint y, T val) {}
  METAL_FUNC uint get_width() const { return 0; }
  METAL_FUNC uint get_height() const { return 0; }
  METAL_FUNC uint get_num_colors() const { return 0; }
  METAL_FUNC uint get_samples() const { return 1; }
};
METAL_FUNC uint get_imageblock_width() { return 0; }
METAL_FUNC uint get_imageblock_height() { return 0; }
METAL_FUNC uint get_imageblock_num_colors() { return 0; }
METAL_FUNC uint get_imageblock_samples() { return 1; }
METAL_FUNC uint get_imageblock_color_coverage_mask() { return 0; }
template <typename T> METAL_FUNC T imageblock_load_implicit(uint2 coord) { return T(); }
template <typename T> METAL_FUNC void imageblock_store_implicit(uint2 coord, T val) {}
template <typename T> METAL_FUNC T imageblock_load_explicit(uint2 coord) { return T(); }
template <typename T> METAL_FUNC void imageblock_store_explicit(uint2 coord, T val) {}
template <typename T> METAL_FUNC void imageblock_write(T val, uint2 coord) {}
template <typename T> METAL_FUNC T imageblock_read(uint2 coord) { return T(); }

METAL_FUNC uint get_num_samples(uint2 coord) { return 1; }
METAL_FUNC uint get_color_coverage_mask() { return 0xFFFFFFFF; }
template <typename T> METAL_FUNC T read(device _imageblock_t *ib, uint2 coord, uint sample = 0) { return T(); }
template <typename T> METAL_FUNC T read(threadgroup _imageblock_t *ib, uint2 coord, uint sample = 0) { return T(); }
template <typename T> METAL_FUNC void write(device _imageblock_t *ib, uint2 coord, T val) {}
template <typename T> METAL_FUNC void write(threadgroup _imageblock_t *ib, uint2 coord, T val) {}
template <typename T> METAL_FUNC void write(device _imageblock_t *ib, uint2 coord, uint sample, T val) {}
template <typename T> METAL_FUNC void write(threadgroup _imageblock_t *ib, uint2 coord, uint sample, T val) {}

} // namespace metal
#endif