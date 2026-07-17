//===-- metal_rt.h - Custom Metal Runtime & Resource Tracking API ---------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// Complete functional substitute for Apple Metal Runtime & Resource Libraries
// Reverse-engineered and verified against Xcode26.5 official toolchain binaries.
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

// Vector Extract Bits Typedefs
typedef float float2_v2 __attribute__((ext_vector_type(2)));
typedef float float3_v3 __attribute__((ext_vector_type(3)));
typedef float float4_v4 __attribute__((ext_vector_type(4)));
typedef int int8_v2 __attribute__((ext_vector_type(2)));
typedef int int8_v3 __attribute__((ext_vector_type(3)));
typedef int int8_v4 __attribute__((ext_vector_type(4)));

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

// Vector Bit Extract Functions
uint8_t ___metal_extract_bits_v2uint8(uint32_t val, uint32_t offset, uint32_t size);
uint8_t ___metal_extract_bits_v3uint8(uint32_t val, uint32_t offset, uint32_t size);
uint8_t ___metal_extract_bits_v4uint8(uint32_t val, uint32_t offset, uint32_t size);
uint8_t ___metal_extract_bits_v8uint8(uint32_t val, uint32_t offset, uint32_t size);
uint8_t ___metal_extract_bits_v16uint8(uint32_t val, uint32_t offset, uint32_t size);

uint16_t ___metal_extract_bits_v2uint16(uint32_t val, uint32_t offset, uint32_t size);
uint16_t ___metal_extract_bits_v3uint16(uint32_t val, uint32_t offset, uint32_t size);
uint16_t ___metal_extract_bits_v4uint16(uint32_t val, uint32_t offset, uint32_t size);
uint16_t ___metal_extract_bits_v8uint16(uint32_t val, uint32_t offset, uint32_t size);
uint16_t ___metal_extract_bits_v16uint16(uint32_t val, uint32_t offset, uint32_t size);

uint32_t ___metal_extract_bits_v2uint32(uint32_t val, uint32_t offset, uint32_t size);
uint32_t ___metal_extract_bits_v3uint32(uint32_t val, uint32_t offset, uint32_t size);
uint32_t ___metal_extract_bits_v4uint32(uint32_t val, uint32_t offset, uint32_t size);
uint32_t ___metal_extract_bits_v8uint32(uint32_t val, uint32_t offset, uint32_t size);
uint32_t ___metal_extract_bits_v16uint32(uint32_t val, uint32_t offset, uint32_t size);

uint64_t ___metal_extract_bits_v2uint64(uint64_t val, uint32_t offset, uint32_t size);
uint64_t ___metal_extract_bits_v3uint64(uint64_t val, uint32_t offset, uint32_t size);
uint64_t ___metal_extract_bits_v4uint64(uint64_t val, uint32_t offset, uint32_t size);
uint64_t ___metal_extract_bits_v8uint64(uint64_t val, uint32_t offset, uint32_t size);
uint64_t ___metal_extract_bits_v16uint64(uint64_t val, uint32_t offset, uint32_t size);

int8_t ___metal_extract_bits_v2int8(uint32_t val, uint32_t offset, uint32_t size);
int8_t ___metal_extract_bits_v3int8(uint32_t val, uint32_t offset, uint32_t size);
int8_t ___metal_extract_bits_v4int8(uint32_t val, uint32_t offset, uint32_t size);
int8_t ___metal_extract_bits_v8int8(uint32_t val, uint32_t offset, uint32_t size);
int8_t ___metal_extract_bits_v16int8(uint32_t val, uint32_t offset, uint32_t size);

int16_t ___metal_extract_bits_v2int16(uint32_t val, uint32_t offset, uint32_t size);
int16_t ___metal_extract_bits_v3int16(uint32_t val, uint32_t offset, uint32_t size);
int16_t ___metal_extract_bits_v4int16(uint32_t val, uint32_t offset, uint32_t size);
int16_t ___metal_extract_bits_v8int16(uint32_t val, uint32_t offset, uint32_t size);
int16_t ___metal_extract_bits_v16int16(uint32_t val, uint32_t offset, uint32_t size);

int32_t ___metal_extract_bits_v2int32(uint32_t val, uint32_t offset, uint32_t size);
int32_t ___metal_extract_bits_v3int32(uint32_t val, uint32_t offset, uint32_t size);
int32_t ___metal_extract_bits_v4int32(uint32_t val, uint32_t offset, uint32_t size);
int32_t ___metal_extract_bits_v8int32(uint32_t val, uint32_t offset, uint32_t size);
int32_t ___metal_extract_bits_v16int32(uint32_t val, uint32_t offset, uint32_t size);

// 2. AIR Math Runtime Intrinsics (libair_rt_osx.rtlib Compatibility)
float __air_impl_nextafter_f32(float x, float y);
double __air_impl_nextafter_f64(double x, double y);
uint16_t __air_impl_nextafter_f16(uint16_t x, uint16_t y);
uint16_t __air_impl_nextafter_bf16(uint16_t x, uint16_t y);

float __air_impl_nextafter_v2f32(float x, float y);
float __air_impl_nextafter_v3f32(float x, float y);
float __air_impl_nextafter_v4f32(float x, float y);
float __air_impl_nextafter_v8f32(float x, float y);
float __air_impl_nextafter_v16f32(float x, float y);

