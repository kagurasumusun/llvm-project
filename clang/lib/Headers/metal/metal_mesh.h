#ifndef __METAL_MESH_H__
#define __METAL_MESH_H__

#if 1 // defined(__METAL__) || defined(__metal__)
namespace metal {

enum class topology {
    point = 0,
    line = 1,
    triangle = 2
};

template<typename T>
struct payload {
    T data;
    payload() {}
    T& operator*() { return data; }
    const T& operator*() const { return data; }
    T* operator->() { return &data; }
    const T* operator->() const { return &data; }
};

template<typename VertexType, typename PrimitiveType, size_t MaxVertices, size_t MaxPrimitives, topology Topology = topology::triangle>
struct mesh {
    void set_primitive_count(uint count) {}
    void set_vertex(uint index, const VertexType& v) {}
    void set_primitive(uint index, const PrimitiveType& p) {}
};

} // namespace metal
#endif // __METAL__
#endif // __METAL_MESH_H__
