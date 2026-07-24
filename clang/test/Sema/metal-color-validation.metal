// RUN: %clang_cc1 -triple air64-apple-macosx26.0 -x metal -std=metal4.0 -fsyntax-only -verify %s

struct GoodColor { int c [[color(7)]]; };
struct BadColor { int c [[color(8)]]; }; // expected-error {{'color' attribute requires integer constant between 0 and 7 inclusive}}

struct DuplicateColorOut {
  int c0 [[color(0)]];
  int c1 [[color(0)]]; // expected-error {{'color' attribute index 0 is already used by another stage output field}}
};
fragment DuplicateColorOut duplicate_color_output() {
  DuplicateColorOut r;
  return r;
}

struct DuplicateDepthOut {
  float d0 [[depth(any)]];
  float d1 [[depth(greater)]]; // expected-error {{'depth' attribute is already used by another stage output field}}
};
fragment DuplicateDepthOut duplicate_depth_output() {
  DuplicateDepthOut r;
  return r;
}
struct BadFragmentDepthType {
  int depth [[depth(any)]]; // expected-error {{'depth' attribute requires field type float}}
};
fragment BadFragmentDepthType bad_fragment_depth_type() {
  BadFragmentDepthType r;
  return r;
}

struct BadVertexOutputTypes {
  int pos [[position]]; // expected-error {{'position' attribute requires field type float4}}
  int point [[point_size]]; // expected-error {{'point_size' attribute requires field type float}}
  int rta [[render_target_array_index]]; // expected-error {{'render_target_array_index' attribute requires field type uint}}
  int viewport [[viewport_array_index]]; // expected-error {{'viewport_array_index' attribute requires field type uint}}
};
vertex BadVertexOutputTypes bad_vertex_output_types() {
  BadVertexOutputTypes r;
  return r;
}

struct BadVertexFragmentOnlyOutput {
  float4 color [[color(0)]]; // expected-error {{'color' attribute cannot be used on vertex output fields}}
  float depth [[depth(any)]]; // expected-error {{'depth' attribute cannot be used on vertex output fields}}
};
vertex BadVertexFragmentOnlyOutput bad_vertex_fragment_only_output() {
  BadVertexFragmentOnlyOutput r;
  return r;
}

struct BadFragmentVertexOnlyOutput {
  float4 pos [[position]]; // expected-error {{'position' attribute cannot be used on fragment output fields}}
  float point [[point_size]]; // expected-error {{'point_size' attribute cannot be used on fragment output fields}}
};
fragment BadFragmentVertexOnlyOutput bad_fragment_vertex_only_output() {
  BadFragmentVertexOnlyOutput r;
  return r;
}
