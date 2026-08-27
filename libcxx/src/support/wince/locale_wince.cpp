//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

// Windows CE libc++ locale backend implementation.  See
// <__locale_dir/support/wince.h> for the design.

#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <winnls.h>

#include <__locale_dir/support/wince.h>
#include <clocale> // std::lconv
#include <cstdarg> // va_start & friends
#include <cstddef>
#include <cstdio>  // vsnprintf
#include <cstdlib> // MB_CUR_MAX, strtod
#include <cstring>
#include <cwchar> // mbrtowc / wcrtomb / mbsrtowcs / wcsrtombs (mingwex)
#include <cwctype>

_LIBCPP_BEGIN_NAMESPACE_STD
namespace __locale {

//
// Locale management
//

// The WinCE CRT has a single, process-wide "C" locale; the only names a
// program may therefore construct are the C-locale spellings.  Any other
// name fails, which libc++ turns into the same runtime_error a desktop
// Windows program gets for an unknown locale name.
__locale_t __newlocale(int /*mask*/, const char* locale, __locale_t /*base*/) {
  if (locale == nullptr)
    return __locale_t();
  if (locale[0] == '\0' || std::strcmp(locale, "C") == 0 || std::strcmp(locale, "POSIX") == 0)
    return __locale_t("C");
  return __locale_t();
}

namespace {

// Query one NLS string for the device locale; returns "" when the NLS
// data does not carry the item (never fails the caller).
std::string nls_string(LCTYPE type) {
  wchar_t wbuf[64];
  int n = ::GetLocaleInfoW(LOCALE_USER_DEFAULT, type, wbuf, sizeof(wbuf) / sizeof(wbuf[0]));
  if (n <= 0)
    return std::string();
  char buf[64];
  // n includes the terminating NUL; convert without it.
  int m = ::WideCharToMultiByte(CP_ACP, 0, wbuf, n - 1, buf, sizeof(buf) - 1, nullptr, nullptr);
  if (m < 0)
    m = 0;
  buf[m] = '\0';
  return std::string(buf);
}

// Query one NLS number (LOCALE_I*); returns `fallback` when unavailable.
int nls_int(LCTYPE type, int fallback) {
  std::string s = nls_string(type);
  if (s.empty())
    return fallback;
  return std::atoi(s.c_str());
}

} // namespace

// The lconv for the WinCE backend carries the device NLS monetary /
// numeric data: it is what a WinCE application sees through
// localeconv().  The strings are copied into the locale's lconv
// storage, so the temporaries below only need to outlive this call.
__lconv_t* __localeconv(__locale_t& loc) {
  static lconv lc_c;
  if (!loc)
    return &lc_c;

  std::string decimal_point(".");
  std::string thousands_sep;
  std::string grouping;
  std::string int_curr_symbol;
  std::string currency_symbol;
  std::string mon_decimal_point;
  std::string mon_thousands_sep;
  std::string mon_grouping;
  std::string positive_sign;
  std::string negative_sign;

  std::string s;
  if (!(s = nls_string(LOCALE_SDECIMAL)).empty())
    decimal_point = s;
  if (!(s = nls_string(LOCALE_STHOUSAND)).empty())
    thousands_sep = s;
  if (!(s = nls_string(LOCALE_SGROUPING)).empty())
    grouping = s;
  if (!(s = nls_string(LOCALE_SINTLSYMBOL)).empty())
    int_curr_symbol = s;
  if (!(s = nls_string(LOCALE_SCURRENCY)).empty())
    currency_symbol = s;
  if (!(s = nls_string(LOCALE_SMONDECIMALSEP)).empty())
    mon_decimal_point = s;
  if (!(s = nls_string(LOCALE_SMONTHOUSANDSEP)).empty())
    mon_thousands_sep = s;
  if (!(s = nls_string(LOCALE_SMONGROUPING)).empty())
    mon_grouping = s;
  if (!(s = nls_string(LOCALE_SPOSITIVESIGN)).empty())
    positive_sign = s;
  if (!(s = nls_string(LOCALE_SNEGATIVESIGN)).empty())
    negative_sign = s;

  lconv lc           = lconv();
  lc.decimal_point     = const_cast<char*>(decimal_point.c_str());
  lc.thousands_sep     = const_cast<char*>(thousands_sep.c_str());
  lc.grouping          = const_cast<char*>(grouping.c_str());
  lc.int_curr_symbol   = const_cast<char*>(int_curr_symbol.c_str());
  lc.currency_symbol   = const_cast<char*>(currency_symbol.c_str());
  lc.mon_decimal_point = const_cast<char*>(mon_decimal_point.c_str());
  lc.mon_thousands_sep = const_cast<char*>(mon_thousands_sep.c_str());
  lc.mon_grouping      = const_cast<char*>(mon_grouping.c_str());
  lc.positive_sign     = const_cast<char*>(positive_sign.c_str());
  lc.negative_sign     = const_cast<char*>(negative_sign.c_str());

  lc.int_frac_digits = nls_int(LOCALE_IINTLCURRDIGITS, -1);
  lc.frac_digits     = nls_int(LOCALE_ICURRDIGITS, -1);
  lc.p_cs_precedes   = nls_int(LOCALE_IPOSSYMPRECEDES, -1);
  lc.p_sep_by_space  = nls_int(LOCALE_IPOSSEPBYSPACE, -1);
  lc.n_cs_precedes   = nls_int(LOCALE_INEGSYMPRECEDES, -1);
  lc.n_sep_by_space  = nls_int(LOCALE_INEGSEPBYSPACE, -1);
  lc.p_sign_posn     = nls_int(LOCALE_IPOSSIGNPOSN, -1);
  lc.n_sign_posn     = nls_int(LOCALE_INEGSIGNPOSN, -1);

  return loc.__store_lconv(&lc);
}

//
// Strtonum functions
//

float __strtof(const char* nptr, char** endptr, __locale_t loc) {
  (void)loc; // "C" locale is the only CRT locale on WinCE
  return static_cast<float>(::strtod(nptr, endptr));
}

//
// Other functions
//

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

// The plain restartable conversions are provided by the CRT supplement
// (mingwex mbrtowc.c / wcrtomb.c) with full WinCE ACP and DBCS support;
// delegate to them.
size_t __mbsrtowcs(
    wchar_t* __restrict dst, const char** __restrict src, size_t len, mbstate_t* __restrict ps, __locale_t loc) {
  (void)loc;
  return ::mbsrtowcs(dst, src, len, ps);
}

// Bounded variants: like the restartable functions but consuming at
// most nms/nwc input units.  mbrtowc is handed the whole remaining
// span so DBCS lead/trail pairs resolve within the bound.
size_t __mbsnrtowcs(
    wchar_t* __restrict dst, const char** __restrict src, size_t nms, size_t len, mbstate_t* __restrict ps, __locale_t loc) {
  (void)loc;
  if (src == nullptr || *src == nullptr)
    return 0;
  mbstate_t state = ps ? *ps : mbstate_t();
  const char* p   = *src;
  size_t produced = 0;
  bool saw_nul     = false;
  while (static_cast<size_t>(p - *src) < nms && !saw_nul) {
    wchar_t wc;
    size_t remaining = nms - static_cast<size_t>(p - *src);
    size_t r         = ::mbrtowc(&wc, p, remaining, &state);
    if (r == (size_t)-1 || r == (size_t)-2)
      return (size_t)-1;
    if (r == 0) { // L'\0' produced
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
  mbstate_t state  = ps ? *ps : mbstate_t();
  const wchar_t* p = *src;
  size_t produced  = 0;
  bool saw_nul      = false;
  // wcrtomb emits at most MB_CUR_MAX bytes; MB_CUR_MAX is a runtime
  // value on WinCE, so give the buffer the largest CE code page size.
  char buf[8];
  while (static_cast<size_t>(p - *src) < nwc && !saw_nul) {
    size_t r = ::wcrtomb(buf, *p, &state);
    if (r == (size_t)-1)
      return (size_t)-1;
    if (*p == L'\0')
      saw_nul = true;
    if (dst != nullptr) {
      size_t needed = saw_nul ? r - 1 : r; // the NUL byte is not counted
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
  (void)loc; // "C" locale formatting
  va_list ap;
  va_start(ap, format);
  _LIBCPP_DIAGNOSTIC_PUSH
  _LIBCPP_CLANG_DIAGNOSTIC_IGNORED("-Wformat-nonliteral")
  int result = std::vsnprintf(ret, n, format, ap);
  _LIBCPP_DIAGNOSTIC_POP
  va_end(ap);
  return result;
}

// Like sprintf, but when return value >= 0 it returns
// a pointer to a malloc'd string in *sptr.
// If return >= 0, use free to delete *sptr.
static int __libcpp_vasprintf(char** sptr, const char* __restrict format, va_list ap) {
  *sptr = nullptr;
  // Query the count required.
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
  // If we haven't used exactly what was required, something is wrong.
  // Maybe bug in vsnprintf. Report the error and return.
  _LIBCPP_DIAGNOSTIC_PUSH
  _LIBCPP_CLANG_DIAGNOSTIC_IGNORED("-Wformat-nonliteral")
  if (vsnprintf(p, buffer_size, format, ap) != count) {
    _LIBCPP_DIAGNOSTIC_POP
    free(p);
    return -1;
  }
  // All good. This is returning memory to the caller not freeing it.
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
