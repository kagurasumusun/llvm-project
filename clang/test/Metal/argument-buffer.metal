// Argument buffers: air.indirect_buffer / air.indirect_argument /
// air.indirect_constant.
//
// A struct whose fields carry [[id(N)]] is an argument buffer. It binds as
// air.indirect_buffer, and its air.struct_type_info gains two extra operands
// per field: the string "air.indirect_argument" and a nested node describing
// the field as if it were a top level entry point argument. Transcribed from
// multi_entry_shared_argbuf and determ_arg_buffer:
//
//   !13 = !{i32 1, !"air.indirect_buffer", !"air.buffer_size", i32 32,
//           !"air.location_index", i32 1, i32 1, !"air.read",
//           !"air.address_space", i32 2, !"air.struct_type_info", !14,
//           !"air.arg_type_size", i32 32, !"air.arg_type_align_size", i32 8,
//           !"air.arg_type_name", !"SharedArgs", !"air.arg_name", !"args"}
//   !14 = !{i32 0, i32 8, i32 0, !"float", !"data",
//           !"air.indirect_argument", !15, ...}
//   !15 = !{i32 0, !"air.buffer", !"air.location_index", i32 0, i32 1, ...}
//   !16 = !{i32 1, !"air.texture", !"air.location_index", i32 1, i32 1,
//           !"air.sample", ...}
//   !17 = !{i32 2, !"air.sampler", !"air.location_index", i32 2, i32 1, ...}
//   !9  = !{i32 1, !"air.indirect_constant", !"air.location_index",
//           i32 1, i32 1, !"air.arg_type_name", !"uint",
//           !"air.arg_name", !"count"}
//
// RUN: %clang_cc1 -x metal -triple air64_v28-apple-macosx26.0.0 \
// RUN:   -std=metal3.2 -emit-llvm -no-opaque-pointers -o - %s | FileCheck %s

struct Args {
  device float *data [[id(0)]];
  unsigned int count [[id(1)]];
};

// The binding itself is indirect and states its size.
// CHECK-DAG: !"air.indirect_buffer", !"air.buffer_size", i32 16, !"air.location_index", i32 0, i32 1, !"air.read", !"air.address_space", i32 2, !"air.struct_type_info"
//
// A pointer field is a nested air.buffer, a by-value field is an
// air.indirect_constant.
// CHECK-DAG: !"air.indirect_argument"
// CHECK-DAG: !{i32 1, !"air.indirect_constant", !"air.location_index", i32 1, i32 1, !"air.arg_type_name", !"uint", !"air.arg_name", !"count"}
//
// A plain struct without [[id]] stays a direct air.buffer.
// CHECK-DAG: !"air.buffer", !"air.location_index", i32 1, i32 1
kernel void k(constant Args &args [[buffer(0)]],
              device unsigned int *o [[buffer(1)]]) {
  o[0] = (unsigned)args.data[0] + args.count;
}
