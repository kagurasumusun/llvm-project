#include <metal_stdlib>
using namespace metal;

// 1. Argument Buffers and Comprehensive Resource Binding [[id(Index)]]
struct AllResources {
    texture1d<float> tex1 [[id(0)]];
    texture1d_array<float> tex1_arr [[id(1)]];
    texture2d<float> tex2 [[id(2)]];
    texture2d_array<float> tex2_arr [[id(3)]];
    texture2d_ms<float> tex2_ms [[id(4)]];
    texture3d<float> tex3 [[id(5)]];
    texturecube<float> texcube [[id(6)]];
    texturecube_array<float> texcube_arr [[id(7)]];
    depth2d<float> dep2 [[id(8)]];
    depth2d_array<float> dep2_arr [[id(9)]];
    depthcube<float> depcube [[id(10)]];
    depth2d_ms<float> dep2_ms [[id(11)]];
    sampler smp [[id(12)]];
    device float4* data_buffer [[id(13)]];
};

[[kernel]] void test_argument_buffer(device AllResources& res [[buffer(0)]],
                                     uint tid [[thread_position_in_grid]]) {
    float4 val;
    val.x = 1.0f; val.y = 2.0f; val.z = 3.0f; val.w = 4.0f;
    res.data_buffer[tid] = val;
}

// 2. Ray Tracing API (MSL 2.3+)
[[kernel]] void test_raytracing(device acceleration_structure* as [[buffer(0)]],
                                device float4* hits [[buffer(1)]],
                                uint tid [[thread_position_in_grid]]) {
    float3 o; o.x = 0.0f; o.y = 0.0f; o.z = 0.0f;
    float3 d; d.x = 0.0f; d.y = 0.0f; d.z = 1.0f;
    ray r(o, d);
    intersection_query<void> query;
    intersector<void> inter;
    inter.assume_geometry_type(intersection_type::triangle);
    float4 hit_res;
    hit_res.x = r.origin.x; hit_res.y = r.direction.z; hit_res.z = 1.0f; hit_res.w = 1.0f;
    hits[tid] = hit_res;
}

// 3. Mesh Shaders & Object Shaders (MSL 3.0+)
struct MeshVertex {
    float4 position [[position]];
    float3 color;
};

struct MeshPrimitive {
    float normal_x;
};

[[kernel]] void test_object_shader(object_data payload<MeshVertex>* p [[buffer(0)]],
                                   uint tid [[thread_position_in_grid]]) {
    float3 c; c.x = 1.0f; c.y = 0.5f; c.z = 0.2f;
    (*p).data.color = c;
}

// 4. Atomics and SIMDgroup Matrix Operations (MSL 2.0+)
[[kernel]] void test_atomics_and_simdgroup(device atomic_int* counter [[buffer(0)]],
                                           device float* mat_data [[buffer(1)]],
                                           threadgroup atomic_uint* local_counter [[buffer(2)]],
                                           uint tid [[thread_position_in_grid]]) {
    atomic_fetch_add_explicit(counter, 1, memory_order::memory_order_relaxed);
    atomic_store_explicit(local_counter, 100u, memory_order::memory_order_release);

    simdgroup_matrix<float, 8, 8> a;
    simdgroup_matrix<float, 8, 8> b;
    simdgroup_matrix<float, 8, 8> c;
    simdgroup_load(a, mat_data);
    simdgroup_multiply_accumulate(c, a, b, c);
    simdgroup_store(c, mat_data);
}

// 5. Function Constants (MSL 1.2+)
constant bool enable_extra_pass [[function_constant(0)]];

[[kernel]] void test_function_constant(device float* buf [[buffer(0)]],
                                       uint id [[thread_position_in_grid]]) {
    if (enable_extra_pass) {
        buf[id] *= 2.0f;
    }
}
