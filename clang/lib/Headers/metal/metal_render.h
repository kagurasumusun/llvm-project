//===----------------------------------------------------------------------===//
// metal_render — MSL render pipeline types
//===----------------------------------------------------------------------===//
#ifndef _METAL_RENDER_H_
#define _METAL_RENDER_H_
#include <metal/metal_common>

namespace metal {

struct render_pipeline_state_t {
  uint get_vertex_count() const;
  uint get_instance_count() const;
  uint get_vertex_start() const;
  uint get_instance_start() const;
};

struct depth_stencil_state_t {};

struct interpolant_t {
  float interpolate_center() const;
  float interpolate_centroid() const;
  float interpolate_offset(float2 offset) const;
  float interpolate_sample(uint sample_id) const;
};

// Fragment output
struct fragment_output {
  float4 color [[color(0)]];
  float depth [[depth(any)]];
  uint stencil [[stencil]];
  bool coverage_mask [[sample_mask]];
};

// Vertex output with position
struct vertex_output {
  float4 position [[position]];
  float point_size [[point_size]];
  float clip_distance [[clip_distance]];
};

} // namespace metal
#endif // _METAL_RENDER_H_
