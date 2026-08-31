//===-----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===-----------------------------------------------------------------------===//

#ifndef _LIBCPP___LOCALE_DIR_SUPPORT_WINCE_H
#define _LIBCPP___LOCALE_DIR_SUPPORT_WINCE_H

// Windows CE locale backend for libc++.
//
// The Windows CE C library (coredll) has no setlocale() and none of the
// MSVCRT _l-variants: CRT-level locale semantics are always the "C"
// locale.  What the OS does provide is the NLS query surface
// (GetLocaleInfoW and friends, exported by coredll) and the wide
// classification functions (iswctype/towupper/towlower).
//
// This backend maps the libc++ locale base API onto exactly that:
//
//   * locale names: only "C", "POSIX" and "" are accepted (anything
//     else fails, exactly like _create_locale fails for unknown names
//     on desktop Windows); CRT conversions are C-locale.
//   * the lconv data returned by localeconv() is populated from the
//     device NLS data via GetLocaleInfoW(LOCALE_USER_DEFAULT), so
//     std::locale::classic() numeric/monetary fields reflect the
//     device locale like a WinCE application expects.
//   * wide character classification delegates to coredll's iswctype /
//     towupper / towlower.
//
// The plain-C multibyte functions (mbrtowc, wcrtomb, mbsinit, ...)
// used by the library-side implementation are provided by the toolchain
// CRT supplement (mingwex), which is linked into every WinCE link by
// the clang driver.

#include <__config>
#include <__cstddef/nullptr_t.h>
#include <clocale>  // std::lconv & friends, LC_ constants
#include <cstddef>
#include <cstdio>   // snprintf family declarations
#include <cstdlib>  // strtod, MB_CUR_MAX
#include <cstring>  // strcmp, strncpy
#include <ctype.h>  // _ALPHA.._HEX classification masks (iswctype backend)
#include <string>
#include <cwchar>   // mbstate_t, wchar_t, btowc/wctob
#include <cwctype>  // wint_t, iswctype, towupper, towlower
#include <time.h>   // ::strftime, struct tm (not pulled in by <ctime> wrappers)

#if !defined(_LIBCPP_HAS_NO_PRAGMA_SYSTEM_HEADER)
#  pragma GCC system_header
#endif

_LIBCPP_BEGIN_NAMESPACE_STD
namespace __locale {

using __lconv_t _LIBCPP_NODEBUG = std::lconv;

// Storage that keeps the strings inside an lconv alive.  Mirrors the
// windows backend: the lconv returned to libc++ points into this object
// which is owned by the __locale_t it was created from.
class __lconv_storage {
public:
  __lconv_storage(const __lconv_t* __lc_input) {
    __lc_ = *__lc_input;

    __decimal_point_     = __lc_input->decimal_point;
    __thousands_sep_     = __lc_input->thousands_sep;
    __grouping_          = __lc_input->grouping;
    __int_curr_symbol_   = __lc_input->int_curr_symbol;
    __currency_symbol_   = __lc_input->currency_symbol;
    __mon_decimal_point_ = __lc_input->mon_decimal_point;
    __mon_thousands_sep_ = __lc_input->mon_thousands_sep;
    __mon_grouping_      = __lc_input->mon_grouping;
    __positive_sign_     = __lc_input->positive_sign;
    __negative_sign_     = __lc_input->negative_sign;

    __lc_.decimal_point     = const_cast<char*>(__decimal_point_.c_str());
    __lc_.thousands_sep     = const_cast<char*>(__thousands_sep_.c_str());
    __lc_.grouping          = const_cast<char*>(__grouping_.c_str());
    __lc_.int_curr_symbol   = const_cast<char*>(__int_curr_symbol_.c_str());
    __lc_.currency_symbol   = const_cast<char*>(__currency_symbol_.c_str());
    __lc_.mon_decimal_point = const_cast<char*>(__mon_decimal_point_.c_str());
    __lc_.mon_thousands_sep = const_cast<char*>(__mon_thousands_sep_.c_str());
    __lc_.mon_grouping      = const_cast<char*>(__mon_grouping_.c_str());
    __lc_.positive_sign     = const_cast<char*>(__positive_sign_.c_str());
    __lc_.negative_sign     = const_cast<char*>(__negative_sign_.c_str());
  }

