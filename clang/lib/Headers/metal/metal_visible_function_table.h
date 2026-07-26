// metal_visible_function_table — cleanroom, full
#ifndef _METAL_VISIBLE_FUNCTION_TABLE_H_
#define _METAL_VISIBLE_FUNCTION_TABLE_H_
#include <metal/metal_common>
namespace metal {

struct function_handle_t {};

template <>
struct visible_function_table<float> {
  METAL_FUNC function_handle_t get_function_handle(uint idx) const { return {}; }
  METAL_FUNC uint size() const { return 0; }
  METAL_FUNC bool empty() const { return true; }
  METAL_FUNC bool is_null() const { return true; }
};

template <>
struct visible_function_table<half> {
  METAL_FUNC function_handle_t get_function_handle(uint idx) const { return {}; }
  METAL_FUNC uint size() const { return 0; }
  METAL_FUNC bool empty() const { return true; }
  METAL_FUNC bool is_null() const { return true; }
};

template <>
struct visible_function_table<int> {
  METAL_FUNC function_handle_t get_function_handle(uint idx) const { return {}; }
  METAL_FUNC uint size() const { return 0; }
  METAL_FUNC bool empty() const { return true; }
  METAL_FUNC bool is_null() const { return true; }
};

template <>
struct visible_function_table<uint> {
  METAL_FUNC function_handle_t get_function_handle(uint idx) const { return {}; }
  METAL_FUNC uint size() const { return 0; }
  METAL_FUNC bool empty() const { return true; }
  METAL_FUNC bool is_null() const { return true; }
};

template <>
struct visible_function_table<short> {
  METAL_FUNC function_handle_t get_function_handle(uint idx) const { return {}; }
  METAL_FUNC uint size() const { return 0; }
  METAL_FUNC bool empty() const { return true; }
  METAL_FUNC bool is_null() const { return true; }
};

template <>
struct visible_function_table<ushort> {
  METAL_FUNC function_handle_t get_function_handle(uint idx) const { return {}; }
  METAL_FUNC uint size() const { return 0; }
  METAL_FUNC bool empty() const { return true; }
  METAL_FUNC bool is_null() const { return true; }
};

template <>
struct visible_function_table<char> {
  METAL_FUNC function_handle_t get_function_handle(uint idx) const { return {}; }
  METAL_FUNC uint size() const { return 0; }
  METAL_FUNC bool empty() const { return true; }
  METAL_FUNC bool is_null() const { return true; }
};

template <>
struct visible_function_table<uchar> {
  METAL_FUNC function_handle_t get_function_handle(uint idx) const { return {}; }
  METAL_FUNC uint size() const { return 0; }
  METAL_FUNC bool empty() const { return true; }
  METAL_FUNC bool is_null() const { return true; }
};

template <>
struct visible_function_table<float2> {
  METAL_FUNC function_handle_t get_function_handle(uint idx) const { return {}; }
  METAL_FUNC uint size() const { return 0; }
  METAL_FUNC bool empty() const { return true; }
  METAL_FUNC bool is_null() const { return true; }
};

template <>
struct visible_function_table<float3> {
  METAL_FUNC function_handle_t get_function_handle(uint idx) const { return {}; }
  METAL_FUNC uint size() const { return 0; }
  METAL_FUNC bool empty() const { return true; }
  METAL_FUNC bool is_null() const { return true; }
};

template <>
struct visible_function_table<float4> {
  METAL_FUNC function_handle_t get_function_handle(uint idx) const { return {}; }
  METAL_FUNC uint size() const { return 0; }
  METAL_FUNC bool empty() const { return true; }
  METAL_FUNC bool is_null() const { return true; }
};

template <>
struct visible_function_table<half2> {
  METAL_FUNC function_handle_t get_function_handle(uint idx) const { return {}; }
  METAL_FUNC uint size() const { return 0; }
  METAL_FUNC bool empty() const { return true; }
  METAL_FUNC bool is_null() const { return true; }
};

template <>
struct visible_function_table<half3> {
  METAL_FUNC function_handle_t get_function_handle(uint idx) const { return {}; }
  METAL_FUNC uint size() const { return 0; }
  METAL_FUNC bool empty() const { return true; }
  METAL_FUNC bool is_null() const { return true; }
};

template <>
struct visible_function_table<half4> {
  METAL_FUNC function_handle_t get_function_handle(uint idx) const { return {}; }
  METAL_FUNC uint size() const { return 0; }
  METAL_FUNC bool empty() const { return true; }
  METAL_FUNC bool is_null() const { return true; }
};

template <>
struct visible_function_table<int2> {
  METAL_FUNC function_handle_t get_function_handle(uint idx) const { return {}; }
  METAL_FUNC uint size() const { return 0; }
  METAL_FUNC bool empty() const { return true; }
  METAL_FUNC bool is_null() const { return true; }
};

template <>
struct visible_function_table<int3> {
  METAL_FUNC function_handle_t get_function_handle(uint idx) const { return {}; }
  METAL_FUNC uint size() const { return 0; }
  METAL_FUNC bool empty() const { return true; }
  METAL_FUNC bool is_null() const { return true; }
};

template <>
struct visible_function_table<int4> {
  METAL_FUNC function_handle_t get_function_handle(uint idx) const { return {}; }
  METAL_FUNC uint size() const { return 0; }
  METAL_FUNC bool empty() const { return true; }
  METAL_FUNC bool is_null() const { return true; }
};

template <>
struct visible_function_table<uint2> {
  METAL_FUNC function_handle_t get_function_handle(uint idx) const { return {}; }
  METAL_FUNC uint size() const { return 0; }
  METAL_FUNC bool empty() const { return true; }
  METAL_FUNC bool is_null() const { return true; }
};

template <>
struct visible_function_table<uint3> {
  METAL_FUNC function_handle_t get_function_handle(uint idx) const { return {}; }
  METAL_FUNC uint size() const { return 0; }
  METAL_FUNC bool empty() const { return true; }
  METAL_FUNC bool is_null() const { return true; }
};

template <>
struct visible_function_table<uint4> {
  METAL_FUNC function_handle_t get_function_handle(uint idx) const { return {}; }
  METAL_FUNC uint size() const { return 0; }
  METAL_FUNC bool empty() const { return true; }
  METAL_FUNC bool is_null() const { return true; }
};

template <>
struct visible_function_table<short2> {
  METAL_FUNC function_handle_t get_function_handle(uint idx) const { return {}; }
  METAL_FUNC uint size() const { return 0; }
  METAL_FUNC bool empty() const { return true; }
  METAL_FUNC bool is_null() const { return true; }
};

template <>
struct visible_function_table<short3> {
  METAL_FUNC function_handle_t get_function_handle(uint idx) const { return {}; }
  METAL_FUNC uint size() const { return 0; }
  METAL_FUNC bool empty() const { return true; }
  METAL_FUNC bool is_null() const { return true; }
};

template <>
struct visible_function_table<short4> {
  METAL_FUNC function_handle_t get_function_handle(uint idx) const { return {}; }
  METAL_FUNC uint size() const { return 0; }
  METAL_FUNC bool empty() const { return true; }
  METAL_FUNC bool is_null() const { return true; }
};

template <>
struct visible_function_table<ushort2> {
  METAL_FUNC function_handle_t get_function_handle(uint idx) const { return {}; }
  METAL_FUNC uint size() const { return 0; }
  METAL_FUNC bool empty() const { return true; }
  METAL_FUNC bool is_null() const { return true; }
};

template <>
struct visible_function_table<ushort3> {
  METAL_FUNC function_handle_t get_function_handle(uint idx) const { return {}; }
  METAL_FUNC uint size() const { return 0; }
  METAL_FUNC bool empty() const { return true; }
  METAL_FUNC bool is_null() const { return true; }
};

template <>
struct visible_function_table<ushort4> {
  METAL_FUNC function_handle_t get_function_handle(uint idx) const { return {}; }
  METAL_FUNC uint size() const { return 0; }
  METAL_FUNC bool empty() const { return true; }
  METAL_FUNC bool is_null() const { return true; }
};

template <>
struct visible_function_table<uchar2> {
  METAL_FUNC function_handle_t get_function_handle(uint idx) const { return {}; }
  METAL_FUNC uint size() const { return 0; }
  METAL_FUNC bool empty() const { return true; }
  METAL_FUNC bool is_null() const { return true; }
};

template <>
struct visible_function_table<uchar3> {
  METAL_FUNC function_handle_t get_function_handle(uint idx) const { return {}; }
  METAL_FUNC uint size() const { return 0; }
  METAL_FUNC bool empty() const { return true; }
  METAL_FUNC bool is_null() const { return true; }
};

template <>
struct visible_function_table<uchar4> {
  METAL_FUNC function_handle_t get_function_handle(uint idx) const { return {}; }
  METAL_FUNC uint size() const { return 0; }
  METAL_FUNC bool empty() const { return true; }
  METAL_FUNC bool is_null() const { return true; }
};

template <>
struct visible_function_table<char2> {
  METAL_FUNC function_handle_t get_function_handle(uint idx) const { return {}; }
  METAL_FUNC uint size() const { return 0; }
  METAL_FUNC bool empty() const { return true; }
  METAL_FUNC bool is_null() const { return true; }
};

template <>
struct visible_function_table<char3> {
  METAL_FUNC function_handle_t get_function_handle(uint idx) const { return {}; }
  METAL_FUNC uint size() const { return 0; }
  METAL_FUNC bool empty() const { return true; }
  METAL_FUNC bool is_null() const { return true; }
};

template <>
struct visible_function_table<char4> {
  METAL_FUNC function_handle_t get_function_handle(uint idx) const { return {}; }
  METAL_FUNC uint size() const { return 0; }
  METAL_FUNC bool empty() const { return true; }
  METAL_FUNC bool is_null() const { return true; }
};

template <>
struct visible_function_table<long> {
  METAL_FUNC function_handle_t get_function_handle(uint idx) const { return {}; }
  METAL_FUNC uint size() const { return 0; }
  METAL_FUNC bool empty() const { return true; }
  METAL_FUNC bool is_null() const { return true; }
};

template <>
struct visible_function_table<ulong> {
  METAL_FUNC function_handle_t get_function_handle(uint idx) const { return {}; }
  METAL_FUNC uint size() const { return 0; }
  METAL_FUNC bool empty() const { return true; }
  METAL_FUNC bool is_null() const { return true; }
};

template <>
struct visible_function_table<long2> {
  METAL_FUNC function_handle_t get_function_handle(uint idx) const { return {}; }
  METAL_FUNC uint size() const { return 0; }
  METAL_FUNC bool empty() const { return true; }
  METAL_FUNC bool is_null() const { return true; }
};

template <>
struct visible_function_table<ulong2> {
  METAL_FUNC function_handle_t get_function_handle(uint idx) const { return {}; }
  METAL_FUNC uint size() const { return 0; }
  METAL_FUNC bool empty() const { return true; }
  METAL_FUNC bool is_null() const { return true; }
};

template <>
struct visible_function_table<long3> {
  METAL_FUNC function_handle_t get_function_handle(uint idx) const { return {}; }
  METAL_FUNC uint size() const { return 0; }
  METAL_FUNC bool empty() const { return true; }
  METAL_FUNC bool is_null() const { return true; }
};

template <>
struct visible_function_table<ulong3> {
  METAL_FUNC function_handle_t get_function_handle(uint idx) const { return {}; }
  METAL_FUNC uint size() const { return 0; }
  METAL_FUNC bool empty() const { return true; }
  METAL_FUNC bool is_null() const { return true; }
};

template <>
struct visible_function_table<long4> {
  METAL_FUNC function_handle_t get_function_handle(uint idx) const { return {}; }
  METAL_FUNC uint size() const { return 0; }
  METAL_FUNC bool empty() const { return true; }
  METAL_FUNC bool is_null() const { return true; }
};

template <>
struct visible_function_table<ulong4> {
  METAL_FUNC function_handle_t get_function_handle(uint idx) const { return {}; }
  METAL_FUNC uint size() const { return 0; }
  METAL_FUNC bool empty() const { return true; }
  METAL_FUNC bool is_null() const { return true; }
};

template <>
struct visible_function_table<long8> {
  METAL_FUNC function_handle_t get_function_handle(uint idx) const { return {}; }
  METAL_FUNC uint size() const { return 0; }
  METAL_FUNC bool empty() const { return true; }
  METAL_FUNC bool is_null() const { return true; }
};

template <>
struct visible_function_table<ulong8> {
  METAL_FUNC function_handle_t get_function_handle(uint idx) const { return {}; }
  METAL_FUNC uint size() const { return 0; }
  METAL_FUNC bool empty() const { return true; }
  METAL_FUNC bool is_null() const { return true; }
};

template <>
struct visible_function_table<long16> {
  METAL_FUNC function_handle_t get_function_handle(uint idx) const { return {}; }
  METAL_FUNC uint size() const { return 0; }
  METAL_FUNC bool empty() const { return true; }
  METAL_FUNC bool is_null() const { return true; }
};

template <>
struct visible_function_table<ulong16> {
  METAL_FUNC function_handle_t get_function_handle(uint idx) const { return {}; }
  METAL_FUNC uint size() const { return 0; }
  METAL_FUNC bool empty() const { return true; }
  METAL_FUNC bool is_null() const { return true; }
};

template <>
struct visible_function_table<float8> {
  METAL_FUNC function_handle_t get_function_handle(uint idx) const { return {}; }
  METAL_FUNC uint size() const { return 0; }
  METAL_FUNC bool empty() const { return true; }
  METAL_FUNC bool is_null() const { return true; }
};

template <>
struct visible_function_table<float16> {
  METAL_FUNC function_handle_t get_function_handle(uint idx) const { return {}; }
  METAL_FUNC uint size() const { return 0; }
  METAL_FUNC bool empty() const { return true; }
  METAL_FUNC bool is_null() const { return true; }
};

template <>
struct visible_function_table<half8> {
  METAL_FUNC function_handle_t get_function_handle(uint idx) const { return {}; }
  METAL_FUNC uint size() const { return 0; }
  METAL_FUNC bool empty() const { return true; }
  METAL_FUNC bool is_null() const { return true; }
};

template <>
struct visible_function_table<half16> {
  METAL_FUNC function_handle_t get_function_handle(uint idx) const { return {}; }
  METAL_FUNC uint size() const { return 0; }
  METAL_FUNC bool empty() const { return true; }
  METAL_FUNC bool is_null() const { return true; }
};

template <>
struct visible_function_table<int8> {
  METAL_FUNC function_handle_t get_function_handle(uint idx) const { return {}; }
  METAL_FUNC uint size() const { return 0; }
  METAL_FUNC bool empty() const { return true; }
  METAL_FUNC bool is_null() const { return true; }
};

template <>
struct visible_function_table<int16> {
  METAL_FUNC function_handle_t get_function_handle(uint idx) const { return {}; }
  METAL_FUNC uint size() const { return 0; }
  METAL_FUNC bool empty() const { return true; }
  METAL_FUNC bool is_null() const { return true; }
};

template <>
struct visible_function_table<uint8> {
  METAL_FUNC function_handle_t get_function_handle(uint idx) const { return {}; }
  METAL_FUNC uint size() const { return 0; }
  METAL_FUNC bool empty() const { return true; }
  METAL_FUNC bool is_null() const { return true; }
};

template <>
struct visible_function_table<uint16> {
  METAL_FUNC function_handle_t get_function_handle(uint idx) const { return {}; }
  METAL_FUNC uint size() const { return 0; }
  METAL_FUNC bool empty() const { return true; }
  METAL_FUNC bool is_null() const { return true; }
};

template <>
struct visible_function_table<short8> {
  METAL_FUNC function_handle_t get_function_handle(uint idx) const { return {}; }
  METAL_FUNC uint size() const { return 0; }
  METAL_FUNC bool empty() const { return true; }
  METAL_FUNC bool is_null() const { return true; }
};

template <>
struct visible_function_table<short16> {
  METAL_FUNC function_handle_t get_function_handle(uint idx) const { return {}; }
  METAL_FUNC uint size() const { return 0; }
  METAL_FUNC bool empty() const { return true; }
  METAL_FUNC bool is_null() const { return true; }
};

template <>
struct visible_function_table<ushort8> {
  METAL_FUNC function_handle_t get_function_handle(uint idx) const { return {}; }
  METAL_FUNC uint size() const { return 0; }
  METAL_FUNC bool empty() const { return true; }
  METAL_FUNC bool is_null() const { return true; }
};

template <>
struct visible_function_table<ushort16> {
  METAL_FUNC function_handle_t get_function_handle(uint idx) const { return {}; }
  METAL_FUNC uint size() const { return 0; }
  METAL_FUNC bool empty() const { return true; }
  METAL_FUNC bool is_null() const { return true; }
};

template <>
struct visible_function_table<char8> {
  METAL_FUNC function_handle_t get_function_handle(uint idx) const { return {}; }
  METAL_FUNC uint size() const { return 0; }
  METAL_FUNC bool empty() const { return true; }
  METAL_FUNC bool is_null() const { return true; }
};

template <>
struct visible_function_table<char16> {
  METAL_FUNC function_handle_t get_function_handle(uint idx) const { return {}; }
  METAL_FUNC uint size() const { return 0; }
  METAL_FUNC bool empty() const { return true; }
  METAL_FUNC bool is_null() const { return true; }
};

template <>
struct visible_function_table<uchar8> {
  METAL_FUNC function_handle_t get_function_handle(uint idx) const { return {}; }
  METAL_FUNC uint size() const { return 0; }
  METAL_FUNC bool empty() const { return true; }
  METAL_FUNC bool is_null() const { return true; }
};

template <>
struct visible_function_table<uchar16> {
  METAL_FUNC function_handle_t get_function_handle(uint idx) const { return {}; }
  METAL_FUNC uint size() const { return 0; }
  METAL_FUNC bool empty() const { return true; }
  METAL_FUNC bool is_null() const { return true; }
};

template <>
struct visible_function_table<bfloat> {
  METAL_FUNC function_handle_t get_function_handle(uint idx) const { return {}; }
  METAL_FUNC uint size() const { return 0; }
  METAL_FUNC bool empty() const { return true; }
  METAL_FUNC bool is_null() const { return true; }
};

template <>
struct visible_function_table<bfloat2> {
  METAL_FUNC function_handle_t get_function_handle(uint idx) const { return {}; }
  METAL_FUNC uint size() const { return 0; }
  METAL_FUNC bool empty() const { return true; }
  METAL_FUNC bool is_null() const { return true; }
};

template <>
struct visible_function_table<bfloat3> {
  METAL_FUNC function_handle_t get_function_handle(uint idx) const { return {}; }
  METAL_FUNC uint size() const { return 0; }
  METAL_FUNC bool empty() const { return true; }
  METAL_FUNC bool is_null() const { return true; }
};

template <>
struct visible_function_table<bfloat4> {
  METAL_FUNC function_handle_t get_function_handle(uint idx) const { return {}; }
  METAL_FUNC uint size() const { return 0; }
  METAL_FUNC bool empty() const { return true; }
  METAL_FUNC bool is_null() const { return true; }
};

template <>
struct visible_function_table<bfloat8> {
  METAL_FUNC function_handle_t get_function_handle(uint idx) const { return {}; }
  METAL_FUNC uint size() const { return 0; }
  METAL_FUNC bool empty() const { return true; }
  METAL_FUNC bool is_null() const { return true; }
};

template <>
struct visible_function_table<bfloat16> {
  METAL_FUNC function_handle_t get_function_handle(uint idx) const { return {}; }
  METAL_FUNC uint size() const { return 0; }
  METAL_FUNC bool empty() const { return true; }
  METAL_FUNC bool is_null() const { return true; }
};

template <>
struct visible_function_table<bool> {
  METAL_FUNC function_handle_t get_function_handle(uint idx) const { return {}; }
  METAL_FUNC uint size() const { return 0; }
  METAL_FUNC bool empty() const { return true; }
  METAL_FUNC bool is_null() const { return true; }
};

template <>
struct visible_function_table<bool2> {
  METAL_FUNC function_handle_t get_function_handle(uint idx) const { return {}; }
  METAL_FUNC uint size() const { return 0; }
  METAL_FUNC bool empty() const { return true; }
  METAL_FUNC bool is_null() const { return true; }
};

template <>
struct visible_function_table<bool3> {
  METAL_FUNC function_handle_t get_function_handle(uint idx) const { return {}; }
  METAL_FUNC uint size() const { return 0; }
  METAL_FUNC bool empty() const { return true; }
  METAL_FUNC bool is_null() const { return true; }
};

template <>
struct visible_function_table<bool4> {
  METAL_FUNC function_handle_t get_function_handle(uint idx) const { return {}; }
  METAL_FUNC uint size() const { return 0; }
  METAL_FUNC bool empty() const { return true; }
  METAL_FUNC bool is_null() const { return true; }
};

template <>
struct visible_function_table<bool8> {
  METAL_FUNC function_handle_t get_function_handle(uint idx) const { return {}; }
  METAL_FUNC uint size() const { return 0; }
  METAL_FUNC bool empty() const { return true; }
  METAL_FUNC bool is_null() const { return true; }
};

template <>
struct visible_function_table<bool16> {
  METAL_FUNC function_handle_t get_function_handle(uint idx) const { return {}; }
  METAL_FUNC uint size() const { return 0; }
  METAL_FUNC bool empty() const { return true; }
  METAL_FUNC bool is_null() const { return true; }
};

struct intersection_function_table_t {
  METAL_FUNC void set_buffer(uint idx, device void *buf) const {}
  METAL_FUNC void set_buffer(uint idx, constant void *buf) const {}
  METAL_FUNC void set_function_handle(uint idx, function_handle_t fh) const {}
  METAL_FUNC function_handle_t get_function_handle(uint idx) const { return {}; }
  METAL_FUNC uint size() const { return 0; }
  METAL_FUNC bool empty() const { return true; }
  METAL_FUNC bool is_null() const { return true; }
};

template <typename T>
METAL_FUNC visible_function_table<T> get_null_visible_function_table() { return {}; }
template <typename T>
METAL_FUNC bool is_null_visible_function_table(visible_function_table<T> t) { return true; }
template <typename T>
METAL_FUNC uint get_size_visible_function_table(visible_function_table<T> t) { return 0; }
template <typename T>
METAL_FUNC function_handle_t get_function_pointer_visible_function_table(visible_function_table<T> t, uint idx) { return {}; }
template <typename T>
METAL_FUNC intersection_function_table_t get_visible_function_table_intersection_function_table(visible_function_table<T> t, uint idx) { return {}; }
template <typename T>
METAL_FUNC void set_visible_function_table_intersection_function_table(intersection_function_table_t d, uint di, visible_function_table<T> s, uint si) {}
METAL_FUNC function_handle_t get_null_function_handle() { return {}; }
METAL_FUNC bool is_null_function_handle(function_handle_t h) { return true; }
METAL_FUNC bool is_equal_function_handle(function_handle_t a, function_handle_t b) { return true; }
METAL_FUNC intersection_function_table_t get_null_intersection_function_table() { return {}; }
METAL_FUNC bool is_null_intersection_function_table(intersection_function_table_t t) { return true; }
METAL_FUNC uint get_size_intersection_function_table(intersection_function_table_t t) { return 0; }
METAL_FUNC void set_buffer_intersection_function_table(intersection_function_table_t t, uint idx, device void *buf) {}
METAL_FUNC void set_buffer_intersection_function_table(intersection_function_table_t t, uint idx, constant void *buf) {}
METAL_FUNC void set_function_handle_intersection_function_table(intersection_function_table_t t, uint idx, function_handle_t fh) {}
METAL_FUNC function_handle_t get_function_handle_intersection_function_table(intersection_function_table_t t, uint idx) { return {}; }

} // namespace metal
#endif