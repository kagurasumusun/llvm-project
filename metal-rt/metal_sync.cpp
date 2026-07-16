// metal_sync.cpp - Metal Synchronization Primitives

extern "C" {

// mem_fence
void ___metal_mem_fence_device(int flags) { __asm__ volatile("" ::: "memory"); }
void ___metal_mem_fence_threadgroup(int flags) { __asm__ volatile("" ::: "memory"); }
void ___metal_mem_fence_texture(int flags) { __asm__ volatile("" ::: "memory"); }

// threadgroup_barrier
void ___metal_threadgroup_barrier(int flags) { __asm__ volatile("" ::: "memory"); }

// simd_barrier
void ___metal_simd_barrier(int flags) { __asm__ volatile("" ::: "memory"); }

// atomic operations are in metal_atomic.cpp

} // extern C
