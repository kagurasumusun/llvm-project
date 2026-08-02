// Entry point signature diagnostics: kernel/vertex/fragment parameter and
// return type validation, resource binding indices, and stage input/output
// attributes.
//
// Every expected text is transcribed verbatim from Apple's compiler output
// in reference/metal-ast-macos-air64/log (fixtures misc_unused_parameter,
// attr_buffer_overflow, attr_sampler_overflow, attr_texture_overflow,
// sig_vertex_too_many_inputs, misc_buffer_scalar, misc_ulong_buffer_pointee
// - the last through type_buffer_ulong_pointee -, address_spaces_ray_data_all,
// sig_stage_in_matrix_member, attr_depth_in_vertex, return_int_of_vertex and
// the duplicate [[position]] / duplicate [[buffer(0)]] fixtures catalogued in
// reference/metal-ast-macos-air64/meta/sema-metal-rules.csv and
// diagnostic-catalog.csv):
//
//   error: invalid type 'int' for input declaration in a kernel function
//   error: 'buffer' attribute parameter is out of bounds: must be between 0
//          and 30
//   error: 'sampler' attribute parameter is out of bounds: must be between 0
//          and 15
//   error: 'attribute' attribute parameter is out of bounds: must be between
//          0 and 30
//   error: cannot reserve 'buffer' resource location at index 0
//   error: t parameter must have texture attribute
//   error: s parameter must have sampler attribute
//   error: tg parameter must have threadgroup attribute
//   error: type 'uint' (aka 'unsigned int') is not valid for attribute
//          'buffer'
//   error: type 'device ulong *' (aka 'device unsigned long *') is not valid
//          for attribute 'buffer'
//   note: type 'ulong' (aka 'unsigned long') cannot be used in buffer
//         pointee type
//   error: type 'ray_data uint &' (aka 'ray_data unsigned int &') is not
//          valid for attribute 'primitive_id'
//   error: type 'metal::float4x4' (aka 'matrix<float; 4; 4>') is not valid
//          for attribute 'attribute'
//   error: type 'MIn' is not valid for attribute 'stage_in'
//   error: declaration with attribute 'position' already specified
//   note: previous declaration with attribute 'position' here
//   error: invalid return type 'VertexOut' for vertex function
//   note: invalid 'color' attribute for output declaration
//   error: 'buffer' attribute takes one argument
//
// Textures carry a target-dependent slot count (see
// Sema::CheckMetalResourceIndexBounds): iOS before Metal 2.0 rejects
// [[texture(32)]] while macOS accepts it at every language version.
//
// RUN: %clang_cc1 -x metal -triple air64_v28-apple-macosx26.0.0 \
// RUN:   -std=metal4.0 -fsyntax-only -verify=expected,new %s
// RUN: %clang_cc1 -x metal -triple air64_v20-apple-macosx10.13.0 \
// RUN:   -std=macos-metal1.1 -fsyntax-only -verify=expected,old %s
// RUN: %clang_cc1 -x metal -triple air64_v28-apple-ios16.5.0 \
// RUN:   -std=ios-metal1.2 -fsyntax-only -verify=expected,ios12 %s
// RUN: %clang_cc1 -x metal -triple air64_v24-apple-macosx13.0.0 \
// RUN:   -std=macos-metal2.2 -fsyntax-only -verify=expected,hn22 %s

// `uint`, `ulong` and the vector types come from Apple's <metal_stdlib>, not
// from the compiler; textures, samplers and matrices are class templates
// there (<metal_texture>, <metal_matrix>). They are declared here so the
// test does not need the real standard library.
typedef unsigned int uint;
typedef unsigned long ulong;
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

template <typename T, int Cols, int Rows = Cols>
struct matrix {
  T columns[Cols][Rows];
};
}

// ---------------------------------------------------------------------------
// Resource binding indices.
// ---------------------------------------------------------------------------

// measured: "invalid type 'int' ..." (misc_unused_parameter)
kernel void scalar_input(int v, device uint *o [[buffer(0)]]) {}
// expected-error@-1 {{invalid type 'int' for input declaration in a kernel function}}

