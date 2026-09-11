//===-- WinCE.cpp - Windows CE Tool Chain ---------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "WinCE.h"
#include "Gnu.h"
#include "clang/Driver/CommonArgs.h"
#include "clang/Driver/Compilation.h"
#include "clang/Driver/Driver.h"
#include "clang/Driver/DriverDiagnostic.h"
#include "clang/Options/Options.h"
#include "llvm/ADT/SmallString.h"
#include "llvm/Option/ArgList.h"
#include "llvm/Support/Path.h"
#include "llvm/Support/VirtualFileSystem.h"

using namespace clang::driver;
using namespace clang::driver::toolchains;
using namespace clang::driver::tools;
using namespace clang;
using namespace llvm::opt;

namespace {

// The name a CE library is passed to lld-link under.  CE import libraries are
// plain COFF archives named lib<name>.a, which is what a GNU-flavoured -l
// refers to on this target; "-l:<file>" selects a file verbatim, as in GNU ld.
const char *getCOFFLibraryName(const ArgList &Args, StringRef Name) {
  if (Name.consume_front(":"))
    return Args.MakeArgString(Name);
  return Args.MakeArgString(Twine("lib") + Name + ".a");
}

// Which coredll import library to name is a property of the SDK rather than of
// Windows CE.  The CE release an image targets comes from the target triple,
// but the file name its sysroot carries does not: the cegcc-family packages
// this tool chain was first built against fold the version and the architecture
// into the name, a GNU sysroot that keeps the architecture in the directory
// spells the library plainly, and a Microsoft SDK calls it coredll.lib.  So ask
// the sysroot which of them is there, most specific for the target first, and
// fall back to the cegcc naming rule where there is nothing to look at, the
// same thing a -### of an uninstalled toolchain has to print.
std::string getCoreDLLLibrary(const ToolChain &TC, StringRef ArchSuffix,
                              unsigned CEVersion) {
  // The name to fall back on is the one this tool chain was written for, and it
  // carries the architecture only for the CE 6 generation: the CE 4 and CE 5
  // import libraries were never given a per-CPU spelling, their sysroots being
  // per-CPU installations instead.
  std::string Default;
  if (CEVersion == 4)
    Default = "libcoredll4.a";
  else if (CEVersion == 5)
    Default = "libcoredll.a";
  else
    Default = (Twine("libcoredll6") + ArchSuffix + ".a").str();

  // Then the other names a sysroot may use for the same library, since the
  // generations and the architecture suffix are both the packager's choice.
  SmallVector<std::string, 8> Candidates;
  Candidates.push_back(Default);
  for (StringRef Suffix : {ArchSuffix, StringRef()})
    for (StringRef Digits : {StringRef("4"), StringRef(), StringRef("6")})
      Candidates.push_back(
          (Twine("libcoredll") + Digits + Suffix + ".a").str());
  Candidates.push_back("coredll.lib");

  for (const std::string &Name : Candidates)
    for (const std::string &Dir : TC.getFilePaths()) {
      SmallString<128> Path(Dir);
      llvm::sys::path::append(Path, Name);
      if (TC.getDriver().getVFS().exists(Path))
        return Name;
    }
  return Default;
}

} // end anonymous namespace

WinCE::WinCE(const Driver &D, const llvm::Triple &Triple, const ArgList &Args)
    : ToolChain(D, Triple, Args) {
  getProgramPaths().push_back(getDriver().Dir);

  // The OS component of the triple is what selects this tool chain: "windowsce"
  // there is canonical, with "wince" and "mingw32ce" as aliases, and a CE
  // version may follow any of them.  The vendor component is not consulted, as
  // it is for no other OS, so the GNU-style "arm-wince-pe", which carries the
  // OS name in that slot and leaves "pe" in the OS slot, names no known OS and
  // does not reach here.

  // compiler-rt comes from the SDK, the directory getCompilerRTPath() names,
  // under the GNU spelling libclang_rt.builtins-<arch>.a that sits next to
  // crt3.o and the import libraries.  The base class searches a per-target
  // runtime directory beside the driver before that one, and reports its
  // unsuffixed file name even when no runtime is installed anywhere, so drop
  // the directory instead of letting it shadow the SDK's copy.
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

  // The CE SDK is not installed in place of the host CRT, so an absent sysroot
  // still permits compiling; only linking needs the import libraries.
  std::string SysRoot = computeSysRoot();
  if (!getVFS().exists(SysRoot))
    D.Diag(diag::warn_drv_wince_sysroot_missing) << SysRoot;

  // Which directory holds them is the SDK's choice rather than a property of
  // CE: a sysroot unpacked at its own root has lib/ next to the per-target
  // directories, while one built by a GNU package with --prefix=/usr keeps them
  // under usr/.  Search both, so that a sysroot for another CE generation,
  // vendor or CPU works with nothing but --sysroot=.
  for (StringRef LibDir : {"lib", "usr/lib"}) {
    SmallString<128> LibPath(SysRoot);
    llvm::sys::path::append(LibPath, LibDir);
    getFilePaths().push_back(std::string(LibPath));
  }
}

