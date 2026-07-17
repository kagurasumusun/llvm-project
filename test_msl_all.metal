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
[[vertex]] ShaderOutput my_vertex_shader(ShaderInput input [[stage_in]],
                                     constant float4x4& mvp_matrix [[buffer(0)]],
                                     uint vertex_id [[thread_position_in_grid]]) {
    ShaderOutput out;
    float4 pos;
    pos.x = input.packed_pos.x;
    pos.y = input.packed_pos.y;
    pos.z = input.packed_pos.z;
    pos.w = 1.0f;
    out.color = pos * 0.5f;
    out.depth = input.position.z;
    return out;
}

// 3. Fragment shader entry point
[[fragment]] float4 my_fragment_shader(ShaderOutput in [[stage_in]],
                                   texture2d<float, access::read> tex [[texture(0)]],
                                   sampler smp [[sampler(0)]]) {
    float3 n;
    n.x = in.color.x;
    n.y = in.color.y;
    n.z = in.color.z;
    float3 norm_n = normalize(n);
    float3 up_dir;
    up_dir.x = 0.0f;
    up_dir.y = 1.0f;
    up_dir.z = 0.0f;
    float d = dot(norm_n, up_dir);
    float4 res;
    res.x = clamp(d, 0.0f, 1.0f);
    res.y = in.color.y;
    res.z = in.color.z;
    res.w = 1.0f;
    return res;
}

// 4. Kernel (Compute) shader entry point with address spaces and synchronization
[[kernel]] void my_compute_kernel(device float4* input_data [[buffer(0)]],
                              device float4* output_data [[buffer(1)]],
                              constant float& scale [[buffer(2)]],
                              threadgroup float4* local_shared [[buffer(3)]],
                              uint3 grid_pos [[thread_position_in_grid]],
                              uint3 tid_in_tg [[thread_position_in_threadgroup]],
                              uint3 tg_size [[threads_per_threadgroup]]) {
    uint linear_tid = tid_in_tg.x + tid_in_tg.y * tg_size.x;
    local_shared[linear_tid] = input_data[grid_pos.x] * scale;
    threadgroup_barrier(mem_flags::mem_threadgroup);
    float4 offset;
    offset.x = 1.0f;
    offset.y = 2.0f;
    offset.z = 3.0f;
    offset.w = 4.0f;
    output_data[grid_pos.x] = local_shared[linear_tid] + offset;
}

// 5. Advanced Metal address spaces and features (MSL 1.2 - 4.1)
struct AdvancedData {
    device float* dev_ptr;
    constant float* const_ptr;
    threadgroup float* tg_ptr;
    thread float* priv_ptr;
    ray_data float* ray_ptr;
    object_data float* obj_ptr;
    threadgroup_imageblock float* imgblock_ptr;
};

[[kernel]] void my_advanced_kernel(device AdvancedData* data [[buffer(0)]],
                               uint id [[thread_position_in_grid]]) {
    float val = data[id].dev_ptr[0] + data[id].const_ptr[0];
    data[id].dev_ptr[0] = min(max(val, 0.0f), 100.0f);
}
