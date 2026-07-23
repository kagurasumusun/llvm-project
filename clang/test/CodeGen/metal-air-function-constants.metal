// RUN: %clang_cc1 -triple air64_v22-apple-macosx10.15 -x metal -std=macos-metal2.0 -emit-llvm -o - %s | FileCheck %s

constant bool fc_bool [[function_constant(0)]];
constant int fc_int [[function_constant(1)]];

kernel void k(device int *out [[buffer(0)]]) {
  out[0] = fc_bool ? fc_int : 0;
}

// CHECK-DAG: !air.function_constants = !{![[FC0:[0-9]+]], ![[FC1:[0-9]+]]}
// CHECK-DAG: ![[FC0]] = !{ptr addrspace(2) @{{.*}}fc_bool{{.*}}, !"bool", !"fc_bool", i32 0}
// CHECK-DAG: ![[FC1]] = !{ptr addrspace(2) @{{.*}}fc_int{{.*}}, !"int", !"fc_int", i32 1}
