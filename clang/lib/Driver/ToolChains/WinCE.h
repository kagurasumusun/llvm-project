//===--- WinCE.h - Windows CE ToolChain Implementations ---------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_CLANG_LIB_DRIVER_TOOLCHAINS_WINCE_H
#define LLVM_CLANG_LIB_DRIVER_TOOLCHAINS_WINCE_H

#include "clang/Driver/Tool.h"
#include "clang/Driver/ToolChain.h"

namespace clang {
namespace driver {
namespace tools {
namespace wince {

class LLVM_LIBRARY_VISIBILITY Linker final : public Tool {
public:
  Linker(const ToolChain &TC) : Tool("wince::Linker", "lld-link", TC) {}

  bool hasIntegratedCPP() const override { return false; }
  bool isLinkJob() const override { return true; }

  void ConstructJob(Compilation &C, const JobAction &JA,
                    const InputInfo &Output, const InputInfoList &Inputs,
                    const llvm::opt::ArgList &TCArgs,
                    const char *LinkingOutput) const override;
};

} // end namespace wince
} // end namespace tools

namespace toolchains {

/// Toolchain for Windows CE, which is a COFF-based OS of its own: it links
/// through lld-link like the UEFI toolchain does.  The runtime it drives is the
/// GNU-flavoured mingw32ce one, so the start-up objects and compatibility
/// libraries named below carry that package's file names; where an SDK keeps
/// them is not assumed, as both the plain and the usr/-prefixed directory
/// layouts are searched.  Nothing here is a property of CE on every CPU it ran
/// on, which is why the architecture-specific questions stay in the per-arch
/// targets.
///
/// That division is the specification of this target rather than an
/// implementation detail.  Which CPU is compiled for, which instruction set is
/// assembled and which FPU is available are answered by the architecture
/// component of the triple and by -mcpu=, -march= and -mfpu=, exactly as for
/// every other ARM target: the OS implies no CPU of its own, so
/// "armv5tej-pc-wince" gets the CPU that architecture has everywhere and a bare
/// "arm-pc-wince" gets LLVM's default for an unspecified ARM architecture.  The
/// OS answers what is the same on every CE release and CPU: the COFF object
/// format and its lld-link driver, the image base and the CE subsystem, the
/// unwind records, the soft-float ARM C ABI, and the import library named after
/// the CE version written in the triple.
class LLVM_LIBRARY_VISIBILITY WinCE : public ToolChain {
public:
  WinCE(const Driver &D, const llvm::Triple &Triple,
        const llvm::opt::ArgList &Args);

  bool HasNativeLLVMSupport() const override { return true; }

  llvm::ExceptionHandling
  GetExceptionModel(const llvm::opt::ArgList &Args) const override {
    return getTriple().getDefaultExceptionHandling();
  }

  UnwindTableLevel
  getDefaultUnwindTableLevel(const llvm::opt::ArgList &Args) const override {
    return UnwindTableLevel::Asynchronous;
  }

  // CE images are never position independent; the loader rebases them by
  // fixing up the image base, which is what /base: plus /fixed express.
  bool isPICDefault() const override { return false; }
  bool isPIEDefault(const llvm::opt::ArgList &Args) const override {
    return false;
  }
  bool isPICDefaultForced() const override { return false; }

  const char *getDefaultLinker() const override { return "lld-link"; }

  RuntimeLibType GetDefaultRuntimeLibType() const override {
    return RLT_CompilerRT;
  }
  CXXStdlibType GetDefaultCXXStdlibType() const override { return CST_Libcxx; }

  // The debug tooling a CE release was shipped with predates DWARF 5, and the
  // version is not recorded in the image, so stay on 4; -gdwarf-5 remains
  // available for an environment whose readers are newer.
  unsigned GetDefaultDwarfVersion() const override { return 4; }

  std::string computeSysRoot() const override;

  std::string getCompilerRTPath() const override;

  void AddClangSystemIncludeArgs(
      const llvm::opt::ArgList &DriverArgs,
      llvm::opt::ArgStringList &CC1Args) const override;
  void AddClangCXXStdlibIncludeArgs(
      const llvm::opt::ArgList &DriverArgs,
      llvm::opt::ArgStringList &CC1Args) const override;
  void AddCXXStdlibLibArgs(const llvm::opt::ArgList &Args,
                           llvm::opt::ArgStringList &CmdArgs) const override;
  void addClangTargetOptions(
      const llvm::opt::ArgList &DriverArgs, llvm::opt::ArgStringList &CC1Args,
      Action::OffloadKind DeviceOffloadKind) const override;

protected:
  Tool *buildAssembler() const override;
  Tool *buildLinker() const override;
};

} // end namespace toolchains
} // end namespace driver
} // end namespace clang

#endif // LLVM_CLANG_LIB_DRIVER_TOOLCHAINS_WINCE_H
