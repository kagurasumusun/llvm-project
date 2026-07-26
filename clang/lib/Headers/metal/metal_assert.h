//===----------------------------------------------------------------------===//
// metal_assert — MSL assert / abort
//===----------------------------------------------------------------------===//
#ifndef _METAL_ASSERT_H_
#define _METAL_ASSERT_H_
#include <metal/metal_common>

namespace metal {
[[noreturn]] inline void abort() { __builtin_trap(); }
} // namespace metal

#ifndef metal_assert
#ifdef NDEBUG
#define metal_assert(e) ((void)0)
#else
#define metal_assert(e) ((e) ? (void)0 : metal::abort())
#endif
#endif

#endif