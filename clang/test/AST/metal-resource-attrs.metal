// RUN: %clang_cc1 -triple air64-apple-macosx26.0 -x metal -std=metal4.0 -ast-dump -ast-dump-filter=resource_attrs %s | FileCheck %s

struct VertexIn { float x; };

kernel void resource_attrs(device int *out [[buffer(0)]],
                           constant int *in [[buffer(1)]],
                           unsigned int gid [[thread_position_in_grid]],
                           unsigned int tid [[thread_index_in_threadgroup]],
                           unsigned int tpg [[threads_per_threadgroup]]) {
  out[gid] = in[0] + tid + tpg;
}

vertex void resource_attrs_vertex(VertexIn in [[stage_in]],
                                  device int *out [[buffer(2)]]) {
  out[0] = 1;
}

// CHECK: FunctionDecl {{.*}} resource_attrs
// CHECK-DAG: DeviceKernelAttr
// CHECK-DAG: MetalBufferAttr {{.*}} 0
// CHECK-DAG: MetalBufferAttr {{.*}} 1
// CHECK-DAG: MetalThreadPositionInGridAttr
// CHECK-DAG: MetalThreadIndexInThreadgroupAttr
// CHECK-DAG: MetalThreadsPerThreadgroupAttr
// CHECK: FunctionDecl {{.*}} resource_attrs_vertex
// CHECK-DAG: MetalVertexAttr
// CHECK-DAG: MetalStageInAttr
// CHECK-DAG: MetalBufferAttr {{.*}} 2
