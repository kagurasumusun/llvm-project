//===--- Metal.cpp - Metal ToolChain Implementation -------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "Metal.h"
#include "CommonArgs.h"
#include "clang/Basic/AIRVersion.h"
#include "clang/Driver/Compilation.h"
#include "clang/Driver/Driver.h"
#include "clang/Driver/DriverDiagnostic.h"
#include "clang/Driver/InputInfo.h"
#include "clang/Driver/Options.h"
#include "llvm/Option/ArgList.h"
#include "llvm/Support/Path.h"
#include "llvm/Support/raw_ostream.h"

using namespace llvm::opt;
using namespace clang;
using namespace clang::driver;
using namespace clang::driver::tools;
using namespace clang::driver::toolchains;

//===----------------------------------------------------------------------===//
// MetalToolChain
//===----------------------------------------------------------------------===//

unsigned MetalToolChain::parseMetalStd(StringRef Std) {
  // Parse -std=metal2.4 -> 240, -std=metal3.2 -> 320, etc.
  // Also handles macos-metal2.4 and ios-metal3.2 forms.
  if (!Std.startswith("metal") && !Std.contains("metal"))
    return 0;

  // Strip platform prefix: macos-metal2.4 -> metal2.4
  size_t MetalPos = Std.find("metal");
  StringRef Suffix = Std.substr(MetalPos + 5); // after "metal"

  unsigned Major = 0, Minor = 0;
  if (Suffix.consumeInteger(10, Major))
    return 0;
  if (Suffix.consume_front("."))
    Suffix.consumeInteger(10, Minor);

  return Major * 100 + Minor * 10;
}

unsigned MetalToolChain::getMetalVersionForStd(llvm::StringRef Std,
                                               const llvm::Triple &Target) {
  unsigned Ver = parseMetalStd(Std);
  if (Ver == 0) {
    // If no -std= was given, pick a reasonable default based on the
    // deployment target.  macOS 26 -> Metal 4.0, macOS 15 -> Metal 3.2, etc.
    unsigned Major = Target.getOSVersion().getMajor();
    if (Target.isMacOSX()) {
      if (Major >= 26) Ver = 400;
      else if (Major >= 15) Ver = 320;
      else if (Major >= 14) Ver = 310;
      else if (Major >= 13) Ver = 300;
      else Ver = 240;
    } else {
      if (Major >= 26) Ver = 400;
      else if (Major >= 18) Ver = 320;
      else if (Major >= 17) Ver = 310;
      else if (Major >= 16) Ver = 300;
      else Ver = 240;
    }
  }
  return Ver;
}

MetalToolChain::MetalToolChain(const Driver &D, const llvm::Triple &Triple,
                               const ArgList &Args)
    : ToolChain(D, Triple, Args) {
  getProgramPaths().push_back(getDriver().getInstalledDir());
  if (getDriver().getInstalledDir() != getDriver().Dir)
    getProgramPaths().push_back(getDriver().Dir);
}

bool MetalToolChain::handlesTarget(const llvm::Triple &Triple) {
  return Triple.isAIR();
}

Tool *MetalToolChain::buildLinker() const {
  return new tools::metal::Linker(*this);
}

void MetalToolChain::AddClangSystemIncludeArgs(
    const ArgList &DriverArgs, ArgStringList &CC1Args) const {
  if (DriverArgs.hasArg(options::OPT_nostdinc))
    return;

  if (!DriverArgs.hasArg(options::OPT_nobuiltininc)) {
    SmallString<128> Dir(getDriver().ResourceDir);
    llvm::sys::path::append(Dir, "include");
    addSystemInclude(DriverArgs, CC1Args, Dir.str());
  }
}

void MetalToolChain::addClangTargetOptions(
    const ArgList &DriverArgs, ArgStringList &CC1Args,
    Action::OffloadKind DeviceOffloadKind) const {
  // Metal always targets AIR.  Set the triple and language options.
  CC1Args.push_back("-triple");
  CC1Args.push_back(DriverArgs.MakeArgString(getTriple().str()));

  // Forward -std= for Metal version selection.
  if (const Arg *A = DriverArgs.getLastArg(options::OPT_std_EQ)) {
    A->render(DriverArgs, CC1Args);

    // Metal version is derived from -std= by the frontend.
  }

  // Forward Metal-specific flags.
  if (const Arg *A = DriverArgs.getLastArg(
          options::OPT_fmetal_math_fp32_functions_EQ)) {
    A->render(DriverArgs, CC1Args);
  }
  if (const Arg *A =
          DriverArgs.getLastArg(options::OPT_fmetal_math_mode_EQ)) {
    A->render(DriverArgs, CC1Args);
  }
  if (DriverArgs.hasArg(options::OPT_fmetal_enable_logging))
    CC1Args.push_back("-fmetal-enable-logging");

  // Disable exception handling and RTTI for Metal.
  CC1Args.push_back("-fno-exceptions");
  CC1Args.push_back("-fno-rtti");

  // Disable standard system includes; Metal uses its own SDK headers.
  CC1Args.push_back("-nostdsysteminc");
}

void MetalToolChain::AddClangCXXStdlibIncludeArgs(
    const ArgList &DriverArgs, ArgStringList &CC1Args) const {
  // Metal does not use a separate C++ stdlib include path; the Metal
  // standard library (<metal_stdlib>) is found through the SDK.
}

