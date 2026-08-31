//===--- WinCE.cpp - Windows CE ToolChain Implementations -------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

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

// Locate the default WinCE sysroot: a "wince-sysroot" directory next to the
// compiler installation.  Overridable with --sysroot=<dir>.
static std::string findDefaultSysRoot(const Driver &D, const ArgList &Args) {
  if (const Arg *A = Args.getLastArg(options::OPT__sysroot_EQ))
    return std::string(A->getValue());
  if (!D.SysRoot.empty() && D.SysRoot != "/")
    return std::string(D.SysRoot);

  SmallString<128> Path(D.Dir); // .../<prefix>/bin
  llvm::sys::path::remove_filename(Path); // .../<prefix>
  llvm::sys::path::append(Path, "wince-sysroot");
  return std::string(Path);
}

// Whether the current compilation is C++ (or Objective-C++).  Used to gate
// language-specific defaults such as -fgnu89-inline, which the front end
// rejects for C++.  The input language comes from -x when present, otherwise
// from the input file extensions.
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
    // lookupTypeForExtension wants "cxx", not ".cxx".  The dotted form
    // made CMake's testCXXCompiler.cxx look like C and inherited
    // -fgnu89-inline (CI 33353039440).
    if (types::isCXX(types::lookupTypeForExtension(Ext)))
      return true;
  }
  return false;
}

WinCE::WinCE(const Driver &D, const llvm::Triple &Triple, const ArgList &Args)
    : Generic_GCC(D, Triple, Args) {
  SysRootPath = findDefaultSysRoot(D, Args);
  if (!llvm::sys::fs::exists(SysRootPath)) {
    D.Diag(diag::warn_drv_wince_sysroot_missing)
        << SysRootPath
        << "utils/wince/build-wince-sysroot.sh";
  }
}

Tool *WinCE::buildLinker() const {
  // ToolChain::getLink() takes ownership via unique_ptr::reset.
  // Returning a second unique_ptr to the same object double-frees on
  // ~WinCE / ~ToolChain and SIGSEGVs the -### link RUN after jobs print
  // (piped stderr is fully buffered, so lit sees no output + -11).
  return new tools::wince::Linker(*this);
}

ToolChain::CXXStdlibType WinCE::GetCXXStdlibType(const ArgList &Args) const {
  return ToolChain::CST_Libcxx;
}

