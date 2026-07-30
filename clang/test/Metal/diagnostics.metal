// Metal specific diagnostics. The expected text is transcribed verbatim from
// Apple's compiler output, harvested from
// reference/metal-ast-macos-air64/log and catalogued in
// reference/metal-ast-macos-air64/meta/diagnostic-catalog.csv.
//
// RUN: %clang_cc1 -x metal -triple air64_v28-apple-macosx26.0.0 \
// RUN:   -std=metal4.0 -fsyntax-only -verify %s

// `uint` and the vector types come from Apple's <metal_stdlib>, not from the
// compiler; the reference AST dumps show them as ordinary typedefs. They are
// declared here so the test does not need the real standard library.
typedef unsigned int uint;
typedef unsigned short ushort;

kernel int bad_kernel_return() { // expected-error {{invalid return type 'int' for kernel function}}
  return 0;
}

thread_local int tls; // expected-error {{'thread_local' is not supported in Metal}}

int program_scope; // expected-error {{program scope variable must reside in constant address space}}

// A buffer whose pointee lives in an address space that cannot back a buffer.
kernel void bad_buffer(threadgroup float *p [[buffer(0)]]) {}
// expected-error@-1 {{invalid address space qualification for buffer pointee type 'threadgroup float'}}
// expected-note@-2 {{valid address space qualifications are device and constant}}

// A stage input used on the wrong stage.
kernel void bad_stage(uint v [[vertex_id]]) {}
// expected-error@-1 {{invalid 'vertex_id' attribute for input declaration in a kernel function}}
