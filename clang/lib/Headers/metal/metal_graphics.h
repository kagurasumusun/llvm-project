#ifndef __METAL_GRAPHICS_H__
#define __METAL_GRAPHICS_H__

#if 1 // defined(__METAL__) || defined(__metal__)
namespace metal {

enum class access { read, write, read_write };

template<typename T, access a = access::read>
struct texture1d {
    T read(uint coord) const;
    void write(T color, uint coord);
};

template<typename T, access a = access::read>
struct texture1d_array {
    T read(uint2 coord) const;
    void write(T color, uint2 coord);
};

template<typename T, access a = access::read>
struct texture2d {
    T read(uint2 coord, uint lod = 0) const;
    void write(T color, uint2 coord, uint lod = 0);
};

template<typename T, access a = access::read>
struct texture2d_array {
    T read(uint3 coord, uint lod = 0) const;
    void write(T color, uint3 coord, uint lod = 0);
};

template<typename T, access a = access::read>
struct texture2d_ms {
    T read(uint2 coord, uint sample) const;
};

template<typename T, access a = access::read>
struct texture3d {
    T read(uint3 coord, uint lod = 0) const;
    void write(T color, uint3 coord, uint lod = 0);
};

template<typename T, access a = access::read>
struct texturecube {
    T read(float3 coord, uint lod = 0) const;
};

template<typename T, access a = access::read>
struct texturecube_array {
    T read(float4 coord, uint lod = 0) const;
};

template<typename T = float, access a = access::read>
struct depth2d {
    T read(uint2 coord, uint lod = 0) const;
};

template<typename T = float, access a = access::read>
struct depth2d_array {
    T read(uint3 coord, uint lod = 0) const;
};

template<typename T = float, access a = access::read>
struct depthcube {
    T read(float3 coord, uint lod = 0) const;
};

template<typename T = float, access a = access::read>
struct depth2d_ms {
    T read(uint2 coord, uint sample) const;
};

struct sampler {
    sampler() {}
};

} // namespace metal
#endif
#endif // __METAL_GRAPHICS_H__
