#ifndef __METAL_TESSELLATION_H__
#define __METAL_TESSELLATION_H__

#if 1 // defined(__METAL__) || defined(__metal__)
namespace metal {

struct quad_tessellation_factors_half {
    half edgeTessellationFactor[4];
    half insideTessellationFactor[2];
};

struct triangle_tessellation_factors_half {
    half edgeTessellationFactor[3];
    half insideTessellationFactor;
};

template<typename T>
struct patch_control_point {
    T data;
    patch_control_point() {}
    T& operator*() { return data; }
    const T& operator*() const { return data; }
    T* operator->() { return &data; }
    const T* operator->() const { return &data; }
};

} // namespace metal
#endif
#endif // __METAL_TESSELLATION_H__
