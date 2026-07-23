// RUN: %clang_cc1 -triple air64-apple-macosx10.15 -x metal -std=metal2.0 -fsyntax-only %s
// RUN: %clang_cc1 -triple air64-apple-macosx26.0 -x metal -std=metal4.0 -fsyntax-only %s

kernel void k(device int *out [[buffer(0)]], constant int *in [[buffer(1)]], threadgroup int *scratch) {
  thread int local = in[0];
  scratch[0] = local;
  out[0] = scratch[0];
}

vertex void v() {}
fragment void f() {}


#if __METAL_VERSION__ >= 400
template <typename T> struct metal4_value { static constexpr int value = 4; };

kernel void metal4_cxx17(device int *out) {
  if constexpr (metal4_value<int>::value == 4)
    out[0] = metal4_value<int>::value;
}
#endif


kernel void builtin_attrs(device int *out [[buffer(0)]],
                          unsigned int id [[thread_position_in_grid]],
                          unsigned int local_id [[thread_index_in_threadgroup]],
                          unsigned int threads [[threads_per_threadgroup]]) {
  out[id] = local_id + threads;
}
