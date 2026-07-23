// RUN: %clang_cc1 -triple air64_v22-apple-macosx10.15 -x metal -std=metal4.0 -emit-llvm -o - %s | FileCheck %s

struct VOut {
  float4 color [[flat]];
  half4 hcolor [[sample_no_perspective]];
  uint4 ids [[center_no_perspective]];
};

vertex VOut generated_type_table() {
  VOut r;
  return r;
}

// CHECK-DAG: !air.vertex = !{![[VERTEX:[0-9]+]]}
// CHECK-DAG: ![[VERTEX]] = !{ptr @{{.*}}generated_type_table{{.*}}, ![[OUTS:[0-9]+]], !{{[0-9]+}}}
// CHECK-DAG: ![[OUTS]] = !{![[C:[0-9]+]], ![[H:[0-9]+]], ![[U:[0-9]+]]}
// CHECK-DAG: ![[C]] = !{!"air.vertex_output", !"generated(5colorDv4_f)", !"air.flat", !"air.arg_type_name", !"float4", !"air.arg_name", !"color"}
// CHECK-DAG: ![[H]] = !{!"air.vertex_output", !"generated(6hcolorDv4_Dh)", !"air.sample", !"air.no_perspective", !"air.arg_type_name", !"half4", !"air.arg_name", !"hcolor"}
// CHECK-DAG: ![[U]] = !{!"air.vertex_output", !"generated(3idsDv4_j)", !"air.center", !"air.no_perspective", !"air.arg_type_name", !"uint4", !"air.arg_name", !"ids"}
