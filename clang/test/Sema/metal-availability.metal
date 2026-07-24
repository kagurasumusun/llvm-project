// RUN: %clang_cc1 -triple air64-apple-macosx10.14 -x metal -std=macos-metal1.1 -fsyntax-only -verify %s

[[early_fragment_tests]] fragment void early_fragment_tests_needs_metal2() {} // expected-error {{'early_fragment_tests' attribute requires Metal 2.0 or later}}
constant int fc [[function_constant(0)]];
// expected-error@-1 {{'function_constant' attribute requires Metal 1.2 or later}}

// Mesh/object-family attributes are modeled as Metal 3.0+ entry points.
kernel void mesh_attr(uint x [[mesh]]) {} // expected-error {{'mesh' attribute requires Metal 3.0 or later}}

tile void tile_needs_metal2() {} // expected-error {{'tile' attribute requires Metal 2.0 or later}}
