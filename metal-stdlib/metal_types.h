// metal_types.h - Complete Metal Type Definitions
// Clean-room implementation

#ifndef __METAL_TYPES_H__
#define __METAL_TYPES_H__

namespace metal_types {

// ============================================================================
// Scalar Types
// ============================================================================
typedef bool BOOL;
typedef char char2 __attribute__((ext_vector_type(2)));
typedef char char3 __attribute__((ext_vector_type(3)));
typedef char char4 __attribute__((ext_vector_type(4)));
typedef char char8 __attribute__((ext_vector_type(8)));
typedef char char16 __attribute__((ext_vector_type(16)));

typedef unsigned char uchar;
typedef uchar uchar2 __attribute__((ext_vector_type(2)));
typedef uchar uchar3 __attribute__((ext_vector_type(3)));
typedef uchar uchar4 __attribute__((ext_vector_type(4)));
typedef uchar uchar8 __attribute__((ext_vector_type(8)));
typedef uchar uchar16 __attribute__((ext_vector_type(16)));

typedef short short2 __attribute__((ext_vector_type(2)));
typedef short short3 __attribute__((ext_vector_type(3)));
typedef short short4 __attribute__((ext_vector_type(4)));
typedef short short8 __attribute__((ext_vector_type(8)));
typedef short short16 __attribute__((ext_vector_type(16)));

typedef unsigned short ushort;
typedef ushort ushort2 __attribute__((ext_vector_type(2)));
typedef ushort ushort3 __attribute__((ext_vector_type(3)));
typedef ushort ushort4 __attribute__((ext_vector_type(4)));
typedef ushort ushort8 __attribute__((ext_vector_type(8)));
typedef ushort ushort16 __attribute__((ext_vector_type(16)));

typedef int int2 __attribute__((ext_vector_type(2)));
typedef int int3 __attribute__((ext_vector_type(3)));
typedef int int4 __attribute__((ext_vector_type(4)));
typedef int int8 __attribute__((ext_vector_type(8)));
typedef int int16 __attribute__((ext_vector_type(16)));

typedef unsigned int uint;
typedef uint uint2 __attribute__((ext_vector_type(2)));
typedef uint uint3 __attribute__((ext_vector_type(3)));
typedef uint uint4 __attribute__((ext_vector_type(4)));
typedef uint uint8 __attribute__((ext_vector_type(8)));
typedef uint uint16 __attribute__((ext_vector_type(16)));

typedef long long2 __attribute__((ext_vector_type(2)));
typedef long long3 __attribute__((ext_vector_type(3)));
typedef long long4 __attribute__((ext_vector_type(4)));
typedef long long8 __attribute__((ext_vector_type(8)));
typedef long long16 __attribute__((ext_vector_type(16)));

typedef unsigned long ulong;
typedef ulong ulong2 __attribute__((ext_vector_type(2)));
typedef ulong ulong3 __attribute__((ext_vector_type(3)));
typedef ulong ulong4 __attribute__((ext_vector_type(4)));
typedef ulong ulong8 __attribute__((ext_vector_type(8)));
typedef ulong ulong16 __attribute__((ext_vector_type(16)));

typedef float half;
typedef half half2 __attribute__((ext_vector_type(2)));
typedef half half3 __attribute__((ext_vector_type(3)));
typedef half half4 __attribute__((ext_vector_type(4)));
typedef half half8 __attribute__((ext_vector_type(8)));
typedef half half16 __attribute__((ext_vector_type(16)));

typedef float float2 __attribute__((ext_vector_type(2)));
typedef float float3 __attribute__((ext_vector_type(3)));
typedef float float4 __attribute__((ext_vector_type(4)));
typedef float float8 __attribute__((ext_vector_type(8)));
typedef float float16 __attribute__((ext_vector_type(16)));

typedef double double2 __attribute__((ext_vector_type(2)));
typedef double double3 __attribute__((ext_vector_type(3)));
typedef double double4 __attribute__((ext_vector_type(4)));
typedef double double8 __attribute__((ext_vector_type(8)));
typedef double double16 __attribute__((ext_vector_type(16)));

// BFloat16 (Metal 4.0+)
typedef __bf16 bfloat;
typedef bfloat bfloat2 __attribute__((ext_vector_type(2)));
typedef bfloat bfloat3 __attribute__((ext_vector_type(3)));
typedef bfloat bfloat4 __attribute__((ext_vector_type(4)));

// Bool vectors
typedef bool bool2 __attribute__((ext_vector_type(2)));
typedef bool bool3 __attribute__((ext_vector_type(3)));
typedef bool bool4 __attribute__((ext_vector_type(4)));

// ============================================================================
// Matrix Types
// ============================================================================
template<typename T, int C, int R>
struct matrix {
    typedef T element_type;
    typedef T column_type __attribute__((ext_vector_type(R)));
    static constexpr int columns = C;
    static constexpr int rows = R;
    column_type columns_data[C];
    
