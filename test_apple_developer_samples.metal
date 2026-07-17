// ==============================================================================
// Apple Developer Website Official Metal Sample Shaders
// Based on official Apple Developer documentation & sample code distributions
// ==============================================================================

#include <metal_stdlib>
using namespace metal;
using namespace metal::raytracing;

constant bool enable_lighting [[function_constant(0)]];
constant int blur_radius [[function_constant(1)]];

struct BasicVertexOutput {
    float4 position [[position]];
    float2 texcoord;
    float3 normal;
};

[[fragment]] float4 apple_dev_function_constant_fragment(BasicVertexOutput in [[stage_in]],
                                                         texture2d<float> color_tex [[texture(0)]],
                                                         sampler s [[sampler(0)]]) {
    float4 color = color_tex.sample(s, in.texcoord);
    if (enable_lighting) {
        float diffuse = max(dot(normalize(in.normal), make_float3(0.0, 1.0, 0.0)), 0.2f);
        color = color * diffuse;
    }
    return color;
}

struct GBufferData {
    float4 albedo [[color(0), raster_order_group(0)]];
    float4 normals [[color(1), raster_order_group(0)]];
};

[[early_fragment_tests]]
[[fragment]] GBufferData apple_dev_gbuffer_fragment(BasicVertexOutput in [[stage_in]],
                                                    texture2d<float> albedo_tex [[texture(0)]],
                                                    sampler s [[sampler(0)]]) {
    GBufferData out;
    out.albedo = albedo_tex.sample(s, in.texcoord);
    out.normals = make_float4(normalize(in.normal) * 0.5f, 1.0f);
    return out;
}

struct TileColorPayload {
    float4 color;
};

[[kernel]] void apple_dev_tile_shader(imageblock<TileColorPayload, imageblock_layout_explicit> img_block [[imageblock_data]],
                                      ushort2 tid [[thread_position_in_threadgroup]]) {
    threadgroup_imageblock TileColorPayload* ptr = img_block.data(tid);
    ptr->color = ptr->color * 0.5f;
}

struct PayloadData {
    float3 center;
    float radius;
    uint lod;
};

struct MeshVertex {
    float4 position [[position]];
    float3 normal;
    float2 texcoord;
};

struct MeshPrimitive {
    float4 color;
};

[[object]] void apple_dev_object_kernel(object_data PayloadData& payload [[payload]],
                                        device const PayloadData* input_data [[buffer(0)]],
                                        uint gid [[thread_position_in_grid]]) {
    payload.center = input_data[gid].center;
    payload.radius = input_data[gid].radius;
    payload.lod = input_data[gid].lod;
}

[[mesh]] void apple_dev_mesh_kernel(object_data const PayloadData& payload [[payload]],
                                    mesh<MeshVertex, MeshPrimitive, 64, 126, topology::triangle> output_mesh,
                                    uint lid [[thread_position_in_threadgroup]],
                                    uint tid [[thread_index_in_threadgroup]]) {
    if (tid == 0) {
        output_mesh.set_primitive_count(126);
    }
    if (lid < 64) {
        MeshVertex v;
        v.position = make_float4(make_float3(lid * 0.1f, 0.0f, 0.0f), 1.0f);
        v.normal = make_float3(0.0f, 1.0f, 0.0f);
        v.texcoord = make_float2(0.0f, 0.0f);
        output_mesh.set_vertex(lid, v);
    }
}

struct RayPayload {
    float4 hit_color;
    float distance;
};

[[kernel]] void apple_dev_raytracing_kernel(intersection_query<instancing, triangle_data> query,
                                            device RayPayload* payloads [[buffer(0)]],
                                            uint gid [[thread_position_in_grid]]) {
    query.reset(ray(make_float3(0.0f, 0.0f, 0.0f), make_float3(0.0f, 0.0f, -1.0f)), 0.001f, 1000.0f);
    query.next();
    if (query.get_committed_intersection_type() != intersection_type::none) {
        payloads[gid].hit_color = make_float4(1.0f, 0.0f, 0.0f, 1.0f);
        payloads[gid].distance = query.get_committed_distance();
    } else {
        payloads[gid].hit_color = make_float4(0.0f, 0.0f, 0.0f, 1.0f);
        payloads[gid].distance = -1.0f;
    }
}
