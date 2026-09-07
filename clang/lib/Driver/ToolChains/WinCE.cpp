#include "WinCE.h"
#include "Gnu.h"
#include "clang/Driver/CommonArgs.h"
#include "clang/Driver/Compilation.h"
#include "clang/Driver/Driver.h"
#include "clang/Driver/DriverDiagnostic.h"
#include "clang/Options/Options.h"
#include "llvm/Option/ArgList.h"
#include "llvm/Support/Path.h"
#include "llvm/Support/VirtualFileSystem.h"

using namespace clang::driver;
using namespace clang::driver::toolchains;
using namespace clang::driver::tools;
using namespace clang;
using namespace llvm::opt;

WinCE::WinCE(const Driver &D, const llvm::Triple &Triple, const ArgList &Args)
    : ToolChain(D, Triple, Args), SysRoot(D.SysRoot) {
  getProgramPaths().push_back(D.Dir);
  getLibraryPaths().clear();
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

  if (!Args.hasArg(options::OPT__sysroot_EQ) &&
      (SysRoot.empty() || SysRoot == "/")) {
    SmallString<128> Path(D.Dir);
    llvm::sys::path::remove_filename(Path);
    llvm::sys::path::append(Path, "wince-sysroot");
    SysRoot = std::string(Path);
  }
  SmallString<128> LibPath(SysRoot);
  llvm::sys::path::append(LibPath, "lib");
  getFilePaths().push_back(std::string(LibPath));
  if (!getVFS().exists(SysRoot))
    D.Diag(diag::warn_drv_wince_sysroot_missing)
        << SysRoot << "build-wince-sysroot.sh in kagurasumusun/cellvm-build";
}

std::string WinCE::computeSysRoot() const { return SysRoot; }

std::string WinCE::getCompilerRTPath() const {
  SmallString<128> Path(computeSysRoot());
  llvm::sys::path::append(Path, "lib");
  return std::string(Path);
}

Tool *WinCE::buildAssembler() const {
  return new tools::gnutools::Assembler(*this);
}

Tool *WinCE::buildLinker() const { return new tools::wince::Linker(*this); }

void WinCE::addClangTargetOptions(const ArgList &DriverArgs,
                                 ArgStringList &CC1Args,
                                 Action::OffloadKind DeviceOffloadKind) const {
  if (DriverArgs.hasArg(options::OPT_mthreads) ||
      DriverArgs.hasFlag(options::OPT_pthread, options::OPT_no_pthread, false))
    CC1Args.push_back("-D_MT");

  for (auto Opt : {options::OPT_mthreads, options::OPT_mwindows,
                   options::OPT_mconsole, options::OPT_mdll})
    if (Arg *A = DriverArgs.getLastArgNoClaim(Opt))
      A->ignoreTargetSpecific();
}

void WinCE::AddClangSystemIncludeArgs(const ArgList &DriverArgs,
                                     ArgStringList &CC1Args) const {
  if (DriverArgs.hasArg(options::OPT_nostdinc))
    return;
  if (!DriverArgs.hasArg(options::OPT_nobuiltininc)) {
    SmallString<128> Dir(getDriver().ResourceDir);
    llvm::sys::path::append(Dir, "include");
    addSystemInclude(DriverArgs, CC1Args, Dir);
  }
  if (DriverArgs.hasArg(options::OPT_nostdlibinc))
    return;
  SmallString<128> Dir(computeSysRoot());
  llvm::sys::path::append(Dir, "include");
  addSystemInclude(DriverArgs, CC1Args, Dir);
}

void WinCE::AddClangCXXStdlibIncludeArgs(const ArgList &DriverArgs,
                                        ArgStringList &CC1Args) const {
  if (DriverArgs.hasArg(options::OPT_nostdinc, options::OPT_nostdincxx,
                        options::OPT_nostdlibinc))
    return;
  if (GetCXXStdlibType(DriverArgs) != CST_Libcxx)
    return;
  SmallString<128> Dir(computeSysRoot());
  llvm::sys::path::append(Dir, "include", "c++", "v1");
  addSystemInclude(DriverArgs, CC1Args, Dir);
}

