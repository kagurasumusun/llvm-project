#include "WinCE.h"
#include "clang/Driver/CommonArgs.h"
#include "clang/Driver/Compilation.h"
#include "clang/Driver/Driver.h"
#include "clang/Driver/DriverDiagnostic.h"
#include "clang/Driver/Types.h"
#include "clang/Options/Options.h"
#include "llvm/Option/ArgList.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/Path.h"
#include "llvm/TargetParser/Host.h"
#include <string>

using namespace clang::driver;
using namespace clang::driver::toolchains;
using namespace clang::driver::tools;
using namespace clang;
using namespace llvm::opt;

static std::string findDefaultSysRoot(const Driver &D, const ArgList &Args) {
  if (const Arg *A = Args.getLastArg(options::OPT__sysroot_EQ))
    return std::string(A->getValue());
  if (!D.SysRoot.empty() && D.SysRoot != "/")
    return std::string(D.SysRoot);

  SmallString<128> Path(D.Dir);
  llvm::sys::path::remove_filename(Path);
  llvm::sys::path::append(Path, "wince-sysroot");
  return std::string(Path);
}

static bool compilingCXX(const ArgList &Args) {
  if (const Arg *A = Args.getLastArg(options::OPT_x)) {
    StringRef V = A->getValue();
    if (V.starts_with("c++") || V == "objective-c++" || V == "c++-module" ||
        V == "c++-header" || V == "c++-pch")
      return true;
  }
  for (StringRef F : Args.getAllArgValues(options::OPT_INPUT)) {
    StringRef Ext = llvm::sys::path::extension(F);
    if (Ext.starts_with("."))
      Ext = Ext.drop_front();
    if (types::isCXX(types::lookupTypeForExtension(Ext)))
      return true;
  }
  return false;
}

WinCE::WinCE(const Driver &D, const llvm::Triple &Triple, const ArgList &Args)
    : Generic_GCC(D, Triple, Args) {
  switch (Triple.getArch()) {
  case llvm::Triple::arm:
  case llvm::Triple::thumb:
  case llvm::Triple::x86:
    break;
  default:
    D.Diag(diag::err_drv_unsupported_wince_arch)
        << Triple.getArchName() << Triple.str();
    break;
  }

  SysRootPath = findDefaultSysRoot(D, Args);
  if (!llvm::sys::fs::exists(SysRootPath)) {
    D.Diag(diag::warn_drv_wince_sysroot_missing)
        << SysRootPath
        << "build-wince-sysroot.sh in kagurasumusun/cellvm-build";
  }
}

Tool *WinCE::buildLinker() const {
  return new tools::wince::Linker(*this);
}

ToolChain::CXXStdlibType WinCE::GetCXXStdlibType(const ArgList &Args) const {
  return ToolChain::CST_Libcxx;
}

void WinCE::addClangTargetOptions(const ArgList &DriverArgs,
                                  ArgStringList &CC1Args,
                                  Action::OffloadKind DeviceOffloadKind) const {
  CC1Args.push_back("-fwchar-type=short");
  CC1Args.push_back("-fno-signed-wchar");
  const bool CLMode = getDriver().IsCLMode();
  if (DriverArgs.hasFlag(options::OPT_fms_extensions,
                         options::OPT_fno_ms_extensions, true))
    CC1Args.push_back("-fms-extensions");
  if (DriverArgs.hasFlag(options::OPT_fms_compatibility,
                         options::OPT_fno_ms_compatibility, true))
    CC1Args.push_back("-fms-compatibility");
  if (!compilingCXX(DriverArgs))
    CC1Args.push_back("-fms-define-stdc");
  if (DriverArgs.hasFlag(options::OPT_fdelayed_template_parsing,
                         options::OPT_fno_delayed_template_parsing, CLMode))
    CC1Args.push_back("-fdelayed-template-parsing");
  if (const Arg *A =
          DriverArgs.getLastArg(options::OPT_fms_compatibility_version))
    CC1Args.push_back(DriverArgs.MakeArgString(
        Twine("-fms-compatibility-version=") + A->getValue()));
  else
    CC1Args.push_back("-fms-compatibility-version=1900");
  if (!DriverArgs.hasArg(options::OPT_fgnuc_version_EQ))
    CC1Args.push_back("-fgnuc-version=14.2");
  if (DriverArgs.hasArg(options::OPT_mthreads) ||
      DriverArgs.hasFlag(options::OPT_pthread, options::OPT_no_pthread,
                         false))
    CC1Args.push_back("-D_MT");
  if (DriverArgs.hasFlag(options::OPT_fgnu89_inline,
                         options::OPT_fno_gnu89_inline, true) &&
      !compilingCXX(DriverArgs) && !getDriver().CCCIsCXX())
    CC1Args.push_back("-fgnu89-inline");
  if (DriverArgs.hasFlag(options::OPT_fcommon, options::OPT_fno_common, true) &&
      !compilingCXX(DriverArgs) && !getDriver().CCCIsCXX())
    CC1Args.push_back("-fcommon");

  for (auto Opt : {options::OPT_mthreads, options::OPT_mwindows,
                   options::OPT_mconsole, options::OPT_mdll}) {
    if (Arg *A = DriverArgs.getLastArgNoClaim(Opt))
      A->ignoreTargetSpecific();
  }

}

