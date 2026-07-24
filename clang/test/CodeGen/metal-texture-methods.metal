// RUN: %clang_cc1 -triple air64_v22-apple-macosx10.15 -x metal -std=macos-metal2.0 -emit-llvm -o - %s | FileCheck %s

kernel void texture_methods(texture2d tex [[texture(0)]],
                            device uint *out [[buffer(0)]]) {
  out[0] = tex.get_width();
  out[1] = tex.get_height();
}

// CHECK: call {{.*}}@__metal_texture_get_width
// CHECK: call {{.*}}@__metal_texture_get_height
// CHECK: !"air.texture"
