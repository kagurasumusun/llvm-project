//===-- metal_rt.h - Custom Metal Runtime & Resource Tracking API ---------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// Complete functional substitute for Apple Metal Runtime & Resource Libraries
//
//===----------------------------------------------------------------------===//

#ifndef METAL_RT_H
#define METAL_RT_H

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

// Memory Address Spaces in Metal IR
typedef enum {
    METAL_ADDRSPACE_DEFAULT = 0,
    METAL_ADDRSPACE_DEVICE = 1,
    METAL_ADDRSPACE_CONSTANT = 2,
    METAL_ADDRSPACE_THREADGROUP = 3,
    METAL_ADDRSPACE_THREAD = 4,
    METAL_ADDRSPACE_RAY_DATA = 5,
    METAL_ADDRSPACE_OBJECT_DATA = 6,
    METAL_ADDRSPACE_THREADGROUP_IMAGEBLOCK = 7
} metal_addrspace_t;

// Resource Binding Descriptor
typedef struct {
    uint32_t resource_id;
    uint32_t binding_type; // 0: Buffer, 1: Texture, 2: Sampler
    uint64_t virtual_address;
    size_t size_bytes;
} metal_rt_resource_binding_t;

// Vector Types Layout (Standard 16-byte alignment for float4/uint4)
typedef struct { float x, y; } metal_float2_t;
typedef struct { float x, y, z; } metal_float3_t;
typedef struct { float x, y, z, w; } metal_float4_t;
typedef struct { uint32_t x, y; } metal_uint2_t;
typedef struct { uint32_t x, y, z; } metal_uint3_t;
typedef struct { uint32_t x, y, z, w; } metal_uint4_t;
typedef struct { uint16_t x, y; } metal_ushort2_t;

// Raytracing Ray Structure
typedef struct {
    metal_float3_t origin;
    metal_float3_t direction;
    float min_distance;
    float max_distance;
} metal_rt_ray_t;

// Intersection Result Types
typedef enum {
    METAL_INTERSECTION_TYPE_NONE = 0,
    METAL_INTERSECTION_TYPE_TRIANGLE = 1,
    METAL_INTERSECTION_TYPE_BOUNDING_BOX = 2
} metal_rt_intersection_type_t;

// 1. Resource Tracking & Memory Management Runtime Functions
void __metal_rt_resource_bind(const metal_rt_resource_binding_t *binding);
void* __metal_rt_get_buffer_pointer(uint32_t buffer_id, uint64_t offset);
uint64_t __metal_rt_get_texture_handle(uint32_t texture_id);
metal_float4_t __metal_rt_texture_sample2d(uint64_t texture_handle, uint64_t sampler_handle, metal_float2_t coord);

// 2. Atomic Operations Runtime
int32_t __metal_rt_atomic_fetch_add_i32(void *ptr, int32_t val, int32_t memory_order);
uint32_t __metal_rt_atomic_store_u32(void *ptr, uint32_t val, int32_t memory_order);

// 3. Raytracing Runtime Support
void __metal_rt_raytracing_query_reset(void *query_handle, const metal_rt_ray_t *ray, float min_d, float max_d);
int32_t __metal_rt_raytracing_query_next(void *query_handle);
float __metal_rt_raytracing_get_committed_distance(const void *query_handle);
metal_rt_intersection_type_t __metal_rt_raytracing_get_intersection_type(const void *query_handle);

// 4. Mesh & Object Shader Runtime Support
void __metal_rt_object_shader_set_payload(void *payload_ptr, size_t size);
void __metal_rt_mesh_set_primitive_count(void *mesh_handle, uint32_t count);
void __metal_rt_mesh_set_vertex(void *mesh_handle, uint32_t index, const void *vertex_ptr, size_t vertex_size);
void __metal_rt_mesh_set_primitive(void *mesh_handle, uint32_t index, const void *primitive_ptr, size_t primitive_size);

// 5. Imageblock & Tile Shader Runtime Support
void* __metal_rt_imageblock_data_ptr(void *imageblock_handle, metal_ushort2_t tid, size_t element_size);

// 6. SIMDgroup Matrix Operations
void __metal_rt_simdgroup_matrix_load(void *matrix_out, const void *src_ptr);
void __metal_rt_simdgroup_matrix_mma(void *out_c, const void *in_a, const void *in_b, const void *in_acc);
void __metal_rt_simdgroup_matrix_store(const void *matrix_in, void *dst_ptr);

// 7. Logging & Assertion Support
void __metal_rt_log_printf(const char *format, ...);
void __metal_rt_assert_failure(const char *file, uint32_t line, const char *condition);

#ifdef __cplusplus
}
#endif

#endif // METAL_RT_H
