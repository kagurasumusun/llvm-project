// Constructs the Metal specification lists as unavailable (MSL 4.1 section
// 1.6.1). Every diagnostic below is the exact text Apple's compiler emits,
// harvested from the reference logs.
//
// RUN: %clang_cc1 -x metal -triple air64_v28-apple-macosx26.0.0 \
// RUN:   -std=metal3.2 -fsyntax-only -verify %s

double d;                    // expected-error {{'double' is not supported in Metal}}
                             // expected-error@-1 {{program scope variable must reside in constant address space}}

struct Base { int x; };
struct Derived : Base { };   // expected-error {{derived classes are not supported in Metal}}

union U { int a; float b; }; // expected-error {{unions are not supported in Metal}}

struct WithVirtual {
  virtual void f();          // expected-error {{virtual member functions are not supported in Metal}}
};

kernel void k(device float *p [[buffer(0)]]) {}

// A resource parameter without a binding attribute is rejected, and the
// message names the parameter.
kernel void missing_attr(device float *p) {}
// expected-error@-1 {{p parameter must have buffer attribute}}
