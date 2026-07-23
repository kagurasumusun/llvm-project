// RUN: %clang_cc1 -triple air64_v22-apple-macosx10.15 -x metal -std=macos-metal2.0 -emit-llvm -o - %s | FileCheck %s

kernel void ref_buffer(constant int &in [[buffer(1)]], device int *out [[buffer(0)]]) {
  out[0] = in;
}

// CHECK-DAG: !air.kernel = !{![[K:[0-9]+]]}
// CHECK-DAG: ![[K]] = !{ptr @{{.*}}ref_buffer{{.*}}, !{{[0-9]+}}, ![[ARGS:[0-9]+]]}
// CHECK-DAG: ![[ARGS]] = !{![[IN:[0-9]+]], ![[OUT:[0-9]+]]}
// CHECK-DAG: ![[IN]] = !{i32 0, !"air.buffer", !"air.location_index", i32 1, i32 1, !"air.read", !"air.arg_type_size", i32 4, !"air.arg_type_align_size", i32 4, !"air.arg_type_name", !"int", !"air.arg_name", !"in"}
