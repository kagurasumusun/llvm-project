//===-- metal_rt.h - Custom Metal Runtime & Resource Tracking API ---------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// Complete functional substitute for Apple Metal Runtime & Resource Libraries
// Analyzed and reverse-engineered from Xcode26.5 official toolchain binaries.
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

// Vector Types Layout
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

// 1. Bit Extraction Intrinsics (Exact symbol compatibility with libmetal_rt)
uint8_t ___metal_extract_bits_uint8(uint32_t val, uint32_t offset, uint32_t size);
uint16_t ___metal_extract_bits_uint16(uint32_t val, uint32_t offset, uint32_t size);
uint32_t ___metal_extract_bits_uint32(uint32_t val, uint32_t offset, uint32_t size);
uint64_t ___metal_extract_bits_uint64(uint64_t val, uint32_t offset, uint32_t size);
int8_t ___metal_extract_bits_int8(uint32_t val, uint32_t offset, uint32_t size);
int16_t ___metal_extract_bits_int16(uint32_t val, uint32_t offset, uint32_t size);
int32_t ___metal_extract_bits_int32(uint32_t val, uint32_t offset, uint32_t size);

// 2. AIR Math Runtime Intrinsics (Exact symbol compatibility with libair_rt)
float __air_impl_nextafter_f32(float x, float y);
double __air_impl_nextafter_f64(double x, double y);
uint16_t __air_impl_nextafter_f16(uint16_t x, uint16_t y);
uint16_t __air_impl_nextafter_bf16(uint16_t x, uint16_t y);

// 3. Resource Tracking & Patching Intrinsics (Exact symbol compatibility with libresource_tracking_rt)
void __resource_tracking_impl_patching_read_p1i8_p1i8(void *dst, const void *src);
void __resource_tracking_impl_patching_read_p1i8_p2i8(void *dst, const void *src);
void __resource_tracking_impl_patching_read_p2i8_p1i8(void *dst, const void *src);
void __resource_tracking_impl_patching_read_p2i8_p2i8(void *dst, const void *src);
void __resource_tracking_impl_patching_texture_read_p1i8(void *tex);
void __resource_tracking_impl_patching_sampler_read_p1i8(void *smp);
void __resource_tracking_impl_patching_write_p1i8_p1i8(void *dst, const void *src);
void __resource_tracking_impl_usage_buffer_read(void *buf, size_t sz);
void __resource_tracking_impl_usage_buffer_write(void *buf, size_t sz);
void __resource_tracking_impl_usage_texture_read(void *tex);
void __resource_tracking_impl_usage_texture_write(void *tex);
void __resource_tracking_impl_usage_texture_sample(void *tex, void *smp);

// 4. Logging & OS Log (Exact symbol compatibility with MTLShaderLoggingRuntime)
void __air_impl_os_log(const char *format, ...);

// 5. Raytracing Runtime Intersect Functions (Exact symbol compatibility with MTLRaytracingRuntime)
void __air_raytracing_impl_intersect(void *accel_struct, void *ray, void *result);
void __air_raytracing_impl_intersect_triangle_data(void *accel_struct, void *ray, void *result);
void __air_raytracing_impl_intersect_instancing(void *accel_struct, void *ray, void *result);

// 6. Resource Management & Custom Metal API Functions
void __metal_rt_resource_bind(const metal_rt_resource_binding_t *binding);
void* __metal_rt_get_buffer_pointer(uint32_t buffer_id, uint64_t offset);
uint64_t __metal_rt_get_texture_handle(uint32_t texture_id);
metal_float4_t __metal_rt_texture_sample2d(uint64_t texture_handle, uint64_t sampler_handle, metal_float2_t coord);

int32_t __metal_rt_atomic_fetch_add_i32(void *ptr, int32_t val, int32_t memory_order);
uint32_t __metal_rt_atomic_store_u32(void *ptr, uint32_t val, int32_t memory_order);

void __metal_rt_raytracing_query_reset(void *query_handle, const metal_rt_ray_t *ray, float min_d, float max_d);
int32_t __metal_rt_raytracing_query_next(void *query_handle);
float __metal_rt_raytracing_get_committed_distance(const void *query_handle);
metal_rt_intersection_type_t __metal_rt_raytracing_get_intersection_type(const void *query_handle);

void __metal_rt_object_shader_set_payload(void *payload_ptr, size_t size);
void __metal_rt_mesh_set_primitive_count(void *mesh_handle, uint32_t count);
void __metal_rt_mesh_set_vertex(void *mesh_handle, uint32_t index, const void *vertex_ptr, size_t vertex_size);
void __metal_rt_mesh_set_primitive(void *mesh_handle, uint32_t index, const void *primitive_ptr, size_t primitive_size);

void* __metal_rt_imageblock_data_ptr(void *imageblock_handle, metal_ushort2_t tid, size_t element_size);

void __metal_rt_simdgroup_matrix_load(void *matrix_out, const void *src_ptr);
void __metal_rt_simdgroup_matrix_mma(void *out_c, const void *in_a, const void *in_b, const void *in_acc);
void __metal_rt_simdgroup_matrix_store(const void *matrix_in, void *dst_ptr);

void __metal_rt_log_printf(const char *format, ...);
void __metal_rt_assert_failure(const char *file, uint32_t line, const char *condition);

#ifdef __cplusplus
}
#endif

#endif // METAL_RT_H
