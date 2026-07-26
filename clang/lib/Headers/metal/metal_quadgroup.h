// metal_quadgroup — MSL quad group (cleanroom)
#ifndef _METAL_QUADGROUP_H_
#define _METAL_QUADGROUP_H_
#include <metal/metal_common>
namespace metal {

template <typename T>
METAL_FUNC T quad_shuffle(T val, int lane) { return __air_quad_shuffle(val, lane); }
template <typename T>
METAL_FUNC T quad_shuffle_down(T val, int lane) { return __air_quad_shuffle_down(val, lane); }
template <typename T>
METAL_FUNC T quad_shuffle_up(T val, int lane) { return __air_quad_shuffle_up(val, lane); }
template <typename T>
METAL_FUNC T quad_shuffle_rotate_down(T val, int delta) { return __air_quad_shuffle_rotate_down(val, delta); }
template <typename T>
METAL_FUNC T quad_shuffle_rotate_up(T val, int delta) { return __air_quad_shuffle_rotate_up(val, delta); }
template <typename T>
METAL_FUNC T quad_shuffle_xor(T val, int lane) { return __air_quad_shuffle_xor(val, lane); }
template <typename T>
METAL_FUNC T quad_shuffle_and_fill_down(T data, T filling_data, ushort delta, ushort modulo) { return __air_quad_shuffle_and_fill_down(data, filling_data, delta, modulo); }
template <typename T>
METAL_FUNC T quad_shuffle_and_fill_down(T data, T filling_data, ushort delta) { return __air_quad_shuffle_and_fill_down(data, filling_data, delta, ushort(4)); }
template <typename T>
METAL_FUNC T quad_shuffle_and_fill_up(T data, T filling_data, ushort delta, ushort modulo) { return __air_quad_shuffle_and_fill_up(data, filling_data, delta, modulo); }
template <typename T>
METAL_FUNC T quad_shuffle_and_fill_up(T data, T filling_data, ushort delta) { return __air_quad_shuffle_and_fill_up(data, filling_data, delta, ushort(4)); }
template <typename T>
METAL_FUNC T quad_broadcast(T val, int lane) { return __air_quad_broadcast(val, lane); }
template <typename T>
METAL_FUNC T quad_broadcast_first(T val) { return __air_quad_broadcast_first(val); }

METAL_FUNC bool quad_vote_all(int pred) { return __air_quad_vote_all(pred); }
METAL_FUNC bool quad_vote_any(int pred) { return __air_quad_vote_any(pred); }
METAL_FUNC bool quad_all(int pred) { return __air_quad_vote_all(pred); }
METAL_FUNC bool quad_any(int pred) { return __air_quad_vote_any(pred); }
METAL_FUNC uint quad_ballot(int pred) { return __air_quad_ballot(pred); }
METAL_FUNC bool quad_is_first() { return __air_quad_is_first(); }
METAL_FUNC uint quad_active_threads_mask() { return __air_quad_active_threads_mask(); }
template <typename T>
METAL_FUNC T quad_sum(T val) { return __air_quad_sum(val); }
template <typename T>
METAL_FUNC T quad_product(T val) { return __air_quad_product(val); }
template <typename T>
METAL_FUNC T quad_min(T val) { return __air_quad_min(val); }
template <typename T>
METAL_FUNC T quad_max(T val) { return __air_quad_max(val); }
template <typename T>
METAL_FUNC T quad_and(T val) { return __air_quad_and(val); }
template <typename T>
METAL_FUNC T quad_or(T val) { return __air_quad_or(val); }
template <typename T>
METAL_FUNC T quad_xor(T val) { return __air_quad_xor(val); }
template <typename T>
METAL_FUNC T quad_prefix_exclusive_sum(T val) { return __air_quad_prefix_exclusive_sum(val); }
template <typename T>
METAL_FUNC T quad_prefix_exclusive_product(T val) { return __air_quad_prefix_exclusive_product(val); }
template <typename T>
METAL_FUNC T quad_prefix_inclusive_sum(T val) { return __air_quad_prefix_inclusive_sum(val); }
template <typename T>
METAL_FUNC T quad_prefix_inclusive_product(T val) { return __air_quad_prefix_inclusive_product(val); }
METAL_FUNC bool quad_is_helper_thread() { return false; }

} // namespace metal
#endif