#include <metal_stdlib>
using namespace metal;

// 1. Vector, packed vector, matrix, and basic types
struct ShaderInput {
    float4 position [[position]];
    float3 normal [[attribute(0)]];
    half2 texcoord [[attribute(1)]];
    packed_float3 packed_pos;
};

struct ShaderOutput {
    float4 color [[color(0)]];
    float depth [[user("depth_out")]];
};

// 2. Vertex shader entry point
vertex ShaderOutput my_vertex_shader(ShaderInput input [[stage_in]],
                                     constant float4x4& mvp_matrix [[buffer(0)]],
                                     uint vertex_id [[thread_position_in_grid]]) {
    ShaderOutput out;
    float4 pos = float4(input.packed_pos.x, input.packed_pos.y, input.packed_pos.z, 1.0f);
    out.color = pos * 0.5f;
    out.depth = input.position.z;
    return out;
}

// 3. Fragment shader entry point
fragment float4 my_fragment_shader(ShaderOutput in [[stage_in]],
                                   texture2d<float, access::read> tex [[texture(0)]],
                                   sampler smp [[sampler(0)]]) {
    float3 n = normalize(float3(in.color.x, in.color.y, in.color.z));
    float d = dot(n, float3(0.0f, 1.0f, 0.0f));
    return float4(clamp(d, 0.0f, 1.0f), in.color.y, in.color.z, 1.0f);
}

// 4. Kernel (Compute) shader entry point with address spaces and synchronization
kernel void my_compute_kernel(device float4* input_data [[buffer(0)]],
                              device float4* output_data [[buffer(1)]],
                              constant float& scale [[buffer(2)]],
                              threadgroup float4* local_shared [[buffer(3)]],
                              uint3 grid_pos [[thread_position_in_grid]],
                              uint3 tid_in_tg [[thread_position_in_threadgroup]],
                              uint3 tg_size [[threads_per_threadgroup]]) {
    uint linear_tid = tid_in_tg.x + tid_in_tg.y * tg_size.x;
    local_shared[linear_tid] = input_data[grid_pos.x] * scale;
    threadgroup_barrier(mem_flags::mem_threadgroup);
    output_data[grid_pos.x] = local_shared[linear_tid] + float4(1.0f, 2.0f, 3.0f, 4.0f);
}

// 5. Advanced Metal address spaces and features (MSL 2.0 - 4.1)
struct AdvancedData {
    device float* dev_ptr;
    constant float* const_ptr;
    threadgroup float* tg_ptr;
    thread float* priv_ptr;
    ray_data float* ray_ptr;
    object_data float* obj_ptr;
    threadgroup_imageblock float* imgblock_ptr;
};

kernel void my_advanced_kernel(device AdvancedData* data [[buffer(0)]],
                               uint id [[thread_position_in_grid]]) {
    float val = data[id].dev_ptr[0] + data[id].const_ptr[0];
    data[id].dev_ptr[0] = min(max(val, 0.0f), 100.0f);
}
