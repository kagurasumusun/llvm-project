// metal_simdgroup — MSL SIMD group (cleanroom)
#ifndef _METAL_SIMDGROUP_H_
#define _METAL_SIMDGROUP_H_
#include <metal/metal_common>
namespace metal {

template <typename T>
METAL_FUNC T simd_shuffle(T val, int lane) { return __air_simd_shuffle(val, lane); }
template <typename T>
METAL_FUNC T simd_shuffle_down(T val, int lane) { return __air_simd_shuffle_down(val, lane); }
template <typename T>
METAL_FUNC T simd_shuffle_up(T val, int lane) { return __air_simd_shuffle_up(val, lane); }
template <typename T>
METAL_FUNC T simd_shuffle_xor(T val, int lane) { return __air_simd_shuffle_xor(val, lane); }
template <typename T>
METAL_FUNC T simd_shuffle_rotate_down(T val, int delta) { return __air_simd_shuffle_rotate_down(val, delta); }
template <typename T>
METAL_FUNC T simd_shuffle_rotate_up(T val, int delta) { return __air_simd_shuffle_rotate_up(val, delta); }
template <typename T>
METAL_FUNC T simd_shuffle_and_fill_down(T val, int delta) { return __air_simd_shuffle_and_fill_down(val, delta); }
template <typename T>
METAL_FUNC T simd_shuffle_and_fill_up(T val, int delta) { return __air_simd_shuffle_and_fill_up(val, delta); }
template <typename T>
METAL_FUNC T simd_broadcast(T val, int lane) { return __air_simd_broadcast(val, lane); }
template <typename T>
METAL_FUNC T simd_broadcast_first(T val) { return __air_simd_broadcast_first(val); }

METAL_FUNC bool simd_vote_all(int pred) { return __air_simd_vote_all(pred); }
METAL_FUNC bool simd_vote_any(int pred) { return __air_simd_vote_any(pred); }
METAL_FUNC bool simd_all(int pred) { return __air_simd_vote_all(pred); }
METAL_FUNC bool simd_any(int pred) { return __air_simd_vote_any(pred); }
METAL_FUNC uint simd_ballot(int pred) { return __air_simd_ballot(pred); }
METAL_FUNC bool simd_is_first() { return __air_simd_is_first(); }
METAL_FUNC uint simd_active_threads_mask() { return __air_simd_active_threads_mask(); }
template <typename T>
METAL_FUNC T simd_sum(T val) { return __air_simd_sum(val); }
template <typename T>
METAL_FUNC T simd_product(T val) { return __air_simd_product(val); }
template <typename T>
METAL_FUNC T simd_min(T val) { return __air_simd_min(val); }
template <typename T>
METAL_FUNC T simd_max(T val) { return __air_simd_max(val); }
template <typename T>
METAL_FUNC T simd_and(T val) { return __air_simd_and(val); }
template <typename T>
METAL_FUNC T simd_or(T val) { return __air_simd_or(val); }
template <typename T>
METAL_FUNC T simd_xor(T val) { return __air_simd_xor(val); }
template <typename T>
METAL_FUNC T simd_prefix_exclusive_sum(T val) { return __air_simd_prefix_exclusive_sum(val); }
template <typename T>
METAL_FUNC T simd_prefix_exclusive_product(T val) { return __air_simd_prefix_exclusive_product(val); }
template <typename T>
METAL_FUNC T simd_prefix_inclusive_sum(T val) { return __air_simd_prefix_inclusive_sum(val); }
template <typename T>
METAL_FUNC T simd_prefix_inclusive_product(T val) { return __air_simd_prefix_inclusive_product(val); }

template <typename T>
METAL_FUNC bool all(T x) { return __air_all(x); }
template <typename T>
METAL_FUNC bool any(T x) { return __air_any(x); }
template <typename T>
METAL_FUNC bool all(T x) { return __air_all(x); }
template <typename T>
METAL_FUNC bool any(T x) { return __air_any(x); }
METAL_FUNC bool simd_is_helper_thread() { return false; }
METAL_FUNC uint get_simdgroup_size() { return 32; }

} // namespace metal
#endif