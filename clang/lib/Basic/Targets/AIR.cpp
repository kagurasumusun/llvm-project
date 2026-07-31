//===--- AIR.cpp - Implement AIR target feature support -------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file implements AIR TargetInfo objects.
//
//===----------------------------------------------------------------------===//

#include "AIR.h"
#include "clang/Basic/AIRVersion.h"
#include "Targets.h"
#include "clang/Basic/MacroBuilder.h"
#include "llvm/ADT/StringRef.h"

using namespace clang;
using namespace clang::targets;

/// Map a deployment target OS version to the AIR version encoded in the target
/// triple as the `_vNN` arch suffix.
///
/// This mapping was established by sweeping the driver `-###` output over
/// every supported deployment target in
/// reference/metal-ast-macos-air64/log:
///
///   macOS 10.11 => air64_v18     macOS 13.x => air64_v25
///   macOS 10.12 => air64_v111    macOS 14.x => air64_v26
///   macOS 10.13 => air64_v20     macOS 15.x => air64_v27
///   macOS 10.14 => air64_v21     macOS 26.x => air64_v28
///   macOS 10.15 => air64_v22
///   macOS 11.x  => air64_v23
///   macOS 12.x  => air64_v24
///
/// Note that the AIR version depends *only* on the deployment target and is
/// completely independent of `-std=`. The two are separate axes: `-std=`
/// selects `!air.language_version` while the deployment target selects the
/// triple suffix and `!air.version`.
///
/// The pre-10.13 values are not table entries but the result of Apple spelling
/// the version as the concatenation of major and minor ("10.11" => "1" + "8"?
/// in fact 10.11 => 18 and 10.12 => 111), a legacy form that predates the
/// modern numbering. They are reproduced verbatim from the measurement.
unsigned clang::targets::getAIRVersionForMacOSVersion(unsigned Major,
                                                      unsigned Minor) {
  if (Major == 10) {
    switch (Minor) {
    case 11:
      return 18;
    case 12:
      return 111;
    case 13:
      return 20;
    case 14:
      return 21;
    case 15:
      return 22;
    default:
      // Older than the oldest measured deployment target; use the oldest
      // observed AIR version rather than inventing one.
      return Minor < 11 ? 18 : 22;
    }
  }
  switch (Major) {
  case 11:
    return 23;
  case 12:
    return 24;
  case 13:
    return 25;
  case 14:
    return 26;
  case 15:
    return 27;
  default:
    // macOS 26 and later. No newer AIR version has been observed.
    return Major >= 26 ? 28 : 27;
  }
}

void AIRTargetInfo::getTargetDefines(const LangOptions &Opts,
                                     MacroBuilder &Builder) const {
  if (!Opts.Metal)
    return;

  const unsigned MetalVersion = static_cast<unsigned>(Opts.getMetalVersion());

  // Apple's compiler splits its "which Metal is this" signalling in two,
  // measured in reference/metal-ast-macos-air64/driver/*.macros:
  //
  //   -std=macos-metalX.Y (X <= 2)  =>  __METAL_MACOS__ 1  (no __METAL__)
  //   -std=ios-metalX.Y   (X <= 2)  =>  __METAL_IOS__ 1    (no __METAL__)
  //   -std=metalX.Y       (X >= 3)  =>  __METAL__ 1        (no __METAL_*OS__)
  //
  // <metal_config> keys its capability macros off exactly this distinction
  // (`#ifdef __METAL_IOS__` / `#ifdef __METAL_MACOS__` for the versioned
  // standards, plain `#if __METAL_VERSION__ == NNN` for 3.0 and later), and
  // <metal_stdlib> tests `defined(__METAL_IOS__) && !defined(__METAL__)`.
  // Getting this wrong silently disables every __HAVE_* capability macro.
  if (MetalVersion >= 300) {
    Builder.defineMacro("__METAL__", "1");
  } else {
    switch (Opts.LangStd) {
    case LangStandard::lang_ios_metal1_0:
    case LangStandard::lang_ios_metal1_1:
    case LangStandard::lang_ios_metal1_2:
    case LangStandard::lang_ios_metal2_0:
    case LangStandard::lang_ios_metal2_1:
    case LangStandard::lang_ios_metal2_2:
    case LangStandard::lang_ios_metal2_3:
    case LangStandard::lang_ios_metal2_4:
      Builder.defineMacro("__METAL_IOS__", "1");
      break;
    default:
      Builder.defineMacro("__METAL_MACOS__", "1");
      break;
    }
  }

  // __METAL_VERSION__ is the MSL version times 100, e.g. 400 for MSL 4.0. The
  // LangOptions enumerator already carries exactly this value.
  Builder.defineMacro("__METAL_VERSION__", Twine(MetalVersion));

  // Pointer size selector consumed by the standard library.
  if (getTriple().getArch() == llvm::Triple::air32)
    Builder.defineMacro("__AIR32__", "1");
  else
    Builder.defineMacro("__AIR64__", "1");

  // __AIR_VERSION__ encodes the AIR version as major * 10000 + minor * 100.
  // Measured pairs: air64_v20 => 20000 ... air64_v28 => 20800. The AIR version
  // comes from the triple, not from -std=.
  unsigned AIRVersion = getTriple().getAIRVersion();
  if (AIRVersion >= 20 && AIRVersion <= 99)
    Builder.defineMacro("__AIR_VERSION__", Twine(20000 + (AIRVersion - 20) * 100));

  // Math mode selectors. Apple's driver defaults to
  // -fmetal-math-fp32-functions=fast, which is reflected here.
  Builder.defineMacro("__METAL_MATH_FP32_FUNCTIONS_FAST__",
                      Opts.getMetalFPMath() ==
                              LangOptions::MetalFPMathFunctions::Precise
                          ? "0"
                          : "1");
  Builder.defineMacro("__METAL_FAST_MATH__", Opts.FastMath ? "1" : "0");

  // The remaining Metal macros are version independent enumeration values.
  // They are transcribed verbatim from a measurement of the reference
  // toolchain; see clang/Basic/MetalMacros.def.
#define METAL_MACRO(Name, Value) Builder.defineMacro(Name, Value);
#include "clang/Basic/MetalMacros.def"

  // __HAVE_* capability macros are deliberately NOT predefined here.
  // Apple's <metal_config> derives all ~200 __HAVE_* macros from
  // __METAL_VERSION__ (and the __METAL_IOS__ / __METAL_MACOS__ /
  // __METAL__ selector).  Defining them in the compiler would conflict
  // with the standard library's own definitions and cause redefinition
  // warnings.  Apple's compiler likewise predefines none of them.
}