// Once the attribute is rejected it is not attached, so the parameter then
// also fails the "must have N attribute" rule; this mirrors the erorr
// cascade of the attr_*_overflow fixtures.
kernel void oob_buf(device uint *a [[buffer(31)]]) {}
// expected-error@-1 {{'buffer' attribute parameter is out of bounds: must be between 0 and 30}}
// expected-error@-2 {{a parameter must have buffer attribute}}

kernel void oob_samp(metal::sampler s [[sampler(16)]]) {}
// expected-error@-1 {{'sampler' attribute parameter is out of bounds: must be between 0 and 15}}
// expected-error@-2 {{s parameter must have sampler attribute}}

// Negative indices take the same out-of-bounds path (measured [[buffer(-1)]]).
// The texture bound is 31 on iOS before Metal 2.0, 128 otherwise.
kernel void neg_tex(metal::texture2d<float> t [[texture(-1)]]) {}
// new-error@-1 {{'texture' attribute parameter is out of bounds: must be between 0 and 127}}
// old-error@-2 {{'texture' attribute parameter is out of bounds: must be between 0 and 127}}
// hn22-error@-3 {{'texture' attribute parameter is out of bounds: must be between 0 and 127}}
// ios12-error@-4 {{'texture' attribute parameter is out of bounds: must be between 0 and 30}}
// expected-error@-5 {{t parameter must have texture attribute}}

// [[texture(32)]] is legal on macOS at every measured standard but rejected
// by 64-bit iOS before Metal 2.0 (the ios-air32 port raises the limit one
// standard earlier, at ios-metal1.1).
kernel void tex_hi(metal::texture2d<float> t [[texture(32)]]) {}
// ios12-error@-1 {{'texture' attribute parameter is out of bounds: must be between 0 and 30}}
// ios12-error@-2 {{t parameter must have texture attribute}}

kernel void dup_buf(device uint *a [[buffer(0)]], device uint *b [[buffer(0)]]) {}
// expected-error@-1 {{cannot reserve 'buffer' resource location at index 0}}

kernel void dup_tex(metal::texture2d<float> a [[texture(2)]],
                    metal::texture2d<float> b [[texture(2)]]) {}
// expected-error@-1 {{cannot reserve 'texture' resource location at index 2}}

kernel void takes_arg(device uint *a [[buffer]]) {}
// expected-error@-1 {{'buffer' attribute takes one argument}}
// expected-error@-2 {{a parameter must have buffer attribute}}

// ---------------------------------------------------------------------------
// Resource parameters without / with mismatched attributes.
// ---------------------------------------------------------------------------

kernel void no_bind_tex(metal::texture2d<float> t) {}
// expected-error@-1 {{t parameter must have texture attribute}}

kernel void no_bind_samp(metal::sampler s) {}
// expected-error@-1 {{s parameter must have sampler attribute}}

kernel void no_bind_tg(threadgroup uint *g) {}
// expected-error@-1 {{g parameter must have threadgroup attribute}}

kernel void scalar_buf(uint v [[buffer(0)]]) {}
// expected-error@-1 {{type 'uint' (aka 'unsigned int') is not valid for attribute 'buffer'}}

kernel void ulong_buf(device ulong *o [[buffer(0)]]) {}
// old-error@-1 {{type 'device ulong *' (aka 'device unsigned long *') is not valid for attribute 'buffer'}}
// ios12-error@-2 {{type 'device ulong *' (aka 'device unsigned long *') is not valid for attribute 'buffer'}}
// hn22-error@-3 {{type 'device ulong *' (aka 'device unsigned long *') is not valid for attribute 'buffer'}}
// old-error@-4 {{type 'ulong' (aka 'unsigned long') cannot be used in buffer pointee type}}
// ios12-error@-5 {{type 'ulong' (aka 'unsigned long') cannot be used in buffer pointee type}}
// hn22-error@-6 {{type 'ulong' (aka 'unsigned long') cannot be used in buffer pointee type}}

kernel void samp_on_int(int s [[sampler(0)]]) {}
// expected-error@-1 {{type 'int' is not valid for attribute 'sampler'}}

kernel void tg_on_int(int g [[threadgroup(0)]]) {}
// expected-error@-1 {{type 'int' is not valid for attribute 'threadgroup'}}

