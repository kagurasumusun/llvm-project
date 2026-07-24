// RUN: %clang_cc1 -triple air64-apple-macosx26.0 -x metal -std=metal4.0 -fsyntax-only %s

kernel void texture_methods(texture2d tex [[texture(0)]],
                            device uint *out [[buffer(0)]]) {
  out[0] = tex.get_width();
  out[1] = tex.get_height();
  out[2] = tex.get_array_size();
  out[3] = tex.get_width(1);
  out[4] = tex.get_height(1);
  out[5] = tex.get_num_mip_levels();
  out[6] = tex.get_num_samples();
  float4 value = tex.read(uint2(0, 0));
  value += tex.read(uint2(1, 1), 0);
  value += tex.sample(sampler(), float2(0.5f, 0.5f));
  tex.write(value, uint2(0, 0));
}