void WinCE::addClangTargetOptions(const ArgList &DriverArgs,
                                  ArgStringList &CC1Args,
                                  Action::OffloadKind DeviceOffloadKind) const {
  // WinCE wchar_t is 16-bit unsigned (CeGCC / UTF-16).  cc1 spelling;
  // the driver flag -fshort-wchar is rejected by -cc1 (CI 33349660794).
  CC1Args.push_back("-fwchar-type=short");
  CC1Args.push_back("-fno-signed-wchar");
  // w32api needs __declspec (-fms-extensions). Delayed template parsing is
  // the clang-cl dialect, not CeGCC (it made libc++ `using ::remove` collide).
  // -fms-compatibility stays on: libc++ `using ::isblank` must overload the
  // locale templates against this CRT; without it Stage 3 fails in __locale.
  const bool CLMode = getDriver().IsCLMode();
  if (DriverArgs.hasFlag(options::OPT_fms_extensions,
                         options::OPT_fno_ms_extensions, true))
    CC1Args.push_back("-fms-extensions");
  if (DriverArgs.hasFlag(options::OPT_fms_compatibility,
                         options::OPT_fno_ms_compatibility, true))
    CC1Args.push_back("-fms-compatibility");
  if (DriverArgs.hasFlag(options::OPT_fdelayed_template_parsing,
                         options::OPT_fno_delayed_template_parsing, CLMode))
    CC1Args.push_back("-fdelayed-template-parsing");
  if (const Arg *A =
          DriverArgs.getLastArg(options::OPT_fms_compatibility_version))
    CC1Args.push_back(DriverArgs.MakeArgString(
        Twine("-fms-compatibility-version=") + A->getValue()));
  else
    CC1Args.push_back("-fms-compatibility-version=1900");
  // The WinCE C library headers are GNU-era sources (mingw-runtime /
  // w32api from CeGCC); they probe the compiler with __GNUC__.  Pretend
  // to be the GCC major the CeGCC runtime was last built with, so the
  // header guards resolve to their GNU branches.
  if (!DriverArgs.hasArg(options::OPT_fgnuc_version_EQ))
    CC1Args.push_back("-fgnuc-version=14.2");
  // CeGCC CPP_SPEC: %{mthreads:-D_MT}.  -pthread is accepted as a synonym.
  if (DriverArgs.hasArg(options::OPT_mthreads) ||
      DriverArgs.hasFlag(options::OPT_pthread, options::OPT_no_pthread,
                         false))
    CC1Args.push_back("-D_MT");
  // CeGCC's GCC and the eMbedded Visual C++ headers use the pre-C99
  // "extern __inline" convention; default to GNU89 inline semantics so
  // headers and sources written for those compilers keep behaving
  // (extern inline stays an external definition, not a C99 inline
  // declaration).  Overridable with -fno-gnu89-inline.  This only applies to
  // C-family inputs: the front end rejects -fgnu89-inline for C++ (and C++
  // has no C99 inline model to restore), so skip it there.
  if (DriverArgs.hasFlag(options::OPT_fgnu89_inline,
                         options::OPT_fno_gnu89_inline, true) &&
      !compilingCXX(DriverArgs) && !getDriver().CCCIsCXX())
    CC1Args.push_back("-fgnu89-inline");
  // GCC < 10 (mingw32ce 4.6, the compiler TECLIB/glpi-wince-agent and
  // other CE apps were written for) merges tentative definitions as
  // COMMON. Clang defaults to -fno-common and reports duplicate symbols.
  if (DriverArgs.hasFlag(options::OPT_fcommon, options::OPT_fno_common, true) &&
      !compilingCXX(DriverArgs) && !getDriver().CCCIsCXX())
    CC1Args.push_back("-fcommon");

  // These MinGW-style -m flags are TargetSpecific.  Without marking them
  // in-range for this triple, the driver errors on -mconsole/-mwindows/-mdll
  // (CI 33345845097: CONSOLE RUN exit 1 after LINK50 passed).  Same loop as
  // MinGW::addClangTargetOptions.  CE images stay subsystem 9 regardless.
  for (auto Opt : {options::OPT_mthreads, options::OPT_mwindows,
                   options::OPT_mconsole, options::OPT_mdll}) {
    if (Arg *A = DriverArgs.getLastArgNoClaim(Opt))
      A->ignoreTargetSpecific();
  }

  // Unwind tables: getDefaultUnwindTableLevel() returns Asynchronous for
  // this target, which makes the driver pass -fasynchronous-unwind-tables
  // (ARM EHABI tables are mandatory; the Itanium-based unwinder has to
  // walk through C frames as well when propagating C++ exceptions).
}

void WinCE::AddClangSystemIncludeArgs(const ArgList &DriverArgs,
                                      ArgStringList &CC1Args) const {
  const Driver &D = getDriver();

  if (DriverArgs.hasArg(options::OPT_nostdinc))
    return;

  // Clang builtin headers (stddef.h, stdarg.h, float.h, intrin.h, ...).
  if (!DriverArgs.hasArg(options::OPT_nobuiltininc)) {
    SmallString<128> Dir(D.ResourceDir);
    llvm::sys::path::append(Dir, "include");
    addSystemInclude(DriverArgs, CC1Args, Dir);
  }

  if (DriverArgs.hasArg(options::OPT_nostdlibinc))
    return;

  // WinCE C library + Win32 platform headers.
  SmallString<128> Dir(SysRootPath);
  llvm::sys::path::append(Dir, "include");
  addSystemInclude(DriverArgs, CC1Args, Dir);
}

