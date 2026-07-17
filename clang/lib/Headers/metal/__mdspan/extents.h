// -*- Metal -*-
//===-- __mdspan/extents.h ------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//                        Kokkos v. 4.0
//       Copyright (2022) National Technology & Engineering
//               Solutions of Sandia, LLC (NTESS).
//
// Under the terms of Contract DE-NA0003525 with NTESS,
// the U.S. Government retains certain rights in this software.
//
// Copyright (c) 2025 Apple Inc. All rights reserved
//===----------------------------------------------------------------------===//

#ifndef _METAL___MDSPAN_EXTENTS_H
#define _METAL___MDSPAN_EXTENTS_H

#if defined(__HAVE_TENSOR__)
#include <metal_array>
#include <metal_assert>
#include <metal_limits>
#include <metal_utility>

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wc++20-extensions"

#define _ASSERT_VALID_ELEMENT_ACCESS(X, M) assert(X && M);

#pragma METAL internals : enable

namespace metal
{
inline constexpr constant size_t dynamic_extent = numeric_limits<size_t>::max();

namespace __mdspan_detail
{
// Function to check whether a value is representable as another type.
// `value` must be a positive integer otherwise returns `false`.
// If `From` is not an integral, we just check positivity.
template <class To, class From>
METAL_INTERNAL enable_if_t<is_integral_v<To> && is_integral_v<From>, bool> constexpr is_representable_as(From value)
{
  using To_u = make_unsigned_t<To>;
  using From_u = make_unsigned_t<From>;
  if constexpr (is_signed_v<From>)
  {
    if (value < 0)
      return false;
  }
  if constexpr (static_cast<To_u>(numeric_limits<To>::max()) >=
                static_cast<From_u>(numeric_limits<From>::max()))
  {
    return true;
  }
  else
  {
    return static_cast<To_u>(numeric_limits<To>::max()) >= static_cast<From_u>(value);
  }
}

template <class To, class From>
METAL_INTERNAL enable_if_t<is_integral_v<To> && !is_integral_v<From>, bool> constexpr is_representable_as(From value)
{
  if constexpr (is_signed_v<To>)
  {
    if (static_cast<To>(value) < 0)
      return false;
  }
  return true;
}

template <class To, class... From,
          typename = enable_if_t<is_integral_v<To>>>
METAL_INTERNAL constexpr bool are_representable_as(From... values)
{
  return (is_representable_as<To>(values) && ... && true);
}
} // namespace __mdspan_detail

template <class T, size_t N>
struct __possibly_empty_array
{
  METAL_FUNC constexpr __possibly_empty_array() thread = default;

  template <class... OtherT,
            typename = enable_if_t<sizeof...(OtherT) == N>>
  METAL_FUNC constexpr __possibly_empty_array(OtherT... other_vals) thread
      : vals{other_vals...}
  {
  }

  METAL_FUNC constexpr T get_dynamic(size_t idx) thread const
  {
    return vals[idx];
  }

  T vals[N];
};

template <class T>
struct __possibly_empty_array<T, 0>
{
  METAL_FUNC constexpr __possibly_empty_array() thread = default;

