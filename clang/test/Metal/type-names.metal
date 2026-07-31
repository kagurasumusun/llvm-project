// air.arg_type_name spells types the way Apple does.
//
// The 139 distinct spellings in the reference corpus fix three rules that the
// default C++ printer gets wrong:
//
//   * defaulted template arguments are written out. Every texture in the
//     corpus states its access mode -- texture2d<float, sample>, never
//     texture2d<float> -- even though `sample` is the default;
//   * enumerators appear bare, not scope-qualified: `sample`, not
//     `access::sample`;
//   * scalars use the MSL names. air.arg_type_name contains uint, ushort,
//     uchar and ulong, and no occurrence of "unsigned int" anywhere.
//
// RUN: %clang_cc1 -x metal -triple air64_v28-apple-macosx26.0.0 \
// RUN:   -std=metal3.2 -emit-llvm -no-opaque-pointers -o - %s | FileCheck %s

typedef unsigned int uint;
typedef unsigned short ushort;
typedef unsigned char uchar;

// Stand-ins for the standard library declarations: the access parameter is
// defaulted exactly as <metal_texture> defaults it.
namespace metal {
enum class access { sample, read, write, read_write };

template <typename T, access a = access::sample>
struct texture2d {
  int handle;
};

template <typename T, access a = access::read>
struct texture3d {
  int handle;
};
}
using namespace metal;

// The defaulted access argument is printed, and printed unqualified.
//
// CHECK-DAG: !"air.arg_type_name", !"texture2d<float, sample>"
// CHECK-DAG: !"air.arg_type_name", !"texture2d<float, write>"
// CHECK-DAG: !"air.arg_type_name", !"texture3d<float, read>"
//
// Scalars use the MSL spelling; "unsigned int" must not appear.
// CHECK-DAG: !"air.arg_type_name", !"uint"
// CHECK-NOT: !"air.arg_type_name", !"unsigned int"
kernel void k(texture2d<float> defaulted [[texture(0)]],
              texture2d<float, access::write> explicit_write [[texture(1)]],
              texture3d<float> depth_defaulted [[texture(2)]],
              device uint *out [[buffer(0)]],
              uint id [[thread_position_in_grid]]) {
  out[id] = id;
}
