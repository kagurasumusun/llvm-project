// -*- Metal -*-
//===-- __exec/units.h ------------------------------------------------===//
// Copyright (c) 2025 Apple Inc. All rights reserved
//===----------------------------------------------------------------------===//

#ifndef __METAL___EXEC_UNITS_H
#define __METAL___EXEC_UNITS_H

#if defined(__HAVE_EXECUTION_UNIT__)

#include <metal_limits>

namespace metal
{
template <size_t>
struct execution_threads;
template <size_t>
struct execution_simdgroups;

#pragma METAL internals : enable

namespace __execution_detail
{
inline constexpr constant size_t dynamic_size = numeric_limits<size_t>::max();

template <class T>
struct __is_execution_threads_impl : false_type
{
};
template <size_t Size>
struct __is_execution_threads_impl<execution_threads<Size>> : true_type
{
};

template <class T>
struct __is_execution_simdgroups_impl : false_type
{
};
template <size_t Size>
struct __is_execution_simdgroups_impl<execution_simdgroups<Size>> : true_type
{
};
} // namespace __execution_detail

template <class T>
struct is_execution_threads : __execution_detail::__is_execution_threads_impl<remove_cv_t<T>>
{
};
template <class T>
constexpr constant bool is_execution_threads_v = is_execution_threads<T>::value;

template <class T>
struct is_execution_simdgroups : __execution_detail::__is_execution_simdgroups_impl<remove_cv_t<T>>
{
};
template <class T>
constexpr constant bool is_execution_simdgroups_v = is_execution_simdgroups<T>::value;

template <class T>
struct is_execution_dthreads : is_same<remove_cv_t<T>, execution_threads<__execution_detail::dynamic_size>>
{
};
template <class T>
constexpr constant bool is_execution_dthreads_v = is_execution_dthreads<T>::value;

template <class T>
struct is_execution_dsimdgroups : is_same<remove_cv_t<T>, execution_simdgroups<__execution_detail::dynamic_size>>
{
};
template <class T>
constexpr constant bool is_execution_dsimdgroups_v = is_execution_dsimdgroups<T>::value;

template <class T>
struct is_execution_thread : is_same<remove_cv_t<T>, execution_threads<1>>
{
};
template <class T>
constexpr constant bool is_execution_thread_v = is_execution_thread<T>::value;

template <class T>
struct is_execution_simdgroup : is_same<remove_cv_t<T>, execution_simdgroups<1>>
{
};
template <class T>
constexpr constant bool is_execution_simdgroup_v = is_execution_simdgroup<T>::value;

#pragma METAL internals : disable

template <size_t Size>
class execution_simdgroups
{
  static_assert(Size != 0, "execution_simgroups<0> is not supported");

  using size_type = uint;

public:
  static constexpr constant size_t static_size = Size;

public:
  METAL_FUNC constexpr size_type size() thread const
  {
    return static_size;
  }
};

template <>
class execution_simdgroups<__execution_detail::dynamic_size>
{
  using size_type = uint;

public:
  static constexpr constant size_t static_size = __execution_detail::dynamic_size;

public:
  METAL_FUNC execution_simdgroups(size_type size) thread : _size(size)
  {
  }

  METAL_FUNC size_type size() thread const
  {
    return _size;
  }

private:
  size_type _size;
};

using execution_dsimdgroups = execution_simdgroups<__execution_detail::dynamic_size>;
using execution_simdgroup = execution_simdgroups<1>;

template <size_t Size>
class execution_threads
{
  static_assert(Size == 1, "Only execution_thread<1> is supported");

  using size_type = uint;

public:
  static constexpr constant size_t static_size = Size;

public:
  METAL_FUNC thread execution_threads &operator=(const thread execution_threads &other) = default;

public:
  METAL_FUNC constexpr size_type size() thread const
  {
    return static_size;
  }
};

template <>
class execution_threads<__execution_detail::dynamic_size>
{
  using size_type = uint;

public:
  static constexpr constant size_t static_size = __execution_detail::dynamic_size;

public:
  // Conversion constructor simdgroups -> threads
  template <size_t OtherSize>
  execution_threads(execution_simdgroups<OtherSize> other) thread
      : _size(other.size() * __metal_get_simdgroup_size(size_type()))
  {
  }

  // Conversion assignment simdgroups -> threads
  template <size_t OtherSize>
  METAL_FUNC thread execution_threads &operator=(const thread execution_simdgroups<OtherSize> &other) thread
  {
    _size = other.size() * __metal_get_simdgroup_size(size_type());
    return *this;
  }

  METAL_FUNC size_type size() thread const
  {
    return _size;
  }

private:
  size_type _size;
};

using execution_dthreads = execution_threads<__execution_detail::dynamic_size>;
using execution_thread = execution_threads<1>;

// Deduction guides
template <class T, typename = enable_if_t<is_integral_v<T>>>
execution_simdgroups(T)
    -> execution_simdgroups<__execution_detail::dynamic_size>;

template <size_t Size>
execution_threads(execution_simdgroups<Size>)
    -> execution_threads<__execution_detail::dynamic_size>;

} // namespace metal

#endif

#endif // __METAL___EXEC_UNITS_H