  METAL_FUNC constexpr T get_dynamic(size_t) thread const
  {
    return T();
  }
};

template <class E, class T, size_t... Vs>
struct __maybe_static_array_impl;

// Specialization for some static and some dynamic values.
template <class T, size_t... Vs>
struct __maybe_static_array_impl<false_type, T, Vs...>
    : __possibly_empty_array<T, ((Vs == dynamic_extent) + ... + 0)>
{
protected:
  static constexpr constant size_t size = sizeof...(Vs);
  static constexpr constant size_t dynamic_size = ((Vs == dynamic_extent) + ... + 0);
  static constexpr constant array<size_t, size> static_vals = {Vs...};

  using dynamic_values_t = __possibly_empty_array<T, dynamic_size>;

  template <size_t I>
  METAL_FUNC static constexpr size_t dynamic_idx()
  {
    size_t dyn_idx = 0;
    for (size_t i = 0; i != I; ++i)
      if (static_vals[i] == dynamic_extent)
        dyn_idx++;
    return dyn_idx;
  }

  static constexpr constant array<size_t, size> dynamic_map =
      []<size_t... Pos>(index_sequence<Pos...>)
  {
    return array<size_t, size>{dynamic_idx<Pos>()...};
  }(make_index_sequence<size>());

  METAL_FUNC static constexpr size_t get_static(size_t idx)
  {
    return static_vals[idx];
  }

  METAL_FUNC constexpr T operator[](size_t idx) thread const
  {
    return (get_static(idx) != dynamic_extent)
               ? get_static(idx)
               : this->get_dynamic(dynamic_map[idx]);
  }

  METAL_FUNC constexpr __maybe_static_array_impl() thread = default;

  template <class... OtherTs>
  METAL_FUNC constexpr __maybe_static_array_impl(OtherTs... dvals) thread
      : dynamic_values_t(static_cast<T>(dvals)...)
  {
  }
};

// Specialization for all dynamic values.
template <class T, size_t... Vs>
struct __maybe_static_array_impl<true_type, T, Vs...>
    : array<T, sizeof...(Vs)>
{
protected:
  METAL_FUNC constexpr __maybe_static_array_impl() thread = default;
  template <class... OtherTs>
  METAL_FUNC constexpr __maybe_static_array_impl(OtherTs... dvals) thread
      : array<T, sizeof...(Vs)>{static_cast<T>(dvals)...}
  {
  }

  METAL_FUNC static constexpr size_t get_static(size_t)
  {
    return dynamic_extent;
  }
  METAL_FUNC constexpr T operator[](size_t idx) thread const
  {
    return this->array<T, sizeof...(Vs)>::operator[](idx);
  }
};

template <class T, size_t... Vs>
struct __maybe_static_array : __maybe_static_array_impl<bool_constant<((Vs == dynamic_extent) && ... && true)>, T, Vs...>
{
private:
  using base = __maybe_static_array_impl<bool_constant<((Vs == dynamic_extent) && ... && true)>, T, Vs...>;
  static constexpr constant size_t dynamic_size = ((Vs == dynamic_extent) + ... + 0);

public:
  METAL_FUNC constexpr __maybe_static_array() thread
      : base()
  {
  }

  template <class... OtherTs,
            typename = enable_if_t<(is_convertible_v<OtherTs, T> && ...)>,
            typename = enable_if_t<sizeof...(OtherTs) == dynamic_size>>
  METAL_FUNC constexpr __maybe_static_array(OtherTs... dvals) thread
      : base(static_cast<T>(dvals)...)
  {
  }

  METAL_FUNC static constexpr size_t get_static(size_t idx)
  {
    return base::get_static(idx);
  }
  METAL_FUNC constexpr T operator[](size_t idx) thread const
  {
    return this->base::operator[](idx);
  }
};

// Class to describe the extents of a multi dimensional array.
// Used by mdspan, mdarray and layout mappings.
// See ISO C++ standard [mdspan.extents]

template <class IndexType, size_t... Extents>
class extents
{
  static_assert(is_integral<IndexType>::value && !is_same<IndexType, bool>::value,
                "extents::index_type must be a signed or unsigned integer type");
  static_assert(((__mdspan_detail::is_representable_as<IndexType>(Extents) || (Extents == dynamic_extent)) && ...),
                "extents ctor: arguments must be representable as index_type and nonnegative");

public:
  // typedefs for integral types used
  using index_type = IndexType;
  using size_type = make_unsigned_t<index_type>;
  using rank_type = size_t;

private:
  static constexpr constant rank_type _rank = sizeof...(Extents);
  static constexpr constant rank_type _rank_dynamic = ((Extents == dynamic_extent) + ... + 0);

  using Values = __maybe_static_array<IndexType, Extents...>;
  Values vals;

public:
  // [mdspan.extents.obs], observers of multidimensional index space
  METAL_FUNC static constexpr rank_type rank()
  {
    return _rank;
  }
  METAL_FUNC static constexpr rank_type rank_dynamic()
  {
    return _rank_dynamic;
  }

  METAL_FUNC constexpr index_type extent(rank_type r) thread const
  {
    return vals[r];
  }
  METAL_FUNC static constexpr size_t static_extent(rank_type r)
  {
    return Values::get_static(r);
  }