void WinCE::AddClangCXXStdlibIncludeArgs(const ArgList &DriverArgs,
                                         ArgStringList &CC1Args) const {
  if (DriverArgs.hasArg(options::OPT_nostdinc, options::OPT_nostdincxx,
                        options::OPT_nostdlibinc))
    return;

  // libc++ headers.
  SmallString<128> Dir(SysRootPath);
  llvm::sys::path::append(Dir, "include", "c++", "v1");
  addSystemInclude(DriverArgs, CC1Args, Dir);
}

void WinCE::AddCXXStdlibLibArgs(const ArgList &Args,
                                ArgStringList &CmdArgs) const {
  // GNU archive names the sysroot script actually installs (libc++.a).
  // Do not invent MS .lib spellings or alias them.
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

  // hasArg of these GNU flags is the same surface addClangTargetOptions
  // already uses.  Do not walk the full ArgList (filtered / getLastArg /
  // ClaimAllArgs) or call fs::exists here.
  CmdArgs.push_back("-wince");
  CmdArgs.push_back("-auto-import");
  CmdArgs.push_back("-runtime-pseudo-reloc");

  // Always emit the sysroot lib dir.  Do not probe it with fs::exists
  // (that was a -### crash vector).  Archives keep their GNU names.
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

  // User objects and their -l first, then the CRT archives. Putting
  // libmingw32.a (winmain_ce.o) before a program that already defines
  // WinMain pulls a second WinMain (TECLIB/glpi-wince-agent).
  for (const auto &II : Inputs) {
    if (II.isFilename())
      CmdArgs.push_back(II.getFilename());
  }
  // GNU -lfoo → libfoo.a (CeGCC LIB_SPEC). Dropping these left
  // CommandBar_* / GetNetworkParams undefined in glpi-wince-agent.
  for (const Arg *A : Args.filtered(options::OPT_l)) {
    A->claim();
    CmdArgs.push_back(Args.MakeArgString(Twine("lib") + A->getValue() + ".a"));
  }

  if (WantThreads) {
    CmdArgs.push_back("libmingwthrd.a");
    CmdArgs.push_back("libpthread.a");
  }
  CmdArgs.push_back("libmingw32.a");
  CmdArgs.push_back("libclang_rt.builtins-arm.a");
  CmdArgs.push_back("libceoldname.a");
  CmdArgs.push_back("libmingwex.a");
  // libposix.a / libpthread.a are third-party shims, not the CE CRT.
  // pthread is linked only with -mthreads / -pthread, above.
  if (WantProfiling)
    CmdArgs.push_back("libgmon.a");

  if (TC.getDriver().CCCIsCXX() || compilingCXX(Args))
    TC.AddCXXStdlibLibArgs(Args, CmdArgs);

  // The COREDLL import surface is version-specific: each generation
  // exported a different set (the def files are parsed from the CE shared
  // source; see the provenance notes in mingwrt/coredll*.def).  Select by
  // the triple's OS version; the bare spelling defaults to CE 6.0.
  llvm::VersionTuple OSVer = TC.getTriple().getOSVersion();
  switch (OSVer.getMajor()) {
  case 3:
    CmdArgs.push_back("libcoredll3.a");
    break;
  case 4:
    CmdArgs.push_back("libcoredll4.a");
    break;
  case 5:
    CmdArgs.push_back("libcoredll.a");
    break;
  default:
    CmdArgs.push_back("libcoredll6.a");
    break;
  }

  // Resolve lld-link next to clang (GetProgramPath). A bare "lld-link"
  // depends on PATH; posix_spawn then fails with ENOENT when a third-party
  // Makefile invokes arm-mingw32ce-gcc to link.
  const char *Exec = Args.MakeArgString(TC.GetProgramPath("lld-link"));
  C.addCommand(std::make_unique<Command>(JA, *this, ResponseFileSupport::None(),
                                         Exec, CmdArgs, Inputs, Output));
}

} // end namespace wince
} // end namespace tools
} // end namespace driver
} // end namespace clang
