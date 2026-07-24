// This test is run by metal-clang-smoke-v6.yml with -I pointing at
// Apple Metal headers fetched from kagurasumusun/metal-info.  It intentionally
// checks the real Apple <metal_stdlib> path rather than Clang's lightweight
// bootstrap prelude.

#include <metal_stdlib>

kernel void apple_stdlib_include_smoke(device float *out [[buffer(0)]]) {
  out[0] = 1.0f;
}
