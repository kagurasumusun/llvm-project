// air.struct_type_info.
//
// The corpus records one five element group per field, repeated inline:
//
//   !51 = !{i32 0, !"air.buffer", !"air.location_index", i32 0, i32 1,
//           !"air.read_write", !"air.struct_type_info", !52,
//           !"air.arg_type_size", i32 4, !"air.arg_type_align_size", i32 4,
//           !"air.arg_type_name", !"AddressBox", !"air.arg_name", !"dev"}
//   !52 = !{i32 0, i32 4, i32 0, !"int", !"value"}
//
// i.e. {byte offset, byte size, 0, MSL type name, field name}. The third
// element is 0 in all 4,853 occurrences. Only record pointees carry the
// operand; `device float *` never does.
//
// RUN: %clang_cc1 -x metal -triple air64_v28-apple-macosx26.0.0 \
// RUN:   -std=metal3.2 -emit-llvm -no-opaque-pointers -o - %s | FileCheck %s

typedef __attribute__((__ext_vector_type__(4))) float float4;

struct AddressBox {
  int value;
};

struct Uniforms {
  float4 tint;
  float scale;
  unsigned int count;
};

// CHECK-DAG: !{i32 0, i32 4, i32 0, !"int", !"value"}
// CHECK-DAG: !{i32 0, i32 16, i32 0, !"float4", !"tint", i32 16, i32 4, i32 0, !"float", !"scale", i32 20, i32 4, i32 0, !"uint", !"count"}
// CHECK-DAG: !"air.struct_type_info"
kernel void k(device AddressBox *box [[buffer(0)]],
              constant Uniforms &u [[buffer(1)]],
              device float *plain [[buffer(2)]],
              unsigned int id [[thread_position_in_grid]]) {
  plain[id] = (float)box->value * u.scale;
}
