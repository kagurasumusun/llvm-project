// `coherent` must be written on top of a `device` qualifier.
//
// Every one of the 1,001 occurrences in the reference AST corpus pairs the two
// (`const device coherent(device) Uniforms &`); the corpus contains no
// `coherent` on any other address space.
//
// RUN: %clang_cc1 -x metal -triple air64_v28-apple-macosx26.0.0 \
// RUN:   -std=metal3.2 -fsyntax-only -verify %s

struct Uniforms {
  float scale;
};

void bad(coherent(device) Uniforms &u); // expected-error {{'coherent' qualifier requires the 'device' address space}}

void good(const device coherent(device) Uniforms &u); // no diagnostic
