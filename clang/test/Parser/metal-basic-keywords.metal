// RUN: %clang_cc1 -triple air64-apple-macosx10.15 -x metal -std=metal2.0 -fsyntax-only %s

kernel void k(device int *out, constant int *in, threadgroup int *scratch) {
  thread int local = in[0];
  scratch[0] = local;
  out[0] = scratch[0];
}

vertex void v() {}
fragment void f() {}
