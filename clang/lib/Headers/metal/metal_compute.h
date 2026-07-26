//===----------------------------------------------------------------------===//
// metal_compute — MSL compute pipeline types
//===----------------------------------------------------------------------===//
#ifndef _METAL_COMPUTE_H_
#define _METAL_COMPUTE_H_
#include <metal/metal_common>

namespace metal {

struct compute_pipeline_state_t {
  uint get_thread_execution_width() const;
  uint get_max_total_threads_per_threadgroup() const;
};

struct mesh_grid_properties_t {
  void set_threadgroups_per_grid(uint3 groups_per_grid) const;
  void set_threads_per_threadgroup(uint3 threads_per_threadgroup) const;
};

// Mesh shader types (MSL 3.1+)
template <typename PayloadT, typename TopologyT, int MaxVertices, int MaxPrimitives>
struct mesh_t {
  void set_vertex(int idx, PayloadT v) const;
  void set_primitive(int idx, TopologyT p) const;
  void set_index(int idx, uint i) const;
  void set_indices(uint *indices) const;
  void set_primitive_count(uint count) const;
};

template <int MaxVertices, int MaxPrimitives>
struct mesh_t<void, triangle, MaxVertices, MaxPrimitives> {
  void set_vertex_count(uint count) const;
  void set_primitive_count(uint count) const;
  void set_index(int idx, uint i) const;
};

struct mesh_grid_properties {
  void set_threadgroups_per_grid(uint3 tpg) const;
  void set_threads_per_threadgroup(uint3 tpt) const;
};

} // namespace metal
#endif // _METAL_COMPUTE_H_
