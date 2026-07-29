// Numeric conversions lower to air.convert.* rather than to LLVM cast
// instructions.
//
// Measured: research/golden/P02 contains, for `float(vid)` on a uint,
//   %6 = tail call fast float @air.convert.f.f32.u.i32(i32 %5)
// The naming is air.convert.<dstKind>.<dstTy>.<srcKind>.<srcTy> with kinds
// f/s/u. All 57 variants seen in the reference corpus are listed in
// docs-metal/data/air_convert_variants.txt.
//
// RUN: %clang_cc1 -x metal -triple air64_v28-apple-macosx26.0.0 \
// RUN:   -std=metal3.2 -emit-llvm -no-opaque-pointers -o - %s | FileCheck %s

typedef unsigned int uint;
typedef __attribute__((__ext_vector_type__(4))) float float4;
typedef __attribute__((__ext_vector_type__(4))) int int4;

kernel void conv(device float *out [[buffer(0)]],
                 uint i [[thread_position_in_grid]]) {
  // CHECK: call{{.*}}@air.convert.f.f32.u.i32
  out[0] = (float)i;
}

kernel void conv_signed(device float *out [[buffer(0)]],
                        device int *in [[buffer(1)]]) {
  // CHECK: call{{.*}}@air.convert.f.f32.s.i32
  out[0] = (float)in[0];
}

kernel void conv_to_int(device int *out [[buffer(0)]],
                        device float *in [[buffer(1)]]) {
  // CHECK: call{{.*}}@air.convert.s.i32.f.f32
  out[0] = (int)in[0];
}

kernel void conv_vector(device float4 *out [[buffer(0)]],
                        device int4 *in [[buffer(1)]]) {
  // CHECK: call{{.*}}@air.convert.f.v4f32.s.v4i32
  out[0] = (float4)in[0];
}
