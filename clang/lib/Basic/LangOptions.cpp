//===- LangOptions.cpp - C Language Family Language Options ---------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
//  This file defines the LangOptions class.
//
//===----------------------------------------------------------------------===//

#include "clang/Basic/LangOptions.h"
#include "llvm/ADT/SmallString.h"
#include "llvm/Support/Path.h"

using namespace clang;

LangOptions::LangOptions() : LangStd(LangStandard::lang_unspecified) {
#define LANGOPT(Name, Bits, Default, Description) Name = Default;
#define ENUM_LANGOPT(Name, Type, Bits, Default, Description) set##Name(Default);
#include "clang/Basic/LangOptions.def"
}

void LangOptions::resetNonModularOptions() {
#define LANGOPT(Name, Bits, Default, Description)
#define BENIGN_LANGOPT(Name, Bits, Default, Description) Name = Default;
#define BENIGN_ENUM_LANGOPT(Name, Type, Bits, Default, Description) \
  Name = static_cast<unsigned>(Default);
#include "clang/Basic/LangOptions.def"

  // These options do not affect AST generation.
  NoSanitizeFiles.clear();
  XRayAlwaysInstrumentFiles.clear();
  XRayNeverInstrumentFiles.clear();

  CurrentModule.clear();
  IsHeaderFile = false;
}

bool LangOptions::isNoBuiltinFunc(StringRef FuncName) const {
  for (unsigned i = 0, e = NoBuiltinFuncs.size(); i != e; ++i)
    if (FuncName.equals(NoBuiltinFuncs[i]))
      return true;
  return false;
}

VersionTuple LangOptions::getOpenCLVersionTuple() const {
  const int Ver = OpenCLCPlusPlus ? OpenCLCPlusPlusVersion : OpenCLVersion;
  if (OpenCLCPlusPlus && Ver != 100)
    return VersionTuple(Ver / 100);
  return VersionTuple(Ver / 100, (Ver % 100) / 10);
}

unsigned LangOptions::getOpenCLCompatibleVersion() const {
  if (!OpenCLCPlusPlus)
    return OpenCLVersion;
  if (OpenCLCPlusPlusVersion == 100)
    return 200;
  if (OpenCLCPlusPlusVersion == 202100)
    return 300;
  llvm_unreachable("Unknown OpenCL version");
}

void LangOptions::remapPathPrefix(SmallVectorImpl<char> &Path) const {
  for (const auto &Entry : MacroPrefixMap)
    if (llvm::sys::path::replace_path_prefix(Path, Entry.first, Entry.second))
      break;
}

std::string LangOptions::getOpenCLVersionString() const {
  std::string Result;
  {
    llvm::raw_string_ostream Out(Result);
    Out << (OpenCLCPlusPlus ? "C++ for OpenCL" : "OpenCL C") << " version "
        << getOpenCLVersionTuple().getAsString();
  }
  return Result;
}

/// Map a Metal language standard to the numeric Metal Shading Language version.
///
/// The values are exactly Apple's `__METAL_VERSION__`; see
/// reference/metal-ast-*/meta/metal-predefined-macros.txt for the measured
/// value of the current toolchain (`__METAL_VERSION__ 400` for -std=metal4.0).
static LangOptions::MetalLangStd
getMetalVersionForLangStandard(LangStandard::Kind LangStd) {
  switch (LangStd) {
  case LangStandard::lang_macos_metal1_0:
  case LangStandard::lang_osx_metal1_0:
    // `macos-metal1.0` (and its `osx-` alias) is a hidden compatibility
    // spelling: it is not listed in any published specification, but the
    // reference toolchain accepts it and lowers it to language version
    // 1.1.0 (measured: research/spec/LEGACY_METAL_SUPPORT.md,
    // "Metal 1.0 ... 1.1に統合"). There never was a separate macOS 1.0.
    return LangOptions::Metal_1_1;
  case LangStandard::lang_ios_metal1_0:
    // iOS genuinely shipped MSL 1.0: the measured `-dM` output of
    // `-std=ios-metal1.0` carries `__METAL_VERSION__ 100`
    // (reference/metal-ast-ios-air64/driver/*ios-metal1.0*macros).
    return LangOptions::Metal_1_0;
  case LangStandard::lang_macos_metal1_1:
  case LangStandard::lang_osx_metal1_1:
  case LangStandard::lang_ios_metal1_1:
    return LangOptions::Metal_1_1;
  case LangStandard::lang_macos_metal1_2:
  case LangStandard::lang_osx_metal1_2:
  case LangStandard::lang_ios_metal1_2:
    return LangOptions::Metal_1_2;
  case LangStandard::lang_macos_metal2_0:
  case LangStandard::lang_osx_metal2_0:
  case LangStandard::lang_ios_metal2_0:
    return LangOptions::Metal_2_0;
  case LangStandard::lang_macos_metal2_1:
  case LangStandard::lang_osx_metal2_1:
  case LangStandard::lang_ios_metal2_1:
    return LangOptions::Metal_2_1;
  case LangStandard::lang_macos_metal2_2:
  case LangStandard::lang_osx_metal2_2:
  case LangStandard::lang_ios_metal2_2:
    return LangOptions::Metal_2_2;
  case LangStandard::lang_macos_metal2_3:
  case LangStandard::lang_osx_metal2_3:
  case LangStandard::lang_ios_metal2_3:
    return LangOptions::Metal_2_3;
  case LangStandard::lang_macos_metal2_4:
  case LangStandard::lang_osx_metal2_4:
  case LangStandard::lang_ios_metal2_4:
    return LangOptions::Metal_2_4;
  case LangStandard::lang_metal3_0:
    return LangOptions::Metal_3_0;
  case LangStandard::lang_metal3_1:
    return LangOptions::Metal_3_1;
  case LangStandard::lang_metal3_2:
    return LangOptions::Metal_3_2;
  case LangStandard::lang_metal4_0:
    return LangOptions::Metal_4_0;
  case LangStandard::lang_metal4_1:
    return LangOptions::Metal_4_1;
  default:
    return LangOptions::Metal_Unset;
  }
}

