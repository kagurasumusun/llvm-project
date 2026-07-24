// RUN: %clang_cc1 -triple air64-apple-macosx26.0 -x metal -std=metal4.0 -fsyntax-only -verify %s

// Metal local automatic variables may be explicitly qualified with the
// thread/private address space; this is the default storage for ordinary
// function-scope objects in MSL, but spelling it should still be accepted.
kernel void accepts_thread_private_local(device int *out [[buffer(0)]],
                                         constant int *in [[buffer(1)]]) {
  thread int local = in[0];
  int plain = local;
  out[0] = plain;
}

// Do not broadly disable Clang's local address-space diagnostic: non-private
// Metal address-space objects still cannot be automatic function-scope values
// in this initial frontend model.
kernel void rejects_device_local_object() {
  device int invalid; // expected-error {{automatic variable qualified with an address space}}
}
