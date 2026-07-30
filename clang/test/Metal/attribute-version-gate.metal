// Attributes are gated on the MSL version they were introduced in.
//
// The minimum version and the exact diagnostic wording for each attribute were
// harvested from Apple's own output in reference/metal-ast-macos-air64/log,
// which contains messages of the form
//   'mesh' attribute requires Metal language standard metal3.0 or higher
// See docs-metal/data/metal_attributes.csv for the per-attribute evidence.
//
// RUN: %clang_cc1 -x metal -triple air64_v20-apple-macosx10.13.0 \
// RUN:   -std=macos-metal1.1 -fsyntax-only -verify %s

// `uint` and the vector types come from Apple's <metal_stdlib>, not from the
// compiler; the reference AST dumps show them as ordinary typedefs. They are
// declared here so the test does not need the real standard library.
typedef unsigned int uint;
typedef unsigned short ushort;

struct S {
  int a [[id(0)]];
  // expected-error@-1 {{'id' attribute requires Metal language standard macos-metal2.0 or higher}}
};

[[early_fragment_tests]] fragment float4 f();
// expected-error@-1 {{'early_fragment_tests' attribute requires Metal language standard macos-metal1.2 or higher}}

kernel void k(uint lane [[thread_index_in_simdgroup]]) {}
// expected-error@-1 {{'thread_index_in_simdgroup' attribute requires Metal language standard macos-metal2.0 or higher}}
