// metal_array — MSL array + array_ref (cleanroom, address-space-qualified)
#ifndef _METAL_ARRAY_H_
#define _METAL_ARRAY_H_
#include <metal/metal_common>

namespace metal
{

// ============================================================================
// array<T, N> — fixed-size aggregate
// ============================================================================
template <typename T, int N>
struct array
{
  T __elems[N ? N : 1];

  // ===================== thread =====================
  METAL_FUNC constexpr size_t size() const thread { return N; }
  METAL_FUNC constexpr size_t max_size() const thread { return N; }
  METAL_FUNC constexpr bool empty() const thread { return N == 0; }
  METAL_FUNC constexpr thread T &front() thread { return __elems[0]; }
  METAL_FUNC constexpr const thread T &front() const thread { return __elems[0]; }
  METAL_FUNC constexpr thread T &back() thread { return __elems[N - (N != 0)]; }
  METAL_FUNC constexpr const thread T &back() const thread { return __elems[N - (N != 0)]; }
  METAL_FUNC constexpr thread T &operator[](size_t pos) thread { return __elems[pos]; }
  METAL_FUNC constexpr const thread T &operator[](size_t pos) const thread { return __elems[pos]; }
  METAL_FUNC constexpr thread T *data() thread { return __elems; }
  METAL_FUNC constexpr const thread T *data() const thread { return __elems; }

  // ===================== device =====================
  METAL_FUNC constexpr size_t size() const device { return N; }
  METAL_FUNC constexpr size_t max_size() const device { return N; }
  METAL_FUNC constexpr bool empty() const device { return N == 0; }
  METAL_FUNC constexpr device T &front() device { return __elems[0]; }
  METAL_FUNC constexpr const device T &front() const device { return __elems[0]; }
  METAL_FUNC constexpr device T &back() device { return __elems[N - (N != 0)]; }
  METAL_FUNC constexpr const device T &back() const device { return __elems[N - (N != 0)]; }
  METAL_FUNC constexpr device T &operator[](size_t pos) device { return __elems[pos]; }
  METAL_FUNC constexpr const device T &operator[](size_t pos) const device { return __elems[pos]; }
  METAL_FUNC constexpr device T *data() device { return __elems; }
  METAL_FUNC constexpr const device T *data() const device { return __elems; }

#if defined(__HAVE_COHERENT__)
  // ===================== device coherent(device) =====================
  METAL_FUNC constexpr size_t size() const device coherent(device) { return N; }
  METAL_FUNC constexpr size_t max_size() const device coherent(device) { return N; }
  METAL_FUNC constexpr bool empty() const device coherent(device) { return N == 0; }
  METAL_FUNC constexpr device coherent(device) T &front() device coherent(device) { return __elems[0]; }
  METAL_FUNC constexpr const device coherent(device) T &front() const device coherent(device) { return __elems[0]; }
  METAL_FUNC constexpr device coherent(device) T &back() device coherent(device) { return __elems[N - (N != 0)]; }
  METAL_FUNC constexpr const device coherent(device) T &back() const device coherent(device) { return __elems[N - (N != 0)]; }
  METAL_FUNC constexpr device coherent(device) T &operator[](size_t pos) device coherent(device) { return __elems[pos]; }
  METAL_FUNC constexpr const device coherent(device) T &operator[](size_t pos) const device coherent(device) { return __elems[pos]; }
  METAL_FUNC constexpr device coherent(device) T *data() device coherent(device) { return __elems; }
  METAL_FUNC constexpr const device coherent(device) T *data() const device coherent(device) { return __elems; }
#endif

