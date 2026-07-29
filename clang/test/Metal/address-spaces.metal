// Metal address space keywords and their lowering to target address spaces.
//
// The target numbers are measured from Apple generated AIR; see
// research/spec/IR_GROUND_TRUTH.md section 2.4 and the golden corpus:
//   thread 0, device 1, constant 2, threadgroup 3,
//   threadgroup_imageblock 4, object_data 7, ray_data 9
//
// RUN: %clang_cc1 -x metal -triple air64_v28-apple-macosx26.0.0 \
// RUN:   -std=metal4.0 -emit-llvm -no-opaque-pointers -o - %s | FileCheck %s

kernel void as_kernel(device int *d [[buffer(0)]],
                      constant int *c [[buffer(1)]],
                      threadgroup int *t [[threadgroup(0)]]) {
  // CHECK: define void @as_kernel(
  // CHECK-SAME: i32 addrspace(1)*
  // CHECK-SAME: i32 addrspace(2)*
  // CHECK-SAME: i32 addrspace(3)*
  *d = *c + *t;
}

// `thread` is the default address space and is therefore unqualified.
// CHECK-LABEL: define {{.*}}@thread_ptr
int thread_load(thread int *p) { return *p; }
