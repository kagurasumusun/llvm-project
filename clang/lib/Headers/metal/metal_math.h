#ifndef __METAL_MATH_H__
#define __METAL_MATH_H__

#ifdef __METAL__
namespace metal {

template<typename T>
inline T min(T a, T b) { return a < b ? a : b; }

template<typename T>
inline T max(T a, T b) { return a > b ? a : b; }

template<typename T>
inline T clamp(T x, T minval, T maxval) { return min(max(x, minval), maxval); }

inline float dot(float3 a, float3 b) { return a.x*b.x + a.y*b.y + a.z*b.z; }
inline float dot(float4 a, float4 b) { return a.x*b.x + a.y*b.y + a.z*b.z + a.w*b.w; }

inline float3 cross(float3 a, float3 b) {
    return float3(a.y*b.z - a.z*b.y, a.z*b.x - a.x*b.z, a.x*b.y - a.y*b.x);
}

// Builtin declarations
float sin(float x);
float cos(float x);
float tan(float x);
float sqrt(float x);
float rsqrt(float x);
float length(float3 x);
float length(float4 x);
float3 normalize(float3 x);
float4 normalize(float4 x);

} // namespace metal
#endif
#endif // __METAL_MATH_H__
