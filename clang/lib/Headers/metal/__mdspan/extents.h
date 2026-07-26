// __mdspan/extents.h — Multidimensional extents (MSL 4.1)
#ifndef __MDSPAN_EXTENTS_H
#define __MDSPAN_EXTENTS_H

namespace metal {

template <typename T, int... Extents>
struct extents {
  static constexpr int rank() { return sizeof...(Extents); }
  static constexpr int rank_dynamic() { return 0; }
  static constexpr int static_extent(int i) { return 0; }
};

template <typename T>
struct extents<T> {
  static constexpr int rank() { return 0; }
  static constexpr int rank_dynamic() { return 0; }
};

template <typename T, int E0>
struct extents<T, E0> {
  static constexpr int rank() { return 1; }
  static constexpr int static_extent(int i) { return E0; }
  int _d0;
  METAL_ALWAYS_INLINE constexpr int extent(int i) const { return _d0; }
};

template <typename T, int E0, int E1>
struct extents<T, E0, E1> {
  static constexpr int rank() { return 2; }
  static constexpr int static_extent(int i) { return i == 0 ? E0 : E1; }
  int _d0, _d1;
  METAL_ALWAYS_INLINE constexpr int extent(int i) const { return i == 0 ? _d0 : _d1; }
};

template <typename T, int E0, int E1, int E2>
struct extents<T, E0, E1, E2> {
  static constexpr int rank() { return 3; }
  static constexpr int static_extent(int i) { return i == 0 ? E0 : (i == 1 ? E1 : E2); }
  int _d0, _d1, _d2;
  METAL_ALWAYS_INLINE constexpr int extent(int i) const { return i == 0 ? _d0 : (i == 1 ? _d1 : _d2); }
};

template <typename T, int... Extents>
struct mdspan {
  T *_data;
  extents<T, Extents...> _ext;
  METAL_ALWAYS_INLINE T& operator[](int idx) { return _data[idx]; }
  METAL_ALWAYS_INLINE const T& operator[](int idx) const { return _data[idx]; }
  METAL_ALWAYS_INLINE T* data() { return _data; }
  METAL_ALWAYS_INLINE constexpr int extent(int i) const { return _ext.extent(i); }
  static constexpr int rank() { return extents<T, Extents...>::rank(); }
};

} // namespace metal
#endif // __MDSPAN_EXTENTS_H