//===----------------------------------------------------------------------===//
// Metal Compiler
//===----------------------------------------------------------------------===//

void metal::Compiler::ConstructJob(Compilation &C, const JobAction &JA,
                                   const InputInfo &Output,
                                   const InputInfoList &Inputs,
                                   const ArgList &Args,
                                   const char *LinkingOutput) const {
  const auto &TC =
      static_cast<const toolchains::MetalToolChain &>(getToolChain());
  ArgStringList CmdArgs;

  // Common Clang cc1 arguments.
  CmdArgs.push_back("-cc1");

  // Forward the triple.
  CmdArgs.push_back("-triple");
  CmdArgs.push_back(Args.MakeArgString(TC.getTriple().str()));

  // Select the Metal language mode.
  CmdArgs.push_back("-x");
  CmdArgs.push_back("metal");

  if (const Arg *A = Args.getLastArg(options::OPT_std_EQ)) {
    A->render(Args, CmdArgs);
    // Metal version is derived from -std= by the frontend.
  }

  // Forward metal-specific flags.
  if (const Arg *A =
          Args.getLastArg(options::OPT_fmetal_math_fp32_functions_EQ))
    A->render(Args, CmdArgs);
  if (const Arg *A = Args.getLastArg(options::OPT_fmetal_math_mode_EQ))
    A->render(Args, CmdArgs);
  if (Args.hasArg(options::OPT_fmetal_enable_logging))
    CmdArgs.push_back("-fmetal-enable-logging");

  // Disable exceptions and RTTI.
  CmdArgs.push_back("-fno-exceptions");
  CmdArgs.push_back("-fno-rtti");

  // Optimization and debug info.
  Args.AddLastArg(CmdArgs, options::OPT_O_Group);
  Args.AddLastArg(CmdArgs, options::OPT_g_Group);

  // Metal uses typed pointers (the reference IR is -- as shipped by Apple --
  // entirely in typed pointer form).
  CmdArgs.push_back("-no-opaque-pointers");

  // Output: emit LLVM bitcode (.air = LLVM bitcode container).
  CmdArgs.push_back("-emit-llvm-bc");

  // Input and output.
  CmdArgs.push_back("-o");
  CmdArgs.push_back(Output.getFilename());
  for (const auto &II : Inputs)
    CmdArgs.push_back(II.getFilename());

  C.addCommand(std::make_unique<Command>(
      JA, *this, ResponseFileSupport::AtFileUTF8(),
      Args.MakeArgString(TC.GetProgramPath("clang")), CmdArgs, Inputs,
      Output));
}

//===----------------------------------------------------------------------===//
// Metal Backend
//===----------------------------------------------------------------------===//

void metal::Backend::ConstructJob(Compilation &C, const JobAction &JA,
                                  const InputInfo &Output,
                                  const InputInfoList &Inputs,
                                  const ArgList &Args,
                                  const char *LinkingOutput) const {
  const auto &TC =
      static_cast<const toolchains::MetalToolChain &>(getToolChain());
  ArgStringList CmdArgs;

  CmdArgs.push_back("-cc1");
  CmdArgs.push_back("-triple");
  CmdArgs.push_back(Args.MakeArgString(TC.getTriple().str()));

  // The backend consumes LLVM bitcode from the compiler and can produce
  // assembly (.s) or object (.o).  For Metal the pipeline stops at bitcode
  // (Backend phase produces .air), so the Backend job is really just a
  // passthrough or a -S lowering.
  if (Args.hasArg(options::OPT_S)) {
    CmdArgs.push_back("-S");
  }

  CmdArgs.push_back("-o");
  CmdArgs.push_back(Output.getFilename());
  for (const auto &II : Inputs)
    CmdArgs.push_back(II.getFilename());

  C.addCommand(std::make_unique<Command>(
      JA, *this, ResponseFileSupport::AtFileUTF8(),
      Args.MakeArgString(TC.GetProgramPath("clang")), CmdArgs, Inputs,
      Output));
}

//===----------------------------------------------------------------------===//
// Metal Linker
//===----------------------------------------------------------------------===//

void metal::Linker::ConstructJob(Compilation &C, const JobAction &JA,
                                 const InputInfo &Output,
                                 const InputInfoList &Inputs,
                                 const ArgList &Args,
                                 const char *LinkingOutput) const {
  const auto &TC =
      static_cast<const toolchains::MetalToolChain &>(getToolChain());
  ArgStringList CmdArgs;

  // Metal produces .metallib archives from .air bitcode files.  The
  // standard tool for this is `metallib`, but for now we use `llvm-link`
  // followed by `metal-libtool`-style archive creation.
  //
  // Apple's `metallib` tool takes .air files and produces a .metallib:
  //   metallib -o output.metallib input1.air input2.air
  //
  // When `metallib` is not available, fall back to `llvm-link` + `ar`.

  for (const auto &II : Inputs)
    CmdArgs.push_back(II.getFilename());

  CmdArgs.push_back("-o");
  CmdArgs.push_back(Output.getFilename());

  // Use metallib if available, otherwise llvm-link.
  const char *Linker = Args.MakeArgString(TC.GetProgramPath("metallib"));
  C.addCommand(std::make_unique<Command>(
      JA, *this, ResponseFileSupport::AtFileUTF8(), Linker, CmdArgs, Inputs,
      Output));
}