void LangOptions::setLangDefaults(LangOptions &Opts, Language Lang,
                                  const llvm::Triple &T,
                                  std::vector<std::string> &Includes,
                                  LangStandard::Kind LangStd) {
  // Set some properties which depend solely on the input kind; it would be nice
  // to move these to the language standard, and have the driver resolve the
  // input kind + language standard.
  //
  // FIXME: Perhaps a better model would be for a single source file to have
  // multiple language standards (C / C++ std, ObjC std, OpenCL std, OpenMP std)
  // simultaneously active?
  if (Lang == Language::Asm) {
    Opts.AsmPreprocessor = 1;
  } else if (Lang == Language::ObjC || Lang == Language::ObjCXX) {
    Opts.ObjC = 1;
  }

  if (LangStd == LangStandard::lang_unspecified)
    LangStd = getDefaultLanguageStandard(Lang, T);
  const LangStandard &Std = LangStandard::getLangStandardForKind(LangStd);
  Opts.LangStd = LangStd;
  Opts.LineComment = Std.hasLineComments();
  Opts.C99 = Std.isC99();
  Opts.C11 = Std.isC11();
  Opts.C17 = Std.isC17();
  Opts.C2x = Std.isC2x();
  Opts.CPlusPlus = Std.isCPlusPlus();
  Opts.CPlusPlus11 = Std.isCPlusPlus11();
  Opts.CPlusPlus14 = Std.isCPlusPlus14();
  Opts.CPlusPlus17 = Std.isCPlusPlus17();
  Opts.CPlusPlus20 = Std.isCPlusPlus20();
  Opts.CPlusPlus2b = Std.isCPlusPlus2b();
  Opts.GNUMode = Std.isGNUMode();
  Opts.GNUCVersion = 0;
  Opts.HexFloats = Std.hasHexFloats();
  Opts.WChar = Std.isCPlusPlus();
  Opts.Digraphs = Std.hasDigraphs();

  Opts.HLSL = Lang == Language::HLSL;
  if (Opts.HLSL && Opts.IncludeDefaultHeader)
    Includes.push_back("hlsl.h");

  // Set OpenCL Version.
  Opts.OpenCL = Std.isOpenCL();
  if (LangStd == LangStandard::lang_opencl10)
    Opts.OpenCLVersion = 100;
  else if (LangStd == LangStandard::lang_opencl11)
    Opts.OpenCLVersion = 110;
  else if (LangStd == LangStandard::lang_opencl12)
    Opts.OpenCLVersion = 120;
  else if (LangStd == LangStandard::lang_opencl20)
    Opts.OpenCLVersion = 200;
  else if (LangStd == LangStandard::lang_opencl30)
    Opts.OpenCLVersion = 300;
  else if (LangStd == LangStandard::lang_openclcpp10)
    Opts.OpenCLCPlusPlusVersion = 100;
  else if (LangStd == LangStandard::lang_openclcpp2021)
    Opts.OpenCLCPlusPlusVersion = 202100;
  else if (LangStd == LangStandard::lang_hlsl2015)
    Opts.HLSLVersion = (unsigned)LangOptions::HLSL_2015;
  else if (LangStd == LangStandard::lang_hlsl2016)
    Opts.HLSLVersion = (unsigned)LangOptions::HLSL_2016;
  else if (LangStd == LangStandard::lang_hlsl2017)
    Opts.HLSLVersion = (unsigned)LangOptions::HLSL_2017;
  else if (LangStd == LangStandard::lang_hlsl2018)
    Opts.HLSLVersion = (unsigned)LangOptions::HLSL_2018;
  else if (LangStd == LangStandard::lang_hlsl2021)
    Opts.HLSLVersion = (unsigned)LangOptions::HLSL_2021;
  else if (LangStd == LangStandard::lang_hlsl202x)
    Opts.HLSLVersion = (unsigned)LangOptions::HLSL_202x;

  // Set the Metal Shading Language version. The numeric value matches Apple's
  // `__METAL_VERSION__` predefined macro.
  Opts.Metal = Std.isMetal();
  if (Opts.Metal) {
    Opts.MetalVersion = (unsigned)getMetalVersionForLangStandard(LangStd);
  }

  // OpenCL has some additional defaults.
  if (Opts.OpenCL) {
    Opts.AltiVec = 0;
    Opts.ZVector = 0;
    Opts.setDefaultFPContractMode(LangOptions::FPM_On);
    Opts.OpenCLCPlusPlus = Opts.CPlusPlus;
    Opts.OpenCLPipes = Opts.getOpenCLCompatibleVersion() == 200;
    Opts.OpenCLGenericAddressSpace = Opts.getOpenCLCompatibleVersion() == 200;

    // Include default header file for OpenCL.
    if (Opts.IncludeDefaultHeader) {
      if (Opts.DeclareOpenCLBuiltins) {
        // Only include base header file for builtin types and constants.
        Includes.push_back("opencl-c-base.h");
      } else {
        Includes.push_back("opencl-c.h");
      }
    }
  }

  Opts.HIP = Lang == Language::HIP;
  Opts.CUDA = Lang == Language::CUDA || Opts.HIP;
  if (Opts.HIP) {
    // HIP toolchain does not support 'Fast' FPOpFusion in backends since it
    // fuses multiplication/addition instructions without contract flag from
    // device library functions in LLVM bitcode, which causes accuracy loss in
    // certain math functions, e.g. tan(-1e20) becomes -0.933 instead of 0.8446.
    // For device library functions in bitcode to work, 'Strict' or 'Standard'
    // FPOpFusion options in backends is needed. Therefore 'fast-honor-pragmas'
    // FP contract option is used to allow fuse across statements in frontend
    // whereas respecting contract flag in backend.
    Opts.setDefaultFPContractMode(LangOptions::FPM_FastHonorPragmas);
  } else if (Opts.CUDA) {
    if (T.isSPIRV()) {
      // Emit OpenCL version metadata in LLVM IR when targeting SPIR-V.
      Opts.OpenCLVersion = 200;
    }
    // Allow fuse across statements disregarding pragmas.
    Opts.setDefaultFPContractMode(LangOptions::FPM_Fast);
  }

  // Metal has some additional defaults. Evidence for each of these is the
  // reference cc1 invocation recorded in
  // reference/metal-ast-*/meta/metal-cc1-invocations.txt.gz together with the
  // MSL specification restrictions catalogued in
  // research/spec/METAL_CXX_MASTER_ATLAS.md section 2.
  if (Opts.Metal) {
    // MSL forbids exceptions and RTTI in every language version
    // (METAL_CXX_MASTER_ATLAS.md FEAT-12 / FEAT-13).
    Opts.Exceptions = 0;
    Opts.CXXExceptions = 0;
    Opts.RTTI = 0;
    Opts.RTTIData = 0;

    // `half` is a first class scalar type in MSL.
    Opts.NativeHalfType = 1;
    Opts.NativeHalfArgsAndReturns = 1;

    // Apple's driver always passes -ffp-contract=fast for Metal.
    Opts.setDefaultFPContractMode(LangOptions::FPM_Fast);

    // Pull in the Metal standard library headers when asked to, matching the
    // observed `-finclude-default-header` behaviour.
    if (Opts.IncludeDefaultHeader)
      Includes.push_back("metal_stdlib");
  }

  Opts.RenderScript = Lang == Language::RenderScript;

  // OpenCL, C++ and C2x have bool, true, false keywords.
  Opts.Bool = Opts.OpenCL || Opts.CPlusPlus || Opts.C2x;

  // OpenCL and HLSL have half keyword
  // OpenCL, HLSL and Metal have the half keyword.
  Opts.Half = Opts.OpenCL || Opts.HLSL || Opts.Metal;
}

FPOptions FPOptions::defaultWithoutTrailingStorage(const LangOptions &LO) {
  FPOptions result(LO);
  return result;
}

FPOptionsOverride FPOptions::getChangesSlow(const FPOptions &Base) const {
  FPOptions::storage_type OverrideMask = 0;
#define OPTION(NAME, TYPE, WIDTH, PREVIOUS)                                    \
  if (get##NAME() != Base.get##NAME())                                         \
    OverrideMask |= NAME##Mask;
#include "clang/Basic/FPOptions.def"
  return FPOptionsOverride(*this, OverrideMask);
}

LLVM_DUMP_METHOD void FPOptions::dump() {
#define OPTION(NAME, TYPE, WIDTH, PREVIOUS)                                    \
  llvm::errs() << "\n " #NAME " " << get##NAME();
#include "clang/Basic/FPOptions.def"
  llvm::errs() << "\n";
}

LLVM_DUMP_METHOD void FPOptionsOverride::dump() {
#define OPTION(NAME, TYPE, WIDTH, PREVIOUS)                                    \
  if (has##NAME##Override())                                                   \
    llvm::errs() << "\n " #NAME " Override is " << get##NAME##Override();
#include "clang/Basic/FPOptions.def"
  llvm::errs() << "\n";
}