  __lconv_t* __get() { return &__lc_; }

private:
  __lconv_t __lc_;
  std::string __decimal_point_;
  std::string __thousands_sep_;
  std::string __grouping_;
  std::string __int_curr_symbol_;
  std::string __currency_symbol_;
  std::string __mon_decimal_point_;
  std::string __mon_thousands_sep_;
  std::string __mon_grouping_;
  std::string __positive_sign_;
  std::string __negative_sign_;
};

//
// Locale management
//
#  define _CATMASK(n) ((1 << (n)) >> 1)
#  define _LIBCPP_COLLATE_MASK _CATMASK(LC_COLLATE)
#  define _LIBCPP_CTYPE_MASK _CATMASK(LC_CTYPE)
#  define _LIBCPP_MONETARY_MASK _CATMASK(LC_MONETARY)
#  define _LIBCPP_NUMERIC_MASK _CATMASK(LC_NUMERIC)
#  define _LIBCPP_TIME_MASK _CATMASK(LC_TIME)
#  define _LIBCPP_MESSAGES_MASK _CATMASK(6)
#  define _LIBCPP_ALL_MASK                                                                                             \
    (_LIBCPP_COLLATE_MASK | _LIBCPP_CTYPE_MASK | _LIBCPP_MESSAGES_MASK | _LIBCPP_MONETARY_MASK | _LIBCPP_NUMERIC_MASK | \
     _LIBCPP_TIME_MASK)
#  define _LIBCPP_LC_ALL LC_ALL

// WinCE locale: the CRT always behaves as the "C" locale; the object
// carries the name it was constructed with and the (lazily built) NLS
// lconv.  There is no OS locale handle to own.
class __locale_t {
public:
  __locale_t() : __locale_str_(nullptr), __lc_(nullptr) {}
  __locale_t(std::nullptr_t) : __locale_str_(nullptr), __lc_(nullptr) {}
  // libc++ passes 0 as the base locale to __newlocale; without this,
  // 0 is ambiguous between nullptr_t and const char*.
  __locale_t(int) : __locale_str_(nullptr), __lc_(nullptr) {}
  __locale_t(const char* __loc_str) : __locale_str_(__loc_str), __lc_(nullptr) {}
  __locale_t(const __locale_t& __loc) : __locale_str_(__loc.__locale_str_), __lc_(nullptr) {}

  ~__locale_t() { delete __lc_; }

  __locale_t& operator=(const __locale_t& __loc) {
    __locale_str_ = __loc.__locale_str_;
    // __lc_ not copied
    return *this;
  }

  friend bool operator==(const __locale_t& __left, const __locale_t& __right) {
    return (__left.__locale_str_ == nullptr) == (__right.__locale_str_ == nullptr) &&
           (__left.__locale_str_ == __right.__locale_str_ ||
            (__left.__locale_str_ != nullptr && __right.__locale_str_ != nullptr &&
             std::strcmp(__left.__locale_str_, __right.__locale_str_) == 0));
  }

  friend bool operator==(const __locale_t& __left, int __right) { return __left.__locale_str_ == nullptr && __right == 0; }

  friend bool operator==(const __locale_t& __left, long long __right) {
    return __left.__locale_str_ == nullptr && __right == 0;
  }

  friend bool operator==(const __locale_t& __left, std::nullptr_t) { return __left.__locale_str_ == nullptr; }

  friend bool operator==(int __left, const __locale_t& __right) { return __left == 0 && nullptr == __right.__locale_str_; }

  friend bool operator==(std::nullptr_t, const __locale_t& __right) { return nullptr == __right.__locale_str_; }

  friend bool operator!=(const __locale_t& __left, const __locale_t& __right) { return !(__left == __right); }

  friend bool operator!=(const __locale_t& __left, int __right) { return !(__left == __right); }

  friend bool operator!=(const __locale_t& __left, long long __right) { return !(__left == __right); }

  friend bool operator!=(const __locale_t& __left, std::nullptr_t __right) { return !(__left == __right); }

  friend bool operator!=(int __left, const __locale_t& __right) { return !(__left == __right); }

  friend bool operator!=(std::nullptr_t __left, const __locale_t& __right) { return !(__left == __right); }

  operator bool() const { return __locale_str_ != nullptr; }

  const char* __get_locale() const { return __locale_str_; }

