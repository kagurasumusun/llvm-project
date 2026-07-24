// RUN: %clang_cc1 -triple air64_v22-apple-macosx10.15 -x metal -std=macos-metal2.0 -emit-llvm -o - %s | FileCheck %s

kernel void texture_methods(texture2d tex [[texture(0)]],
                            device uint *out [[buffer(0)]]) {
  out[0] = tex.get_width();
}

// CHECK: call {{.*}}get_width
// CHECK: !air.texture