  // ===================== threadgroup =====================
  METAL_FUNC constexpr size_t size() const threadgroup { return N; }
  METAL_FUNC constexpr size_t max_size() const threadgroup { return N; }
  METAL_FUNC constexpr bool empty() const threadgroup { return N == 0; }
  METAL_FUNC constexpr threadgroup T &front() threadgroup { return __elems[0]; }
  METAL_FUNC constexpr const threadgroup T &front() const threadgroup { return __elems[0]; }
  METAL_FUNC constexpr threadgroup T &back() threadgroup { return __elems[N - (N != 0)]; }
  METAL_FUNC constexpr const threadgroup T &back() const threadgroup { return __elems[N - (N != 0)]; }
  METAL_FUNC constexpr threadgroup T &operator[](size_t pos) threadgroup { return __elems[pos]; }
  METAL_FUNC constexpr const threadgroup T &operator[](size_t pos) const threadgroup { return __elems[pos]; }
  METAL_FUNC constexpr threadgroup T *data() threadgroup { return __elems; }
  METAL_FUNC constexpr const threadgroup T *data() const threadgroup { return __elems; }

#if defined(__HAVE_IMAGEBLOCKS__)
  // ===================== threadgroup_imageblock =====================
  METAL_FUNC constexpr size_t size() const threadgroup_imageblock { return N; }
  METAL_FUNC constexpr size_t max_size() const threadgroup_imageblock { return N; }
  METAL_FUNC constexpr bool empty() const threadgroup_imageblock { return N == 0; }
  METAL_FUNC constexpr threadgroup_imageblock T &front() threadgroup_imageblock { return __elems[0]; }
  METAL_FUNC constexpr const threadgroup_imageblock T &front() const threadgroup_imageblock { return __elems[0]; }
  METAL_FUNC constexpr threadgroup_imageblock T &back() threadgroup_imageblock { return __elems[N - (N != 0)]; }
  METAL_FUNC constexpr const threadgroup_imageblock T &back() const threadgroup_imageblock { return __elems[N - (N != 0)]; }
  METAL_FUNC constexpr threadgroup_imageblock T &operator[](size_t pos) threadgroup_imageblock { return __elems[pos]; }
  METAL_FUNC constexpr const threadgroup_imageblock T &operator[](size_t pos) const threadgroup_imageblock { return __elems[pos]; }
  METAL_FUNC constexpr threadgroup_imageblock T *data() threadgroup_imageblock { return __elems; }
  METAL_FUNC constexpr const threadgroup_imageblock T *data() const threadgroup_imageblock { return __elems; }
#endif

#if defined(__HAVE_RAYTRACING__)
  // ===================== ray_data =====================
  METAL_FUNC constexpr size_t size() const ray_data { return N; }
  METAL_FUNC constexpr size_t max_size() const ray_data { return N; }
  METAL_FUNC constexpr bool empty() const ray_data { return N == 0; }
  METAL_FUNC constexpr ray_data T &front() ray_data { return __elems[0]; }
  METAL_FUNC constexpr const ray_data T &front() const ray_data { return __elems[0]; }
  METAL_FUNC constexpr ray_data T &back() ray_data { return __elems[N - (N != 0)]; }
  METAL_FUNC constexpr const ray_data T &back() const ray_data { return __elems[N - (N != 0)]; }
  METAL_FUNC constexpr ray_data T &operator[](size_t pos) ray_data { return __elems[pos]; }
  METAL_FUNC constexpr const ray_data T &operator[](size_t pos) const ray_data { return __elems[pos]; }
  METAL_FUNC constexpr ray_data T *data() ray_data { return __elems; }
  METAL_FUNC constexpr const ray_data T *data() const ray_data { return __elems; }
#endif

#if defined(__HAVE_MESH__)
  // ===================== object_data =====================
  METAL_FUNC constexpr size_t size() const object_data { return N; }
  METAL_FUNC constexpr size_t max_size() const object_data { return N; }
  METAL_FUNC constexpr bool empty() const object_data { return N == 0; }
  METAL_FUNC constexpr object_data T &front() object_data { return __elems[0]; }
  METAL_FUNC constexpr const object_data T &front() const object_data { return __elems[0]; }
  METAL_FUNC constexpr object_data T &back() object_data { return __elems[N - (N != 0)]; }
  METAL_FUNC constexpr const object_data T &back() const object_data { return __elems[N - (N != 0)]; }
  METAL_FUNC constexpr object_data T &operator[](size_t pos) object_data { return __elems[pos]; }
  METAL_FUNC constexpr const object_data T &operator[](size_t pos) const object_data { return __elems[pos]; }
  METAL_FUNC constexpr object_data T *data() object_data { return __elems; }
  METAL_FUNC constexpr const object_data T *data() const object_data { return __elems; }
#endif