    matrix() = default;
    explicit matrix(T diag) {
        for (int c = 0; c < C; c++)
            for (int r = 0; r < R; r++)
                columns_data[c][r] = (c == r) ? diag : T(0);
    }
    
    column_type& operator[](int i) { return columns_data[i]; }
    const column_type& operator[](int i) const { return columns_data[i]; }
    
    static matrix identity() { return matrix(T(1)); }
};

typedef matrix<float, 2, 2> float2x2;
typedef matrix<float, 2, 3> float2x3;
typedef matrix<float, 2, 4> float2x4;
typedef matrix<float, 3, 2> float3x2;
typedef matrix<float, 3, 3> float3x3;
typedef matrix<float, 3, 4> float3x4;
typedef matrix<float, 4, 2> float4x2;
typedef matrix<float, 4, 3> float4x3;
typedef matrix<float, 4, 4> float4x4;

typedef matrix<half, 2, 2> half2x2;
typedef matrix<half, 2, 3> half2x3;
typedef matrix<half, 2, 4> half2x4;
typedef matrix<half, 3, 2> half3x2;
typedef matrix<half, 3, 3> half3x3;
typedef matrix<half, 3, 4> half3x4;
typedef matrix<half, 4, 2> half4x2;
typedef matrix<half, 4, 3> half4x3;
typedef matrix<half, 4, 4> half4x4;

// ============================================================================
// Address Space Qualifiers
// ============================================================================
// These are Clang builtins - defined here for reference
// __attribute__((address_space(1))) - device
// __attribute__((address_space(2))) - constant
// __attribute__((address_space(3))) - threadgroup
// __attribute__((address_space(0))) - thread (default)

// ============================================================================
// Texture Types (forward declarations - full in metal_texture.h)
// ============================================================================
template<typename T, access a = access::sample, bool r = false>
struct texture_1d;
template<typename T, access a = access::sample, bool r = false>
struct texture_2d;
template<typename T, access a = access::sample, bool r = false>
struct texture_3d;
template<typename T, access a = access::sample, bool r = false>
struct texture_cube;
template<typename T, access a = access::sample, bool r = false>
struct texture_2d_array;
template<typename T, access a = access::sample, bool r = false>
struct texture_cube_array;
template<typename T, access a = access::sample, bool r = false>
struct texture_2d_ms;
template<typename T, access a = access::sample, bool r = false>
struct texture_2d_ms_array;
template<typename T, access a = access::sample, bool r = false>
struct texture_depth_2d;
template<typename T, access a = access::sample, bool r = false>
struct texture_depth_cube;
template<typename T, access a = access::sample, bool r = false>
struct texture_depth_2d_array;
template<typename T, access a = access::sample, bool r = false>
struct texture_depth_cube_array;
template<typename T, access a = access::read>
struct texture_1d_array;
template<typename T, access a = access::read>
struct texture_2d_array_read;

// Buffer type
template<typename T>
struct buffer {
    T& operator[](size_t i);
    const T& operator[](size_t i) const;
    size_t size() const;
};

// Sampler
enum class min_filter { nearest, linear };
enum class mag_filter { nearest, linear };
enum class mip_filter { none, nearest, linear };
enum class address { clamp_to_zero, clamp_to_edge, repeat, mirrored_repeat };
enum class border_color { transparent_black, opaque_black, opaque_white };

struct sampler {
    sampler() = default;
    sampler(min_filter min, mag_filter mag, mip_filter mip,
            address addr = address::clamp_to_edge,
            border_color border = border_color::transparent_black);
};

// ============================================================================
// Function Qualifier Attributes
// ============================================================================
// These are applied via Clang attributes:
// __attribute__((vertex)) - vertex function
// __attribute__((fragment)) - fragment function
// __attribute__((kernel)) - kernel function
// __attribute__((intersection)) - intersection function (Metal 4.0+)
// __attribute__((object)) - object function (Metal 4.1+)
// __attribute__((mesh)) - mesh function (Metal 4.1+)

// ============================================================================
// Function Argument Attributes
// ============================================================================
// [[buffer(n)]] - bind to buffer n
// [[thread_position_in_grid]] - thread position in grid
// [[thread_position_in_threadgroup]] - thread position in threadgroup
// [[threadgroup_position_in_grid]] - threadgroup position in grid
// [[threads_per_threadgroup]] - threads per threadgroup
// [[thread_index_in_threadgroup]] - thread index in threadgroup
// [[thread_execution_width]] - thread execution width
// [[simdgroup_index_in_threadgroup]] - simd group index
// [[thread_index_in_simdgroup]] - thread index in simd group
// [[threads_per_simdgroup]] - threads per simd group
// [[stage_in]] - stage input data
// [[color(n)]] - fragment color input
// [[position]] - vertex position output
// [[point_size]] - point size output
// [[clip_distance]] - clip distance
// [[center_perspective]] - perspective correction
// [[centroid_perspective]] - centroid perspective
// [[sample_perspective]] - sample perspective
// [[flat]] - no interpolation
// [[perspective]] - perspective interpolation
// [[texture(n)]] - texture binding
// [[sampler(n)]] - sampler binding
// [[imageblock_data]] - imageblock data (Apple Silicon)
// [[max_total_threads_per_threadgroup]] - threadgroup size hint
// [[threads_per_threadgroup]] - required threadgroup size
// [[pixel_position]] - pixel position (fragment)
// [[point_coord]] - point coordinate (fragment)
// [[front_facing]] - front-facing flag (fragment)
// [[sample_id]] - sample ID (fragment)
// [[render_target_array_index]] - render target array index
// [[viewport_array_index]] - viewport array index
// [[early_fragment_tests]] - early fragment test
// [[quad_broadcast_thread_index]] - quad broadcast index

// ============================================================================
// Metal Constants
// ============================================================================
constexpr float M_PI_F = 3.14159265358979323846f;
constexpr float M_PI_2_F = 1.5707963267948966f;
constexpr float M_PI_4_F = 0.7853981633974483f;
constexpr float M_1_PI_F = 0.3183098861837907f;
constexpr float M_2_PI_F = 0.6366197723675813f;
constexpr float M_2_SQRTPI_F = 1.1283791670955126f;
constexpr float M_SQRT2_F = 1.4142135623730951f;
constexpr float M_SQRT1_2_F = 0.7071067811865475f;
constexpr float M_E_F = 2.718281828459045f;
constexpr float M_LOG2E_F = 1.4426950408889634f;
constexpr float M_LOG10E_F = 0.4342944819032518f;
constexpr float M_LN2_F = 0.6931471805599453f;
constexpr float M_LN10_F = 2.302585092994046f;

constexpr float MAXFLOAT = 3.4028234663852886e+38f;
constexpr float HUGE_VALF = __builtin_huge_valf();
constexpr float INFINITY = __builtin_inff();
constexpr float NAN = __builtin_nanf("");

// ============================================================================
// Memory Order (for atomics)
// ============================================================================
enum class memory_order {
    relaxed = __ATOMIC_RELAXED,
    acquire = __ATOMIC_ACQUIRE,
    release = __ATOMIC_RELEASE,
    acq_rel = __ATOMIC_ACQ_REL,
    seq_cst = __ATOMIC_SEQ_CST
};

enum class memory_scope {
    device = 1,
    threadgroup = 2,
    simdgroup = 3
};

// ============================================================================
// Access Qualifier (for textures)
// ============================================================================
enum class access {
    read,
    write,
    read_write,
    sample
};

// ============================================================================
// Rasterization Group
// ============================================================================
enum class interpolation {
    perspective,
    perspective_correct,
    flat,
    center_no_perspective,
    centroid_no_perspective,
    sample_no_perspective,
    center_perspective,
    centroid_perspective,
    sample_perspective
};

// ============================================================================
// Vertex/Fragment Attributes (Metal 4.0+)
// ============================================================================
enum class vertex_format {
    invalid = 0,
    uchar2, uchar4, char2, char4,
    uchar2_norm, uchar4_norm, char2_norm, char4_norm,
    ushort, ushort2, ushort4, short_, short2, short4,
    ushort_norm, ushort2_norm, ushort4_norm, short_norm, short2_norm, short4_norm,
    half, half2, half3, half4,
    float_, float2, float3, float4,
    int_, int2, int3, int4,
    uint, uint2, uint32, uint4,
    int1010102_norm, uint1010102_norm,
    uchar4_norm_bgra
};

// ============================================================================
// Pixel Format
// ============================================================================
enum class pixel_format {
    r8unorm, r8snorm, r8uint, r8sint,
    r16unorm, r16snorm, r16uint, r16sint, r16float,
    rg8unorm, rg8snorm, rg8uint, rg8sint,
    r32uint, r32sint, r32float,
    rg16unorm, rg16snorm, rg16uint, rg16sint, rg16float,
    rgba8unorm, rgba8snorm, rgba8uint, rgba8sint,
    bgra8unorm, bgra8unorm_srgb,
    rgb10a2, rg11b10f, rgb9e5,
    rg32uint, rg32sint, rg32float,
    rgba16unorm, rgba16snorm, rgba16uint, rgba16sint, rgba16float,
    rgba32uint, rgba32sint, rgba32float,
    depth16unorm, depth32float,
    stencil8,
    x24_stencil8, x32_stencil8,
    // Compressed formats
    pvrtc_rgb_2bpp, pvrtc_rgb_4bpp, pvrtc_rgba_2bpp, pvrtc_rgba_4bpp,
    etc2_rgb8, etc2_rgb8_srgb, etc2_rgb8a1, etc2_rgb8a1_srgb,
    eac_r11unorm, eac_r11snorm, eac_rg11unorm, eac_rg11snorm, eac_rgba8, eac_rgba8_srgb,
    astc_4x4_srgb, astc_5x4_srgb, astc_5x5_srgb, astc_6x5_srgb, astc_6x6_srgb,
    astc_8x5_srgb, astc_8x6_srgb, astc_8x8_srgb, astc_10x5_srgb, astc_10x6_srgb,
    astc_10x8_srgb, astc_10x10_srgb, astc_12x12_srgb,
    bc1_rgba, bc1_rgba_srgb, bc2_rgba, bc2_rgba_srgb, bc3_rgba, bc3_rgba_srgb,
    bc4_runorm, bc4_rsnorm, bc5_rgunorm, bc5_rgsnrom,
    bc6h_rgbfloat, bc6h_rgbsfloat, bc7_rgbaunorm, bc7_rgbaunorm_srgb
};

// ============================================================================
// Blend Factor/Operation (Metal 4.0+)
// ============================================================================
enum class blend_factor {
    zero, one,
    src_color, one_minus_src_color,
    src_alpha, one_minus_src_alpha,
    dst_color, one_minus_dst_color,
    dst_alpha, one_minus_dst_alpha,
    source_alpha_saturated,
    blend_color, one_minus_blend_color,
    blend_alpha, one_minus_blend_alpha
};

enum class blend_operation {
    add, subtract, reverse_subtract,
    min, max
};

} // namespace metal_types

#endif // __METAL_TYPES_H__
