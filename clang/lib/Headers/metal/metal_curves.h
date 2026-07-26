#ifndef _METAL_CURVES_H_
#define _METAL_CURVES_H_
#include <metal/metal_common>
namespace metal {
struct curve_basis { static constexpr uint catmull_rom = 0; static constexpr uint bspline = 1; static constexpr uint linear = 2; };
} // namespace metal
#endif
