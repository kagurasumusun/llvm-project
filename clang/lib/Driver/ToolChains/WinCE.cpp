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
#include "clang/Options/Options.h"
#include "llvm/Option/ArgList.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/Path.h"
#include "llvm/TargetParser/Host.h"

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

WinCE::WinCE(const Driver &D, const llvm::Triple &Triple, const ArgList &Args)
    : Generic_GCC(D, Triple, Args) {
  SysRootPath = findDefaultSysRoot(D, Args);
}

Tool *WinCE::getTool(Action::ActionClass AC) const {
  return Generic_GCC::getTool(AC);
}

Tool *WinCE::buildLinker() const {
  if (!LinkerTool)
    LinkerTool = std::make_unique<wince::Linker>(*this);
  return LinkerTool.get();
}

ToolChain::CXXStdlibType WinCE::GetCXXStdlibType(const ArgList &Args) const {
  return ToolChain::CST_Libcxx;
}

void WinCE::addClangTargetOptions(const ArgList &DriverArgs,
                                  ArgStringList &CC1Args,
                                  Action::OffloadKind DeviceOffloadKind) const {
  // Maximum MSVC source compatibility is a core requirement for this target.
  // Users may turn individual pieces off (e.g. -fno-delayed-template-parsing).
  if (DriverArgs.hasFlag(options::OPT_fms_extensions,
                         options::OPT_fno_ms_extensions, true))
    CC1Args.push_back("-fms-extensions");
  if (DriverArgs.hasFlag(options::OPT_fms_compatibility,
                         options::OPT_fno_ms_compatibility, true))
    CC1Args.push_back("-fms-compatibility");
  if (DriverArgs.hasFlag(options::OPT_fdelayed_template_parsing,
                         options::OPT_fno_delayed_template_parsing, true))
    CC1Args.push_back("-fdelayed-template-parsing");
  // MSVC 14.x (Visual Studio 2015, the last MSVC to ship WinCE-era
  // compatibility support) is the compatibility reference unless overridden
  // with -fms-compatibility-version=.
  if (!DriverArgs.hasArg(options::OPT_fms_compatibility_version))
    CC1Args.push_back("-fms-compatibility-version=1900");
  // The WinCE C library headers are GNU-era sources (mingw-runtime /
  // w32api from CeGCC); they probe the compiler with __GNUC__.  Pretend
  // to be the GCC major the CeGCC runtime was last built with, so the
  // header guards resolve to their GNU branches.
  if (!DriverArgs.hasArg(options::OPT_fgnuc_version_EQ))
    CC1Args.push_back("-fgnuc-version=14.2");

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
  CmdArgs.push_back("-lc++");
  CmdArgs.push_back("-lc++abi");
  CmdArgs.push_back("-lunwind");
}