void WinCE::AddCXXStdlibLibArgs(const ArgList &Args,
                               ArgStringList &CmdArgs) const {
  if (GetCXXStdlibType(Args) == CST_Libstdcxx) {
    CmdArgs.push_back("libstdc++.a");
    return;
  }
  CmdArgs.push_back("libc++.a");
  if (Args.hasArg(options::OPT_fexperimental_library))
    CmdArgs.push_back("libc++experimental.a");
  CmdArgs.push_back("libc++abi.a");
  CmdArgs.push_back("libunwind.a");
}

static const char *getCOFFLibraryName(const ArgList &Args, StringRef Name) {
  if (Name.consume_front(":"))
    return Args.MakeArgString(Name);
  return Args.MakeArgString(Twine("lib") + Name + ".a");
}

void tools::wince::Linker::ConstructJob(
    Compilation &C, const JobAction &JA, const InputInfo &Output,
    const InputInfoList &Inputs, const ArgList &Args,
    const char *LinkingOutput) const {
  const ToolChain &TC = getToolChain();
  ArgStringList CmdArgs;
  CmdArgs.push_back("-wince");

  if (Args.hasFlag(options::OPT_fauto_import, options::OPT_fno_auto_import, true)) {
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
  for (const auto &Path : TC.getFilePaths())
    CmdArgs.push_back(Args.MakeArgString(Twine("/libpath:") + Path));

  if (Output.isFilename())
    CmdArgs.push_back(Args.MakeArgString(Twine("/out:") + Output.getFilename()));
  else
    CmdArgs.push_back("/out:a.exe");

  const bool IsDLL = Args.hasArg(options::OPT_shared, options::OPT_mdll);
  const bool WantProfiling = Args.hasArg(options::OPT_pg);
  const bool WantThreads = Args.hasArg(options::OPT_mthreads) ||
                           Args.hasFlag(options::OPT_pthread,
                                        options::OPT_no_pthread, false);
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

  if (!Args.hasArg(options::OPT_nostdlib, options::OPT_nostartfiles)) {
    const char *StartFile = IsDLL ? "dllcrt3.o"
                                 : WantProfiling ? "gcrt3.o" : "crt3.o";
    CmdArgs.push_back(Args.MakeArgString(TC.GetFilePath(StartFile)));
  }

  Args.AddAllArgValues(CmdArgs, options::OPT__SLASH_link);
  InputInfoList LinkerInputs;
  for (const InputInfo &Input : Inputs) {
    if (Input.isInputArg() &&
        Input.getInputArg().getOption().matches(options::OPT_l)) {
      const Arg &A = Input.getInputArg();
      A.claim();
      InputInfo Library(Input.getType(), getCOFFLibraryName(Args, A.getValue()),
                        Input.getBaseInput());
      Library.setAction(Input.getAction());
      LinkerInputs.push_back(Library);
    } else {
      LinkerInputs.push_back(Input);
    }
  }
  AddLinkerInputs(TC, LinkerInputs, Args, CmdArgs, JA);

  if (!Args.hasArg(options::OPT_nostdlib, options::OPT_nodefaultlibs)) {
    if (WantThreads) {
      CmdArgs.push_back("libmingwthrd.a");
      CmdArgs.push_back("libpthread.a");
    }
    CmdArgs.push_back("libmingw32.a");
    ArgStringList RuntimeArgs;
    AddRunTimeLibs(TC, TC.getDriver(), RuntimeArgs, Args);
    for (StringRef Arg : RuntimeArgs)
      CmdArgs.push_back(Arg.starts_with("-l")
                            ? getCOFFLibraryName(Args, Arg.drop_front(2))
                            : Args.MakeArgString(Arg));
    CmdArgs.push_back("libceoldname.a");
    CmdArgs.push_back("libmingwex.a");
    if (WantProfiling)
      CmdArgs.push_back("libgmon.a");
    if (TC.ShouldLinkCXXStdlib(Args))
      TC.AddCXXStdlibLibArgs(Args, CmdArgs);

    unsigned CEVersion = TC.getTriple().getOSVersion().getMajor();
    if (CEVersion && CEVersion < 4) {
      TC.getDriver().Diag(diag::err_drv_unsupported_wince_version) << CEVersion;
      return;
    }
    switch (CEVersion) {
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
  }

  C.addCommand(std::make_unique<Command>(
      JA, *this, ResponseFileSupport::AtFileUTF16(),
      Args.MakeArgString(TC.GetLinkerPath()), CmdArgs, Inputs, Output));
}
