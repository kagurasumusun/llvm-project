// metal_atomic — MSL atomic operations (cleanroom)
#ifndef _METAL_ATOMIC_H_
#define _METAL_ATOMIC_H_
#include <metal/metal_common>
namespace metal {

enum class memory_order : int { relaxed = 0, acquire = 1, release = 2, acq_rel = 3, seq_cst = 4 };
enum class memory_scope : int { threadgroup = 0, device = 1 };

METAL_FUNC void atomic_thread_fence(memory_order order) { __air_atomic_fence((int)order); }

template <typename T> METAL_FUNC T atomic_load_explicit(volatile device T *addr, memory_order order) { return __air_atomic_load_explicit(addr, (int)order); }
template <typename T> METAL_FUNC T atomic_load_explicit(volatile threadgroup T *addr, memory_order order) { return __air_atomic_load_explicit(addr, (int)order); }
template <typename T> METAL_FUNC void atomic_store_explicit(volatile device T *addr, T val, memory_order order) { __air_atomic_store_explicit(addr, val, (int)order); }
template <typename T> METAL_FUNC void atomic_store_explicit(volatile threadgroup T *addr, T val, memory_order order) { __air_atomic_store_explicit(addr, val, (int)order); }
template <typename T> METAL_FUNC T atomic_exchange_explicit(volatile device T *addr, T val, memory_order order) { return __air_atomic_exchange_explicit(addr, val, (int)order); }
template <typename T> METAL_FUNC T atomic_exchange_explicit(volatile threadgroup T *addr, T val, memory_order order) { return __air_atomic_exchange_explicit(addr, val, (int)order); }
template <typename T> METAL_FUNC bool atomic_compare_exchange_weak_explicit(volatile device T *addr, T *expected, T desired, memory_order success, memory_order failure) { return __air_atomic_compare_exchange_weak_explicit(addr, expected, desired, (int)success, (int)failure); }
template <typename T> METAL_FUNC bool atomic_compare_exchange_weak_explicit(volatile threadgroup T *addr, T *expected, T desired, memory_order success, memory_order failure) { return __air_atomic_compare_exchange_weak_explicit(addr, expected, desired, (int)success, (int)failure); }

template <typename T> METAL_FUNC T atomic_fetch_add_explicit(volatile device T *addr, T val, memory_order order) { return __air_atomic_fetch_add_explicit(addr, val, (int)order); }
template <typename T> METAL_FUNC T atomic_fetch_add_explicit(volatile threadgroup T *addr, T val, memory_order order) { return __air_atomic_fetch_add_explicit(addr, val, (int)order); }
template <typename T> METAL_FUNC T atomic_fetch_sub_explicit(volatile device T *addr, T val, memory_order order) { return __air_atomic_fetch_sub_explicit(addr, val, (int)order); }
template <typename T> METAL_FUNC T atomic_fetch_sub_explicit(volatile threadgroup T *addr, T val, memory_order order) { return __air_atomic_fetch_sub_explicit(addr, val, (int)order); }
template <typename T> METAL_FUNC T atomic_fetch_and_explicit(volatile device T *addr, T val, memory_order order) { return __air_atomic_fetch_and_explicit(addr, val, (int)order); }
template <typename T> METAL_FUNC T atomic_fetch_and_explicit(volatile threadgroup T *addr, T val, memory_order order) { return __air_atomic_fetch_and_explicit(addr, val, (int)order); }
template <typename T> METAL_FUNC T atomic_fetch_or_explicit(volatile device T *addr, T val, memory_order order) { return __air_atomic_fetch_or_explicit(addr, val, (int)order); }
template <typename T> METAL_FUNC T atomic_fetch_or_explicit(volatile threadgroup T *addr, T val, memory_order order) { return __air_atomic_fetch_or_explicit(addr, val, (int)order); }
template <typename T> METAL_FUNC T atomic_fetch_xor_explicit(volatile device T *addr, T val, memory_order order) { return __air_atomic_fetch_xor_explicit(addr, val, (int)order); }
template <typename T> METAL_FUNC T atomic_fetch_xor_explicit(volatile threadgroup T *addr, T val, memory_order order) { return __air_atomic_fetch_xor_explicit(addr, val, (int)order); }
template <typename T> METAL_FUNC T atomic_fetch_max_explicit(volatile device T *addr, T val, memory_order order) { return __air_atomic_fetch_max_explicit(addr, val, (int)order); }
template <typename T> METAL_FUNC T atomic_fetch_max_explicit(volatile threadgroup T *addr, T val, memory_order order) { return __air_atomic_fetch_max_explicit(addr, val, (int)order); }
template <typename T> METAL_FUNC T atomic_fetch_min_explicit(volatile device T *addr, T val, memory_order order) { return __air_atomic_fetch_min_explicit(addr, val, (int)order); }
template <typename T> METAL_FUNC T atomic_fetch_min_explicit(volatile threadgroup T *addr, T val, memory_order order) { return __air_atomic_fetch_min_explicit(addr, val, (int)order); }

template <typename T> METAL_FUNC T atomic_max_explicit(volatile device T *addr, T val, memory_order order) { return __air_atomic_max_explicit(addr, val, (int)order); }
template <typename T> METAL_FUNC T atomic_min_explicit(volatile device T *addr, T val, memory_order order) { return __air_atomic_min_explicit(addr, val, (int)order); }

} // namespace metal
#endif