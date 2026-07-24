// RUN: %clang_cc1 -triple air64-apple-macosx26.0 -x metal -std=metal4.0 -E -P -I%S/Inputs/apple-metal-include %s
// This file is used by the fast GitHub smoke workflow with the real Apple
// Metal stdlib headers fetched from metal-info.  The RUN line documents the
// intent; the workflow supplies the real include path.

#include <metal_stdlib>

kernel void apple_stdlib_preprocess_probe(device int *out [[buffer(0)]]) {
  out[0] = 0;
}
