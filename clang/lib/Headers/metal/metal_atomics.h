#ifndef __METAL_ATOMICS_H__
#define __METAL_ATOMICS_H__

#if 1 // defined(__METAL__) || defined(__metal__)
namespace metal {

enum class memory_order {
    memory_order_relaxed = 0,
    memory_order_consume = 1,
    memory_order_acquire = 2,
    memory_order_release = 3,
    memory_order_acq_rel = 4,
    memory_order_seq_cst = 5
};

template<typename T>
struct atomic {
    T value;
    atomic() {}
    atomic(T val) : value(val) {}
};

typedef atomic<int> atomic_int;
typedef atomic<uint> atomic_uint;
typedef atomic<float> atomic_float;

template<typename T>
inline T atomic_load_explicit(const volatile device atomic<T>* obj, memory_order order = memory_order::memory_order_relaxed) {
    return obj->value;
}
template<typename T>
inline T atomic_load_explicit(const volatile threadgroup atomic<T>* obj, memory_order order = memory_order::memory_order_relaxed) {
    return obj->value;
}

template<typename T>
inline void atomic_store_explicit(volatile device atomic<T>* obj, T desired, memory_order order = memory_order::memory_order_relaxed) {
    obj->value = desired;
}
template<typename T>
inline void atomic_store_explicit(volatile threadgroup atomic<T>* obj, T desired, memory_order order = memory_order::memory_order_relaxed) {
    obj->value = desired;
}

template<typename T>
inline T atomic_fetch_add_explicit(volatile device atomic<T>* obj, T arg, memory_order order = memory_order::memory_order_relaxed) {
    T old = obj->value;
    obj->value += arg;
    return old;
}
template<typename T>
inline T atomic_fetch_add_explicit(volatile threadgroup atomic<T>* obj, T arg, memory_order order = memory_order::memory_order_relaxed) {
    T old = obj->value;
    obj->value += arg;
    return old;
}

template<typename T>
inline T atomic_fetch_min_explicit(volatile device atomic<T>* obj, T arg, memory_order order = memory_order::memory_order_relaxed) {
    T old = obj->value;
    if (arg < old) obj->value = arg;
    return old;
}
template<typename T>
inline T atomic_fetch_max_explicit(volatile device atomic<T>* obj, T arg, memory_order order = memory_order::memory_order_relaxed) {
    T old = obj->value;
    if (arg > old) obj->value = arg;
    return old;
}

} // namespace metal
#endif // __METAL__
#endif // __METAL_ATOMICS_H__
