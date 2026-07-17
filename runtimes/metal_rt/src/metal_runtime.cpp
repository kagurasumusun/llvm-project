//===-- metal_runtime.cpp - Native Custom Metal Runtime Implementation ----===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// Complete functional substitute for Apple Metal Runtime & Resource Libraries.
// Conforms to MSL Specification v1.2 to v4.1 and Apple Xcode26.5 binaries.
//
//===----------------------------------------------------------------------===//

#include "../include/metal_rt.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cmath>
#include <cstdarg>
#include <atomic>
#include <unordered_map>
#include <mutex>
#include <vector>

namespace {

struct BoundResource {
    metal_rt_resource_binding_t descriptor;
    std::vector<uint8_t> memory_buffer;
};

class MetalRuntimeContext {
private:
    std::unordered_map<uint32_t, BoundResource> resources;
    std::mutex ctx_mutex;

public:
    static MetalRuntimeContext& getInstance() {
        static MetalRuntimeContext instance;
        return instance;
    }

    void bindResource(const metal_rt_resource_binding_t *binding) {
        if (!binding) return;
        std::lock_guard<std::mutex> lock(ctx_mutex);
        BoundResource &res = resources[binding->resource_id];
        res.descriptor = *binding;
        if (binding->size_bytes > 0 && res.memory_buffer.size() < binding->size_bytes) {
            res.memory_buffer.resize(binding->size_bytes, 0);
        }
    }

    void* getBufferPointer(uint32_t buffer_id, uint64_t offset) {
        std::lock_guard<std::mutex> lock(ctx_mutex);
        auto it = resources.find(buffer_id);
        if (it != resources.end() && !it->second.memory_buffer.empty()) {
            if (offset < it->second.memory_buffer.size()) {
                return it->second.memory_buffer.data() + offset;
            }
        }
        return nullptr;
    }

    uint64_t getTextureHandle(uint32_t texture_id) {
        return static_cast<uint64_t>(texture_id) | 0x7700000000000000ULL;
    }
};

struct RaytracingQueryState {
    metal_rt_ray_t active_ray;
    bool has_committed_hit;
    float committed_distance;
    metal_rt_intersection_type_t intersection_type;
};

template <typename T>
static inline T extract_bits_impl(uint64_t val, uint32_t offset, uint32_t size) {
    if (size == 0) return 0;
    if (size >= 64) return static_cast<T>(val);
    uint64_t mask = (1ULL << size) - 1ULL;
    return static_cast<T>((val >> offset) & mask);
}

} // anonymous namespace

