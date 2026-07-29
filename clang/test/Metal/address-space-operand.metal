// air.address_space is gated on the AIR version, not on -std.
//
// Established by sweeping every reference .ll on all four platforms and
// grouping by the _vNN suffix in the triple. The operand is absent for v20
// through v24 and present from v25 onwards, with no exception anywhere:
//
//   macOS   v20 10.13 no   v24 12.7 no   v25 13.7 YES  v28 26.0 YES
//   iOS     v20 11.4 no    v24 15.8 no   v25 16.7 YES  v28 26.0 YES
//   tvOS    v20 11.4 no    v24 15.8 no   v25 16.7 YES  v28 26.0 YES
//   watchOS v26 10.3 YES   v27 11.4 YES  v28 26.0 YES
//
// It follows the deployment target: the same source at -std=macos-metal1.1
// gains the operand once the deployment target moves from 10.13 to 26.0.
//
// RUN: %clang_cc1 -x metal -triple air64_v28-apple-macosx26.0.0 \
// RUN:   -std=metal3.2 -emit-llvm -no-opaque-pointers -o - %s \
// RUN:   | FileCheck --check-prefix=NEW %s
// RUN: %clang_cc1 -x metal -triple air64_v20-apple-macosx10.13.0 \
// RUN:   -std=macos-metal1.1 -emit-llvm -no-opaque-pointers -o - %s \
// RUN:   | FileCheck --check-prefix=OLD %s

// NEW: !"air.buffer", !"air.location_index", i32 0, i32 1, !"air.read_write", !"air.address_space", i32 1, !"air.arg_type_size", i32 4
// OLD: !"air.buffer", !"air.location_index", i32 0, i32 1, !"air.read_write", !"air.arg_type_size", i32 4
// OLD-NOT: air.address_space
kernel void k(device float *out [[buffer(0)]],
              unsigned int id [[thread_position_in_grid]]) {
  out[id] = 1.0f;
}