  // ===================== constant (read-only) =====================
  METAL_FUNC constexpr size_t size() const constant { return N; }
  METAL_FUNC constexpr size_t max_size() const constant { return N; }
  METAL_FUNC constexpr bool empty() const constant { return N == 0; }
  METAL_FUNC constexpr const constant T &front() const constant { return __elems[0]; }
  METAL_FUNC constexpr const constant T &back() const constant { return __elems[N - (N != 0)]; }
  METAL_FUNC constexpr const constant T &operator[](size_t pos) const constant { return __elems[pos]; }
  METAL_FUNC constexpr const constant T *data() const constant { return __elems; }

  // ===================== at() =====================
  METAL_FUNC constexpr thread T &at(size_t pos) thread { return __elems[pos]; }
  METAL_FUNC constexpr const thread T &at(size_t pos) const thread { return __elems[pos]; }
  METAL_FUNC constexpr device T &at(size_t pos) device { return __elems[pos]; }
  METAL_FUNC constexpr const device T &at(size_t pos) const device { return __elems[pos]; }
#if defined(__HAVE_COHERENT__)
  METAL_FUNC constexpr device coherent(device) T &at(size_t pos) device coherent(device) { return __elems[pos]; }
  METAL_FUNC constexpr const device coherent(device) T &at(size_t pos) const device coherent(device) { return __elems[pos]; }
#endif
  METAL_FUNC constexpr threadgroup T &at(size_t pos) threadgroup { return __elems[pos]; }
  METAL_FUNC constexpr const threadgroup T &at(size_t pos) const threadgroup { return __elems[pos]; }
#if defined(__HAVE_IMAGEBLOCKS__)
  METAL_FUNC constexpr threadgroup_imageblock T &at(size_t pos) threadgroup_imageblock { return __elems[pos]; }
  METAL_FUNC constexpr const threadgroup_imageblock T &at(size_t pos) const threadgroup_imageblock { return __elems[pos]; }
#endif
#if defined(__HAVE_RAYTRACING__)
  METAL_FUNC constexpr ray_data T &at(size_t pos) ray_data { return __elems[pos]; }
  METAL_FUNC constexpr const ray_data T &at(size_t pos) const ray_data { return __elems[pos]; }
#endif
#if defined(__HAVE_MESH__)
  METAL_FUNC constexpr object_data T &at(size_t pos) object_data { return __elems[pos]; }
  METAL_FUNC constexpr const object_data T &at(size_t pos) const object_data { return __elems[pos]; }
#endif

  // ===================== fill() =====================
  METAL_FUNC void fill(const T &value) thread { for (size_t i = 0; i < N; ++i) __elems[i] = value; }
  METAL_FUNC void fill(const T &value) device { for (size_t i = 0; i < N; ++i) __elems[i] = value; }
#if defined(__HAVE_COHERENT__)
  METAL_FUNC void fill(const T &value) device coherent(device) { for (size_t i = 0; i < N; ++i) __elems[i] = value; }
#endif
  METAL_FUNC void fill(const T &value) threadgroup { for (size_t i = 0; i < N; ++i) __elems[i] = value; }
#if defined(__HAVE_IMAGEBLOCKS__)
  METAL_FUNC void fill(const T &value) threadgroup_imageblock { for (size_t i = 0; i < N; ++i) __elems[i] = value; }
#endif
#if defined(__HAVE_RAYTRACING__)
  METAL_FUNC void fill(const T &value) ray_data { for (size_t i = 0; i < N; ++i) __elems[i] = value; }
#endif
#if defined(__HAVE_MESH__)
  METAL_FUNC void fill(const T &value) object_data { for (size_t i = 0; i < N; ++i) __elems[i] = value; }
#endif