// CE has no installed default sysroot to probe, so the SDK directory next to
// the driver is used unless --sysroot= says otherwise.  "/" is what the driver
// falls back to when nothing is configured and is not a usable SDK root.
std::string WinCE::computeSysRoot() const {
  StringRef SysRoot = getDriver().SysRoot;
  if (!SysRoot.empty() && SysRoot != "/")
    return SysRoot.str();

  SmallString<128> Dir(getDriver().Dir);
  llvm::sys::path::remove_filename(Dir);
  llvm::sys::path::append(Dir, "wince-sysroot");
  return std::string(Dir);
}

std::string WinCE::getCompilerRTPath() const {
  // The runtime is installed next to the SDK's other libraries, which is either
  // of the two library directories; name the one that exists so that the path
  // never points nowhere.
  for (StringRef LibDir : {"lib", "usr/lib"}) {
    SmallString<128> Path(computeSysRoot());
    llvm::sys::path::append(Path, LibDir);
    if (getVFS().exists(Path))
      return std::string(Path);
  }

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
  // The runtime package this tool chain links against builds its CRT threaded
  // only, and _MT is what its headers look for, so the define follows -mthreads
  // rather than being asked for.  The flavour switches are linker-level and are
  // consumed by the Linker below, so keep cc1 from rejecting them.
  if (DriverArgs.hasArg(options::OPT_mthreads) ||
      DriverArgs.hasFlag(options::OPT_pthread, options::OPT_no_pthread, false))
    CC1Args.push_back("-D_MT");

  for (options::ID Opt :
       {options::OPT_mthreads, options::OPT_mwindows, options::OPT_mconsole,
        options::OPT_mdll})
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
    addSystemInclude(DriverArgs, CC1Args, Dir.str());
  }

  if (DriverArgs.hasArg(options::OPT_nostdlibinc))
    return;

  // The headers sit where the SDK put its headers, which for the same reason as
  // the libraries is either directly in the sysroot or below usr/.
  for (StringRef IncDir : {"include", "usr/include"}) {
    SmallString<128> Dir(computeSysRoot());
    llvm::sys::path::append(Dir, IncDir);
    addSystemInclude(DriverArgs, CC1Args, Dir.str());
  }
}

