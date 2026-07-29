// Vertex and fragment output metadata.
//
// Transcribed from research/golden/P02:
//   !10 = !{!11, !12}
//   !11 = !{!"air.position", !"air.arg_type_name", !"float4",
//           !"air.arg_name", !"pos"}
//   !12 = !{!"air.vertex_output", !"generated(2uvDv2_f)",
//           !"air.arg_type_name", !"float2", !"air.arg_name", !"uv"}
//   !20 = !{!"air.render_target", i32 0, i32 0,
//           !"air.arg_type_name", !"float4"}
//
// The generated(...) id is <strlen(name)><name><itanium type>, so a float2
// named uv becomes 2uvDv2_f. The fragment input repeats the same string, which
// is how the runtime pairs the stages.
//
// RUN: %clang_cc1 -x metal -triple air64_v28-apple-macosx26.0.0 \
// RUN:   -std=metal3.2 -emit-llvm -no-opaque-pointers -o - %s | FileCheck %s

typedef __attribute__((__ext_vector_type__(4))) float float4;
typedef __attribute__((__ext_vector_type__(2))) float float2;

struct VSOut {
  float4 pos [[position]];
  float2 uv;
};

vertex VSOut v(uint vid [[vertex_id]]) {
  VSOut o;
  return o;
}

fragment float4 f(VSOut in [[stage_in]]) { return in.pos; }

// The vertex output list names the position and the user output, the latter
// carrying its generated connection id.
// CHECK-DAG: !{!"air.position", !"air.arg_type_name", !"float4", !"air.arg_name", !"pos"}
// CHECK-DAG: !{!"air.vertex_output", !"generated(2uvDv2_f)"

// The fragment writes a single unnamed colour attachment.
// CHECK-DAG: !{!"air.render_target", i32 0, i32 0, !"air.arg_type_name", !"float4"}
