// RUN: %clang_cc1 -triple air64_v22-apple-macosx10.15 -x metal -std=metal2.0 -emit-llvm -o - %s | FileCheck %s

vertex void v(uint vid [[vertex_id]], uint iid [[instance_id]], uint basev [[base_vertex]], uint basei [[base_instance]]) {}
fragment void f(bool front [[front_facing]], uint sid [[sample_id]], uint mask [[sample_mask]], uint pid [[primitive_id]]) {}
kernel void k(uint3 tg [[threadgroup_position_in_grid]], uint3 gridsz [[threads_per_grid]], uint lane [[thread_index_in_simdgroup]], uint simdidx [[simdgroup_index_in_threadgroup]], uint simds [[simdgroups_per_threadgroup]]) {}

// CHECK-DAG: !"air.vertex_id"
// CHECK-DAG: !"air.instance_id"
// CHECK-DAG: !"air.base_vertex"
// CHECK-DAG: !"air.base_instance"
// CHECK-DAG: !"air.front_facing"
// CHECK-DAG: !"air.sample_id"
// CHECK-DAG: !"air.sample_mask"
// CHECK-DAG: !"air.primitive_id"
// CHECK-DAG: !"air.threadgroup_position_in_grid"
// CHECK-DAG: !"air.threads_per_grid"
// CHECK-DAG: !"air.thread_index_in_simdgroup"
// CHECK-DAG: !"air.simdgroup_index_in_threadgroup"
// CHECK-DAG: !"air.simdgroups_per_threadgroup"
