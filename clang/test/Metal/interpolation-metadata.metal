// Interpolation qualifiers, depth qualifiers and stage_in flattening.
//
// All of the expected metadata below is transcribed verbatim from the
// reference corpus, established by a full sweep of every .ll it contains
// (129,323 files, 10,831,381 lines, docs-metal/data/fullscan/ir_airmd_raw.txt):
//
//   !{i32 2, !"air.fragment_input", !"generated(4v_cpDv4_f)",
//     !"air.center", !"air.perspective",
//     !"air.arg_type_name", !"float4", !"air.arg_name", !"v_cp"}
//   !{i32 1, !"air.fragment_input", !"generated(6v_flatDv4_f)", !"air.flat",
//     !"air.arg_type_name", !"float4", !"air.arg_name", !"v_flat"}
//   !{i32 0, !"air.position", !"air.center", !"air.no_perspective",
//     !"air.arg_type_name", !"float4", !"air.arg_name", !"position"}
//   !{!"air.depth", !"air.depth_qualifier", !"air.any",
//     !"air.arg_type_name", !"float", !"air.arg_name", !"d"}
//
// RUN: %clang_cc1 -x metal -triple air64_v28-apple-macosx26.0.0 \
// RUN:   -std=metal3.2 -emit-llvm -no-opaque-pointers -o - %s | FileCheck %s

typedef __attribute__((__ext_vector_type__(4))) float float4;

struct FragIn {
  float4 position [[position]];
  float4 v_flat [[flat]];
  float4 v_cp [[center_perspective]];
  float4 v_cnp [[center_no_perspective]];
  float4 v_ctp [[centroid_perspective]];
  float4 v_ctnp [[centroid_no_perspective]];
  float4 v_sp [[sample_perspective]];
  float4 v_snp [[sample_no_perspective]];
};

struct FragOut {
  float4 color [[color(0)]];
  float d [[depth(any)]];
};

// A [[stage_in]] struct is flattened into one node per field, and the
// argument index keeps counting across the fields.
//
// CHECK-DAG: !{i32 0, !"air.position", !"air.center", !"air.no_perspective", !"air.arg_type_name", !"float4", !"air.arg_name", !"position"}
// CHECK-DAG: !{i32 1, !"air.fragment_input", !"generated(6v_flatDv4_f)", !"air.flat", !"air.arg_type_name", !"float4", !"air.arg_name", !"v_flat"}
// CHECK-DAG: !{i32 2, !"air.fragment_input", !"generated(4v_cpDv4_f)", !"air.center", !"air.perspective", !"air.arg_type_name", !"float4", !"air.arg_name", !"v_cp"}
// CHECK-DAG: !{i32 3, !"air.fragment_input", !"generated(5v_cnpDv4_f)", !"air.center", !"air.no_perspective", !"air.arg_type_name", !"float4", !"air.arg_name", !"v_cnp"}
// CHECK-DAG: !{i32 4, !"air.fragment_input", !"generated(5v_ctpDv4_f)", !"air.centroid", !"air.perspective", !"air.arg_type_name", !"float4", !"air.arg_name", !"v_ctp"}
// CHECK-DAG: !{i32 5, !"air.fragment_input", !"generated(6v_ctnpDv4_f)", !"air.centroid", !"air.no_perspective", !"air.arg_type_name", !"float4", !"air.arg_name", !"v_ctnp"}
// CHECK-DAG: !{i32 6, !"air.fragment_input", !"generated(4v_spDv4_f)", !"air.sample", !"air.perspective", !"air.arg_type_name", !"float4", !"air.arg_name", !"v_sp"}
// CHECK-DAG: !{i32 7, !"air.fragment_input", !"generated(5v_snpDv4_f)", !"air.sample", !"air.no_perspective", !"air.arg_type_name", !"float4", !"air.arg_name", !"v_snp"}
//
// CHECK-DAG: !{!"air.depth", !"air.depth_qualifier", !"air.any", !"air.arg_type_name", !"float", !"air.arg_name", !"d"}
fragment FragOut frag(FragIn in [[stage_in]]) {
  FragOut o;
  o.color = in.v_cp + in.v_flat + in.position + in.v_cnp + in.v_ctp +
            in.v_ctnp + in.v_sp + in.v_snp;
  o.d = 0.5f;
  return o;
}
