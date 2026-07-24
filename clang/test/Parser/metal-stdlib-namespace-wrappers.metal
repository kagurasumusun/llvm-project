// RUN: %clang_cc1 -triple air64-apple-macosx26.0 -x metal -std=metal4.0 -fsyntax-only %s

kernel void namespace_stdlib_wrappers(device int *out [[buffer(0)]]) {
  float x = metal::sin(1.0f) + metal::cos(1.0f) + metal::sqrt(4.0f);
  x += metal::pow(2.0f, 3.0f) + metal::clamp(2.0f, 0.0f, 1.0f);
  out[0] = int(x) + metal::abs(-7) + metal::select(1, 2, true);
}
