// RUN: %clang_cc1 -triple air64-apple-macosx26.0 -x metal -std=metal4.0 -fsyntax-only %s

struct VertexIn {
  float4 pos [[attribute(0)]];
  float4 normal [[attribute(1), user(surface_normal)]];
};

struct VertexOut {
  float4 pos [[position]];
  float point [[point_size]];
  uint layer [[render_target_array_index]];
  uint viewport [[viewport_array_index]];
  float4 color [[color(0), center_perspective]];
  float4 color2 [[flat, centroid_no_perspective]];
  float4 color3 [[sample_perspective]];
};

struct FragmentOut {
  float4 color [[color(0)]];
  float depth [[depth(any)]];
};

vertex VertexOut io_vertex(VertexIn in [[stage_in]]) {
  VertexOut out;
  return out;
}

fragment FragmentOut io_fragment(VertexOut in [[stage_in]], float4 p [[position]]) {
  FragmentOut out;
  return out;
}

fragment void early_fragment_tests_ok() [[early_fragment_tests]] {}
