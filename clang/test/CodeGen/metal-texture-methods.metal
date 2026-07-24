// RUN: %clang_cc1 -triple air64_v22-apple-macosx10.15 -x metal -std=macos-metal2.0 -emit-llvm -o - %s | FileCheck %s

kernel void texture_methods(texture2d tex [[texture(0)]],
                            device uint *out [[buffer(0)]]) {
  out[0] = tex.get_width();
  out[1] = tex.get_height();
  out[2] = tex.get_width(1);
  out[3] = tex.get_num_mip_levels();
  float4 value = tex.read(uint2(0, 0));
  value += tex.sample(sampler(), float2(0.5f, 0.5f));
  tex.write(value, uint2(0, 0));
}

// CHECK: call {{.*}}@air.texture.get_width
// CHECK: call {{.*}}@air.texture.get_height
// CHECK: call {{.*}}@air.texture.get_width.lod
// CHECK: call {{.*}}@air.texture.get_num_mip_levels
// CHECK: call {{.*}}@air.texture.read
// CHECK: call {{.*}}@air.texture.sample
// CHECK: call {{.*}}@air.texture.write
// CHECK: !"air.texture"
