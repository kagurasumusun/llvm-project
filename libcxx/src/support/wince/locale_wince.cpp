//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

// Windows CE locale backend -- transitional state (policy v2, 2026-09-17).
//
// Windows CE provides its own NLS (GetLocaleInfo/SetLocaleInfo, the Enum*
// NLS functions, CompareString, LCMapString, GetStringType*, Coreloc.lib
// -- all officially documented), and project policy (cellvm-sdk
// docs/clean-room.md v2) requires this backend to obtain locale data
// through that WinCE-specific mechanism instead of a generic C
// substitute.  The former claim that a CE image offers "only the classic
// C locale" is withdrawn as factually wrong; see the note in
// <__locale_dir/support/wince.h>.
//
// History of this file: it originally queried the device NLS data via
// GetLocaleInfoW using the LOCALE_* LCTYPE constants and
// LOCALE_USER_DEFAULT/CP_ACP.  The numeric values of those constants are
// printed by no official Windows CE page (the LCTYPE Constants references
// of every generation list names only), so the WinCE SDK headers hold
// them as documented-but-unpublished and the TU could not compile
// against them.  It was then reworked (cc5c872a8) to the C-locale
// constant below -- under the meanwhile-withdrawn "C locale only"
// convention.  That rework's standard-C-contract fixes (no MSVC-internal
// wctype masks, standard strtof/strtold, no null-pointer arithmetic, no
// lconv leak) remain valid; its localeconv constant is a TRANSITIONAL
// FALLBACK.  The pending NLS rework (task D-2) reconnects __localeconv
// to GetLocaleInfoW once the LCTYPE/LCID constants are re-verified and
// re-adopted under policy v2 (official pages first; CeGCC-lineage value
// confirmation where the pages print names only), at which point this
// TU gains its <windows.h>/<winnls.h> dependency back.

#include <__locale_dir/support/wince.h>

#include <climits>
#include <clocale>
#include <cstdarg>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cwchar>
#include <cwctype>

