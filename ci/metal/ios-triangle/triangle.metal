// Colourful triangle for the end-to-end pipeline test.
//
// Compiled by the fork's clang on Linux:
//   clang --target=air64-apple-ios26.0 -miphoneos-version-min=26.0 -x metal -std=metal3.2 \
//         -emit-llvm -c triangle.metal -o triangle.air
//
// then wrapped into a .metallib on macOS by llvm-metallib, and loaded by the
// unsigned iOS test app through -[MTLDevice makeLibraryWithURL:].
//
// The shader deliberately needs no <metal_stdlib>: the fork does not ship
// Apple's headers, and `uint`/float2/float4 are ordinary typedefs in that
// header. The declarations below are the same form clang/test/Metal uses.

typedef unsigned int uint;
typedef __attribute__((__ext_vector_type__(2))) float float2;
typedef __attribute__((__ext_vector_type__(4))) float float4;

struct VSOut {
  float4 position [[position]];
  float4 color;
};

vertex VSOut triangle_vertex(uint vid [[vertex_id]]) {
  const float2 tri[3] = {{-0.8f, -0.8f}, {0.8f, -0.8f}, {0.0f, 0.8f}};
  const float4 col[3] = {{1.0f, 0.0f, 0.0f, 1.0f},
                         {0.0f, 1.0f, 0.0f, 1.0f},
                         {0.0f, 0.0f, 1.0f, 1.0f}};
  float4 p = {tri[vid].x, tri[vid].y, 0.0f, 1.0f};

  VSOut o;
  o.position = p;
  o.color = col[vid];
  return o;
}

fragment float4 triangle_fragment(VSOut in [[stage_in]]) { return in.color; }
