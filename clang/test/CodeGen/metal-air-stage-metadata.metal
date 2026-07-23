// RUN: %clang_cc1 -triple air64_v22-apple-macosx10.15 -x metal -std=macos-metal2.0 -emit-llvm -o - %s | FileCheck %s

struct VertexIn { int x; };

vertex void v(VertexIn in [[stage_in]], device int *out [[buffer(0)]]) {
  out[0] = in.x;
}

fragment void f(VertexIn in [[stage_in]], device int *out [[buffer(1)]]) {
  out[0] = in.x;
}

// CHECK: define{{.*}} void @{{.*}}v{{.*}}(
// CHECK: define{{.*}} void @{{.*}}f{{.*}}(
// CHECK-DAG: !air.vertex = !{![[VERTEX:[0-9]+]]}
// CHECK-DAG: !air.fragment = !{![[FRAG:[0-9]+]]}
// CHECK-DAG: ![[VERTEX]] = !{ptr @{{.*}}v{{.*}}, !{{[0-9]+}}, ![[VARGS:[0-9]+]]}
// CHECK-DAG: ![[VARGS]] = !{![[VIN:[0-9]+]], ![[VOUT:[0-9]+]]}
// CHECK-DAG: ![[VIN]] = !{i32 0, !"air.vertex_input", !"air.location_index", i32 0, i32 1
// CHECK-DAG: ![[VOUT]] = !{i32 1, !"air.buffer", !"air.location_index", i32 0, i32 1, !"air.read_write"
// CHECK-DAG: ![[FRAG]] = !{ptr @{{.*}}f{{.*}}, !{{[0-9]+}}, ![[FARGS:[0-9]+]]}
// CHECK-DAG: ![[FARGS]] = !{![[FIN:[0-9]+]], ![[FOUT:[0-9]+]]}
// CHECK-DAG: ![[FIN]] = !{i32 0, !"air.fragment_input", !"air.location_index", i32 0, i32 1
// CHECK-DAG: ![[FOUT]] = !{i32 1, !"air.buffer", !"air.location_index", i32 1, i32 1, !"air.read_write"