  __lconv_t* __store_lconv(const __lconv_t* __input_lc) {
    delete __lc_;
    __lc_ = new __lconv_storage(__input_lc);
    return __lc_->__get();
  }

private:
  const char* __locale_str_;
  __lconv_storage* __lc_ = nullptr;
};

#  if defined(_LIBCPP_BUILDING_LIBRARY)
_LIBCPP_EXPORTED_FROM_ABI __locale_t __newlocale(int __mask, const char* __locale, __locale_t __base);
inline _LIBCPP_HIDE_FROM_ABI void __freelocale(__locale_t __loc) {
  // No OS locale handle; the __locale_t members clean up after themselves.
  (void)__loc;
}
inline _LIBCPP_HIDE_FROM_ABI char* __setlocale(int __category, const char* __locale) {
  (void)__category;
  // Windows CE has no process locale: queries and requests for the "C"
  // locale succeed; anything else is an error the caller surfaces.
  if (__locale == nullptr || __locale[0] == '\0' || std::strcmp(__locale, "C") == 0 ||
      std::strcmp(__locale, "POSIX") == 0) {
    static char __c_name[] = "C";
    return __c_name;
  }
  std::__throw_runtime_error("locale::global: only the \"C\" locale exists on Windows CE");
}
_LIBCPP_EXPORTED_FROM_ABI __lconv_t* __localeconv(__locale_t& __loc);
#  endif // _LIBCPP_BUILDING_LIBRARY

//
// Strtonum functions
//
// coredll provides strtod but not strtof/strtold; the library side
// derives them (ARM WinCE: long double has the same representation as
// double, and float is a narrowing of the double result).
_LIBCPP_EXPORTED_FROM_ABI float __strtof(const char*, char**, __locale_t);
inline _LIBCPP_HIDE_FROM_ABI double __strtod(const char* __nptr, char** __endptr, __locale_t __loc) {
  (void)__loc; // "C" locale is the only CRT locale on WinCE
  return ::strtod(__nptr, __endptr);
}
inline _LIBCPP_HIDE_FROM_ABI long double __strtold(const char* __nptr, char** __endptr, __locale_t __loc) {
  (void)__loc;
  return ::strtod(__nptr, __endptr);
}

//
// Character manipulation functions
//
#  if defined(_LIBCPP_BUILDING_LIBRARY)
inline _LIBCPP_HIDE_FROM_ABI int __toupper(int __c, __locale_t __loc) {
  (void)__loc;
  return ::toupper(__c);
}

inline _LIBCPP_HIDE_FROM_ABI int __tolower(int __c, __locale_t __loc) {
  (void)__loc;
  return ::tolower(__c);
}

inline _LIBCPP_HIDE_FROM_ABI int __strcoll(const char* __s1, const char* __s2, __locale_t __loc) {
  (void)__loc; // C-locale collation is bytewise comparison
  return ::strcmp(__s1, __s2);
}

inline _LIBCPP_HIDE_FROM_ABI size_t __strxfrm(char* __dest, const char* __src, size_t __n, __locale_t __loc) {
  (void)__loc; // in the C locale strxfrm is a plain copy
  size_t __len = ::strlen(__src);
  if (__n > 0)
    ::strncpy(__dest, __src, __n);
  return __len;
}

#    if _LIBCPP_HAS_WIDE_CHARACTERS
inline _LIBCPP_HIDE_FROM_ABI int __iswctype(wint_t __c, wctype_t __type, __locale_t __loc) {
  (void)__loc;
  return ::iswctype(__c, __type);
}
inline _LIBCPP_HIDE_FROM_ABI int __iswspace(wint_t __c, __locale_t __loc) {
  (void)__loc;
  return ::iswctype(__c, _SPACE);
}
inline _LIBCPP_HIDE_FROM_ABI int __iswprint(wint_t __c, __locale_t __loc) {
  (void)__loc;
  return ::iswctype(__c, (_BLANK | _PUNCT | _ALPHA | _DIGIT));
}
inline _LIBCPP_HIDE_FROM_ABI int __iswcntrl(wint_t __c, __locale_t __loc) {
  (void)__loc;
  return ::iswctype(__c, _CONTROL);
}
inline _LIBCPP_HIDE_FROM_ABI int __iswupper(wint_t __c, __locale_t __loc) {
  (void)__loc;
  return ::iswctype(__c, _UPPER);
}
inline _LIBCPP_HIDE_FROM_ABI int __iswlower(wint_t __c, __locale_t __loc) {
  (void)__loc;
  return ::iswctype(__c, _LOWER);
}
inline _LIBCPP_HIDE_FROM_ABI int __iswalpha(wint_t __c, __locale_t __loc) {
  (void)__loc;
  return ::iswctype(__c, _ALPHA);
}
inline _LIBCPP_HIDE_FROM_ABI int __iswblank(wint_t __c, __locale_t __loc) {
  (void)__loc;
  return ::iswctype(__c, _BLANK);
}
inline _LIBCPP_HIDE_FROM_ABI int __iswdigit(wint_t __c, __locale_t __loc) {
  (void)__loc;
  return ::iswctype(__c, _DIGIT);
}
inline _LIBCPP_HIDE_FROM_ABI int __iswpunct(wint_t __c, __locale_t __loc) {
  (void)__loc;
  return ::iswctype(__c, _PUNCT);
}
inline _LIBCPP_HIDE_FROM_ABI int __iswxdigit(wint_t __c, __locale_t __loc) {
  (void)__loc;
  return ::iswctype(__c, _HEX);
}
inline _LIBCPP_HIDE_FROM_ABI wint_t __towupper(wint_t __c, __locale_t __loc) {
  (void)__loc;
  return ::towupper(__c);
}
inline _LIBCPP_HIDE_FROM_ABI wint_t __towlower(wint_t __c, __locale_t __loc) {
  (void)__loc;
  return ::towlower(__c);
}

inline _LIBCPP_HIDE_FROM_ABI int __wcscoll(const wchar_t* __ws1, const wchar_t* __ws2, __locale_t __loc) {
  (void)__loc; // C-locale collation is code-unit comparison
  return ::wcscmp(__ws1, __ws2);
}

inline _LIBCPP_HIDE_FROM_ABI size_t __wcsxfrm(wchar_t* __dest, const wchar_t* __src, size_t __n, __locale_t __loc) {
  (void)__loc;
  size_t __len = ::wcslen(__src);
  if (__n > 0)
    ::wcsncpy(__dest, __src, __n);
  return __len;
}
#    endif // _LIBCPP_HAS_WIDE_CHARACTERS

inline _LIBCPP_HIDE_FROM_ABI size_t
__strftime(char* __ret, size_t __n, const char* __format, const struct tm* __tm, __locale_t __loc) {
  (void)__loc; // the mingwex strftime has C-locale names only
  return ::strftime(__ret, __n, __format, __tm);
}

//
// Other functions
//
_LIBCPP_EXPORTED_FROM_ABI decltype(MB_CUR_MAX) __mb_len_max(__locale_t);
_LIBCPP_EXPORTED_FROM_ABI wint_t __btowc(int, __locale_t);
_LIBCPP_EXPORTED_FROM_ABI int __wctob(wint_t, __locale_t);
_LIBCPP_EXPORTED_FROM_ABI size_t
__wcsnrtombs(char* __restrict, const wchar_t** __restrict, size_t, size_t, mbstate_t* __restrict, __locale_t);
_LIBCPP_EXPORTED_FROM_ABI size_t __wcrtomb(char* __restrict, wchar_t, mbstate_t* __restrict, __locale_t);
_LIBCPP_EXPORTED_FROM_ABI size_t
__mbsnrtowcs(wchar_t* __restrict, const char** __restrict, size_t, size_t, mbstate_t* __restrict, __locale_t);
_LIBCPP_EXPORTED_FROM_ABI size_t
__mbrtowc(wchar_t* __restrict, const char* __restrict, size_t, mbstate_t* __restrict, __locale_t);

inline _LIBCPP_HIDE_FROM_ABI int __mbtowc(wchar_t* __pwc, const char* __pmb, size_t __max, __locale_t __loc) {
  (void)__loc;
  // COREDLL does not export mbtowc; CRT conversions on CE are the C locale.
  if (__pmb == nullptr)
    return 0;
  if (__max == 0 || *__pmb == '\0') {
    if (__pwc)
      *__pwc = L'\0';
    return 0;
  }
  if (__pwc)
    *__pwc = static_cast<unsigned char>(*__pmb);
  return 1;
}

_LIBCPP_EXPORTED_FROM_ABI size_t __mbrlen(const char* __restrict, size_t, mbstate_t* __restrict, __locale_t);

_LIBCPP_EXPORTED_FROM_ABI size_t
__mbsrtowcs(wchar_t* __restrict, const char** __restrict, size_t, mbstate_t* __restrict, __locale_t);
#  endif   // _LIBCPP_BUILDING_LIBRARY

_LIBCPP_EXPORTED_FROM_ABI _LIBCPP_ATTRIBUTE_FORMAT(__printf__, 4, 5) int __snprintf(
    char* __ret, size_t __n, __locale_t __loc, const char* __format, ...);

_LIBCPP_EXPORTED_FROM_ABI
_LIBCPP_ATTRIBUTE_FORMAT(__printf__, 3, 4) int __asprintf(char** __ret, __locale_t __loc, const char* __format, ...);

#  if defined(_LIBCPP_BUILDING_LIBRARY)
// WinCE has no per-thread locale state (no setlocale at all), so the
// guard that swaps the global CRT locale on desktop Windows reduces to
// nothing here.
struct __locale_guard {
  _LIBCPP_HIDE_FROM_ABI __locale_guard(__locale_t) {}
};
#  endif // _LIBCPP_BUILDING_LIBRARY

} // namespace __locale
_LIBCPP_END_NAMESPACE_STD

#endif // _LIBCPP___LOCALE_DIR_SUPPORT_WINCE_H
