#ifndef __METAL_COMPUTE_H__
#define __METAL_COMPUTE_H__

#if 1 // __METAL__
namespace metal {

enum class mem_flags {
    mem_none = 0,
    mem_device = 1,
    mem_threadgroup = 2,
    mem_texture = 4
};

inline void threadgroup_barrier(mem_flags flags = mem_flags::mem_threadgroup) {}
inline void simdgroup_barrier(mem_flags flags = mem_flags::mem_none) {}

} // namespace metal
#endif
#endif // __METAL_COMPUTE_H__
