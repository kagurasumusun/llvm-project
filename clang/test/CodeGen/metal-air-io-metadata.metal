// RUN: %clang_cc1 -triple air64_v22-apple-macosx10.15 -x metal -std=macos-metal2.0 -emit-llvm -o - %s | FileCheck %s

struct VOut {
  float4 pos [[position]];
  int flatValue [[flat]];
  int userValue [[user(uid)]];
  float pointSize [[point_size]];
};

struct FOut {
  float4 color0 [[color(0)]];
  float depthValue [[depth(any)]];
};

vertex VOut vertex_io(device int *out [[buffer(0)]]) {
  VOut r;
  return r;
}

[[early_fragment_tests]] fragment FOut fragment_io() {
  FOut r;
  return r;
}

// CHECK-DAG: !air.vertex = !{![[VERTEX:[0-9]+]]}
// CHECK-DAG: !air.fragment = !{![[FRAG:[0-9]+]]}
// CHECK-DAG: ![[VERTEX]] = !{ptr @{{.*}}vertex_io{{.*}}, ![[VOUTS:[0-9]+]], !{{[0-9]+}}}
// CHECK-DAG: ![[VOUTS]] = !{![[POS:[0-9]+]], ![[FLAT:[0-9]+]], ![[USER:[0-9]+]], ![[PS:[0-9]+]]}
// CHECK-DAG: ![[POS]] = !{!"air.position", !"air.arg_type_name", !"float4", !"air.arg_name", !"pos"}
// CHECK-DAG: ![[FLAT]] = !{!"air.vertex_output", !"generated(9flatValuei)", !"air.flat", !"air.arg_type_name", !"int", !"air.arg_name", !"flatValue"}
// CHECK-DAG: ![[USER]] = !{!"air.vertex_output", !"user(uid)", !"air.arg_type_name", !"int", !"air.arg_name", !"userValue"}
// CHECK-DAG: ![[PS]] = !{!"air.point_size", !"air.arg_type_name", !"float", !"air.arg_name", !"pointSize"}
// CHECK-DAG: ![[FRAG]] = !{ptr @{{.*}}fragment_io{{.*}}, ![[FOUTS:[0-9]+]], !{{[0-9]+}}, !"early_fragment_tests"}
// CHECK-DAG: ![[FOUTS]] = !{![[COLOR:[0-9]+]], ![[DEPTH:[0-9]+]]}
// CHECK-DAG: ![[COLOR]] = !{!"air.render_target", i32 0, i32 0, !"air.arg_type_name", !"float4", !"air.arg_name", !"color0"}
// CHECK-DAG: ![[DEPTH]] = !{!"air.depth", !"air.depth_qualifier", !"air.any", !"air.arg_type_name", !"float", !"air.arg_name", !"depthValue"}
