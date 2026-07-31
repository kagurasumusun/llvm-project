// Metal member functions may name the address space their object lives in,
// written after the parameter list. Apple's standard library uses 7,668 of
// these, so without it the real headers cannot be parsed at all.
//
// RUN: %clang_cc1 -x metal -triple air64_v28-apple-macosx26.0.0 \
// RUN:   -std=metal3.2 -fsyntax-only -verify %s

// expected-no-diagnostics

struct S {
  int f() thread;
  int g() device;
  int h() const constant;
  int i() ray_data;
  // The shape Apple's own headers use.
  S &operator=(const device S &) thread;
};
