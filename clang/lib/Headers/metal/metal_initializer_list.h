#ifndef _METAL_INITIALIZER_LIST_H_
#define _METAL_INITIALIZER_LIST_H_
#include <metal/metal_common>
namespace metal {
template <typename T>
struct initializer_list {
  const T *_begin;
  const T *_end;
  METAL_ALWAYS_INLINE const T* begin() const { return _begin; }
  METAL_ALWAYS_INLINE const T* end() const { return _end; }
  METAL_ALWAYS_INLINE uint size() const { return _end - _begin; }
};
} // namespace metal
#endif
