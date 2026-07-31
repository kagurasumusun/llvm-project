//===--- Metal.h - Metal ToolChain Implementations --------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_CLANG_LIB_DRIVER_TOOLCHAINS_METAL_H
#define LLVM_CLANG_LIB_DRIVER_TOOLCHAINS_METAL_H

#include "clang/Driver/Tool.h"
#include "clang/Driver/ToolChain.h"

namespace clang {
namespace driver {

namespace toolchains {

class LLVM_LIBRARY_VISIBILITY MetalToolChain : public ToolChain {
public:
  MetalToolChain(const Driver &D, const llvm::Triple &Triple,
                 const llvm::opt::ArgList &Args);
  ~MetalToolChain() override = default;

  static bool handlesTarget(const llvm::Triple &Triple);

  bool useIntegratedAs() const override { return true; }
  bool isCrossCompiling() const override { return true; }
  bool isPICDefault() const override { return false; }
  bool isPIEDefault(const llvm::opt::ArgList &Args) const override {
    return false;
  }
  bool isPICDefaultForced() const override { return false; }
  bool SupportsProfiling() const override { return false; }
  bool IsMathErrnoDefault() const override { return false; }

  StringRef getOSLibName() const override { return "metal"; }

  RuntimeLibType GetDefaultRuntimeLibType() const override {
    return ToolChain::RLT_CompilerRT;
  }
  CXXStdlibType GetDefaultCXXStdlibType() const override {
    return ToolChain::CST_Libcxx;
  }

  const char *getDefaultLinker() const override { return "ld64.lld"; }

  void
  AddClangSystemIncludeArgs(const llvm::opt::ArgList &DriverArgs,
                            llvm::opt::ArgStringList &CC1Args) const override;
  void
  addClangTargetOptions(const llvm::opt::ArgList &DriverArgs,
                        llvm::opt::ArgStringList &CC1Args,
                        Action::OffloadKind DeviceOffloadKind) const override;

  void AddClangCXXStdlibIncludeArgs(
      const llvm::opt::ArgList &DriverArgs,
      llvm::opt::ArgStringList &CC1Args) const override;

  /// Derive the Metal language version from -std= and the deployment target.
  static unsigned getMetalVersionForStd(llvm::StringRef Std,
                                        const llvm::Triple &Target);

protected:
  Tool *buildLinker() const override;

private:
  /// Parse the Metal version from -std=metalX.Y.
  static unsigned parseMetalStd(llvm::StringRef Std);
};

} // namespace toolchains

namespace tools {
namespace metal {

class LLVM_LIBRARY_VISIBILITY Compiler : public Tool {
public:
  Compiler(const ToolChain &TC) : Tool("metal::Compiler", "clang", TC) {}

  bool hasIntegratedCPP() const override { return true; }
  bool hasIntegratedBackend() const override { return false; }
  bool canEmitIR() const override { return true; }
  bool hasIntegratedAssembler() const override { return false; }

  void ConstructJob(Compilation &C, const JobAction &JA,
                    const InputInfo &Output, const InputInfoList &Inputs,
                    const llvm::opt::ArgList &TCArgs,
                    const char *LinkingOutput) const override;
};

class LLVM_LIBRARY_VISIBILITY Backend : public Tool {
public:
  Backend(const ToolChain &TC) : Tool("metal::Backend", "clang", TC) {}

  bool hasIntegratedCPP() const override { return true; }
  bool hasIntegratedBackend() const override { return true; }
  bool canEmitIR() const override { return true; }

  void ConstructJob(Compilation &C, const JobAction &JA,
                    const InputInfo &Output, const InputInfoList &Inputs,
                    const llvm::opt::ArgList &TCArgs,
                    const char *LinkingOutput) const override;
};

class LLVM_LIBRARY_VISIBILITY Linker : public Tool {
public:
  Linker(const ToolChain &TC) : Tool("metal::Linker", "ld64.lld", TC) {}
  bool isLinkJob() const override { return true; }
  bool hasIntegratedCPP() const override { return false; }
  void ConstructJob(Compilation &C, const JobAction &JA,
                    const InputInfo &Output, const InputInfoList &Inputs,
                    const llvm::opt::ArgList &TCArgs,
                    const char *LinkingOutput) const override;
};

} // namespace metal
} // namespace tools

} // namespace driver
} // namespace clang

#endif
