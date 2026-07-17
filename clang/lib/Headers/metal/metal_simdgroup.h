#ifndef __METAL_SIMDGROUP_H__
#define __METAL_SIMDGROUP_H__

#if 1 // defined(__METAL__) || defined(__metal__)
namespace metal {

template<typename T, int Cols, int Rows>
struct simdgroup_matrix {
    T elements[Cols * Rows];
    simdgroup_matrix() {}
};

template<typename T, int Cols, int Rows>
inline void simdgroup_load(simdgroup_matrix<T, Cols, Rows>& mat, const device T* ptr, size_t stride = Cols, bool transpose = false) {}

template<typename T, int Cols, int Rows>
inline void simdgroup_load(simdgroup_matrix<T, Cols, Rows>& mat, const threadgroup T* ptr, size_t stride = Cols, bool transpose = false) {}

template<typename T, int Cols, int Rows>
inline void simdgroup_store(const simdgroup_matrix<T, Cols, Rows>& mat, device T* ptr, size_t stride = Cols, bool transpose = false) {}

template<typename T, int Cols, int Rows>
inline void simdgroup_store(const simdgroup_matrix<T, Cols, Rows>& mat, threadgroup T* ptr, size_t stride = Cols, bool transpose = false) {}

template<typename T, int Cols, int Rows>
inline void simdgroup_multiply_accumulate(simdgroup_matrix<T, Cols, Rows>& d,
                                          const simdgroup_matrix<T, Cols, Rows>& a,
                                          const simdgroup_matrix<T, Cols, Rows>& b,
                                          const simdgroup_matrix<T, Cols, Rows>& c) {}

} // namespace metal
#endif // __METAL__
#endif // __METAL_SIMDGROUP_H__