void WinCE::AddClangSystemIncludeArgs(const ArgList &DriverArgs,
                                      ArgStringList &CC1Args) const {
  const Driver &D = getDriver();

  if (DriverArgs.hasArg(options::OPT_nostdinc))
    return;

  if (!DriverArgs.hasArg(options::OPT_nobuiltininc)) {
    SmallString<128> Dir(D.ResourceDir);
    llvm::sys::path::append(Dir, "include");
    addSystemInclude(DriverArgs, CC1Args, Dir);
  }

  if (DriverArgs.hasArg(options::OPT_nostdlibinc))
    return;

  SmallString<128> Dir(SysRootPath);
  llvm::sys::path::append(Dir, "include");
  addSystemInclude(DriverArgs, CC1Args, Dir);
}

void WinCE::AddClangCXXStdlibIncludeArgs(const ArgList &DriverArgs,
                                         ArgStringList &CC1Args) const {
  if (DriverArgs.hasArg(options::OPT_nostdinc, options::OPT_nostdincxx,
                        options::OPT_nostdlibinc))
    return;

  SmallString<128> Dir(SysRootPath);
  llvm::sys::path::append(Dir, "include", "c++", "v1");
  addSystemInclude(DriverArgs, CC1Args, Dir);
}

void WinCE::AddCXXStdlibLibArgs(const ArgList &Args,
                                ArgStringList &CmdArgs) const {
  CmdArgs.push_back("libc++.a");
  CmdArgs.push_back("libc++abi.a");
  CmdArgs.push_back("libunwind.a");
}


namespace clang {
namespace driver {
namespace tools {
namespace wince {

void Linker::ConstructJob(Compilation &C, const JobAction &JA,
                          const InputInfo &Output, const InputInfoList &Inputs,
                          const ArgList &Args, const char *LinkingOutput) const {
  const ToolChain &TC = getToolChain();
  ArgStringList CmdArgs;

  CmdArgs.push_back("-wince");

  if (Args.hasFlag(options::OPT_fauto_import, options::OPT_fno_auto_import,
                   true)) {
    CmdArgs.push_back("-auto-import");
    CmdArgs.push_back("-runtime-pseudo-reloc");
  } else {
    CmdArgs.push_back("-auto-import:no");
    CmdArgs.push_back("-runtime-pseudo-reloc:no");
  }

  for (const Arg *A : Args.filtered(options::OPT_L)) {
    A->claim();
    CmdArgs.push_back(Args.MakeArgString(Twine("/libpath:") + A->getValue()));
  }
  const auto &WCE = static_cast<const toolchains::WinCE &>(TC);
  CmdArgs.push_back(Args.MakeArgString(
      Twine("/libpath:") + WCE.getSysRootPath() + "/lib"));

  if (Output.isFilename())
    CmdArgs.push_back(Args.MakeArgString(Twine("/out:") + Output.getFilename()));
  else
    CmdArgs.push_back("/out:a.exe");

  const bool IsDLL = Args.hasArg(options::OPT_shared);
  const bool WantProfiling = Args.hasArg(options::OPT_pg);
  const bool WantThreads = Args.hasArg(options::OPT_mthreads) ||
                           Args.hasArg(options::OPT_pthread);
  if (const Arg *A = Args.getLastArg(options::OPT_g_Group))
    if (!A->getOption().matches(options::OPT_g0))
      CmdArgs.push_back("-debug");

  CmdArgs.push_back("/subsystem:windowsce");
  if (IsDLL) {
    CmdArgs.push_back("/dll");
    CmdArgs.push_back("/entry:DllMainCRTStartup");
    CmdArgs.push_back("/base:0x10000000");
  } else {
    CmdArgs.push_back("/entry:WinMainCRTStartup");
    CmdArgs.push_back("/base:0x10000");
    CmdArgs.push_back("/fixed");
  }

  const char *StartFile =
      IsDLL ? "dllcrt3.o" : (WantProfiling ? "gcrt3.o" : "crt3.o");
  CmdArgs.push_back(StartFile);

  for (const auto &II : Inputs) {
    if (II.isFilename())
      CmdArgs.push_back(II.getFilename());
  }
  for (const Arg *A : Args.filtered(options::OPT_l)) {
    A->claim();
    CmdArgs.push_back(Args.MakeArgString(Twine("lib") + A->getValue() + ".a"));
  }

  if (WantThreads) {
    CmdArgs.push_back("libmingwthrd.a");
    CmdArgs.push_back("libpthread.a");
  }
  CmdArgs.push_back("libmingw32.a");
  CmdArgs.push_back(TC.getTriple().getArch() == llvm::Triple::x86
                        ? "libclang_rt.builtins-i386.a"
                        : "libclang_rt.builtins-arm.a");
  CmdArgs.push_back("libceoldname.a");
  CmdArgs.push_back("libmingwex.a");
  if (WantProfiling)
    CmdArgs.push_back("libgmon.a");

  if (TC.getDriver().CCCIsCXX() || compilingCXX(Args))
    TC.AddCXXStdlibLibArgs(Args, CmdArgs);

  llvm::VersionTuple OSVer = TC.getTriple().getOSVersion();
  unsigned CEVer = OSVer.getMajor();
  if (CEVer != 0 && CEVer < 4) {
    TC.getDriver().Diag(diag::err_drv_unsupported_wince_version) << CEVer;
    return;
  }
  switch (CEVer) {
  case 4:
    CmdArgs.push_back("libcoredll4.a");
    break;
  case 5:
    CmdArgs.push_back("libcoredll.a");
    break;
  default:
    CmdArgs.push_back(TC.getTriple().getArch() == llvm::Triple::x86
                          ? "libcoredll6-x86.a"
                          : "libcoredll6.a");
    break;
  }

  const char *Exec = Args.MakeArgString(TC.GetProgramPath("lld-link"));
  C.addCommand(std::make_unique<Command>(JA, *this, ResponseFileSupport::None(),
                                         Exec, CmdArgs, Inputs, Output));
}

}
}
}
}
