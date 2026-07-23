// RUN: %clang_cc1 -triple air64_v22-apple-macosx10.15 -x metal -std=macos-metal2.0 -emit-llvm -o - %s | FileCheck %s

kernel void k(device int *out [[buffer(0)]],
              constant int *in [[buffer(1)]],
              unsigned int gid [[thread_position_in_grid]]) {
  out[gid] = in[0];
}

// CHECK: target triple = "air64_v22-apple-macosx10.15"
// CHECK: define{{.*}} void @k(
// CHECK: !air.kernel = !{![[KERNEL:[0-9]+]]}
// CHECK-DAG: !air.version = !{![[AIRVER:[0-9]+]]}
// CHECK-DAG: !air.language_version = !{![[LANGVER:[0-9]+]]}
// CHECK-DAG: !air.compile_options = !{!{{[0-9]+}}, !{{[0-9]+}}, !{{[0-9]+}}}
// CHECK-DAG: !air.source_file_name = !{!{{[0-9]+}}}
// CHECK-DAG: ![[KERNEL]] = !{ptr @k, !{{[0-9]+}}, ![[ARGS:[0-9]+]]}
// CHECK-DAG: ![[ARGS]] = !{![[OUT:[0-9]+]], ![[IN:[0-9]+]], ![[GID:[0-9]+]]}
// CHECK-DAG: ![[OUT]] = !{i32 0, !"air.buffer", !"air.location_index", i32 0, i32 1, !"air.read_write"
// CHECK-DAG: ![[IN]] = !{i32 1, !"air.buffer", !"air.location_index", i32 1, i32 1, !"air.read"
// CHECK-DAG: ![[GID]] = !{i32 2, !"air.thread_position_in_grid", !"air.arg_type_name", !"uint", !"air.arg_name", !"gid"}
// CHECK-DAG: ![[AIRVER]] = !{i32 2, i32 2, i32 0}
// CHECK-DAG: ![[LANGVER]] = !{!"Metal", i32 2, i32 0, i32 0}
