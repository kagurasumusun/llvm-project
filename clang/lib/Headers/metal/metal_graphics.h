#ifndef __METAL_GRAPHICS_H__
#define __METAL_GRAPHICS_H__

#if 1 // __METAL__
namespace metal {

enum class access { read, write, read_write };

template<typename T, access a = access::read>
struct texture2d {
    T read(uint2 coord, uint lod = 0) const;
    void write(T color, uint2 coord, uint lod = 0);
};

struct sampler {
    sampler() {}
};

} // namespace metal
#endif
#endif // __METAL_GRAPHICS_H__
