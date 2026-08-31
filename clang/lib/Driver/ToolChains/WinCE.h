//===--- WinCE.h - Windows CE ToolChain Implementations ---------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_CLANG_LIB_DRIVER_TOOLCHAINS_WINCE_H
#define LLVM_CLANG_LIB_DRIVER_TOOLCHAINS_WINCE_H

#include "Gnu.h"
#include "clang/Driver/Tool.h"
#include "clang/Driver/ToolChain.h"

namespace clang {
namespace driver {
namespace tools {

/// Windows CE link tool: drives the LLD COFF driver (lld-link) with the
/// Windows CE image defaults (subsystem 9, image base 0x10000, no leading
/// underscore decoration, DllMainCRTStartup for DLLs).
namespace wince {
class LLVM_LIBRARY_VISIBILITY Linker final : public Tool {
public:
  Linker(const ToolChain &TC) : Tool("WinCE::Linker", "linker", TC) {}

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

class LLVM_LIBRARY_VISIBILITY WinCE : public Generic_GCC {
public:
  WinCE(const Driver &D, const llvm::Triple &Triple,
        const llvm::opt::ArgList &Args);

  bool HasNativeLLVMSupport() const override { return true; }
  bool IsIntegratedAssemblerDefault() const override { return true; }

  llvm::ExceptionHandling
  GetExceptionModel(const llvm::opt::ArgList &Args) const override {
    // Userland ARM EHABI unwinding via libunwind; the Windows CE kernel
    // provides no exception dispatch of its own.
    return llvm::ExceptionHandling::ARM;
  }

  UnwindTableLevel
  getDefaultUnwindTableLevel(const llvm::opt::ArgList &Args) const override {
    // ARM EHABI .ARM.exidx tables are mandatory: even C frames are walked
    // by the Itanium-based unwinder during C++ exception propagation.
    return UnwindTableLevel::Asynchronous;
  }

  bool isPICDefault() const override { return false; }
  bool isPIEDefault(const llvm::opt::ArgList &Args) const override {
    return false;
  }
  bool isPICDefaultForced() const override { return false; }

  CXXStdlibType GetCXXStdlibType(const llvm::opt::ArgList &Args) const override;

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
  void AddCXXStdlibLibArgs(const llvm::opt::ArgList &Args,
                           llvm::opt::ArgStringList &CmdArgs) const override;

  unsigned GetDefaultDwarfVersion() const override { return 4; }

  /// Absolute path of the WinCE sysroot directory (may not exist, in which
  /// case a warning is emitted at link time).
  StringRef getSysRootPath() const { return SysRootPath; }

protected:
  Tool *buildLinker() const override;

private:
  std::string SysRootPath;
};

} // end namespace toolchains
} // end namespace driver
} // end namespace clang

#endif // LLVM_CLANG_LIB_DRIVER_TOOLCHAINS_WINCE_H
