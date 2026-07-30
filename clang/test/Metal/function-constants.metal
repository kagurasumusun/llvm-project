// !air.function_constants.
//
// INFO_SET.md A-6 records the IR representation of [[function_constant]] as
// OPEN. It is settled by the reference corpus, which carries the named node in
// 5,057 modules. From function_constant_fc_bool (macos-metal1.2, 10.14):
//
//   @_ZL10use_path_a = internal unnamed_addr addrspace(2) global i8 undef
//   @_Z10use_path_a.MTL_FC_INIT_0_b = linkonce_odr hidden local_unnamed_addr
//       addrspace(2) externally_initialized constant i8 undef, align 1
//   !air.function_constants = !{!10}
//   !10 = !{i8 addrspace(2)* @_Z10use_path_a.MTL_FC_INIT_0_b,
//           !"bool", !"use_path_a", i32 0}
//
// The placeholder suffix is .MTL_FC_INIT_<index>_<itanium type code>; the
// corpus shows _b for bool, _i for int, _j for uint and _f for float.
//
// RUN: %clang_cc1 -x metal -triple air64_v28-apple-macosx26.0.0 \
// RUN:   -std=metal3.2 -emit-llvm -no-opaque-pointers -o - %s | FileCheck %s

// `uint` and the vector types come from Apple's <metal_stdlib>, not from the
// compiler; the reference AST dumps show them as ordinary typedefs. They are
// declared here so the test does not need the real standard library.
typedef unsigned int uint;
typedef unsigned short ushort;

constant bool use_path_a [[function_constant(0)]];
constant int fc_int [[function_constant(1)]];
constant float alpha [[function_constant(2)]];

// CHECK-DAG: @_Z10use_path_a.MTL_FC_INIT_0_b = linkonce_odr hidden local_unnamed_addr addrspace(2) externally_initialized constant i8 undef
// CHECK-DAG: @_Z6fc_int.MTL_FC_INIT_1_i = linkonce_odr hidden local_unnamed_addr addrspace(2) externally_initialized constant i32 undef
// CHECK-DAG: @_Z5alpha.MTL_FC_INIT_2_f = linkonce_odr hidden local_unnamed_addr addrspace(2) externally_initialized constant float undef

// CHECK-DAG: !{i8 addrspace(2)* @_Z10use_path_a.MTL_FC_INIT_0_b, !"bool", !"use_path_a", i32 0}
// CHECK-DAG: !{i32 addrspace(2)* @_Z6fc_int.MTL_FC_INIT_1_i, !"int", !"fc_int", i32 1}
// CHECK-DAG: !{float addrspace(2)* @_Z5alpha.MTL_FC_INIT_2_f, !"float", !"alpha", i32 2}

kernel void k(device unsigned int *out [[buffer(0)]],
              unsigned int id [[thread_position_in_grid]]) {
  out[id] = use_path_a ? (unsigned)fc_int : (unsigned)alpha;
}
