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
  Linker(const ToolChain &TC) : Tool("WinCE::Linker", "linker", TC) {}

  bool hasIntegratedCPP() const override { return false; }
  bool isLinkJob() const override { return true; }

  void ConstructJob(Compilation &C, const JobAction &JA,
                    const InputInfo &Output, const InputInfoList &Inputs,
                    const llvm::opt::ArgList &TCArgs,
                    const char *LinkingOutput) const override;
};

}
}

namespace toolchains {

class LLVM_LIBRARY_VISIBILITY WinCE : public ToolChain {
public:
  WinCE(const Driver &D, const llvm::Triple &Triple,
        const llvm::opt::ArgList &Args);

  bool HasNativeLLVMSupport() const override { return true; }
  bool IsIntegratedAssemblerDefault() const override { return true; }

  llvm::ExceptionHandling
  GetExceptionModel(const llvm::opt::ArgList &Args) const override {
    return getTriple().getDefaultExceptionHandling();
  }

  UnwindTableLevel
  getDefaultUnwindTableLevel(const llvm::opt::ArgList &Args) const override {
    return UnwindTableLevel::Asynchronous;
  }

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

  std::string computeSysRoot() const override;
  std::string getCompilerRTPath() const override;

  void AddClangSystemIncludeArgs(
      const llvm::opt::ArgList &DriverArgs,
      llvm::opt::ArgStringList &CC1Args) const override;
  void addClangTargetOptions(const llvm::opt::ArgList &DriverArgs,
                             llvm::opt::ArgStringList &CC1Args,
                             Action::OffloadKind DeviceOffloadKind) const override;
  void AddClangCXXStdlibIncludeArgs(
      const llvm::opt::ArgList &DriverArgs,
      llvm::opt::ArgStringList &CC1Args) const override;
  void AddCXXStdlibLibArgs(const llvm::opt::ArgList &Args,
                          llvm::opt::ArgStringList &CmdArgs) const override;

  unsigned GetDefaultDwarfVersion() const override { return 4; }

protected:
  Tool *buildAssembler() const override;
  Tool *buildLinker() const override;

private:
  std::string SysRoot;
};

}
}
}

#endif
