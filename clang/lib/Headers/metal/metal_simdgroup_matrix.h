// metal_simdgroup_matrix — MSL SIMD group matrix (cleanroom)
#ifndef _METAL_SIMDGROUP_MATRIX_H_
#define _METAL_SIMDGROUP_MATRIX_H_
#include <metal/metal_common>
namespace metal {

template <typename T, int Rows, int Cols>
struct simdgroup_matrix {
  T elements[Rows * Cols];
  METAL_FUNC T get(int r, int c) const { return elements[r * Cols + c]; }
  METAL_FUNC void set(int r, int c, T val) { elements[r * Cols + c] = val; }
  static constexpr int num_rows = Rows;
  static constexpr int num_cols = Cols;
  METAL_FUNC T* thread_elements() { return elements; }
  METAL_FUNC const T* thread_elements() const { return elements; }
  METAL_FUNC T& operator[](int i) { return elements[i]; }
  METAL_FUNC const T& operator[](int i) const { return elements[i]; }
};

template <typename T, int R, int C>
METAL_FUNC simdgroup_matrix<T,R,C> operator+(simdgroup_matrix<T,R,C> a, simdgroup_matrix<T,R,C> b) {
  simdgroup_matrix<T,R,C> r; for(int i=0;i<R*C;++i) r.elements[i]=a.elements[i]+b.elements[i]; return r; }
template <typename T, int R, int C>
METAL_FUNC simdgroup_matrix<T,R,C> operator-(simdgroup_matrix<T,R,C> a, simdgroup_matrix<T,R,C> b) {
  simdgroup_matrix<T,R,C> r; for(int i=0;i<R*C;++i) r.elements[i]=a.elements[i]-b.elements[i]; return r; }
template <typename T, int R, int C>
METAL_FUNC simdgroup_matrix<T,R,C> operator*(simdgroup_matrix<T,R,C> a, T s) {
  simdgroup_matrix<T,R,C> r; for(int i=0;i<R*C;++i) r.elements[i]=a.elements[i]*s; return r; }
template <typename T, int R, int C>
METAL_FUNC simdgroup_matrix<T,R,C> operator/(simdgroup_matrix<T,R,C> a, simdgroup_matrix<T,R,C> b) {
  simdgroup_matrix<T,R,C> r; for(int i=0;i<R*C;++i) r.elements[i]=a.elements[i]/b.elements[i]; return r; }

template <typename T, int M, int N, int K>
METAL_FUNC simdgroup_matrix<T,M,N> simdgroup_matrix_multiply_accumulate(
    simdgroup_matrix<T,M,K> a, simdgroup_matrix<T,K,N> b, simdgroup_matrix<T,M,N> c) {
  simdgroup_matrix<T,M,N> r = c;
  for(int m=0;m<M;++m) for(int n=0;n<N;++n) for(int k=0;k<K;++k) r.elements[m*N+n]+=a.elements[m*K+k]*b.elements[k*N+n];
  return r;}

// Apple-compatible: simdgroup_multiply_accumulate with output reference
template <typename T, int M, int K, int N>
METAL_FUNC void simdgroup_multiply_accumulate(
    thread simdgroup_matrix<T,M,N> &d,
    simdgroup_matrix<T,M,K> a,
    simdgroup_matrix<T,K,N> b,
    simdgroup_matrix<T,M,N> c) {
  d = simdgroup_matrix_multiply_accumulate(a, b, c);
}

// Apple-compatible: simdgroup_multiply with output reference
template <typename T, int M, int K, int N>
METAL_FUNC void simdgroup_multiply(
    thread simdgroup_matrix<T,M,N> &d,
    simdgroup_matrix<T,M,K> a,
    simdgroup_matrix<T,K,N> b) {
  simdgroup_matrix<T,M,N> c = {};
  simdgroup_multiply_accumulate(d, a, b, c);
}
template <typename T, int R, int C>
METAL_FUNC void simdgroup_load(simdgroup_matrix<T,R,C> &m, const device T *ptr, uint stride, uint2 coord) {
  for(int r=0;r<R;++r) for(int c=0;c<C;++c) m.set(r,c,ptr[(coord.y+r)*stride+coord.x+c]);}
template <typename T, int R, int C>
METAL_FUNC void simdgroup_store(const simdgroup_matrix<T,R,C> &m, device T *ptr, uint stride, uint2 coord) {
  for(int r=0;r<R;++r) for(int c=0;c<C;++c) ptr[(coord.y+r)*stride+coord.x+c]=m.get(r,c);}
template <typename T, int R, int C>
METAL_FUNC void simdgroup_load(simdgroup_matrix<T,R,C> &m, const threadgroup T *ptr, uint stride, uint2 coord) {
  for(int r=0;r<R;++r) for(int c=0;c<C;++c) m.set(r,c,ptr[(coord.y+r)*stride+coord.x+c]);}
template <typename T, int R, int C>
METAL_FUNC void simdgroup_store(const simdgroup_matrix<T,R,C> &m, threadgroup T *ptr, uint stride, uint2 coord) {
  for(int r=0;r<R;++r) for(int c=0;c<C;++c) ptr[(coord.y+r)*stride+coord.x+c]=m.get(r,c);}
template <typename T, int N>
METAL_FUNC simdgroup_matrix<T,N,N> simdgroup_matrix_init_diag(T val) {
  simdgroup_matrix<T,N,N> m = {}; for(int i=0;i<N;++i) m.set(i,i,val); return m;}
template <typename T, int R, int C>
METAL_FUNC simdgroup_matrix<T,R,C> simdgroup_matrix_init_filled(T val) {
  simdgroup_matrix<T,R,C> m; for(int i=0;i<R*C;++i) m.elements[i]=val; return m;}


// matrix * vector
template <typename T, int R, int C>
METAL_FUNC vec<T, R> operator*(simdgroup_matrix<T,R,C> m, vec<T, C> v) {
  vec<T, R> r = vec<T, R>(0);
  for (int c = 0; c < C; ++c) r += m.columns[c] * v[c];
  return r;
}


// Unary negation
template <typename T, int R, int C>
METAL_FUNC simdgroup_matrix<T,R,C> operator-(simdgroup_matrix<T,R,C> a) {
  simdgroup_matrix<T,R,C> r;
  for (int i = 0; i < R*C; ++i) r.elements[i] = -a.elements[i];
  return r;
}

// Compound assignment operators
template <typename T, int R, int C>
METAL_FUNC simdgroup_matrix<T,R,C>& operator+=(simdgroup_matrix<T,R,C>& a, const simdgroup_matrix<T,R,C>& b) {
  for (int i = 0; i < R*C; ++i) a.elements[i] += b.elements[i];
  return a;
}
template <typename T, int R, int C>
METAL_FUNC simdgroup_matrix<T,R,C>& operator-=(simdgroup_matrix<T,R,C>& a, const simdgroup_matrix<T,R,C>& b) {
  for (int i = 0; i < R*C; ++i) a.elements[i] -= b.elements[i];
  return a;
}
template <typename T, int R, int C>
METAL_FUNC simdgroup_matrix<T,R,C>& operator*=(simdgroup_matrix<T,R,C>& a, T s) {
  for (int i = 0; i < R*C; ++i) a.elements[i] *= s;
  return a;
}

} // namespace metal
#endif