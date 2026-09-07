#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <winnls.h>

#include <__locale_dir/support/wince.h>
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


__locale_t __newlocale(int         , const char* locale, __locale_t         ) {
  if (locale == nullptr)
    return __locale_t();
  if (locale[0] == '\0' || std::strcmp(locale, "C") == 0 || std::strcmp(locale, "POSIX") == 0)
    return __locale_t("C");
  return __locale_t();
}

namespace {

std::string nls_string(LCTYPE type) {
  wchar_t wbuf[64];
  int n = ::GetLocaleInfoW(LOCALE_USER_DEFAULT, type, wbuf, sizeof(wbuf) / sizeof(wbuf[0]));
  if (n <= 0)
    return std::string();
  char buf[64];
  int m = ::WideCharToMultiByte(CP_ACP, 0, wbuf, n - 1, buf, sizeof(buf) - 1, nullptr, nullptr);
  if (m < 0)
    m = 0;
  buf[m] = '\0';
  return std::string(buf);
}

int nls_int(LCTYPE type, int fallback) {
  std::string s = nls_string(type);
  if (s.empty())
    return fallback;
  return std::atoi(s.c_str());
}

}

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


float __strtof(const char* nptr, char** endptr, __locale_t loc) {
  (void)loc;
  return static_cast<float>(::strtod(nptr, endptr));
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
  const char* p   = *src;
  size_t produced = 0;
  bool saw_nul     = false;
  while (static_cast<size_t>(p - *src) < nms && !saw_nul) {
    wchar_t wc;
    size_t remaining = nms - static_cast<size_t>(p - *src);
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
  mbstate_t state  = ps ? *ps : mbstate_t();
  const wchar_t* p = *src;
  size_t produced  = 0;
  bool saw_nul      = false;
  char buf[8];
  while (static_cast<size_t>(p - *src) < nwc && !saw_nul) {
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

}
_LIBCPP_END_NAMESPACE_STD