namespace clang {
namespace driver {
namespace tools {
namespace wince {

namespace {
// Translate one GNU-ld style linker flag received via -Wl, into its
// lld-link equivalent.  Returns false when the flag is not recognized; the
// caller then forwards the raw value to lld-link so that unsupported flags
// fail visibly instead of silently.
bool translateGNUFlag(const ArgList &Args, ArrayRef<const char *> Vals,
                      unsigned &Idx, ArgStringList &CmdArgs, bool &HaveEntry,
                      bool &HaveBase, bool &HaveSubsystem, bool &WantConsole,
                      bool &HaveDynamicBase, bool &HaveDLL,
                      int &MajorImageVer, int &MinorImageVer) {
  StringRef V(Vals[Idx]);
  auto Next = [&]() -> StringRef {
    if (Idx + 1 < Vals.size())
      return StringRef(Vals[++Idx]);
    return StringRef();
  };
  if (V == "-subsystem") {
    StringRef S = Next();
    HaveSubsystem = true;
    if (S == "console")
      WantConsole = true;
    CmdArgs.push_back(Args.MakeArgString(Twine("/subsystem:") + S));
    return true;
  }
  if (V == "-e" || V == "-entry" || V == "--entry") {
    StringRef S = Next();
    HaveEntry = true;
    CmdArgs.push_back(Args.MakeArgString(Twine("/entry:") + S));
    return true;
  }
  if (V == "--image-base" || V == "-image-base") {
    StringRef S = Next();
    HaveBase = true;
    CmdArgs.push_back(Args.MakeArgString(Twine("/base:") + S));
    return true;
  }
  if (V == "--stack") {
    StringRef S = Next();
    CmdArgs.push_back(Args.MakeArgString(Twine("/stack:") + S));
    return true;
  }
  if (V == "--major-image-version") {
    MajorImageVer = std::stoi(std::string(Next()));
    return true;
  }
  if (V == "--minor-image-version") {
    MinorImageVer = std::stoi(std::string(Next()));
    return true;
  }
  if (V == "--dll" || V == "-dll") {
    HaveDLL = true;
    return true; // rendered once, below, with -shared/-mdll
  }
  if (V == "--def" || V == "-def") {
    StringRef S = Next();
    CmdArgs.push_back(Args.MakeArgString(Twine("/def:") + S));
    return true;
  }
  if (V == "--out-implib") {
    StringRef S = Next();
    CmdArgs.push_back(Args.MakeArgString(Twine("/implib:") + S));
    return true;
  }
  if (V == "--dynamicbase") {
    HaveDynamicBase = true;
    CmdArgs.push_back("/dynamicbase");
    return true;
  }
  return false;
}
} // namespace

// Render a GCC-style argument list as an lld-link (MS COFF) command line.
void Linker::ConstructJob(Compilation &C, const JobAction &JA,
                          const InputInfo &Output, const InputInfoList &Inputs,
                          const ArgList &Args, const char *LinkingOutput) const {
  const ToolChain &TC = getToolChain();
  ArgStringList CmdArgs;

  CmdArgs.push_back("lld-link");

  // --- Output file --------------------------------------------------------
  SmallString<128> OutFile(Output.getFilename());
  // clang-cl's /LD (and /LDd) create a DLL, like -shared/-mdll do.
  const bool WantDLL = Args.hasArg(options::OPT__SLASH_LD) ||
                       Args.hasArg(options::OPT__SLASH_LDd) ||
                       Args.hasArg(options::OPT_shared) ||
                       Args.hasArg(options::OPT_mdll);
  const bool IsDLL = WantDLL;
  if (!llvm::sys::path::has_extension(OutFile))
    llvm::sys::path::replace_extension(OutFile, IsDLL ? ".dll" : ".exe");
  CmdArgs.push_back(Args.MakeArgString(Twine("/out:") + OutFile));

  // Needed early: the -l translation below probes the sysroot lib dir to
  // decide between the GNU ("lib<name>.a") and MS ("<name>.lib") spellings.
  const auto &WinCETC = static_cast<const toolchains::WinCE &>(TC);
  SmallString<128> LibDir(WinCETC.getSysRootPath());
  llvm::sys::path::append(LibDir, "lib");

  // --- Translate GCC-style flags ------------------------------------------
  bool HaveSubsystem = false, HaveEntry = false, HaveBase = false;
  bool HaveDynamicBase = false, WantConsole = false, HaveDLL = false;
  int MajorImageVer = -1, MinorImageVer = -1;

  for (const Arg *A : Args) {
    const Option &Opt = A->getOption();
    if (Opt.getKind() == Option::InputClass)
      continue; // Rendered from Inputs below.
    A->claim();
    switch (Opt.getID()) {
    case options::OPT_o:
    case options::OPT_L:
    case options::OPT_shared:
    case options::OPT_mdll:
    case options::OPT_static:
    case options::OPT_nostdlib:
    case options::OPT_nostartfiles:
    case options::OPT_nodefaultlibs:
    case options::OPT_s:
      // Consumed here or handled through other channels; nothing to forward.
      break;
    case options::OPT_l: {
      // Our wince-crt sysroot names its import libraries/archives
      // "<name>.lib" (see wince-crt/CMakeLists.txt), which is what
      // lld-link looks for by default. If a user or a future sysroot
      // build (e.g. a straight, unmodified mingwrt/w32api `make install`
      // via utils/wince/build-wince-sysroot.sh) instead installs GNU-style
      // "lib<name>.a" dlltool archives, prefer that spelling when it is
      // actually present so both sysroot layouts resolve correctly.
      StringRef Name = A->getValue();
      SmallString<128> GNUName(LibDir);
      llvm::sys::path::append(GNUName, (Twine("lib") + Name + ".a").str());
      if (llvm::sys::fs::exists(GNUName))
        CmdArgs.push_back(Args.MakeArgString(Twine("lib") + Name + ".a"));
      else
        CmdArgs.push_back(Args.MakeArgString(Name + ".lib"));
      break;
    }
    case options::OPT_e:
      HaveEntry = true;
      CmdArgs.push_back(Args.MakeArgString(Twine("/entry:") + A->getValue()));
      break;
    case options::OPT_mwindows:
      HaveSubsystem = true;
      WantConsole = false;
      CmdArgs.push_back("/subsystem:windowsce");
      break;
    case options::OPT_mconsole:
      HaveSubsystem = true;
      WantConsole = true;
      CmdArgs.push_back("/subsystem:console");
      break;
    case options::OPT_Xlinker:
      CmdArgs.push_back(A->getValue());
      break;
    case options::OPT_Wl_COMMA: {
      // -Wl,<flag>...: translate the common GNU spellings, forward the rest.
      ArrayRef<const char *> Vals = A->getValues();
      for (unsigned I = 0; I < Vals.size(); ++I) {
        if (!translateGNUFlag(Args, Vals, I, CmdArgs, HaveEntry, HaveBase,
                              HaveSubsystem, WantConsole, HaveDynamicBase,
                              HaveDLL, MajorImageVer, MinorImageVer)) {
          // Unrecognized: forward raw.  lld-link reports a visible error for
          // anything it does not understand.
          CmdArgs.push_back(Vals[I]);
        }
      }
      break;
    }
    default: {
      // Forward lld-link style overrides (values starting with '/') given via
      // -Wl or -Xcompiler-like options; everything else stays at the driver.
      for (const char *Val : A->getValues()) {
        StringRef V(Val);
        if (!V.starts_with("/"))
          continue;
        if (V.starts_with_insensitive("/subsystem:")) {
          HaveSubsystem = true;
          if (V.starts_with_insensitive("/subsystem:console"))
            WantConsole = true;
        } else if (V.starts_with_insensitive("/entry:")) {
          HaveEntry = true;
        } else if (V.starts_with_insensitive("/base:")) {
          HaveBase = true;
        } else if (V.starts_with_insensitive("/dynamicbase")) {
          HaveDynamicBase = true;
        }
        CmdArgs.push_back(Val);
      }
      break;
    }
    }
  }

  if (MajorImageVer >= 0)
    CmdArgs.push_back(Args.MakeArgString(Twine("/version:") +
                                         Twine(MajorImageVer) + "." +
                                         Twine(MinorImageVer < 0 ? 0 : MinorImageVer)));

  // --- Windows CE image defaults ------------------------------------------
  if (IsDLL || HaveDLL)
    // -shared/-mdll/-Wl,--dll: produce a DLL image.
    CmdArgs.push_back("/dll");
  if (!HaveSubsystem)
    // CeGCC/binutils arm-wince emulation default:
    // IMAGE_SUBSYSTEM_WINDOWS_CE_GUI (9).
    CmdArgs.push_back("/subsystem:windowsce");
  if (!HaveEntry)
    // ld pe.em default entry table: subsystem 9 -> WinMainCRTStartup,
    // console subsystem -> mainCRTStartup.  Both CRT entries dispatch to
    // main() (or, through libwce's main->WinMain adapter, to WinMain), so
    // either default is safe for either user entry point.
    CmdArgs.push_back(WantConsole ? "/entry:mainCRTStartup"
                                  : "/entry:WinMainCRTStartup");
  if (!HaveBase)
    // arm-wince emulation defaults: EXE base 0x10000, DLL base 0x10000000.
    CmdArgs.push_back(IsDLL || HaveDLL ? "/base:0x10000000" : "/base:0x10000");
  if (!IsDLL && !HaveDLL && !HaveDynamicBase)
    // EXEs are always mapped at their fixed image base by the CE kernel, so
    // base relocations are unnecessary.  DLLs are relocated by the kernel at
    // load time and therefore keep their .reloc section (matching the
    // binutils arm-wince emulation behavior).
    CmdArgs.push_back("/fixed");

  // --- Runtime objects and default libraries ------------------------------
  // The sysroot layout mirrors a stock CeGCC install tree: mingwrt and
  // w32api are built (see utils/wince/build-wince-sysroot.sh) against this
  // very toolchain and staged as <sysroot>/{include,lib}, exactly as they
  // would be under real arm-mingw32ce binutils/gcc. This lets clang consume
  // an unmodified mingwrt/w32api checkout instead of a bespoke CRT.
  CmdArgs.push_back(Args.MakeArgString(Twine("/libpath:") + LibDir));

  const bool WantThreads = Args.hasArg(options::OPT_mthreads);

  if (!Args.hasArg(options::OPT_nostdlib) &&
      !Args.hasArg(options::OPT_nostartfiles)) {
    // wce.lib's crt0.obj/dllcrt0.obj are now compiled directly from
    // mingwrt's own crt3.c/dllcrt1.c (mingwrt's CRT0S picks exactly these
    // two sources for *-*-mingw32ce* hosts; see mingwrt/Makefile.in) plus
    // mingwrt/winmain_ce.c for the main()<->WinMain() adapter -- see
    // wince-crt/CMakeLists.txt. Must precede all user objects.
    SmallString<128> Crt(LibDir);
    llvm::sys::path::append(Crt, IsDLL || HaveDLL ? "dllcrt0.obj" : "crt0.obj");
    CmdArgs.push_back(Args.MakeArgString(Crt));
  }

  if (!Args.hasArg(options::OPT_nostdlib) &&
      !Args.hasArg(options::OPT_nodefaultlibs)) {
    // Ordering follows the CeGCC link line
    //   -lmingw32 -lgcc -lceoldname -lmingwex -lcoredll
    // with libgcc replaced by compiler-rt builtins. wce.lib now holds only
    // the pthread shim libc++'s threading backend needs (no pthreads API
    // exists on WinCE/mingwrt/w32api) plus the handful of mingwrt .c files
    // CeGCC's own libmingw32.a would have pulled in as objects rather than
    // an archive; the CRT startup objects and main/WinMain glue linked
    // above are mingwrt's own crt3.c/dllcrt1.c/winmain_ce.c, compiled
    // as-is -- see wince-crt/CMakeLists.txt.
    (void)WantThreads; // mingwrt's -mthreads variant (libmingwthrd) is not
                       // wired up in the sysroot build yet; see the roadmap.
    CmdArgs.push_back("wce.lib");
    CmdArgs.push_back("mingwex.lib");
    CmdArgs.push_back("clang_rt.builtins-arm.lib");
    CmdArgs.push_back("ceoldname.lib");
    CmdArgs.push_back("coredll.lib");
  }

  // --- Inputs (objects, archives, reserved C++ stdlib marker) --------------
  for (const auto &II : Inputs) {
    if (II.isNothing())
      continue;
    if (II.isFilename()) {
      CmdArgs.push_back(II.getFilename());
      continue;
    }
    const Arg &A = II.getInputArg();
    if (A.getOption().matches(options::OPT_Z_reserved_lib_stdcxx))
      TC.AddCXXStdlibLibArgs(Args, CmdArgs);
    else
      A.renderAsInput(Args, CmdArgs);
  }

  const char *Exec = Args.MakeArgString(
      TC.GetProgramPath("lld-link"));
  C.addCommand(std::make_unique<Command>(JA, *this,
                                         ResponseFileSupport::AtFileUTF8(),
                                         Exec, CmdArgs, Inputs, Output));
}

} // end namespace wince
} // end namespace tools
} // end namespace driver
} // end namespace clang
