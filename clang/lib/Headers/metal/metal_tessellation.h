#ifndef _METAL_TESSELLATION_H_
#define _METAL_TESSELLATION_H_
#include <metal/metal_common>
namespace metal {
struct patch_control_point_t {
  template <typename T>
  METAL_ALWAYS_INLINE T get(uint idx) const { return T(0); }
};
struct hull_parameters {
  uint num_patch_control_points;
  float3 tessellation_factor[3];
};
struct domain_parameters {
  float2 position;
};
} // namespace metal
#endif
