// RUN: %clang_cc1 -triple air64-apple-macosx26.0 -x metal -std=metal4.0 -fsyntax-only %s

kernel void texture_methods(texture2d tex [[texture(0)]],
                            device uint *out [[buffer(0)]]) {
  out[0] = tex.get_width();
  out[1] = tex.get_height();
  out[2] = tex.get_array_size();
}