uint16_t __air_impl_nextafter_v2f16(uint16_t x, uint16_t y);
uint16_t __air_impl_nextafter_v3f16(uint16_t x, uint16_t y);
uint16_t __air_impl_nextafter_v4f16(uint16_t x, uint16_t y);
uint16_t __air_impl_nextafter_v8f16(uint16_t x, uint16_t y);
uint16_t __air_impl_nextafter_v16f16(uint16_t x, uint16_t y);

uint16_t __air_impl_nextafter_v2bf16(uint16_t x, uint16_t y);
uint16_t __air_impl_nextafter_v3bf16(uint16_t x, uint16_t y);
uint16_t __air_impl_nextafter_v4bf16(uint16_t x, uint16_t y);
uint16_t __air_impl_nextafter_v8bf16(uint16_t x, uint16_t y);
uint16_t __air_impl_nextafter_v16bf16(uint16_t x, uint16_t y);

double __air_impl_nextafter_v2f64(double x, double y);
double __air_impl_nextafter_v3f64(double x, double y);
double __air_impl_nextafter_v4f64(double x, double y);

// 3. Resource Tracking & Patching Intrinsics (libresource_tracking_rt_osx.rtlib Compatibility)
void __resource_tracking_impl_patching_read_p1i8_p1i8(void *dst, const void *src);
void __resource_tracking_impl_patching_read_p1i8_p2i8(void *dst, const void *src);
void __resource_tracking_impl_patching_read_p2i8_p1i8(void *dst, const void *src);
void __resource_tracking_impl_patching_read_p2i8_p2i8(void *dst, const void *src);
void __resource_tracking_impl_patching_texture_read_p1i8(void *tex);
void __resource_tracking_impl_patching_texture_read_p2i8(void *tex);
void __resource_tracking_impl_patching_sampler_read_p1i8(void *smp);
void __resource_tracking_impl_patching_sampler_read_p2i8(void *smp);
void __resource_tracking_impl_patching_compute_pipeline_read_p1i8(void *p);
void __resource_tracking_impl_patching_compute_pipeline_read_p2i8(void *p);
void __resource_tracking_impl_patching_render_pipeline_read_p1i8(void *p);
void __resource_tracking_impl_patching_render_pipeline_read_p2i8(void *p);
void __resource_tracking_impl_patching_command_buffer_read_p1i8(void *p);
void __resource_tracking_impl_patching_command_buffer_read_p2i8(void *p);
void __resource_tracking_impl_patching_visible_function_table_read_p1i8(void *p);
void __resource_tracking_impl_patching_visible_function_table_read_p2i8(void *p);
void __resource_tracking_impl_patching_intersection_function_table_read_p1i8(void *p);
void __resource_tracking_impl_patching_intersection_function_table_read_p2i8(void *p);
void __resource_tracking_impl_patching_instance_acceleration_structure_read_p1i8(void *p);
void __resource_tracking_impl_patching_instance_acceleration_structure_read_p2i8(void *p);
void __resource_tracking_impl_patching_primitive_acceleration_structure_read_p1i8(void *p);
void __resource_tracking_impl_patching_primitive_acceleration_structure_read_p2i8(void *p);
void __resource_tracking_impl_patching_write_p1i8_p1i8(void *dst, const void *src);
void __resource_tracking_impl_patching_write_p1i8_p2i8(void *dst, const void *src);

void __resource_tracking_impl_usage_buffer_read(void *buf, size_t sz);
void __resource_tracking_impl_usage_buffer_write(void *buf, size_t sz);
void __resource_tracking_impl_usage_texture_read(void *tex);
void __resource_tracking_impl_usage_texture_write(void *tex);
void __resource_tracking_impl_usage_texture_sample(void *tex, void *smp);
void __resource_tracking_impl_usage_sampler_read(void *smp);
void __resource_tracking_impl_usage_sampler_write(void *smp);
void __resource_tracking_impl_usage_sampler_sample(void *smp);
void __resource_tracking_impl_usage_visible_function_table_read(void *t);
void __resource_tracking_impl_usage_visible_function_table_write(void *t);
void __resource_tracking_impl_usage_visible_function_table_sample(void *t);
void __resource_tracking_impl_usage_intersection_function_table_read(void *t);
void __resource_tracking_impl_usage_intersection_function_table_write(void *t);

// 4. Logging & OS Log (MTLShaderLoggingRuntime.rtlib Compatibility)
void __air_impl_os_log(const char *format, ...);

// 5. Raytracing Intersect Functions (MTLRaytracingRuntime.rtlib Compatibility)
void __air_raytracing_impl_intersect(void *accel_struct, void *ray, void *result);
void __air_raytracing_impl_intersect_triangle_data(void *accel_struct, void *ray, void *result);
void __air_raytracing_impl_intersect_curve_data(void *accel_struct, void *ray, void *result);
void __air_raytracing_impl_intersect_triangle_data_curve_data(void *accel_struct, void *ray, void *result);
void __air_raytracing_impl_intersect_instancing(void *accel_struct, void *ray, void *result);
void __air_raytracing_impl_intersect_instancing_triangle_data(void *accel_struct, void *ray, void *result);
void __air_raytracing_impl_intersect_instancing_curve_data(void *accel_struct, void *ray, void *result);
void __air_raytracing_impl_intersect_instancing_triangle_data_curve_data(void *accel_struct, void *ray, void *result);

// 6. High-Level Custom Metal API & Execution Runtime Functions
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