// ---------------------------------------------------------------------------
// Param declarations (Sema::CheckMetalParamDecl).
// ---------------------------------------------------------------------------

// measured (type_register): the register specifier is banned everywhere.
kernel void reg_param(register int r, device uint *o [[buffer(0)]]) {}
// expected-error@-1 {{Metal does not support the 'register' storage class specifier}}

// measured pre-Metal 2.1 (era_function_pointer_before_metal21).
kernel void fnp_param(int (*fp)(int), device uint *o [[buffer(0)]]) {}
// old-error@-1 {{pointers to functions are not allowed}}
// ios12-error@-2 {{pointers to functions are not allowed}}

// measured: "parameter may not be qualified with an address space".
kernel void pas_param(constant int v, device uint *o [[buffer(0)]]) {}
// expected-error@-1 {{parameter may not be qualified with an address space}}

// Stage input attributes travel by value; on a reference parameter the
// measured diagnostic names the reference type (address_spaces_ray_data_all
// measures 'ray_data uint &' ... 'primitive_id').
kernel void ref_stage(constant uint &s [[thread_position_in_grid]]) {}
// expected-error@-1 {{type 'constant uint &' (aka 'constant unsigned int &') is not valid for attribute 'thread_position_in_grid'}}

// ---------------------------------------------------------------------------
// Stage inputs and outputs.
// ---------------------------------------------------------------------------

// measured (attr_invalid_position_scalar): [[position]] is float4 only.
fragment int pos_bad(int pos [[position]]) { return 0; }
// expected-error@-1 {{type 'int' is not valid for attribute 'position'}}

struct VIn {
  // expected-note@+1 {{previous declaration with attribute 'position' here}}
  float4 p [[position]];
};

// measured: conflict between the struct member and the direct parameter.
vertex float4 dup_pos(VIn in [[stage_in]], float4 q [[position]]) {
  return q;
}
// expected-error@-2 {{declaration with attribute 'position' already specified}}

struct BadIn {
  float f [[attribute(0)]];
  // expected-error@+1 {{type 'metal::matrix<float, 4, 4>' is not valid for attribute 'attribute'}}
  metal::matrix<float, 4, 4> m [[attribute(1)]];
};

// measured (sig_stage_in_matrix_member): the member error cascades to the
// whole stage_in declaration. Apple names the stdlib alias, which is
// 'metal::float4x4' (aka 'matrix<float; 4; 4>') in its output; the fork's
// matrix is the same class template, printed 'metal::matrix<float, 4, 4>'.
vertex float4 stage_bad(BadIn in [[stage_in]]) { return {}; }
// expected-error@-1 {{type 'BadIn' is not valid for attribute 'stage_in'}}

struct TooManyIn {
  float f [[attribute(31)]]; // expected-error {{'attribute' attribute parameter is out of bounds: must be between 0 and 30}}
};
vertex float4 too_many(TooManyIn in [[stage_in]]) { return {}; }

// color is a fragment output; measured on a vertex it invalidates the
// return type with a cascaded note on the member.
struct BadOut {
  float4 p [[position]];
  // expected-note@+1 {{invalid 'color' attribute for output declaration}}
  float4 c [[color(0)]];
};
vertex BadOut bad_out(VIn in [[stage_in]]) { return {}; }
// expected-error@-1 {{invalid return type 'BadOut' for vertex function}}

// measured (return_int_of_vertex): pointer returns are never legal.
vertex int *bad_vret(int vid [[vertex_id]]) { return nullptr; }
// expected-error@-1 {{invalid return type 'int *' for vertex function}}

// ---------------------------------------------------------------------------
// [[host_name]].
// ---------------------------------------------------------------------------

// measured quirk: the empty string is rejected only at macos-metal2.2, the
// standard that introduced the attribute; earlier standards gate the
// attribute itself and 2.3+ accept the empty string.
[[host_name("")]] kernel void named(device uint *o [[buffer(0)]]) {}
// old-error@-1 {{'host_name' attribute requires Metal language standard macos-metal2.2 or higher}}
// ios12-error@-2 {{'host_name' attribute requires Metal language standard ios-metal2.2 or higher}}
// hn22-error@-3 {{invalid string literal value for 'host_name' attribute}}
