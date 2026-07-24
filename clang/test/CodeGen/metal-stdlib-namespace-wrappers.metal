// RUN: %clang_cc1 -triple air64_v22-apple-macosx10.15 -x metal -std=macos-metal2.0 -emit-llvm -o - %s | FileCheck %s

kernel void namespace_stdlib_wrappers(device int *out [[buffer(0)]]) {
  float x = metal::sin(1.0f) + metal::sqrt(4.0f);
  x += metal::pow(2.0f, 3.0f) + metal::clamp(2.0f, 0.0f, 1.0f);
  out[0] = int(x) + metal::abs(-7) + metal::select(1, 2, true);
}

// CHECK: @__metal_sin
// CHECK: @__metal_sqrt
// CHECK: @__metal_pow
// CHECK: @__metal_clamp
// CHECK: @__metal_abs
// CHECK: @__metal_select
