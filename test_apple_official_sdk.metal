#include <metal_stdlib>
using namespace metal;

struct VertexInput {
    float4 position [[attribute(0)]];
    float3 normal [[attribute(1)]];
    float2 texcoord [[attribute(2)]];
};

struct VertexOutput {
    float4 position [[position]];
    float3 world_normal;
    float2 texcoord;
};

[[kernel]] void test_official_compute(device float4* input_buf [[buffer(0)]],
                                      device float4* output_buf [[buffer(1)]],
                                      constant float& multiplier [[buffer(2)]],
                                      uint id [[thread_position_in_grid]]) {
    output_buf[id] = input_buf[id] * multiplier;
}

[[vertex]] VertexOutput test_official_vertex(VertexInput in [[stage_in]],
                                             constant float4x4& mvp [[buffer(0)]],
                                             uint vid [[vertex_id]]) {
    VertexOutput out;
    out.position = mvp * in.position;
    out.world_normal = in.normal;
    out.texcoord = in.texcoord;
    return out;
}

[[fragment]] float4 test_official_fragment(VertexOutput in [[stage_in]],
                                           texture2d<float, access::sample> albedo [[texture(0)]],
                                           sampler smp [[sampler(0)]]) {
    float4 tex_color = albedo.sample(smp, in.texcoord);
    float diffuse = max(dot(normalize(in.world_normal), float3(0.0f, 1.0f, 0.0f)), 0.1f);
    return float4(tex_color.xyz * diffuse, tex_color.w);
}
