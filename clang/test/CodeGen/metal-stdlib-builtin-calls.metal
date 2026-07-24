// RUN: %clang_cc1 -triple air64_v22-apple-macosx10.15 -x metal -std=macos-metal2.0 -emit-llvm -o - %s | FileCheck %s

kernel void call_stdlib_builtin(device int *out [[buffer(0)]]) {
  out[0] = __metal_abs(-7);
}

// CHECK: call {{.*}}@__metal_abs
