#include <metal_stdlib>
using namespace metal;

// 1. Tile Shaders & Imageblock Operations (MSL 2.0+)
struct TilePixel {
    float4 color;
    float depth;
};

[[kernel]] void test_tile_and_imageblock(imageblock_data<TilePixel> img_data [[imageblock_data]],
                                         ushort2 tid [[thread_position_in_threadgroup]]) {
    TilePixel p;
    float4 c; c.x = 1.0f; c.y = 0.8f; c.z = 0.3f; c.w = 1.0f;
    p.color = c;
    p.depth = 0.5f;
    img_data.write(p, tid);
}

// 2. SIMD Wavefront Shuffle, Reductions, Voting & Quad Group Communication (MSL 2.0+)
[[kernel]] void test_simd_and_quad(device float* out_buf [[buffer(0)]],
                                   uint id [[thread_position_in_grid]],
                                   ushort lane [[thread_index_in_simdgroup]]) {
    float val = out_buf[id];
    float bcast = simd_broadcast(val, 0u);
    float shuf = simd_shuffle_down(val, 1u);
    float sum = simd_sum(val);
    float qshuf = quad_shuffle(val, 2u);
    unsigned long mask = simd_ballot(val > 0.0f);
    if (mask != 0UL) {
        out_buf[id] = bcast + shuf + sum + qshuf;
    }
}

// 3. Tessellation API and Patch Control Points (MSL 1.2+)
struct PatchCP {
    float3 pos;
    float2 uv;
};

[[kernel]] void test_tessellation(device patch_control_point<PatchCP>* cps [[buffer(0)]],
                                  device triangle_tessellation_factors_half* factors [[buffer(1)]],
                                  uint id [[thread_position_in_grid]]) {
    factors[id].edgeTessellationFactor[0] = 4.0f;
    factors[id].edgeTessellationFactor[1] = 4.0f;
    factors[id].edgeTessellationFactor[2] = 4.0f;
    factors[id].insideTessellationFactor = 4.0f;
    float3 p; p.x = 0.0f; p.y = 1.0f; p.z = 0.0f;
    (*cps).data.pos = p;
}

// 4. Advanced Texture & Sampler Operations (MSL 1.2+)
[[fragment]] float4 test_advanced_sampling(texture2d<float, access::read> tex2 [[texture(0)]],
                                           depth2d<float, access::read> dep [[texture(1)]],
                                           texturecube<float, access::read> cube [[texture(2)]],
                                           sampler smp [[sampler(0)]],
                                           float2 uv [[stage_in]]) {
    float4 c1 = tex2.sample(smp, uv);
    float4 c2 = tex2.gather(smp, uv);
    float d = dep.sample_compare(smp, uv, 0.5f);
    float3 dir; dir.x = uv.x; dir.y = uv.y; dir.z = 1.0f;
    float4 c3 = cube.sample(smp, dir);
    uint w = tex2.get_width(0);
    uint mips = tex2.get_num_mip_levels();
    float4 res;
    res.x = c1.x + c2.x + d;
    res.y = c3.y + float(w);
    res.z = float(mips);
    res.w = 1.0f;
    return res;
}
