// RUN: %clang_cc1 -triple air64_v22-apple-macosx10.15 -x metal -std=macos-metal2.0 -emit-llvm -o - %s | FileCheck %s

struct VertexIn {
  int position [[attribute(0)]];
  int normal [[attribute(1)]];
  int shade [[user(shade_id), flat]];
};

vertex void stage_in_struct(VertexIn in [[stage_in]], device int *out [[buffer(0)]]) {
  out[0] = in.position + in.normal + in.shade;
}

// CHECK-DAG: !air.vertex = !{![[VERTEX:[0-9]+]]}
// CHECK-DAG: ![[VERTEX]] = !{ptr @{{.*}}stage_in_struct{{.*}}, !{{[0-9]+}}, ![[ARGS:[0-9]+]]}
// CHECK-DAG: ![[ARGS]] = !{![[P:[0-9]+]], ![[N:[0-9]+]], ![[S:[0-9]+]], ![[B:[0-9]+]]}
// CHECK-DAG: ![[P]] = !{i32 0, !"air.vertex_input", !"air.location_index", i32 0, i32 1, !"air.arg_type_name", !"int", !"air.arg_name", !"position"}
// CHECK-DAG: ![[N]] = !{i32 1, !"air.vertex_input", !"air.location_index", i32 1, i32 1, !"air.arg_type_name", !"int", !"air.arg_name", !"normal"}
// CHECK-DAG: ![[S]] = !{i32 2, !"air.vertex_input", !"user(shade_id)", !"air.flat", !"air.arg_type_name", !"int", !"air.arg_name", !"shade"}
// CHECK-DAG: ![[B]] = !{i32 3, !"air.buffer", !"air.location_index", i32 0, i32 1, !"air.read_write"