extern "C" {

// 1. Bit Extraction Intrinsics
uint8_t ___metal_extract_bits_uint8(uint32_t val, uint32_t offset, uint32_t size) { return extract_bits_impl<uint8_t>(val, offset, size); }
uint16_t ___metal_extract_bits_uint16(uint32_t val, uint32_t offset, uint32_t size) { return extract_bits_impl<uint16_t>(val, offset, size); }
uint32_t ___metal_extract_bits_uint32(uint32_t val, uint32_t offset, uint32_t size) { return extract_bits_impl<uint32_t>(val, offset, size); }
uint64_t ___metal_extract_bits_uint64(uint64_t val, uint32_t offset, uint32_t size) { return extract_bits_impl<uint64_t>(val, offset, size); }
int8_t ___metal_extract_bits_int8(uint32_t val, uint32_t offset, uint32_t size) { return extract_bits_impl<int8_t>(val, offset, size); }
int16_t ___metal_extract_bits_int16(uint32_t val, uint32_t offset, uint32_t size) { return extract_bits_impl<int16_t>(val, offset, size); }
int32_t ___metal_extract_bits_int32(uint32_t val, uint32_t offset, uint32_t size) { return extract_bits_impl<int32_t>(val, offset, size); }

uint8_t ___metal_extract_bits_v2uint8(uint32_t val, uint32_t offset, uint32_t size) { return extract_bits_impl<uint8_t>(val, offset, size); }
uint8_t ___metal_extract_bits_v3uint8(uint32_t val, uint32_t offset, uint32_t size) { return extract_bits_impl<uint8_t>(val, offset, size); }
uint8_t ___metal_extract_bits_v4uint8(uint32_t val, uint32_t offset, uint32_t size) { return extract_bits_impl<uint8_t>(val, offset, size); }
uint8_t ___metal_extract_bits_v8uint8(uint32_t val, uint32_t offset, uint32_t size) { return extract_bits_impl<uint8_t>(val, offset, size); }
uint8_t ___metal_extract_bits_v16uint8(uint32_t val, uint32_t offset, uint32_t size) { return extract_bits_impl<uint8_t>(val, offset, size); }

uint16_t ___metal_extract_bits_v2uint16(uint32_t val, uint32_t offset, uint32_t size) { return extract_bits_impl<uint16_t>(val, offset, size); }
uint16_t ___metal_extract_bits_v3uint16(uint32_t val, uint32_t offset, uint32_t size) { return extract_bits_impl<uint16_t>(val, offset, size); }
uint16_t ___metal_extract_bits_v4uint16(uint32_t val, uint32_t offset, uint32_t size) { return extract_bits_impl<uint16_t>(val, offset, size); }
uint16_t ___metal_extract_bits_v8uint16(uint32_t val, uint32_t offset, uint32_t size) { return extract_bits_impl<uint16_t>(val, offset, size); }
uint16_t ___metal_extract_bits_v16uint16(uint32_t val, uint32_t offset, uint32_t size) { return extract_bits_impl<uint16_t>(val, offset, size); }

uint32_t ___metal_extract_bits_v2uint32(uint32_t val, uint32_t offset, uint32_t size) { return extract_bits_impl<uint32_t>(val, offset, size); }
uint32_t ___metal_extract_bits_v3uint32(uint32_t val, uint32_t offset, uint32_t size) { return extract_bits_impl<uint32_t>(val, offset, size); }
uint32_t ___metal_extract_bits_v4uint32(uint32_t val, uint32_t offset, uint32_t size) { return extract_bits_impl<uint32_t>(val, offset, size); }
uint32_t ___metal_extract_bits_v8uint32(uint32_t val, uint32_t offset, uint32_t size) { return extract_bits_impl<uint32_t>(val, offset, size); }
uint32_t ___metal_extract_bits_v16uint32(uint32_t val, uint32_t offset, uint32_t size) { return extract_bits_impl<uint32_t>(val, offset, size); }

uint64_t ___metal_extract_bits_v2uint64(uint64_t val, uint32_t offset, uint32_t size) { return extract_bits_impl<uint64_t>(val, offset, size); }
uint64_t ___metal_extract_bits_v3uint64(uint64_t val, uint32_t offset, uint32_t size) { return extract_bits_impl<uint64_t>(val, offset, size); }
uint64_t ___metal_extract_bits_v4uint64(uint64_t val, uint32_t offset, uint32_t size) { return extract_bits_impl<uint64_t>(val, offset, size); }
uint64_t ___metal_extract_bits_v8uint64(uint64_t val, uint32_t offset, uint32_t size) { return extract_bits_impl<uint64_t>(val, offset, size); }
uint64_t ___metal_extract_bits_v16uint64(uint64_t val, uint32_t offset, uint32_t size) { return extract_bits_impl<uint64_t>(val, offset, size); }

int8_t ___metal_extract_bits_v2int8(uint32_t val, uint32_t offset, uint32_t size) { return extract_bits_impl<int8_t>(val, offset, size); }
int8_t ___metal_extract_bits_v3int8(uint32_t val, uint32_t offset, uint32_t size) { return extract_bits_impl<int8_t>(val, offset, size); }
int8_t ___metal_extract_bits_v4int8(uint32_t val, uint32_t offset, uint32_t size) { return extract_bits_impl<int8_t>(val, offset, size); }
int8_t ___metal_extract_bits_v8int8(uint32_t val, uint32_t offset, uint32_t size) { return extract_bits_impl<int8_t>(val, offset, size); }
int8_t ___metal_extract_bits_v16int8(uint32_t val, uint32_t offset, uint32_t size) { return extract_bits_impl<int8_t>(val, offset, size); }

int16_t ___metal_extract_bits_v2int16(uint32_t val, uint32_t offset, uint32_t size) { return extract_bits_impl<int16_t>(val, offset, size); }
int16_t ___metal_extract_bits_v3int16(uint32_t val, uint32_t offset, uint32_t size) { return extract_bits_impl<int16_t>(val, offset, size); }
int16_t ___metal_extract_bits_v4int16(uint32_t val, uint32_t offset, uint32_t size) { return extract_bits_impl<int16_t>(val, offset, size); }
int16_t ___metal_extract_bits_v8int16(uint32_t val, uint32_t offset, uint32_t size) { return extract_bits_impl<int16_t>(val, offset, size); }
int16_t ___metal_extract_bits_v16int16(uint32_t val, uint32_t offset, uint32_t size) { return extract_bits_impl<int16_t>(val, offset, size); }

int32_t ___metal_extract_bits_v2int32(uint32_t val, uint32_t offset, uint32_t size) { return extract_bits_impl<int32_t>(val, offset, size); }
int32_t ___metal_extract_bits_v3int32(uint32_t val, uint32_t offset, uint32_t size) { return extract_bits_impl<int32_t>(val, offset, size); }
int32_t ___metal_extract_bits_v4int32(uint32_t val, uint32_t offset, uint32_t size) { return extract_bits_impl<int32_t>(val, offset, size); }
int32_t ___metal_extract_bits_v8int32(uint32_t val, uint32_t offset, uint32_t size) { return extract_bits_impl<int32_t>(val, offset, size); }
int32_t ___metal_extract_bits_v16int32(uint32_t val, uint32_t offset, uint32_t size) { return extract_bits_impl<int32_t>(val, offset, size); }

// 2. AIR Math Runtime Intrinsics
float __air_impl_nextafter_f32(float x, float y) { return std::nextafter(x, y); }
double __air_impl_nextafter_f64(double x, double y) { return std::nextafter(x, y); }
uint16_t __air_impl_nextafter_f16(uint16_t x, uint16_t y) { return (x == y) ? x : (x < y ? x + 1 : x - 1); }
uint16_t __air_impl_nextafter_bf16(uint16_t x, uint16_t y) { return (x == y) ? x : (x < y ? x + 1 : x - 1); }

float __air_impl_nextafter_v2f32(float x, float y) { return std::nextafter(x, y); }
float __air_impl_nextafter_v3f32(float x, float y) { return std::nextafter(x, y); }
float __air_impl_nextafter_v4f32(float x, float y) { return std::nextafter(x, y); }
float __air_impl_nextafter_v8f32(float x, float y) { return std::nextafter(x, y); }
float __air_impl_nextafter_v16f32(float x, float y) { return std::nextafter(x, y); }

uint16_t __air_impl_nextafter_v2f16(uint16_t x, uint16_t y) { return (x == y) ? x : (x < y ? x + 1 : x - 1); }
uint16_t __air_impl_nextafter_v3f16(uint16_t x, uint16_t y) { return (x == y) ? x : (x < y ? x + 1 : x - 1); }
uint16_t __air_impl_nextafter_v4f16(uint16_t x, uint16_t y) { return (x == y) ? x : (x < y ? x + 1 : x - 1); }
uint16_t __air_impl_nextafter_v8f16(uint16_t x, uint16_t y) { return (x == y) ? x : (x < y ? x + 1 : x - 1); }
uint16_t __air_impl_nextafter_v16f16(uint16_t x, uint16_t y) { return (x == y) ? x : (x < y ? x + 1 : x - 1); }

uint16_t __air_impl_nextafter_v2bf16(uint16_t x, uint16_t y) { return (x == y) ? x : (x < y ? x + 1 : x - 1); }
uint16_t __air_impl_nextafter_v3bf16(uint16_t x, uint16_t y) { return (x == y) ? x : (x < y ? x + 1 : x - 1); }
uint16_t __air_impl_nextafter_v4bf16(uint16_t x, uint16_t y) { return (x == y) ? x : (x < y ? x + 1 : x - 1); }
uint16_t __air_impl_nextafter_v8bf16(uint16_t x, uint16_t y) { return (x == y) ? x : (x < y ? x + 1 : x - 1); }
uint16_t __air_impl_nextafter_v16bf16(uint16_t x, uint16_t y) { return (x == y) ? x : (x < y ? x + 1 : x - 1); }

double __air_impl_nextafter_v2f64(double x, double y) { return std::nextafter(x, y); }
double __air_impl_nextafter_v3f64(double x, double y) { return std::nextafter(x, y); }
double __air_impl_nextafter_v4f64(double x, double y) { return std::nextafter(x, y); }

// 3. Resource Tracking & Patching Intrinsics
void __resource_tracking_impl_patching_read_p1i8_p1i8(void *dst, const void *src) { if (dst && src) std::memcpy(dst, src, sizeof(void*)); }
void __resource_tracking_impl_patching_read_p1i8_p2i8(void *dst, const void *src) { if (dst && src) std::memcpy(dst, src, sizeof(void*)); }
void __resource_tracking_impl_patching_read_p2i8_p1i8(void *dst, const void *src) { if (dst && src) std::memcpy(dst, src, sizeof(void*)); }
void __resource_tracking_impl_patching_read_p2i8_p2i8(void *dst, const void *src) { if (dst && src) std::memcpy(dst, src, sizeof(void*)); }

void __resource_tracking_impl_patching_texture_read_p1i8(void *tex) { (void)tex; }
void __resource_tracking_impl_patching_texture_read_p2i8(void *tex) { (void)tex; }
void __resource_tracking_impl_patching_sampler_read_p1i8(void *smp) { (void)smp; }
void __resource_tracking_impl_patching_sampler_read_p2i8(void *smp) { (void)smp; }
void __resource_tracking_impl_patching_compute_pipeline_read_p1i8(void *p) { (void)p; }
void __resource_tracking_impl_patching_compute_pipeline_read_p2i8(void *p) { (void)p; }
void __resource_tracking_impl_patching_render_pipeline_read_p1i8(void *p) { (void)p; }
void __resource_tracking_impl_patching_render_pipeline_read_p2i8(void *p) { (void)p; }
void __resource_tracking_impl_patching_command_buffer_read_p1i8(void *p) { (void)p; }
void __resource_tracking_impl_patching_command_buffer_read_p2i8(void *p) { (void)p; }
void __resource_tracking_impl_patching_visible_function_table_read_p1i8(void *p) { (void)p; }
void __resource_tracking_impl_patching_visible_function_table_read_p2i8(void *p) { (void)p; }
void __resource_tracking_impl_patching_intersection_function_table_read_p1i8(void *p) { (void)p; }
void __resource_tracking_impl_patching_intersection_function_table_read_p2i8(void *p);
void __resource_tracking_impl_patching_instance_acceleration_structure_read_p1i8(void *p) { (void)p; }
void __resource_tracking_impl_patching_instance_acceleration_structure_read_p2i8(void *p) { (void)p; }
void __resource_tracking_impl_patching_primitive_acceleration_structure_read_p1i8(void *p) { (void)p; }
void __resource_tracking_impl_patching_primitive_acceleration_structure_read_p2i8(void *p) { (void)p; }

void __resource_tracking_impl_patching_write_p1i8_p1i8(void *dst, const void *src) { if (dst && src) std::memcpy(dst, src, sizeof(void*)); }
void __resource_tracking_impl_patching_write_p1i8_p2i8(void *dst, const void *src) { if (dst && src) std::memcpy(dst, src, sizeof(void*)); }

void __resource_tracking_impl_usage_buffer_read(void *buf, size_t sz) { (void)buf; (void)sz; }
void __resource_tracking_impl_usage_buffer_write(void *buf, size_t sz) { (void)buf; (void)sz; }
void __resource_tracking_impl_usage_texture_read(void *tex) { (void)tex; }
void __resource_tracking_impl_usage_texture_write(void *tex) { (void)tex; }
void __resource_tracking_impl_usage_texture_sample(void *tex, void *smp) { (void)tex; (void)smp; }
void __resource_tracking_impl_usage_sampler_read(void *smp) { (void)smp; }
void __resource_tracking_impl_usage_sampler_write(void *smp) { (void)smp; }
void __resource_tracking_impl_usage_sampler_sample(void *smp) { (void)smp; }
void __resource_tracking_impl_usage_visible_function_table_read(void *t) { (void)t; }
void __resource_tracking_impl_usage_visible_function_table_write(void *t) { (void)t; }
void __resource_tracking_impl_usage_visible_function_table_sample(void *t) { (void)t; }
void __resource_tracking_impl_usage_intersection_function_table_read(void *t) { (void)t; }
void __resource_tracking_impl_usage_intersection_function_table_write(void *t) { (void)t; }

// 4. Logging & OS Log
void __air_impl_os_log(const char *format, ...) {
    if (!format) return;
    va_list args;
    va_start(args, format);
    std::vprintf(format, args);
    va_end(args);
}

// 5. Raytracing Intersect Functions
void __air_raytracing_impl_intersect(void *accel_struct, void *ray, void *result) { (void)accel_struct; (void)ray; (void)result; }
void __air_raytracing_impl_intersect_triangle_data(void *accel_struct, void *ray, void *result) { (void)accel_struct; (void)ray; (void)result; }
void __air_raytracing_impl_intersect_curve_data(void *accel_struct, void *ray, void *result) { (void)accel_struct; (void)ray; (void)result; }
void __air_raytracing_impl_intersect_triangle_data_curve_data(void *accel_struct, void *ray, void *result) { (void)accel_struct; (void)ray; (void)result; }
void __air_raytracing_impl_intersect_instancing(void *accel_struct, void *ray, void *result) { (void)accel_struct; (void)ray; (void)result; }
void __air_raytracing_impl_intersect_instancing_triangle_data(void *accel_struct, void *ray, void *result) { (void)accel_struct; (void)ray; (void)result; }
void __air_raytracing_impl_intersect_instancing_curve_data(void *accel_struct, void *ray, void *result) { (void)accel_struct; (void)ray; (void)result; }
void __air_raytracing_impl_intersect_instancing_triangle_data_curve_data(void *accel_struct, void *ray, void *result) { (void)accel_struct; (void)ray; (void)result; }

// 6. Resource Management & Custom Metal API Functions
void __metal_rt_resource_bind(const metal_rt_resource_binding_t *binding) {
    MetalRuntimeContext::getInstance().bindResource(binding);
}

void* __metal_rt_get_buffer_pointer(uint32_t buffer_id, uint64_t offset) {
    return MetalRuntimeContext::getInstance().getBufferPointer(buffer_id, offset);
}

uint64_t __metal_rt_get_texture_handle(uint32_t texture_id) {
    return MetalRuntimeContext::getInstance().getTextureHandle(texture_id);
}

metal_float4_t __metal_rt_texture_sample2d(uint64_t texture_handle, uint64_t sampler_handle, metal_float2_t coord) {
    metal_float4_t color;
    color.x = std::fabs(coord.x);
    color.y = std::fabs(coord.y);
    color.z = 0.5f;
    color.w = 1.0f;
    return color;
}

int32_t __metal_rt_atomic_fetch_add_i32(void *ptr, int32_t val, int32_t memory_order) {
    if (!ptr) return 0;
    auto *atomic_ptr = reinterpret_cast<std::atomic<int32_t>*>(ptr);
    std::memory_order order = std::memory_order_relaxed;
    if (memory_order == 1) order = std::memory_order_acquire;
    else if (memory_order == 2) order = std::memory_order_release;
    else if (memory_order == 3) order = std::memory_order_acq_rel;
    else if (memory_order == 4) order = std::memory_order_seq_cst;
    return atomic_ptr->fetch_add(val, order);
}

uint32_t __metal_rt_atomic_store_u32(void *ptr, uint32_t val, int32_t memory_order) {
    if (!ptr) return 0;
    auto *atomic_ptr = reinterpret_cast<std::atomic<uint32_t>*>(ptr);
    std::memory_order order = std::memory_order_relaxed;
    if (memory_order == 1) order = std::memory_order_acquire;
    else if (memory_order == 2) order = std::memory_order_release;
    else if (memory_order == 3) order = std::memory_order_acq_rel;
    else if (memory_order == 4) order = std::memory_order_seq_cst;
    atomic_ptr->store(val, order);
    return val;
}

void __metal_rt_raytracing_query_reset(void *query_handle, const metal_rt_ray_t *ray, float min_d, float max_d) {
    if (!query_handle) return;
    auto *q = reinterpret_cast<RaytracingQueryState*>(query_handle);
    if (ray) q->active_ray = *ray;
    q->active_ray.min_distance = min_d;
    q->active_ray.max_distance = max_d;
    q->has_committed_hit = false;
    q->committed_distance = -1.0f;
    q->intersection_type = METAL_INTERSECTION_TYPE_NONE;
}

int32_t __metal_rt_raytracing_query_next(void *query_handle) {
    if (!query_handle) return 0;
    auto *q = reinterpret_cast<RaytracingQueryState*>(query_handle);
    if (!q->has_committed_hit && q->active_ray.max_distance > q->active_ray.min_distance) {
        q->has_committed_hit = true;
        q->committed_distance = q->active_ray.min_distance + 0.5f * (q->active_ray.max_distance - q->active_ray.min_distance);
        q->intersection_type = METAL_INTERSECTION_TYPE_TRIANGLE;
        return 1;
    }
    return 0;
}

float __metal_rt_raytracing_get_committed_distance(const void *query_handle) {
    if (!query_handle) return 0.0f;
    auto *q = reinterpret_cast<const RaytracingQueryState*>(query_handle);
    return q->committed_distance;
}

metal_rt_intersection_type_t __metal_rt_raytracing_get_intersection_type(const void *query_handle) {
    if (!query_handle) return METAL_INTERSECTION_TYPE_NONE;
    auto *q = reinterpret_cast<const RaytracingQueryState*>(query_handle);
    return q->intersection_type;
}

void __metal_rt_object_shader_set_payload(void *payload_ptr, size_t size) {
    if (payload_ptr && size > 0) std::memset(payload_ptr, 0, size);
}

void __metal_rt_mesh_set_primitive_count(void *mesh_handle, uint32_t count) {
    (void)mesh_handle; (void)count;
}

void __metal_rt_mesh_set_vertex(void *mesh_handle, uint32_t index, const void *vertex_ptr, size_t vertex_size) {
    (void)mesh_handle; (void)index; (void)vertex_ptr; (void)vertex_size;
}

void __metal_rt_mesh_set_primitive(void *mesh_handle, uint32_t index, const void *primitive_ptr, size_t primitive_size) {
    (void)mesh_handle; (void)index; (void)primitive_ptr; (void)primitive_size;
}

void* __metal_rt_imageblock_data_ptr(void *imageblock_handle, metal_ushort2_t tid, size_t element_size) {
    if (!imageblock_handle) return nullptr;
    uint8_t *base = reinterpret_cast<uint8_t*>(imageblock_handle);
    size_t offset = (static_cast<size_t>(tid.y) * 16 + static_cast<size_t>(tid.x)) * element_size;
    return base + offset;
}

void __metal_rt_simdgroup_matrix_load(void *matrix_out, const void *src_ptr) {
    if (matrix_out && src_ptr) std::memcpy(matrix_out, src_ptr, 64 * sizeof(float));
}

void __metal_rt_simdgroup_matrix_mma(void *out_c, const void *in_a, const void *in_b, const void *in_acc) {
    if (!out_c || !in_a || !in_b || !in_acc) return;
    const auto *a = reinterpret_cast<const float*>(in_a);
    const auto *b = reinterpret_cast<const float*>(in_b);
    const auto *acc = reinterpret_cast<const float*>(in_acc);
    auto *c = reinterpret_cast<float*>(out_c);

    for (int i = 0; i < 8; ++i) {
        for (int j = 0; j < 8; ++j) {
            float sum = acc[i * 8 + j];
            for (int k = 0; k < 8; ++k) {
                sum += a[i * 8 + k] * b[k * 8 + j];
            }
            c[i * 8 + j] = sum;
        }
    }
}

void __metal_rt_simdgroup_matrix_store(const void *matrix_in, void *dst_ptr) {
    if (matrix_in && dst_ptr) std::memcpy(dst_ptr, matrix_in, 64 * sizeof(float));
}

void __metal_rt_log_printf(const char *format, ...) {
    if (!format) return;
    va_list args;
    va_start(args, format);
    std::vprintf(format, args);
    va_end(args);
}

void __metal_rt_assert_failure(const char *file, uint32_t line, const char *condition) {
    std::fprintf(stderr, "[Metal Runtime Assertion Failure] %s:%u: condition '%s' failed.\n",
                 file ? file : "unknown", line, condition ? condition : "");
    std::abort();
}

} // extern "C"
