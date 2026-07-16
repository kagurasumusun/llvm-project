// metal_atomic.cpp - Metal Atomic Operations

extern "C" {

// 32-bit atomic operations
int32_t ___metal_atomic_load_explicit_int32(const volatile int32_t* obj, int order) { return __atomic_load_n(obj, order); }
void ___metal_atomic_store_explicit_int32(volatile int32_t* obj, int32_t desired, int order) { __atomic_store_n(obj, desired, order); }
int32_t ___metal_atomic_exchange_explicit_int32(volatile int32_t* obj, int32_t desired, int order) { return __atomic_exchange_n(obj, desired, order); }
bool ___metal_atomic_compare_exchange_weak_explicit_int32(volatile int32_t* obj, int32_t* expected, int32_t desired, int success, int failure) { return __atomic_compare_exchange_n(obj, expected, desired, true, success, failure); }
bool ___metal_atomic_compare_exchange_strong_explicit_int32(volatile int32_t* obj, int32_t* expected, int32_t desired, int success, int failure) { return __atomic_compare_exchange_n(obj, expected, desired, false, success, failure); }
int32_t ___metal_atomic_fetch_add_explicit_int32(volatile int32_t* obj, int32_t arg, int order) { return __atomic_fetch_add(obj, arg, order); }
int32_t ___metal_atomic_fetch_sub_explicit_int32(volatile int32_t* obj, int32_t arg, int order) { return __atomic_fetch_sub(obj, arg, order); }
int32_t ___metal_atomic_fetch_and_explicit_int32(volatile int32_t* obj, int32_t arg, int order) { return __atomic_fetch_and(obj, arg, order); }
int32_t ___metal_atomic_fetch_or_explicit_int32(volatile int32_t* obj, int32_t arg, int order) { return __atomic_fetch_or(obj, arg, order); }
int32_t ___metal_atomic_fetch_xor_explicit_int32(volatile int32_t* obj, int32_t arg, int order) { return __atomic_fetch_xor(obj, arg, order); }
int32_t ___metal_atomic_fetch_min_explicit_int32(volatile int32_t* obj, int32_t arg, int order) {
    int32_t old = *obj;
    while (old > arg) { if (__atomic_compare_exchange_n(obj, &old, arg, false, order, order)) return arg; }
    return old;
}
int32_t ___metal_atomic_fetch_max_explicit_int32(volatile int32_t* obj, int32_t arg, int order) {
    int32_t old = *obj;
    while (old < arg) { if (__atomic_compare_exchange_n(obj, &old, arg, false, order, order)) return arg; }
    return old;
}

// 32-bit unsigned
uint32_t ___metal_atomic_load_explicit_uint32(const volatile uint32_t* obj, int order) { return __atomic_load_n(obj, order); }
void ___metal_atomic_store_explicit_uint32(volatile uint32_t* obj, uint32_t desired, int order) { __atomic_store_n(obj, desired, order); }
uint32_t ___metal_atomic_exchange_explicit_uint32(volatile uint32_t* obj, uint32_t desired, int order) { return __atomic_exchange_n(obj, desired, order); }
uint32_t ___metal_atomic_fetch_add_explicit_uint32(volatile uint32_t* obj, uint32_t arg, int order) { return __atomic_fetch_add(obj, arg, order); }
uint32_t ___metal_atomic_fetch_sub_explicit_uint32(volatile uint32_t* obj, uint32_t arg, int order) { return __atomic_fetch_sub(obj, arg, order); }
uint32_t ___metal_atomic_fetch_and_explicit_uint32(volatile uint32_t* obj, uint32_t arg, int order) { return __atomic_fetch_and(obj, arg, order); }
uint32_t ___metal_atomic_fetch_or_explicit_uint32(volatile uint32_t* obj, uint32_t arg, int order) { return __atomic_fetch_or(obj, arg, order); }
uint32_t ___metal_atomic_fetch_xor_explicit_uint32(volatile uint32_t* obj, uint32_t arg, int order) { return __atomic_fetch_xor(obj, arg, order); }

// 64-bit atomic operations
int64_t ___metal_atomic_load_explicit_int64(const volatile int64_t* obj, int order) { return __atomic_load_n(obj, order); }
void ___metal_atomic_store_explicit_int64(volatile int64_t* obj, int64_t desired, int order) { __atomic_store_n(obj, desired, order); }
int64_t ___metal_atomic_exchange_explicit_int64(volatile int64_t* obj, int64_t desired, int order) { return __atomic_exchange_n(obj, desired, order); }
int64_t ___metal_atomic_fetch_add_explicit_int64(volatile int64_t* obj, int64_t arg, int order) { return __atomic_fetch_add(obj, arg, order); }
int64_t ___metal_atomic_fetch_sub_explicit_int64(volatile int64_t* obj, int64_t arg, int order) { return __atomic_fetch_sub(obj, arg, order); }
int64_t ___metal_atomic_fetch_min_explicit_int64(volatile int64_t* obj, int64_t arg, int order) {
    int64_t old = *obj;
    while (old > arg) { if (__atomic_compare_exchange_n(obj, &old, arg, false, order, order)) return arg; }
    return old;
}
int64_t ___metal_atomic_fetch_max_explicit_int64(volatile int64_t* obj, int64_t arg, int order) {
    int64_t old = *obj;
    while (old < arg) { if (__atomic_compare_exchange_n(obj, &old, arg, false, order, order)) return arg; }
    return old;
}

uint64_t ___metal_atomic_load_explicit_uint64(const volatile uint64_t* obj, int order) { return __atomic_load_n(obj, order); }
uint64_t ___metal_atomic_fetch_add_explicit_uint64(volatile uint64_t* obj, uint64_t arg, int order) { return __atomic_fetch_add(obj, arg, order); }
uint64_t ___metal_atomic_fetch_sub_explicit_uint64(volatile uint64_t* obj, uint64_t arg, int order) { return __atomic_fetch_sub(obj, arg, order); }

// Float atomics (via int32 reinterpretation)
float ___metal_atomic_load_explicit_float(const volatile float* obj, int order) { int32_t v = __atomic_load_n((const volatile int32_t*)obj, order); float r; __builtin_memcpy(&r, &v, 4); return r; }
void ___metal_atomic_store_explicit_float(volatile float* obj, float desired, int order) { int32_t v; __builtin_memcpy(&v, &desired, 4); __atomic_store_n((volatile int32_t*)obj, v, order); }
float ___metal_atomic_exchange_explicit_float(volatile float* obj, float desired, int order) { int32_t v; __builtin_memcpy(&v, &desired, 4); int32_t r = __atomic_exchange_n((volatile int32_t*)obj, v, order); float fr; __builtin_memcpy(&fr, &r, 4); return fr; }
float ___metal_atomic_fetch_add_explicit_float(volatile float* obj, float arg, int order) {
    float old = *obj;
    float desired;
    do { desired = old + arg; } while (!__atomic_compare_exchange_n((volatile int32_t*)obj, (int32_t*)&old, *(int32_t*)&desired, false, order, order));
    return old;
}

} // extern C