_LIBCPP_BEGIN_NAMESPACE_STD
namespace __locale {

__locale_t __newlocale(int, const char* locale, __locale_t) {
  if (locale == nullptr)
    return __locale_t();
  if (locale[0] == '\0' || std::strcmp(locale, "C") == 0 || std::strcmp(locale, "POSIX") == 0)
    return __locale_t("C");
  return __locale_t();
}

namespace {

// The "C" locale lconv of C99 7.11.2.1: decimal_point is ".", every
// other string category is "", and every numeric category is CHAR_MAX
// (the standard's "value is not available").  Members are assigned by
// name because the member order of struct lconv is implementation-
// defined; llvm-libc, the C library this runtimes stack migrates to,
// uses the MSVC-compatible order and its own localeconv() produces
// exactly these values.
lconv __make_c_lconv() {
  static char __dot[]   = ".";
  static char __empty[] = "";

  lconv __lc = {};

  __lc.decimal_point     = __dot;
  __lc.thousands_sep     = __empty;
  __lc.grouping          = __empty;
  __lc.int_curr_symbol   = __empty;
  __lc.currency_symbol   = __empty;
  __lc.mon_decimal_point = __empty;
  __lc.mon_thousands_sep = __empty;
  __lc.mon_grouping      = __empty;
  __lc.positive_sign     = __empty;
  __lc.negative_sign     = __empty;

  __lc.frac_digits     = CHAR_MAX;
  __lc.int_frac_digits = CHAR_MAX;
  __lc.p_cs_precedes   = CHAR_MAX;
  __lc.p_sep_by_space  = CHAR_MAX;
  __lc.n_cs_precedes   = CHAR_MAX;
  __lc.n_sep_by_space  = CHAR_MAX;
  __lc.p_sign_posn     = CHAR_MAX;
  __lc.n_sign_posn     = CHAR_MAX;

  // The C99 international members, plus the int_p_sign_posn/
  // int_n_sign_posn pair that the MSVC-compatible layout implemented
  // by llvm-libc adds; all are CHAR_MAX in the "C" locale.
  __lc.int_p_cs_precedes  = CHAR_MAX;
  __lc.int_n_cs_precedes  = CHAR_MAX;
  __lc.int_p_sep_by_space = CHAR_MAX;
  __lc.int_n_sep_by_space = CHAR_MAX;
  __lc.int_p_sign_posn    = CHAR_MAX;
  __lc.int_n_sign_posn    = CHAR_MAX;

  return __lc;
}

} // namespace

__lconv_t* __localeconv(__locale_t& loc) {
  // TRANSITIONAL FALLBACK (see the file-top note): while __newlocale
  // hands out nothing but "C", the lconv is the constant C99 7.11.2.1
  // itself specifies.  Task D-2 replaces this with the WinCE NLS query
  // (GetLocaleInfoW over the re-verified LOCALE_* constants) once the
  // backend accepts non-C locales.
  (void)loc;
  static lconv __c_lconv = __make_c_lconv();
  return &__c_lconv;
}

float __strtof(const char* nptr, char** endptr, __locale_t loc) {
  (void)loc;
  return ::strtof(nptr, endptr);
}

decltype(MB_CUR_MAX) __mb_len_max(__locale_t __l) {
  (void)__l;
  return MB_CUR_MAX;
}

wint_t __btowc(int c, __locale_t loc) {
  (void)loc;
  return std::btowc(c);
}

int __wctob(wint_t c, __locale_t loc) {
  (void)loc;
  return std::wctob(c);
}

size_t __mbrtowc(wchar_t* __restrict pwc, const char* __restrict s, size_t n, mbstate_t* __restrict ps, __locale_t loc) {
  (void)loc;
  return ::mbrtowc(pwc, s, n, ps);
}

size_t __mbrlen(const char* __restrict s, size_t n, mbstate_t* __restrict ps, __locale_t loc) {
  (void)loc;
  return ::mbrlen(s, n, ps);
}

size_t __wcrtomb(char* __restrict s, wchar_t wc, mbstate_t* __restrict ps, __locale_t loc) {
  (void)loc;
  return ::wcrtomb(s, wc, ps);
}

size_t __mbsrtowcs(
    wchar_t* __restrict dst, const char** __restrict src, size_t len, mbstate_t* __restrict ps, __locale_t loc) {
  (void)loc;
  return ::mbsrtowcs(dst, src, len, ps);
}

size_t __mbsnrtowcs(
    wchar_t* __restrict dst, const char** __restrict src, size_t nms, size_t len, mbstate_t* __restrict ps, __locale_t loc) {
  (void)loc;
  if (src == nullptr || *src == nullptr)
    return 0;
  mbstate_t state = ps ? *ps : mbstate_t();
  // Keep the start pointer locally: *src is set to nullptr once the
  // terminating null character converts, and measuring progress as
  // (p - *src) afterwards would subtract through a null pointer (UB).
  const char* base    = *src;
  const char* p       = base;
  size_t produced     = 0;
  bool saw_nul        = false;
  while (!saw_nul && static_cast<size_t>(p - base) < nms) {
    wchar_t wc;
    size_t remaining = nms - static_cast<size_t>(p - base);
    size_t r         = ::mbrtowc(&wc, p, remaining, &state);
    if (r == (size_t)-1 || r == (size_t)-2)
      return (size_t)-1;
    if (r == 0) {
      r       = 1;
      saw_nul = true;
      if (dst != nullptr) {
        if (produced == len)
          return (size_t)-1;
        dst[produced] = L'\0';
      }
      *src = nullptr;
    } else if (dst != nullptr) {
      if (produced == len)
        return (size_t)-1;
      dst[produced] = wc;
    }
    if (!saw_nul)
      ++produced;
    p += r;
  }
  if (!saw_nul && dst != nullptr)
    *src = p;
  if (ps != nullptr)
    *ps = state;
  return produced;
}

size_t __wcsnrtombs(
    char* __restrict dst, const wchar_t** __restrict src, size_t nwc, size_t len, mbstate_t* __restrict ps, __locale_t loc) {
  (void)loc;
  if (src == nullptr || *src == nullptr)
    return 0;
  mbstate_t state = ps ? *ps : mbstate_t();
  // Same null-pointer-subtraction guard as __mbsnrtowcs above.
  const wchar_t* base = *src;
  const wchar_t* p    = base;
  size_t produced     = 0;
  bool saw_nul        = false;
  char buf[8];
  while (!saw_nul && static_cast<size_t>(p - base) < nwc) {
    size_t r = ::wcrtomb(buf, *p, &state);
    if (r == (size_t)-1)
      return (size_t)-1;
    if (*p == L'\0')
      saw_nul = true;
    if (dst != nullptr) {
      size_t needed = saw_nul ? r - 1 : r;
      if (produced + needed > len)
        return (size_t)-1;
      for (size_t i = 0; i < r; ++i)
        dst[produced + i] = buf[i];
      produced += r;
    } else {
      produced += saw_nul ? r - 1 : r;
    }
    if (saw_nul)
      *src = nullptr;
    else
      ++p;
  }
  if (!saw_nul && dst != nullptr)
    *src = p;
  if (ps != nullptr)
    *ps = state;
  return produced;
}

int __snprintf(char* ret, size_t n, __locale_t loc, const char* format, ...) {
  (void)loc;
  va_list ap;
  va_start(ap, format);
  _LIBCPP_DIAGNOSTIC_PUSH
  _LIBCPP_CLANG_DIAGNOSTIC_IGNORED("-Wformat-nonliteral")
  int result = std::vsnprintf(ret, n, format, ap);
  _LIBCPP_DIAGNOSTIC_POP
  va_end(ap);
  return result;
}

static int __libcpp_vasprintf(char** sptr, const char* __restrict format, va_list ap) {
  *sptr = nullptr;
  va_list ap_copy;
  va_copy(ap_copy, ap);
  _LIBCPP_DIAGNOSTIC_PUSH
  _LIBCPP_CLANG_DIAGNOSTIC_IGNORED("-Wformat-nonliteral")
  int count = vsnprintf(nullptr, 0, format, ap_copy);
  _LIBCPP_DIAGNOSTIC_POP
  va_end(ap_copy);
  if (count < 0)
    return count;
  size_t buffer_size = static_cast<size_t>(count) + 1;
  char* p            = static_cast<char*>(malloc(buffer_size));
  if (!p)
    return -1;
  _LIBCPP_DIAGNOSTIC_PUSH
  _LIBCPP_CLANG_DIAGNOSTIC_IGNORED("-Wformat-nonliteral")
  if (vsnprintf(p, buffer_size, format, ap) != count) {
    _LIBCPP_DIAGNOSTIC_POP
    free(p);
    return -1;
  }
  *sptr = p;
  return count;
}

int __asprintf(char** ret, __locale_t loc, const char* format, ...) {
  (void)loc;
  va_list ap;
  va_start(ap, format);
  return __libcpp_vasprintf(ret, format, ap);
}

} // namespace __locale
_LIBCPP_END_NAMESPACE_STD
