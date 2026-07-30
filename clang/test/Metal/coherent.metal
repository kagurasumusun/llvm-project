// The `coherent(device)` memory coherence qualifier.
//
// Measured across the full AST corpus (8.26 GB, 109,945 dumps): the type is
// printed as `const device coherent(device) Uniforms &` (1,001 occurrences)
// and mangles with two vendor qualifiers back to back, the address space
// first and the coherence second:
//
//   _ZN8UniformsC1ERU9MTLdeviceU18MTLcoherent_deviceKS_
//   _ZNU14MTLthreadgroup8UniformsC1ERU9MTLdeviceU18MTLcoherent_deviceKS_
//
// Only `coherent(device)` occurs; the corpus contains no `coherent(threadgroup)`
// anywhere, and the qualifier is always written on top of a `device` one.
//
// RUN: %clang_cc1 -x metal -triple air64_v28-apple-macosx26.0.0 \
// RUN:   -std=metal3.2 -emit-llvm -no-opaque-pointers -o - %s | FileCheck %s
// RUN: %clang_cc1 -x metal -triple air64_v28-apple-macosx26.0.0 \
// RUN:   -std=metal3.2 -ast-dump %s | FileCheck --check-prefix=AST %s

struct Uniforms {
  float scale;
};

// AST: {{.*}}const device coherent(device) Uniforms &
// CHECK: define {{.*}}@_Z8consumerRU9MTLdeviceU18MTLcoherent_deviceK8Uniforms
float consumer(const device coherent(device) Uniforms &u) { return u.scale; }

// The coherence qualifier lowers to the same target address space as plain
// `device`, which is 1.
// CHECK: addrspace(1)
kernel void k(device float *out [[buffer(0)]],
              const device coherent(device) Uniforms &u [[buffer(1)]],
              unsigned int id [[thread_position_in_grid]]) {
  out[id] = consumer(u);
}