  // ===================== swap() =====================
  METAL_FUNC void swap(thread array &other) thread { for (size_t i = 0; i < N; ++i) { T tmp = __elems[i]; __elems[i] = other.__elems[i]; other.__elems[i] = tmp; } }
  METAL_FUNC void swap(device array &other) device { for (size_t i = 0; i < N; ++i) { T tmp = __elems[i]; __elems[i] = other.__elems[i]; other.__elems[i] = tmp; } }
#if defined(__HAVE_COHERENT__)
  METAL_FUNC void swap(device coherent(device) array &other) device coherent(device) { for (size_t i = 0; i < N; ++i) { T tmp = __elems[i]; __elems[i] = other.__elems[i]; other.__elems[i] = tmp; } }
#endif
  METAL_FUNC void swap(threadgroup array &other) threadgroup { for (size_t i = 0; i < N; ++i) { T tmp = __elems[i]; __elems[i] = other.__elems[i]; other.__elems[i] = tmp; } }
#if defined(__HAVE_IMAGEBLOCKS__)
  METAL_FUNC void swap(threadgroup_imageblock array &other) threadgroup_imageblock { for (size_t i = 0; i < N; ++i) { T tmp = __elems[i]; __elems[i] = other.__elems[i]; other.__elems[i] = tmp; } }
#endif
#if defined(__HAVE_RAYTRACING__)
  METAL_FUNC void swap(ray_data array &other) ray_data { for (size_t i = 0; i < N; ++i) { T tmp = __elems[i]; __elems[i] = other.__elems[i]; other.__elems[i] = tmp; } }
#endif
#if defined(__HAVE_MESH__)
  METAL_FUNC void swap(object_data array &other) object_data { for (size_t i = 0; i < N; ++i) { T tmp = __elems[i]; __elems[i] = other.__elems[i]; other.__elems[i] = tmp; } }
#endif
};

// ============================================================================
// array_ref<T> — lightweight non-owning view (thread specialization)
// ============================================================================
template <typename T>
struct array_ref {
  METAL_FUNC constexpr array_ref() thread : d(nullptr), sz(0) {}
  METAL_FUNC constexpr array_ref(const thread T &elt) thread : d(&elt), sz(1) {}
  METAL_FUNC constexpr array_ref(const thread T *data, size_t size) thread : d(data), sz(size) {}
  template <int M>
  METAL_FUNC constexpr array_ref(const thread T (&data)[M]) thread : d(data), sz(M) {}
  template <int M>
  METAL_FUNC constexpr array_ref(const thread array<T, M> &data) thread : d(data.data()), sz(data.size()) {}
  METAL_FUNC constexpr size_t size() const thread { return sz; }
  METAL_FUNC constexpr bool empty() const thread { return sz == 0; }
  METAL_FUNC constexpr const thread T &front() const thread { return d[0]; }
  METAL_FUNC constexpr const thread T &back() const thread { return d[sz - 1]; }
  METAL_FUNC constexpr const thread T &operator[](size_t pos) const thread { return d[pos]; }
  METAL_FUNC constexpr const thread T *data() const thread { return d; }
private:
  const thread T *d;
  size_t sz;
};

// ============================================================================
// array_ref device specialization
// ============================================================================
template <typename T>
struct _array_ref_device {
  METAL_FUNC constexpr _array_ref_device() thread : d(nullptr), sz(0) {}
  METAL_FUNC constexpr _array_ref_device(const device T &elt) thread : d(&elt), sz(1) {}
  METAL_FUNC constexpr _array_ref_device(const device T *data, size_t size) thread : d(data), sz(size) {}
  template <int M>
  METAL_FUNC constexpr _array_ref_device(const device T (&data)[M]) thread : d(data), sz(M) {}
  template <int M>
  METAL_FUNC constexpr _array_ref_device(const device array<T, M> &data) thread : d(data.data()), sz(data.size()) {}
  METAL_FUNC constexpr size_t size() const thread { return sz; }
  METAL_FUNC constexpr bool empty() const thread { return sz == 0; }
  METAL_FUNC constexpr const device T &front() const thread { return d[0]; }
  METAL_FUNC constexpr const device T &back() const thread { return d[sz - 1]; }
  METAL_FUNC constexpr const device T &operator[](size_t pos) const thread { return d[pos]; }
  METAL_FUNC constexpr const device T *data() const thread { return d; }
private:
  const device T *d;
  size_t sz;
};

// ============================================================================
// array_ref threadgroup specialization
// ============================================================================
template <typename T>
struct _array_ref_threadgroup {
  METAL_FUNC constexpr _array_ref_threadgroup() thread : d(nullptr), sz(0) {}
  METAL_FUNC constexpr _array_ref_threadgroup(const threadgroup T &elt) thread : d(&elt), sz(1) {}
  METAL_FUNC constexpr _array_ref_threadgroup(const threadgroup T *data, size_t size) thread : d(data), sz(size) {}
  template <int M>
  METAL_FUNC constexpr _array_ref_threadgroup(const threadgroup T (&data)[M]) thread : d(data), sz(M) {}
  template <int M>
  METAL_FUNC constexpr _array_ref_threadgroup(const threadgroup array<T, M> &data) thread : d(data.data()), sz(data.size()) {}
  METAL_FUNC constexpr size_t size() const thread { return sz; }
  METAL_FUNC constexpr bool empty() const thread { return sz == 0; }
  METAL_FUNC constexpr const threadgroup T &front() const thread { return d[0]; }
  METAL_FUNC constexpr const threadgroup T &back() const thread { return d[sz - 1]; }
  METAL_FUNC constexpr const threadgroup T &operator[](size_t pos) const thread { return d[pos]; }
  METAL_FUNC constexpr const threadgroup T *data() const thread { return d; }
private:
  const threadgroup T *d;
  size_t sz;
};

#if defined(__HAVE_COHERENT__)
// ============================================================================
// array_ref device coherent specialization
// ============================================================================
template <typename T>
struct _array_ref_coherent {
  METAL_FUNC constexpr _array_ref_coherent() thread : d(nullptr), sz(0) {}
  METAL_FUNC constexpr _array_ref_coherent(const device coherent(device) T &elt) thread : d(&elt), sz(1) {}
  METAL_FUNC constexpr _array_ref_coherent(const device coherent(device) T *data, size_t size) thread : d(data), sz(size) {}
  template <int M>
  METAL_FUNC constexpr _array_ref_coherent(const device coherent(device) T (&data)[M]) thread : d(data), sz(M) {}
  template <int M>
  METAL_FUNC constexpr _array_ref_coherent(const device coherent(device) array<T, M> &data) thread : d(data.data()), sz(data.size()) {}
  METAL_FUNC constexpr size_t size() const thread { return sz; }
  METAL_FUNC constexpr bool empty() const thread { return sz == 0; }
  METAL_FUNC constexpr const device coherent(device) T &front() const thread { return d[0]; }
  METAL_FUNC constexpr const device coherent(device) T &back() const thread { return d[sz - 1]; }
  METAL_FUNC constexpr const device coherent(device) T &operator[](size_t pos) const thread { return d[pos]; }
  METAL_FUNC constexpr const device coherent(device) T *data() const thread { return d; }
private:
  const device coherent(device) T *d;
  size_t sz;
};
#endif

#if defined(__HAVE_IMAGEBLOCKS__)
// ============================================================================
// array_ref threadgroup_imageblock specialization
// ============================================================================
template <typename T>
struct _array_ref_imageblock {
  METAL_FUNC constexpr _array_ref_imageblock() thread : d(nullptr), sz(0) {}
  METAL_FUNC constexpr _array_ref_imageblock(const threadgroup_imageblock T &elt) thread : d(&elt), sz(1) {}
  METAL_FUNC constexpr _array_ref_imageblock(const threadgroup_imageblock T *data, size_t size) thread : d(data), sz(size) {}
  template <int M>
  METAL_FUNC constexpr _array_ref_imageblock(const threadgroup_imageblock T (&data)[M]) thread : d(data), sz(M) {}
  template <int M>
  METAL_FUNC constexpr _array_ref_imageblock(const threadgroup_imageblock array<T, M> &data) thread : d(data.data()), sz(data.size()) {}
  METAL_FUNC constexpr size_t size() const thread { return sz; }
  METAL_FUNC constexpr bool empty() const thread { return sz == 0; }
  METAL_FUNC constexpr const threadgroup_imageblock T &front() const thread { return d[0]; }
  METAL_FUNC constexpr const threadgroup_imageblock T &back() const thread { return d[sz - 1]; }
  METAL_FUNC constexpr const threadgroup_imageblock T &operator[](size_t pos) const thread { return d[pos]; }
  METAL_FUNC constexpr const threadgroup_imageblock T *data() const thread { return d; }
private:
  const threadgroup_imageblock T *d;
  size_t sz;
};
#endif

#if defined(__HAVE_RAYTRACING__)
// ============================================================================
// array_ref ray_data specialization
// ============================================================================
template <typename T>
struct _array_ref_raydata {
  METAL_FUNC constexpr _array_ref_raydata() thread : d(nullptr), sz(0) {}
  METAL_FUNC constexpr _array_ref_raydata(const ray_data T &elt) thread : d(&elt), sz(1) {}
  METAL_FUNC constexpr _array_ref_raydata(const ray_data T *data, size_t size) thread : d(data), sz(size) {}
  template <int M>
  METAL_FUNC constexpr _array_ref_raydata(const ray_data T (&data)[M]) thread : d(data), sz(M) {}
  template <int M>
  METAL_FUNC constexpr _array_ref_raydata(const ray_data array<T, M> &data) thread : d(data.data()), sz(data.size()) {}
  METAL_FUNC constexpr size_t size() const thread { return sz; }
  METAL_FUNC constexpr bool empty() const thread { return sz == 0; }
  METAL_FUNC constexpr const ray_data T &front() const thread { return d[0]; }
  METAL_FUNC constexpr const ray_data T &back() const thread { return d[sz - 1]; }
  METAL_FUNC constexpr const ray_data T &operator[](size_t pos) const thread { return d[pos]; }
  METAL_FUNC constexpr const ray_data T *data() const thread { return d; }
private:
  const ray_data T *d;
  size_t sz;
};
#endif

#if defined(__HAVE_MESH__)
// ============================================================================
// array_ref object_data specialization
// ============================================================================
template <typename T>
struct _array_ref_objectdata {
  METAL_FUNC constexpr _array_ref_objectdata() thread : d(nullptr), sz(0) {}
  METAL_FUNC constexpr _array_ref_objectdata(const object_data T &elt) thread : d(&elt), sz(1) {}
  METAL_FUNC constexpr _array_ref_objectdata(const object_data T *data, size_t size) thread : d(data), sz(size) {}
  template <int M>
  METAL_FUNC constexpr _array_ref_objectdata(const object_data T (&data)[M]) thread : d(data), sz(M) {}
  template <int M>
  METAL_FUNC constexpr _array_ref_objectdata(const object_data array<T, M> &data) thread : d(data.data()), sz(data.size()) {}
  METAL_FUNC constexpr size_t size() const thread { return sz; }
  METAL_FUNC constexpr bool empty() const thread { return sz == 0; }
  METAL_FUNC constexpr const object_data T &front() const thread { return d[0]; }
  METAL_FUNC constexpr const object_data T &back() const thread { return d[sz - 1]; }
  METAL_FUNC constexpr const object_data T &operator[](size_t pos) const thread { return d[pos]; }
  METAL_FUNC constexpr const object_data T *data() const thread { return d; }
private:
  const object_data T *d;
  size_t sz;
};
#endif

// ============================================================================
// array_ref constant specialization
// ============================================================================
template <typename T>
struct _array_ref_constant {
  METAL_FUNC constexpr _array_ref_constant() thread : d(nullptr), sz(0) {}
  METAL_FUNC constexpr _array_ref_constant(const constant T &elt) thread : d(&elt), sz(1) {}
  METAL_FUNC constexpr _array_ref_constant(const constant T *data, size_t size) thread : d(data), sz(size) {}
  template <int M>
  METAL_FUNC constexpr _array_ref_constant(const constant T (&data)[M]) thread : d(data), sz(M) {}
  template <int M>
  METAL_FUNC constexpr _array_ref_constant(const constant array<T, M> &data) thread : d(data.data()), sz(data.size()) {}
  METAL_FUNC constexpr size_t size() const thread { return sz; }
  METAL_FUNC constexpr bool empty() const thread { return sz == 0; }
  METAL_FUNC constexpr const constant T &front() const thread { return d[0]; }
  METAL_FUNC constexpr const constant T &back() const thread { return d[sz - 1]; }
  METAL_FUNC constexpr const constant T &operator[](size_t pos) const thread { return d[pos]; }
  METAL_FUNC constexpr const constant T *data() const thread { return d; }
private:
  const constant T *d;
  size_t sz;
};

// ============================================================================
// make_array_ref — factory functions (thread)
// ============================================================================
template <typename T>
METAL_FUNC constexpr array_ref<T> make_array_ref(const thread T &elt) { return array_ref<T>(elt); }
template <typename T>
METAL_FUNC constexpr array_ref<T> make_array_ref(const thread T *data, size_t size) { return array_ref<T>(data, size); }
template <typename T, int N>
METAL_FUNC constexpr array_ref<T> make_array_ref(const thread T (&data)[N]) { return array_ref<T>(data); }
template <typename T, int N>
METAL_FUNC constexpr array_ref<T> make_array_ref(const thread array<T, N> &data) { return array_ref<T>(data); }

// ============================================================================
// make_array_ref — factory functions (device)
// ============================================================================
template <typename T>
METAL_FUNC constexpr _array_ref_device<T> make_array_ref(const device T &elt) { return _array_ref_device<T>(elt); }
template <typename T>
METAL_FUNC constexpr _array_ref_device<T> make_array_ref(const device T *data, size_t size) { return _array_ref_device<T>(data, size); }
template <typename T, int N>
METAL_FUNC constexpr _array_ref_device<T> make_array_ref(const device T (&data)[N]) { return _array_ref_device<T>(data); }
template <typename T, int N>
METAL_FUNC constexpr _array_ref_device<T> make_array_ref(const device array<T, N> &data) { return _array_ref_device<T>(data); }

#if defined(__HAVE_COHERENT__)
// ============================================================================
// make_array_ref — factory functions (device coherent)
// ============================================================================
template <typename T>
METAL_FUNC constexpr _array_ref_coherent<T> make_array_ref(const device coherent(device) T &elt) { return _array_ref_coherent<T>(elt); }
template <typename T>
METAL_FUNC constexpr _array_ref_coherent<T> make_array_ref(const device coherent(device) T *data, size_t size) { return _array_ref_coherent<T>(data, size); }
template <typename T, int N>
METAL_FUNC constexpr _array_ref_coherent<T> make_array_ref(const device coherent(device) T (&data)[N]) { return _array_ref_coherent<T>(data); }
template <typename T, int N>
METAL_FUNC constexpr _array_ref_coherent<T> make_array_ref(const device coherent(device) array<T, N> &data) { return _array_ref_coherent<T>(data); }
#endif

// ============================================================================
// make_array_ref — factory functions (threadgroup)
// ============================================================================
template <typename T>
METAL_FUNC constexpr _array_ref_threadgroup<T> make_array_ref(const threadgroup T &elt) { return _array_ref_threadgroup<T>(elt); }
template <typename T>
METAL_FUNC constexpr _array_ref_threadgroup<T> make_array_ref(const threadgroup T *data, size_t size) { return _array_ref_threadgroup<T>(data, size); }
template <typename T, int N>
METAL_FUNC constexpr _array_ref_threadgroup<T> make_array_ref(const threadgroup T (&data)[N]) { return _array_ref_threadgroup<T>(data); }
template <typename T, int N>
METAL_FUNC constexpr _array_ref_threadgroup<T> make_array_ref(const threadgroup array<T, N> &data) { return _array_ref_threadgroup<T>(data); }

#if defined(__HAVE_IMAGEBLOCKS__)
// ============================================================================
// make_array_ref — factory functions (threadgroup_imageblock)
// ============================================================================
template <typename T>
METAL_FUNC constexpr _array_ref_imageblock<T> make_array_ref(const threadgroup_imageblock T &elt) { return _array_ref_imageblock<T>(elt); }
template <typename T>
METAL_FUNC constexpr _array_ref_imageblock<T> make_array_ref(const threadgroup_imageblock T *data, size_t size) { return _array_ref_imageblock<T>(data, size); }
template <typename T, int N>
METAL_FUNC constexpr _array_ref_imageblock<T> make_array_ref(const threadgroup_imageblock T (&data)[N]) { return _array_ref_imageblock<T>(data); }
template <typename T, int N>
METAL_FUNC constexpr _array_ref_imageblock<T> make_array_ref(const threadgroup_imageblock array<T, N> &data) { return _array_ref_imageblock<T>(data); }
#endif

#if defined(__HAVE_RAYTRACING__)
// ============================================================================
// make_array_ref — factory functions (ray_data)
// ============================================================================
template <typename T>
METAL_FUNC constexpr _array_ref_raydata<T> make_array_ref(const ray_data T &elt) { return _array_ref_raydata<T>(elt); }
template <typename T>
METAL_FUNC constexpr _array_ref_raydata<T> make_array_ref(const ray_data T *data, size_t size) { return _array_ref_raydata<T>(data, size); }
template <typename T, int N>
METAL_FUNC constexpr _array_ref_raydata<T> make_array_ref(const ray_data T (&data)[N]) { return _array_ref_raydata<T>(data); }
template <typename T, int N>
METAL_FUNC constexpr _array_ref_raydata<T> make_array_ref(const ray_data array<T, N> &data) { return _array_ref_raydata<T>(data); }
#endif

#if defined(__HAVE_MESH__)
// ============================================================================
// make_array_ref — factory functions (object_data)
// ============================================================================
template <typename T>
METAL_FUNC constexpr _array_ref_objectdata<T> make_array_ref(const object_data T &elt) { return _array_ref_objectdata<T>(elt); }
template <typename T>
METAL_FUNC constexpr _array_ref_objectdata<T> make_array_ref(const object_data T *data, size_t size) { return _array_ref_objectdata<T>(data, size); }
template <typename T, int N>
METAL_FUNC constexpr _array_ref_objectdata<T> make_array_ref(const object_data T (&data)[N]) { return _array_ref_objectdata<T>(data); }
template <typename T, int N>
METAL_FUNC constexpr _array_ref_objectdata<T> make_array_ref(const object_data array<T, N> &data) { return _array_ref_objectdata<T>(data); }
#endif

// ============================================================================
// make_array_ref — factory functions (constant)
// ============================================================================
template <typename T>
METAL_FUNC constexpr _array_ref_constant<T> make_array_ref(const constant T &elt) { return _array_ref_constant<T>(elt); }
template <typename T>
METAL_FUNC constexpr _array_ref_constant<T> make_array_ref(const constant T *data, size_t size) { return _array_ref_constant<T>(data, size); }
template <typename T, int N>
METAL_FUNC constexpr _array_ref_constant<T> make_array_ref(const constant T (&data)[N]) { return _array_ref_constant<T>(data); }
template <typename T, int N>
METAL_FUNC constexpr _array_ref_constant<T> make_array_ref(const constant array<T, N> &data) { return _array_ref_constant<T>(data); }

} // namespace metal
#endif // _METAL_ARRAY_H_
