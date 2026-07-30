// Four builtins lower to the runtime library, not to an AIR intrinsic.
//
// research/datasets/builtin_to_air_map.v2.csv records these with an `rtlib:`
// prefix and marks the mapping as established by following the call graph in
// Apple's shipping rtlib. For two of them it says outright that the
// plausible-looking AIR name is wrong:
//
//   __metal_nextafter  rtlib:__air_impl_nextafter
//       "AIR intrinsic ではなく __air_impl_nextafter 系呼出に lowering
//        (callgraph 実証; v1 候補 air.nextafter.f16 は偽)"
//   __metal_os_log     rtlib:__air_impl_os_log
//   __metal_frexp      rtlib:___metal_frexp_float_pthreadint32
//   __metal_ilogb      rtlib:___metal_ilogb_float
//
// The prefix is a marker, not part of the symbol, so it has to be stripped
// and the remaining name used verbatim -- no `air.` and no type suffix.
//
// RUN: %clang_cc1 -x metal -triple air64_v28-apple-macosx26.0.0 \
// RUN:   -std=metal3.2 -emit-llvm -no-opaque-pointers -o - %s | FileCheck %s

// CHECK-LABEL: define void @k(
// CHECK: call{{.*}}@__air_impl_nextafter
// CHECK-NOT: @rtlib:
// CHECK-NOT: @air.nextafter
kernel void k(device float *out [[buffer(0)]],
              unsigned int id [[thread_position_in_grid]]) {
  out[id] = __metal_nextafter(out[id], 1.0f);
}
