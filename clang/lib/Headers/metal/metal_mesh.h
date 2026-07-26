// metal_mesh — MSL mesh shader (cleanroom)
#ifndef _METAL_MESH_H_
#define _METAL_MESH_H_
#include <metal/metal_common>
namespace metal {

template <typename M, typename V>
struct _mesh_vertex_ext {
  METAL_FUNC void set_vertex(uint i, V v) thread {
    thread auto *derived = static_cast<thread M *>(this);
    __air_set_vertex_mesh(derived->m, i, v);
  }
};

template <typename M>
struct _mesh_vertex_ext<M, void> {};

template <typename M, typename P>
struct _mesh_primitive_ext {
  METAL_FUNC void set_primitive(uint i, P p) thread {
    thread auto *derived = static_cast<thread M *>(this);
    __air_set_primitive_mesh(derived->m, i, p);
  }
};

template <typename M>
struct _mesh_primitive_ext<M, void> {};

template <typename V, typename P, uint MaxVertices, uint MaxPrimitives, int Topo = 3>
struct mesh
    : _mesh_vertex_ext<mesh<V, P, MaxVertices, MaxPrimitives, Topo>, V>,
      _mesh_primitive_ext<mesh<V, P, MaxVertices, MaxPrimitives, Topo>, P> {
  typedef V vertex_type;
  typedef P primitive_type;
  static constexpr constant uint max_vertices = MaxVertices;
  static constexpr constant uint max_primitives = MaxPrimitives;

  METAL_FUNC mesh() thread = delete;
  METAL_FUNC mesh(const thread mesh &) thread = default;
  METAL_FUNC thread mesh &operator=(const thread mesh &) thread = default;

  METAL_FUNC void set_index(uint i, uchar v) thread {
    __air_set_index_mesh(m, i, v);
  }
  METAL_FUNC void set_indices(uint i, uchar2 v) thread {
    __air_set_indices_mesh(m, i, v);
  }
  METAL_FUNC void set_indices(uint i, uchar4 v) thread {
    __air_set_indices_mesh(m, i, v);
  }
  METAL_FUNC void set_primitive_count(uint n) thread {
    __air_set_primitive_count_mesh(m, n);
  }

private:
  __air_mesh_t m;
  template <typename, typename> friend struct _mesh_vertex_ext;
  template <typename, typename> friend struct _mesh_primitive_ext;
};

struct mesh_grid_properties {
  METAL_FUNC mesh_grid_properties() thread = delete;
  METAL_FUNC mesh_grid_properties(const thread mesh_grid_properties &) thread = default;
  METAL_FUNC thread mesh_grid_properties &operator=(const thread mesh_grid_properties &) thread = default;

  METAL_FUNC void set_threadgroups_per_grid(uint3 tgs) thread {
    __air_set_threadgroups_per_grid_mesh_grid_properties(p, tgs);
  }

private:
  __air_mesh_grid_properties_t p;
};

} // namespace metal
#endif