// RUN: %clang_cc1 -triple air64_v30-apple-macosx15.0 -x metal -std=metal3.0 -emit-llvm -o - %s | FileCheck %s

struct Payload { int x; };

kernel void mesh_ray_attrs(device int *out [[buffer(0)]],
                           uint local [[local_index]],
                           uint ident [[id]],
                           Payload objectData [[object]],
                           __metal_mesh_t meshData [[mesh]],
                           Payload payloadData [[payload]],
                           Payload intersectionData [[intersection]],
                           visible_function_table table [[visible]]) {
  out[0] = local + ident;
}

// CHECK-DAG: !air.kernel = !{![[KERNEL:[0-9]+]]}
// CHECK-DAG: ![[KERNEL]] = !{ptr @{{.*}}mesh_ray_attrs{{.*}}, !{{[0-9]+}}, ![[ARGS:[0-9]+]]}
// CHECK-DAG: ![[ARGS]] = !{!{{[0-9]+}}, ![[LOCAL:[0-9]+]], ![[ID:[0-9]+]], ![[OBJECT:[0-9]+]], ![[MESH:[0-9]+]], ![[PAYLOAD:[0-9]+]], ![[INTERSECTION:[0-9]+]], ![[VISIBLE:[0-9]+]]}
// CHECK-DAG: ![[LOCAL]] = !{i32 1, !"air.local_index", !"air.arg_type_name", !"uint", !"air.arg_name", !"local"}
// CHECK-DAG: ![[ID]] = !{i32 2, !"air.id", !"air.arg_type_name", !"uint", !"air.arg_name", !"ident"}
// CHECK-DAG: ![[OBJECT]] = !{i32 3, !"air.object"
// CHECK-DAG: ![[MESH]] = !{i32 4, !"air.mesh"
// CHECK-DAG: ![[PAYLOAD]] = !{i32 5, !"air.payload"
// CHECK-DAG: ![[INTERSECTION]] = !{i32 6, !"air.intersection"
// CHECK-DAG: ![[VISIBLE]] = !{i32 7, !"air.visible"
