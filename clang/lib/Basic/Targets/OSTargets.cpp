//===--- OSTargets.cpp - Implement OS target feature support --------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file implements OS specific TargetInfo types.
//===----------------------------------------------------------------------===//

#include "OSTargets.h"
#include "AArch64.h"
#include "clang/Basic/MacroBuilder.h"
#include "llvm/ADT/StringRef.h"

using namespace clang;
using namespace clang::targets;

namespace clang {
namespace targets {

void getAppleMachODefines(MacroBuilder &Builder, const LangOptions &Opts,
                          const llvm::Triple &Triple) {
  Builder.defineMacro("__APPLE_CC__", "6000");
  Builder.defineMacro("__APPLE__");

  // AddressSanitizer doesn't play well with source fortification, which is on
  // by default on Apple platforms.
  if (Opts.Sanitize.has(SanitizerKind::Address))
    Builder.defineMacro("_FORTIFY_SOURCE", "0");

  // Apple defines __weak, __strong, and __unsafe_unretained even in C mode.
  if (!Opts.ObjC) {
    // __weak is always defined, for use in blocks and with objc pointers.
    Builder.defineMacro("__weak", "__attribute__((objc_gc(weak)))");
    Builder.defineMacro("__strong", "");
    Builder.defineMacro("__unsafe_unretained", "");
  }

  if (Opts.Static)
    Builder.defineMacro("__STATIC__");
  else
    Builder.defineMacro("__DYNAMIC__");

  if (Opts.POSIXThreads)
    Builder.defineMacro("_REENTRANT");

  // __MACH__ originally meant "will run in a Mach kernel based OS", but it has
  // come to also mean "uses Apple Mach-O linking/symbol visibility semantics".
  // Notably libc++'s __configuration/platform.h and Swift's shims/Visibility.h
  // take __MACH__ for the more general meaning.
  if (Triple.isAppleMachO() || Triple.isOSDarwin())
    Builder.defineMacro("__MACH__");
}

void getDarwinDefines(MacroBuilder &Builder, const LangOptions &Opts,
                      const llvm::Triple &Triple, StringRef &PlatformName,
                      VersionTuple &PlatformMinVersion) {
  getAppleMachODefines(Builder, Opts, Triple);

  // Darwin's libc doesn't have threads.h
  Builder.defineMacro("__STDC_NO_THREADS__");

  // Get the platform type and version number from the triple.
  VersionTuple OsVersion;
  if (Triple.isMacOSX()) {
    Triple.getMacOSXVersion(OsVersion);
    PlatformName = "macos";
  } else {
    OsVersion = Triple.getOSVersion();
    PlatformName = llvm::Triple::getOSTypeName(Triple.getOS());
    if (PlatformName == "ios" && Triple.isMacCatalystEnvironment())
      PlatformName = "maccatalyst";
  }

  // If -target arch-pc-win32-macho option specified, we're
  // generating code for Win32 ABI. No need to emit
  // __ENVIRONMENT_XX_OS_VERSION_MIN_REQUIRED__.
  if (PlatformName == "win32") {
    PlatformMinVersion = OsVersion;
    return;
  }

  assert(OsVersion < VersionTuple(100) && "Invalid version!");
  char Str[7];
  if (Triple.isMacOSX() && OsVersion < VersionTuple(10, 10)) {
    Str[0] = '0' + (OsVersion.getMajor() / 10);
    Str[1] = '0' + (OsVersion.getMajor() % 10);
    Str[2] = '0' + std::min(OsVersion.getMinor().value_or(0), 9U);
    Str[3] = '0' + std::min(OsVersion.getSubminor().value_or(0), 9U);
    Str[4] = '\0';
  } else if (!Triple.isMacOSX() && OsVersion.getMajor() < 10) {
    Str[0] = '0' + OsVersion.getMajor();
    Str[1] = '0' + (OsVersion.getMinor().value_or(0) / 10);
    Str[2] = '0' + (OsVersion.getMinor().value_or(0) % 10);
    Str[3] = '0' + (OsVersion.getSubminor().value_or(0) / 10);
    Str[4] = '0' + (OsVersion.getSubminor().value_or(0) % 10);
    Str[5] = '\0';
  } else {
    // Handle versions >= 10.
    Str[0] = '0' + (OsVersion.getMajor() / 10);
    Str[1] = '0' + (OsVersion.getMajor() % 10);
    Str[2] = '0' + (OsVersion.getMinor().value_or(0) / 10);
    Str[3] = '0' + (OsVersion.getMinor().value_or(0) % 10);
    Str[4] = '0' + (OsVersion.getSubminor().value_or(0) / 10);
    Str[5] = '0' + (OsVersion.getSubminor().value_or(0) % 10);
    Str[6] = '\0';
  }

  // Set the appropriate OS version define.
  if (Triple.isTvOS()) {
    Builder.defineMacro("__ENVIRONMENT_TV_OS_VERSION_MIN_REQUIRED__", Str);
  } else if (Triple.isiOS()) {
    Builder.defineMacro("__ENVIRONMENT_IPHONE_OS_VERSION_MIN_REQUIRED__", Str);
  } else if (Triple.isWatchOS()) {
    Builder.defineMacro("__ENVIRONMENT_WATCH_OS_VERSION_MIN_REQUIRED__", Str);
  } else if (Triple.isDriverKit()) {
    assert(OsVersion.getMinor().value_or(0) < 100 &&
           OsVersion.getSubminor().value_or(0) < 100 && "Invalid version!");
    Builder.defineMacro("__ENVIRONMENT_DRIVERKIT_VERSION_MIN_REQUIRED__", Str);
  } else if (Triple.isMacOSX()) {
    Builder.defineMacro("__ENVIRONMENT_MAC_OS_X_VERSION_MIN_REQUIRED__", Str);
  }

  if (Triple.isOSDarwin()) {
    // Any darwin OS defines a general darwin OS version macro in addition
    // to the other OS specific macros.
    assert(OsVersion.getMinor().value_or(0) < 100 &&
           OsVersion.getSubminor().value_or(0) < 100 && "Invalid version!");
    Builder.defineMacro("__ENVIRONMENT_OS_VERSION_MIN_REQUIRED__", Str);
  }

  PlatformMinVersion = OsVersion;
}

static void addMinGWDefines(const llvm::Triple &Triple, const LangOptions &Opts,
                            MacroBuilder &Builder) {
  DefineStd(Builder, "WIN32", Opts);
  DefineStd(Builder, "WINNT", Opts);
  if (Triple.isArch64Bit()) {
    DefineStd(Builder, "WIN64", Opts);
    Builder.defineMacro("__MINGW64__");
  }
  Builder.defineMacro("__MSVCRT__");
  Builder.defineMacro("__MINGW32__");
  addCygMingDefines(Opts, Builder);
}

static void addVisualCDefines(const LangOptions &Opts, MacroBuilder &Builder) {
  if (Opts.CPlusPlus) {
    if (Opts.RTTIData)
      Builder.defineMacro("_CPPRTTI");

    if (Opts.CXXExceptions)
      Builder.defineMacro("_CPPUNWIND");
  }

  if (Opts.Bool)
    Builder.defineMacro("__BOOL_DEFINED");

  if (!Opts.CharIsSigned)
    Builder.defineMacro("_CHAR_UNSIGNED");

  // "The /fp:contract option allows the compiler to generate floating-point
  // contractions [...]"
  if (Opts.getDefaultFPContractMode() != LangOptions::FPModeKind::FPM_Off)
    Builder.defineMacro("_M_FP_CONTRACT");

  // "The /fp:except option generates code to ensures that any unmasked
  // floating-point exceptions are raised at the exact point at which they
  // occur, and that no other floating-point exceptions are raised."
  if (Opts.getDefaultExceptionMode() ==
      LangOptions::FPExceptionModeKind::FPE_Strict)
    Builder.defineMacro("_M_FP_EXCEPT");

  // "The /fp:fast option allows the compiler to reorder, combine, or simplify
  // floating-point operations to optimize floating-point code for speed and
  // space. The compiler may omit rounding at assignment statements,
  // typecasts, or function calls. It may reorder operations or make algebraic
  // transforms, for example, by use of associative and distributive laws. It
  // may reorder code even if such transformations result in observably
  // different rounding behavior."
  //
  // "Under /fp:precise and /fp:strict, the compiler doesn't do any mathematical
  // transformation unless the transformation is guaranteed to produce a bitwise
  // identical result."
  const bool any_imprecise_flags = Opts.FastMath || Opts.UnsafeFPMath ||
                                   Opts.AllowFPReassoc || Opts.NoHonorNaNs ||
                                   Opts.NoHonorInfs || Opts.NoSignedZero ||
                                   Opts.AllowRecip || Opts.ApproxFunc;

  // "Under both /fp:precise and /fp:fast, the compiler generates code intended
  // to run in the default floating-point environment."
  //
  // "[The] default floating point environment [...] sets the rounding mode
  // to round to nearest."
  if (Opts.getDefaultRoundingMode() ==
      LangOptions::RoundingMode::NearestTiesToEven) {
    if (any_imprecise_flags) {
      Builder.defineMacro("_M_FP_FAST");
    } else {
      Builder.defineMacro("_M_FP_PRECISE");
    }
  } else if (!any_imprecise_flags && Opts.getDefaultRoundingMode() ==
                                         LangOptions::RoundingMode::Dynamic) {
    // "Under /fp:strict, the compiler generates code that allows the
    // program to safely unmask floating-point exceptions, read or write
    // floating-point status registers, or change rounding modes."
    Builder.defineMacro("_M_FP_STRICT");
  }

  // FIXME: POSIXThreads isn't exactly the option this should be defined for,
  //        but it works for now.
  if (Opts.POSIXThreads)
    Builder.defineMacro("_MT");

  if (Opts.MSCompatibilityVersion) {
    Builder.defineMacro("_MSC_VER",
                        Twine(Opts.MSCompatibilityVersion / 100000));
    Builder.defineMacro("_MSC_FULL_VER", Twine(Opts.MSCompatibilityVersion));
    // FIXME We cannot encode the revision information into 32-bits
    Builder.defineMacro("_MSC_BUILD", Twine(1));
    // Exposed by MSVC, used in their stddef.h.
    Builder.defineMacro("_CRT_USE_BUILTIN_OFFSETOF", Twine(1));

    if (Opts.CPlusPlus11 && Opts.isCompatibleWithMSVC(LangOptions::MSVC2015))
      Builder.defineMacro("_HAS_CHAR16_T_LANGUAGE_SUPPORT", Twine(1));

    if (Opts.isCompatibleWithMSVC(LangOptions::MSVC2015)) {
      if (Opts.CPlusPlus26)
        // TODO update to the proper value.
        Builder.defineMacro("_MSVC_LANG", "202400L");
      else if (Opts.CPlusPlus23)
        Builder.defineMacro("_MSVC_LANG", "202302L");
      else if (Opts.CPlusPlus20)
        Builder.defineMacro("_MSVC_LANG", "202002L");
      else if (Opts.CPlusPlus17)
        Builder.defineMacro("_MSVC_LANG", "201703L");
      else if (Opts.CPlusPlus14)
        Builder.defineMacro("_MSVC_LANG", "201402L");
    }

    if (Opts.isCompatibleWithMSVC(LangOptions::MSVC2022_3))
      Builder.defineMacro("_MSVC_CONSTEXPR_ATTRIBUTE");
  }

  if (Opts.MicrosoftExt) {
    Builder.defineMacro("_MSC_EXTENSIONS");

    if (Opts.CPlusPlus11) {
      Builder.defineMacro("_RVALUE_REFERENCES_V2_SUPPORTED");
      Builder.defineMacro("_RVALUE_REFERENCES_SUPPORTED");
      Builder.defineMacro("_NATIVE_NULLPTR_SUPPORTED");
    }
  }

  if (!Opts.MSVolatile)
    Builder.defineMacro("_ISO_VOLATILE");

  if (Opts.Kernel)
    Builder.defineMacro("_KERNEL_MODE");

  Builder.defineMacro("_INTEGRAL_MAX_BITS", "64");
  // Define __STDC_NO_THREADS__ based on MSVC version, threads.h availability,
  // and language standard.
  if (!(Opts.isCompatibleWithMSVC(LangOptions::MSVC2022_9) && Opts.C11))
    Builder.defineMacro("__STDC_NO_THREADS__");
  // Starting with VS 2022 17.1, MSVC predefines the below macro to inform
  // users of the execution character set defined at compile time.
  // The value given is the Windows Code Page Identifier:
  // https://docs.microsoft.com/en-us/windows/win32/intl/code-page-identifiers
  //
  // Clang currently only supports UTF-8, so we'll use 65001
  Builder.defineMacro("_MSVC_EXECUTION_CHARACTER_SET", "65001");
}

void addWindowsDefines(const llvm::Triple &Triple, const LangOptions &Opts,
                       MacroBuilder &Builder) {
  Builder.defineMacro("_WIN32");
  if (Triple.isArch64Bit())
    Builder.defineMacro("_WIN64");
  if (Triple.isWindowsGNUEnvironment())
    addMinGWDefines(Triple, Opts, Builder);
  else if (Triple.isKnownWindowsMSVCEnvironment() ||
           (Triple.isWindowsItaniumEnvironment() && Opts.MSVCCompat))
    addVisualCDefines(Opts, Builder);
}

void addWinCEDefines(const llvm::Triple &Triple, MacroBuilder &Builder) {
  // The WinCE version may be encoded in the triple (e.g. arm-pc-wince6.00);
  // default to Windows Embedded CE 6.0, the deployment this toolchain
  // primarily targets.  CeGCC defaulted to 5.0; overridable per invocation
  // with -D_WIN32_WCE=0x500 (or a versioned triple, wince5.0).
  unsigned _WIN32_WCE = 0x0600;
  llvm::VersionTuple OSVer = Triple.getOSVersion();
  if (OSVer.getMajor()) {
    // Microsoft spell this macro in "VRR" form: V is the major version and RR
    // is the *two-digit decimal* revision, with each decimal digit occupying
    // one hexadecimal nibble.  The CE "core version" of Windows CE 4.2 is
    // 4.20, so _WIN32_WCE is 0x0420 -- NOT 0x0402.  Likewise CE 4.1 is core
    // version 4.10 -> 0x0410, CE 2.10 -> 0x0210, CE 2.11 -> 0x0211,
    // CE 2.12 -> 0x0212, CE 3.0 -> 0x0300, CE 5.0 -> 0x0500, CE 6.0 -> 0x0600.
    //
    // w32api encodes exactly this convention: aygshell.h guards CE 4.2
    // features on `_WIN32_WCE >= 0x0420`, shellapi.h on `>= 0x420`, and
    // windef.h documents "UNDER_CE=420, _WIN32_WCE=0x420".  Adding the raw
    // minor value here instead produced 0x0402 for a versioned triple such as
    // arm-pc-wince4.2, and 0x0402 < 0x0420, so every CE 4.1/4.2-gated API
    // was silently disabled.
    //
    // A single-digit minor (wince4.2) is read as core version 4.20, i.e. the
    // minor is the tens digit; spell the two-digit form (wince4.20, wince2.11)
    // to be explicit.  A revision with a leading zero (CE 1.01 -> 0x0101)
    // cannot be expressed through the triple because VersionTuple drops the
    // leading zero -- pass -D_WIN32_WCE=0x0101 for those historical builds.
    unsigned RR = OSVer.getMinor() ? OSVer.getMinor().value() : 0;
    if (RR < 10)
      RR *= 10; // 4.2 means core version 4.20
    _WIN32_WCE = OSVer.getMajor() * 0x100 + (RR / 10) * 0x10 + (RR % 10);
  }
  Builder.defineMacro("_WIN32_WCE", Twine(_WIN32_WCE));
  Builder.defineMacro("UNDER_CE", Twine(_WIN32_WCE));
  Builder.defineMacro("WINCE");
  Builder.defineMacro("__WINCE__");
  // CeGCC/CeGCC-header compatibility: the mingwrt and w32api sources
  // guard their WinCE support behind this macro.
  Builder.defineMacro("__MINGW32CE__");
  // The macros every CeGCC flavor shares (kept here so the ARM and x86
  // target infos cannot drift apart).  w32api's include/cegcc.h.in
  // hard-errors ("__CEGCC_VERSION__ isn't defined by the compiler") unless
  // this is predefined; it then #undefs and recomputes it from
  // __CEGCC_VERSION_{MAJOR,MINOR,PATCHLEVEL}__, so the numeric value
  // predefined here is never actually observed by user code.  Kept in sync
  // with the historical CeGCC value.  Real Windows CE builds are
  // Unicode-only (there is no ANSI Win32 subset on the platform), and
  // mingwrt/w32api's headers assume _UNICODE/UNICODE are always set.
  Builder.defineMacro("__CEGCC_VERSION__", "0x090909");
  Builder.defineMacro("__COREDLL__");
  Builder.defineMacro("__MINGW32__");
  Builder.defineMacro("WIN32");
  Builder.defineMacro("WINNT");
  Builder.defineMacro("_UNICODE");
  Builder.defineMacro("UNICODE");
}

} // namespace targets
} // namespace clang
