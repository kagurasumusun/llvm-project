#ifndef __METAL_GRAPHICS_H__
#define __METAL_GRAPHICS_H__

#if 1 // defined(__METAL__) || defined(__metal__)
namespace metal {

enum class access { read, write, read_write };

struct sampler {
    sampler() {}
};

// Imageblock classes (MSL 2.0+)
template<typename T>
struct imageblock_layout {
    typedef T type;
    imageblock_layout() {}
};

template<typename T>
struct imageblock_slice {
    T read(ushort2 coord) const;
    void write(T color, ushort2 coord);
};

template<typename T, access a = access::read>
struct imageblock_data {
    T read(ushort2 coord) const;
    void write(T color, ushort2 coord);
};

template<typename T, access a = access::read>
struct texture1d {
    T read(uint coord) const;
    void write(T color, uint coord);
    T sample(sampler smp, float coord) const;
    uint get_width(uint lod = 0) const;
};

template<typename T, access a = access::read>
struct texture1d_array {
    T read(uint2 coord) const;
    void write(T color, uint2 coord);
    T sample(sampler smp, float2 coord) const;
};

template<typename T, access a = access::read>
struct texture2d {
    T read(uint2 coord, uint lod = 0) const;
    void write(T color, uint2 coord, uint lod = 0);
    T sample(sampler smp, float2 coord) const;
    T sample(sampler smp, float2 coord, float lod) const;
    T gather(sampler smp, float2 coord) const;
    uint get_width(uint lod = 0) const;
    uint get_height(uint lod = 0) const;
    uint get_num_mip_levels() const;
};

template<typename T, access a = access::read>
struct texture2d_array {
    T read(uint3 coord, uint lod = 0) const;
    void write(T color, uint3 coord, uint lod = 0);
    T sample(sampler smp, float3 coord) const;
};

template<typename T, access a = access::read>
struct texture2d_ms {
    T read(uint2 coord, uint sample) const;
    uint get_num_samples() const;
};

template<typename T, access a = access::read>
struct texture3d {
    T read(uint3 coord, uint lod = 0) const;
    void write(T color, uint3 coord, uint lod = 0);
    T sample(sampler smp, float3 coord) const;
};

template<typename T, access a = access::read>
struct texturecube {
    T read(float3 coord, uint lod = 0) const;
    T sample(sampler smp, float3 coord) const;
};

template<typename T, access a = access::read>
struct texturecube_array {
    T read(float4 coord, uint lod = 0) const;
    T sample(sampler smp, float4 coord) const;
};

template<typename T = float, access a = access::read>
struct depth2d {
    T read(uint2 coord, uint lod = 0) const;
    T sample_compare(sampler smp, float2 coord, float compare_value) const;
};

template<typename T = float, access a = access::read>
struct depth2d_array {
    T read(uint3 coord, uint lod = 0) const;
    T sample_compare(sampler smp, float3 coord, float compare_value) const;
};

template<typename T = float, access a = access::read>
struct depthcube {
    T read(float3 coord, uint lod = 0) const;
    T sample_compare(sampler smp, float3 coord, float compare_value) const;
};

template<typename T = float, access a = access::read>
struct depth2d_ms {
    T read(uint2 coord, uint sample) const;
};

} // namespace metal
#endif
#endif // __METAL_GRAPHICS_H__