void WinCE::AddClangCXXStdlibIncludeArgs(const ArgList &DriverArgs,
                                         ArgStringList &CC1Args) const {
  if (DriverArgs.hasArg(options::OPT_nostdinc, options::OPT_nostdincxx,
                        options::OPT_nostdlibinc))
    return;

  if (GetCXXStdlibType(DriverArgs) != CST_Libcxx)
    return;

  // The runtime package carries libc++ next to its own headers rather than in a
  // versioned directory, and there is no GCC installation to search; which of
  // the two layouts the headers sit in is again the SDK's choice.
  for (StringRef IncDir : {"include", "usr/include"}) {
    SmallString<128> Dir(computeSysRoot());
    llvm::sys::path::append(Dir, IncDir, "c++", "v1");
    addSystemInclude(DriverArgs, CC1Args, Dir.str());
  }
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

void tools::wince::Linker::ConstructJob(
    Compilation &C, const JobAction &JA, const InputInfo &Output,
    const InputInfoList &Inputs, const ArgList &Args,
    const char *LinkingOutput) const {
  const ToolChain &TC = getToolChain();
  const Driver &D = TC.getDriver();
  ArgStringList CmdArgs;

  // Silence warnings for "clang -g/-w/-emit-llvm foo.o -o foo".
  Args.ClaimAllArgs(options::OPT_g_Group);
  Args.ClaimAllArgs(options::OPT_emit_llvm);
  Args.ClaimAllArgs(options::OPT_w);

  // Two lld options cover the case where a variable looks module-local in an
  // object file but has to be imported from a DLL: -auto-import synthesizes the
  // thunk, and -runtime-pseudo-reloc records the fixups of the code that
  // references it as runtime pseudo relocations (v2 items), which are applied
  // when the image is loaded.  They are on by default because the import
  // libraries this tool chain finds in a CE sysroot are among those that name
  // functions only; -fno-auto-import turns both off for a runtime whose import
  // libraries name everything.
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
  for (const std::string &Path : TC.getFilePaths())
    CmdArgs.push_back(Args.MakeArgString(Twine("/libpath:") + Path));

  assert((Output.isFilename() || Output.isNothing()) && "invalid output");
  if (Output.isFilename())
    CmdArgs.push_back(
        Args.MakeArgString(Twine("/out:") + Output.getFilename()));
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

  // There is no other CE switch to pass on: lld takes the CE image defaults,
  // the exception tables among them, from this subsystem name and from the CE
  // machine type of the objects.  The subsystem version is different: it states
  // the release the image was built for, and that is what the triple carries.
  // Leaving it out would keep every CE 4.x and 5.x image claiming the version
  // lld defaults the CE subsystem to.  Only major and minor go into the field,
  // so a build number as in 6.0.19041 stays out of it.
  std::string Subsystem = "/subsystem:windowsce";
  if (llvm::VersionTuple OSVersion = TC.getTriple().getOSVersion();
      !OSVersion.empty())
    Subsystem = (Twine("/subsystem:windowsce,") +
                 std::to_string(OSVersion.getMajor()) + "." +
                 std::to_string(OSVersion.getMinor().value_or(0)))
                    .str();
  CmdArgs.push_back(Args.MakeArgString(Subsystem));
  // The base addresses are the defaults the PE/COFF specification records for
  // this OS: 0x10000000 for a DLL and 0x00010000 for an executable.  A DLL
  // keeps its base relocations, being the image kind that can be mapped
  // elsewhere than the address it was linked for; an executable is bound to its
  // base instead, with /fixed, which is lld's switch for writing no base
  // relocations and for clearing the dynamic base flag it would otherwise set.
  if (IsDLL) {
    CmdArgs.push_back("/dll");
    CmdArgs.push_back("/entry:DllMainCRTStartup");
    CmdArgs.push_back("/base:0x10000000");
  } else {
    // CE has a single subsystem for both kinds of program, so -mconsole does
    // not change /subsystem: the way it does on the desktop; it chooses which
    // C runtime startup routine the image enters instead.  The two names are
    // Microsoft's convention for the startup of main() and WinMain(); a runtime
    // that names its own startup otherwise is given /entry: by hand, which wins
    // as it is passed on after this one.  lld picks an entry point from the
    // subsystem when not told otherwise, and its table knows only the desktop
    // GUI/console pair, so the name is spelled out here.
    const Arg *SubsysArg =
        Args.getLastArg(options::OPT_mwindows, options::OPT_mconsole);
    bool IsConsole =
        SubsysArg && SubsysArg->getOption().matches(options::OPT_mconsole);
    CmdArgs.push_back(
        IsConsole ? "/entry:mainCRTStartup" : "/entry:WinMainCRTStartup");
    CmdArgs.push_back("/base:0x10000");
    CmdArgs.push_back("/fixed");
  }

  if (!Args.hasArg(options::OPT_nostdlib, options::OPT_nostartfiles)) {
    const char *StartFile =
        IsDLL ? "dllcrt3.o" : WantProfiling ? "gcrt3.o" : "crt3.o";
    CmdArgs.push_back(Args.MakeArgString(TC.GetFilePath(StartFile)));
  }

  Args.AddAllArgValues(CmdArgs, options::OPT__SLASH_link);

  // Translate -l options to the COFF library names, which is what a plain -l
  // refers to on this target; everything else is passed through as a file.
  InputInfoList LinkerInputs;
  for (const InputInfo &Input : Inputs) {
    if (Input.isInputArg() &&
        Input.getInputArg().getOption().matches(options::OPT_l)) {
      const Arg &A = Input.getInputArg();
      A.claim();
      InputInfo Library(Input.getType(),
                        getCOFFLibraryName(Args, A.getValue()),
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
    AddRunTimeLibs(TC, D, RuntimeArgs, Args);
    for (StringRef Arg : RuntimeArgs)
      CmdArgs.push_back(Arg.starts_with("-l")
                            ? getCOFFLibraryName(Args, Arg.drop_front(2))
                            : Args.MakeArgString(Arg));
    // mingw32ce redirects the API names that only exist on desktop Windows.
    CmdArgs.push_back("libceoldname.a");
    CmdArgs.push_back("libmingwex.a");
    if (WantProfiling)
      CmdArgs.push_back("libgmon.a");
    if (TC.ShouldLinkCXXStdlib(Args))
      TC.AddCXXStdlibLibArgs(Args, CmdArgs);

    // COREDLL is versioned per CE release, and a sysroot that carries several
    // CPUs' import libraries in one directory has to be asked for this CPU's,
    // rather than silently linking against ARM's.
    const llvm::Triple &T = TC.getTriple();
    unsigned CEVersion = T.getOSVersion().getMajor();
    // CE 4.0 is the oldest release whose SDK layout this tool chain follows,
    // and 8.0 (Windows Embedded Compact 2013) is the newest one that was ever
    // shipped, so nothing outside that range can name a library credibly.
    if (CEVersion && (CEVersion < 4 || CEVersion > 8)) {
      TC.getDriver().Diag(diag::err_drv_unsupported_wince_version)
          << CEVersion;
      return;
    }
    StringRef ArchSuffix;
    switch (T.getArch()) {
    case llvm::Triple::arm:
    case llvm::Triple::thumb:
      break;
    case llvm::Triple::x86:
      ArchSuffix = "-x86";
      break;
    default:
      TC.getDriver().Diag(diag::err_drv_unsupported_wince_arch)
          << T.getArchName() << T.str();
      return;
    }
    // 6.0, 7.0 and 8.0 have no distinct import library of their own: coredll
    // kept its name through those releases, and a sysroot carries one CE 6 era
    // set of libraries for all three.  Only the CPU mix changes across them --
    // Compact 7 dropped SHx, and Compact 2013 dropped everything but ARMv7 and
    // x86 -- which is what the architecture switch above reports.
    CmdArgs.push_back(Args.MakeArgString(
        getCoreDLLLibrary(TC, ArchSuffix, CEVersion)));
  }

  // lld's COFF driver is named after the object format, so the "lld" flavour
  // spelled by -fuse-ld= has to become that name here, as in the UEFI
  // toolchain, and --ld-path= is taken as the program to run, as everywhere
  // else.
  std::string LinkerPath;
  if (!Args.hasArg(options::OPT_ld_path_EQ) &&
      Args.getLastArgValue(options::OPT_fuse_ld_EQ,
                           D.getPreferredLinker()) == "lld")
    LinkerPath = TC.GetProgramPath("lld-link");
  else
    LinkerPath = TC.GetLinkerPath();

  // Nothing but that driver takes the arguments collected above, so a -fuse-ld=
  // naming another flavour is refused here instead of being handed COFF
  // spellings it would read as file names.  GetLinkerPath() objects only to a
  // flavour it cannot resolve to a program at all, and its treatment is copied
  // exactly here: the same diagnostic, and the default linker takes over so
  // that the rest of the job list is still reported.  A --ld-path= is not
  // refused on its own, and not refused next to -fuse-ld=lld either, that pair
  // being how a copy of lld outside the search path is declared to the driver.
  if (const Arg *A = Args.getLastArg(options::OPT_fuse_ld_EQ))
    if (StringRef(A->getValue()) != "lld") {
      StringRef LinkerName = llvm::sys::path::filename(LinkerPath);
      if (LinkerName != "lld-link" && LinkerName != "lld-link.exe") {
        D.Diag(diag::err_drv_invalid_linker_name) << A->getAsString(Args);
        // This Tool's own name, which is what the default linker is.
        LinkerPath = TC.GetProgramPath("lld-link");
      }
    }

  C.addCommand(std::make_unique<Command>(
      JA, *this, ResponseFileSupport::AtFileUTF16(),
      Args.MakeArgString(LinkerPath), CmdArgs, Inputs, Output));
}