  // [mdspan.extents.cons], constructors
  METAL_FUNC constexpr extents() thread = default;

  // Construction from just dynamic or all values.
  template <class... OtherIndexTypes,
            typename = enable_if_t<(is_convertible_v<OtherIndexTypes, index_type> && ...)>,
            typename = enable_if_t<sizeof...(OtherIndexTypes) == rank_dynamic()>>
  METAL_FUNC constexpr explicit extents(OtherIndexTypes... dynvals) thread
      : vals{static_cast<index_type>(dynvals)...}
  {
    // Not catching this could lead to out of bounds errors later
    // e.g. mdspan m(ptr, dextents<char, 1>(200u)); leads to an extent of -56 on m
    _ASSERT_VALID_ELEMENT_ACCESS(
        __mdspan_detail::are_representable_as<index_type>(dynvals...),
        "extents ctor: arguments must be representable as index_type and nonnegative");
  }

private:
  // Function to construct extents storage from other extents.
  template <size_t Idx, class OtherExtents, class... DynamicValues>
  METAL_INTERNAL constexpr enable_if_t<(Idx < rank()), Values>
  construct_vals_from_extents(integral_constant<size_t, Idx>,
                              thread const OtherExtents &exts,
                              DynamicValues... dynamic_values) thread
  {
    return construct_vals_from_extents(
        integral_constant<size_t, Idx + 1>(),
        exts,
        dynamic_values...,
        exts.extent(Idx));
  }

  template <size_t Idx, class OtherExtents, class... DynamicValues>
  METAL_INTERNAL constexpr enable_if_t<Idx == rank(), Values> construct_vals_from_extents(
      integral_constant<size_t, Idx>,
      thread const OtherExtents &,
      DynamicValues... dynamic_values) thread
  {
    return Values{static_cast<index_type>(dynamic_values)...};
  }

  template <class _OtherIndexType, size_t... OtherExtents>
  METAL_INTERNAL constexpr void check_representability(
      thread const extents<_OtherIndexType, OtherExtents...> &other) thread
  {
    if constexpr (rank() > 0)
    {
      for (size_t r = 0; r < rank(); r++)
      {
        if constexpr (static_cast<make_unsigned_t<index_type>>(numeric_limits<index_type>::max()) <
                      static_cast<make_unsigned_t<_OtherIndexType>>(numeric_limits<_OtherIndexType>::max()))
        {
          // Not catching this could lead to out of bounds errors later
          // e.g. dextents<char,1>> e(dextents<unsigned,1>(200)) leads to an extent of -56 on e
          _ASSERT_VALID_ELEMENT_ACCESS(
              __mdspan_detail::is_representable_as<index_type>(other.extent(r)),
              "extents ctor: arguments must be representable as index_type and nonnegative");
        }
      }
    }
  }

public:
  // Converting constructor from other extents specializations
  template <class _OtherIndexType, size_t... OtherExtents,
            typename = enable_if_t<(sizeof...(OtherExtents) == sizeof...(Extents))>,
            enable_if_t<(static_cast<make_unsigned_t<index_type>>(numeric_limits<index_type>::max()) <
                         static_cast<make_unsigned_t<_OtherIndexType>>(numeric_limits<_OtherIndexType>::max())),
                        short> = 0>
  METAL_FUNC explicit constexpr extents(
      thread const extents<_OtherIndexType, OtherExtents...> &other) thread
      : vals(construct_vals_from_extents(integral_constant<size_t, 0>(), other))
  {
    check_representability(other);
  }

  template <class _OtherIndexType, size_t... OtherExtents,
            typename = enable_if_t<(sizeof...(OtherExtents) == sizeof...(Extents))>,
            enable_if_t<(static_cast<make_unsigned_t<index_type>>(numeric_limits<index_type>::max()) >=
                         static_cast<make_unsigned_t<_OtherIndexType>>(numeric_limits<_OtherIndexType>::max())),
                        int> = 0>
  METAL_FUNC constexpr extents(thread const extents<_OtherIndexType, OtherExtents...> &other) thread
      : vals(construct_vals_from_extents(integral_constant<size_t, 0>(), other))
  {
    check_representability(other);
  }

