// Entry point alias scopes: the "air-buffer-no-alias" parameter attribute
// and the per-function air-alias-scopes domain, wired through every memory
// access as !alias.scope / !noalias.
//
// The schema is transcribed from golden P01
// (research/golden/P01/metal32_macosx26/probe.ll), which measures a kernel
// with a device buffer, a constant struct reference, a texture, a sampler
// and a threadgroup buffer:
//
//   define void @probe_p01_kernel(
//       float addrspace(1)* nocapture noundef "air-buffer-no-alias" %0,
//       %struct.Params addrspace(2)* nocapture noundef readonly align 16
//           dereferenceable(32) "air-buffer-no-alias" %1,
//       %struct._texture_2d_t addrspace(1)* %2,
//       %struct._sampler_t addrspace(2)* nocapture readonly %3,
//       float addrspace(3)* nocapture noundef "air-buffer-no-alias" %4,
//       i32 noundef %5)
//   ...
//   !31 = distinct !{!31, !32, !"air-alias-scope-arg(0)"}
//   !32 = distinct !{!32, !"air-alias-scopes(probe_p01_kernel)"}
//   !35 = distinct !{!35, !32, !"air-alias-scope-textures"}
//   !36 = distinct !{!36, !32, !"air-alias-scope-samplers"}
//   !37 = distinct !{!37, !32, !"air-alias-scope-arg(4)"}
//
// Texture and sampler handles never carry the attribute; loads/stores trace
// back through GEPs and the unoptimised parameter-promotion alloca to the
// argument they access.
//
// RUN: %clang_cc1 -x metal -triple air64_v28-apple-macosx26.0.0 \
// RUN:   -std=metal3.2 -emit-llvm -no-opaque-pointers -discard-value-names \
// RUN:   -o - %s | FileCheck %s

typedef unsigned int uint;
typedef __attribute__((__ext_vector_type__(4))) float float4;

namespace metal {
enum class access { sample, read, write, read_write };

template <typename T, access a = access::sample>
struct texture2d {
  int handle;
};

struct sampler {
  int handle;
};
}
using namespace metal;

struct Params {
  float4 member;
};

// CHECK: define void @probe(
// CHECK-SAME: "air-buffer-no-alias" %0
// CHECK-SAME: "air-buffer-no-alias" %1
// CHECK-SAME: "air-buffer-no-alias" %4
// CHECK-NOT: "air-buffer-no-alias" %2
// CHECK-NOT: "air-buffer-no-alias" %3
kernel void probe(device float *o [[buffer(0)]],
                  constant Params &p [[buffer(1)]],
                  texture2d<float> t [[texture(0)]],
                  sampler s [[sampler(0)]],
                  threadgroup float *tg [[threadgroup(0)]],
                  uint i [[thread_position_in_grid]]) {
  // CHECK: load float, float addrspace(1)* %{{[0-9]+}}{{.*}}, !alias.scope ![[AS0:[0-9]+]], !noalias ![[NA0:[0-9]+]]
  float x = o[i];
  // CHECK: load <4 x float>, <4 x float> addrspace(2)* %{{[0-9]+}}{{.*}}, !alias.scope ![[AS1:[0-9]+]], !noalias ![[NA1:[0-9]+]]
  float4 member = p.member;
  // CHECK: store float {{.*}}, float addrspace(3)* %{{[0-9]+}}{{.*}}, !alias.scope ![[AST:[0-9]+]], !noalias ![[NAT:[0-9]+]]
  tg[i] = x + member.x;
  // CHECK: load float, float addrspace(3)* %{{[0-9]+}}{{.*}}, !alias.scope ![[AST]], !noalias ![[NAT]]
  // CHECK: store float {{.*}}, float addrspace(1)* %{{[0-9]+}}{{.*}}, !alias.scope ![[AS0]], !noalias ![[NA0]]
  o[i] = tg[i];
}

// The domain and its scopes, with the fixed creation order
// (arg buffers first, textures, samplers): the !noalias operand order in
// the golden output is exactly this order with the access's own scope
// filtered out.
//
// CHECK-DAG: ![[SA0:[0-9]+]] = distinct !{![[SA0]], ![[DOM:[0-9]+]], !"air-alias-scope-arg(0)"}
// CHECK-DAG: ![[SA1:[0-9]+]] = distinct !{![[SA1]], ![[DOM]], !"air-alias-scope-arg(1)"}
// CHECK-DAG: ![[STX:[0-9]+]] = distinct !{![[STX]], ![[DOM]], !"air-alias-scope-textures"}
// CHECK-DAG: ![[SSM:[0-9]+]] = distinct !{![[SSM]], ![[DOM]], !"air-alias-scope-samplers"}
// CHECK-DAG: ![[SA4:[0-9]+]] = distinct !{![[SA4]], ![[DOM]], !"air-alias-scope-arg(4)"}
// CHECK-DAG: ![[DOM]] = distinct !{![[DOM]], !"air-alias-scopes(probe)"}
//
// CHECK-DAG: ![[AS0]] = !{![[SA0]]}
// CHECK-DAG: ![[NA0]] = !{![[SA1]], ![[STX]], ![[SSM]], ![[SA4]]}
// CHECK-DAG: ![[AS1]] = !{![[SA1]]}
// CHECK-DAG: ![[NA1]] = !{![[SA0]], ![[STX]], ![[SSM]], ![[SA4]]}
// CHECK-DAG: ![[AST]] = !{![[SA4]]}
// CHECK-DAG: ![[NAT]] = !{![[SA0]], ![[SA1]], ![[STX]], ![[SSM]]}

// A single buffer argument has no other scope to exclude, and the measured
// output then carries no !noalias at all (multi_entry_with_helpers).
//
// CHECK: define void @solo(
// CHECK-SAME: "air-buffer-no-alias" %0
kernel void solo(device int *b [[buffer(0)]],
                 uint i [[thread_position_in_grid]]) {
  // CHECK: store i32 1, i32 addrspace(1)* %{{[0-9]+}}{{.*}}, !alias.scope ![[SAS0:[0-9]+]]
  // CHECK-NOT: !noalias
  b[i] = 1;
}
// CHECK-DAG: ![[SAS0]] = !{![[SB0:[0-9]+]]}
// CHECK-DAG: ![[SB0]] = distinct !{![[SB0]], ![[SDOM:[0-9]+]], !"air-alias-scope-arg(0)"}
// CHECK-DAG: ![[SDOM]] = distinct !{![[SDOM]], !"air-alias-scopes(solo)"}