  // Comparison operator
  template <class _OtherIndexType, size_t... OtherExtents>
  METAL_FUNC friend constexpr bool
  operator==(thread const extents &lhs,
             thread const extents<_OtherIndexType, OtherExtents...> &rhs)
  {
    if constexpr (rank() != sizeof...(OtherExtents))
    {
      return false;
    }
    else
    {
      for (rank_type r = 0; r < rank(); r++)
      {
        // avoid warning when comparing signed and unsigner integers
        // and pick the wider of two types
        using CommonType = common_type_t<index_type, _OtherIndexType>;
        if (static_cast<CommonType>(lhs.extent(r)) !=
            static_cast<CommonType>(rhs.extent(r)))
        {
          return false;
        }
      }
    }
    return true;
  }
};

// Recursive helper classes to implement dextents alias for extents
namespace __mdspan_detail
{
template <class IndexType, size_t Rank, class Extents = extents<IndexType>>
struct make_dextents;

template <class IndexType, size_t Rank, size_t... ExtentsPack>
struct make_dextents<IndexType, Rank, extents<IndexType, ExtentsPack...>>
{
  using type =
      typename make_dextents<IndexType, Rank - 1,
                             extents<IndexType, dynamic_extent, ExtentsPack...>>::type;
};

template <class IndexType, size_t... ExtentsPack>
struct make_dextents<IndexType, 0, extents<IndexType, ExtentsPack...>>
{
  using type = extents<IndexType, ExtentsPack...>;
};
} // namespace __mdspan_detail

// [mdspan.extents.dextents], alias template
template <class IndexType, size_t Rank>
using dextents = typename __mdspan_detail::make_dextents<IndexType, Rank>::type;

// Deduction guide for extents
template <class... IndexTypes>
extents(IndexTypes...) -> extents<size_t, size_t(((void)sizeof(IndexTypes), dynamic_extent))...>;

namespace __mdspan_detail
{
// Helper type traits for identifying a class as extents.
template <class Tp>
struct is_extents : false_type
{
};

template <class IndexType, size_t... ExtentsPack>
struct is_extents<extents<IndexType, ExtentsPack...>> : true_type
{
};

template <class Tp>
constant inline constexpr bool is_extents_v = is_extents<Tp>::value;

// Function to check whether a set of indices are a multidimensional
// index into extents. This is a word of power in the C++ standard
// requiring that the indices are larger than 0 and smaller than
// the respective extents.

template <class IndexType, class From>
METAL_INTERNAL enable_if_t<is_integral_v<IndexType> && is_integral_v<From>, bool> constexpr _is_index_in_extent(IndexType extent, From value)
{
  if constexpr (is_signed_v<From>)
  {
    if (value < 0)
      return false;
  }
  using Tp = common_type_t<IndexType, From>;
  return static_cast<Tp>(value) < static_cast<Tp>(extent);
}

template <class IndexType, class From>
METAL_INTERNAL enable_if_t<is_integral_v<IndexType> && !is_integral_v<From>, bool> constexpr _is_index_in_extent(IndexType extent, From value)
{
  if constexpr (is_signed_v<IndexType>)
  {
    if (static_cast<IndexType>(value) < 0)
      return false;
  }
  return static_cast<IndexType>(value) < extent;
}

template <size_t... Idxs, class Extents, class... From>
METAL_INTERNAL constexpr bool _is_multidimensional_index_in_impl(
    index_sequence<Idxs...>, thread const Extents &ext, From... values)
{
  return (__mdspan_detail::_is_index_in_extent(ext.extent(Idxs), values) && ...);
}

template <class Extents, class... From>
METAL_INTERNAL constexpr bool is_multidimensional_index_in(thread const Extents &ext, From... values)
{
  return __mdspan_detail::_is_multidimensional_index_in_impl(
      make_index_sequence<Extents::rank()>(), ext, values...);
}
} // namespace __mdspan_detail
} // namespace metal

#pragma METAL internals : disable

#undef _ASSERT_VALID_ELEMENT_ACCESS

#pragma clang diagnostic pop

#endif

#endif // _METAL___MDSPAN_EXTENTS_H
