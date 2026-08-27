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
  if (!llvm::sys::fs::exists(SysRootPath)) {
    D.Diag(diag::warn_drv_wince_sysroot_missing)
        << SysRootPath
        << "utils/wince/build-wince-sysroot.sh";
  }
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
  // CeGCC CPP_SPEC: %{mthreads:-D_MT}.  -pthread is accepted as a synonym.
  if (DriverArgs.hasArg(options::OPT_mthreads) ||
      DriverArgs.hasFlag(options::OPT_pthread, options::OPT_no_pthread,
                         false))
    CC1Args.push_back("-D_MT");
  // CeGCC's GCC and the eMbedded Visual C++ headers use the pre-C99
  // "extern __inline" convention; default to GNU89 inline semantics so
  // headers and sources written for those compilers keep behaving
  // (extern inline stays an external definition, not a C99 inline
  // declaration).  Overridable with -fno-gnu89-inline.
  if (DriverArgs.hasFlag(options::OPT_fgnu89_inline,
                         options::OPT_fno_gnu89_inline, true))
    CC1Args.push_back("-fgnu89-inline");

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
  // The sysroot holds GNU-named archives (libc++.a, libc++abi.a,
  // libunwind.a) as installed by utils/wince/build-wince-sysroot.sh;
  // probe for them and fall back to the MS-style spellings.
  static const char *CXXLibs[] = {"c++", "c++abi", "unwind"};
  SmallString<128> LibDir(SysRootPath);
  llvm::sys::path::append(LibDir, "lib");
  for (StringRef L : CXXLibs) {
    SmallString<128> GNUName(LibDir);
    llvm::sys::path::append(GNUName, (Twine("lib") + L + ".a").str());
    if (llvm::sys::fs::exists(GNUName))
      CmdArgs.push_back(Args.MakeArgString(Twine("lib") + L + ".a"));
    else
      CmdArgs.push_back(Args.MakeArgString(L + ".lib"));
  }
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
                      bool &HaveBase, bool &HaveSubsystem,
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
    // CE: every image is subsystem 9, console or not (see OPT_mconsole).
    CmdArgs.push_back(Args.MakeArgString(
        Twine("/subsystem:") + (S == "console" ? StringRef("windowsce")
                                               : S)));
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

// Resolve a -l<name> style library to the sysroot's actual spelling.  The
// sysroot assembled by utils/wince/build-wince-sysroot.sh contains
// GNU-style "lib<name>.a" archives straight out of the unmodified
// mingwrt/w32api builds (and libclang_rt.builtins-*.a / libc++.a from the
// LLVM runtime stages); MS-style "<name>.lib" files are honored as a
// fallback so both sysroot layouts resolve.
static void addWinCELibrary(const ArgList &Args, ArgStringList &CmdArgs,
                            StringRef LibDir, StringRef Name) {
  SmallString<128> GNUName(LibDir);
  llvm::sys::path::append(GNUName, (Twine("lib") + Name + ".a").str());
  if (llvm::sys::fs::exists(GNUName))
    CmdArgs.push_back(Args.MakeArgString(Twine("lib") + Name + ".a"));
  else
    CmdArgs.push_back(Args.MakeArgString(Name + ".lib"));
}

// The -lgcc replacement: LLVM compiler-rt builtins, GNU-named in the
// sysroot stage-3 layout, MS-named from a compiler-rt MSVC-style build.
static void addCompilerRTBuiltins(const ToolChain &TC, const ArgList &Args,
                                  ArgStringList &CmdArgs, StringRef LibDir) {
  StringRef Arch;
  switch (TC.getArch()) {
  case llvm::Triple::arm:
  case llvm::Triple::thumb:
    Arch = "arm";
    break;
  case llvm::Triple::x86:
    Arch = "i386";
    break;
  default:
    Arch = TC.getArchName();
    break;
  }
  SmallString<128> GNUName(LibDir);
  llvm::sys::path::append(
      GNUName, (Twine("libclang_rt.builtins-") + Arch + ".a").str());
  if (llvm::sys::fs::exists(GNUName)) {
    CmdArgs.push_back(Args.MakeArgString(
        Twine("libclang_rt.builtins-") + Arch + ".a"));
    return;
  }
  SmallString<128> MSName(LibDir);
  llvm::sys::path::append(
      MSName, (Twine("clang_rt.builtins-") + Arch + ".lib").str());
  if (llvm::sys::fs::exists(MSName))
    CmdArgs.push_back(
        Args.MakeArgString(Twine("clang_rt.builtins-") + Arch + ".lib"));
  else // Let lld-link report the missing archive visibly.
    CmdArgs.push_back(Args.MakeArgString(
        Twine("clang_rt.builtins-") + Arch + ".lib"));
}

// Render a GCC-style argument list as an lld-link (MS COFF) command line.
void Linker::ConstructJob(Compilation &C, const JobAction &JA,
                          const InputInfo &Output, const InputInfoList &Inputs,
                          const ArgList &Args, const char *LinkingOutput) const {
  const ToolChain &TC = getToolChain();
  ArgStringList CmdArgs;

  CmdArgs.push_back("lld-link");
  // Windows CE image mode: bracket .ctors/.dtors into __CTOR_LIST__ /
  // __DTOR_LIST__ (mingwrt's __main walks them for global C++
  // constructors/destructors) and otherwise treat the image as CeGCC's
  // arm-wince emulation did.
  CmdArgs.push_back("-wince");
  // CeGCC's binutils default: --enable-auto-import + runtime pseudo relocs.
  // mingwrt's _pei386_runtime_relocator (in libmingw32) processes the
  // __RUNTIME_PSEUDO_RELOC_LIST__ lld-link synthesizes.
  CmdArgs.push_back("-auto-import");
  CmdArgs.push_back("-runtime-pseudo-reloc");

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
  bool HaveDynamicBase = false, HaveDLL = false;
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
      // -l<name>: resolve against the sysroot lib dir (see addWinCELibrary).
      addWinCELibrary(Args, CmdArgs, LibDir, A->getValue());
      break;
    }
    case options::OPT_e:
      HaveEntry = true;
      CmdArgs.push_back(Args.MakeArgString(Twine("/entry:") + A->getValue()));
      break;
    case options::OPT_mwindows:
    case options::OPT_mconsole:
      // Windows CE images are always IMAGE_SUBSYSTEM_WINDOWS_CE_GUI (9) -
      // CeGCC's arm-wince emulation forces 9 for both -mconsole and
      // -mwindows; "console apps" are just programs with main(), bridged
      // by mingwrt's winmain_ce.o WinMain adapter.
      HaveSubsystem = true;
      CmdArgs.push_back("/subsystem:windowsce");
      break;
    case options::OPT_Xlinker:
      CmdArgs.push_back(A->getValue());
      break;
    case options::OPT_Wl_COMMA: {
      // -Wl,<flag>...: translate the common GNU spellings, forward the rest.
      ArrayRef<const char *> Vals = A->getValues();
      for (unsigned I = 0; I < Vals.size(); ++I) {
        if (!translateGNUFlag(Args, Vals, I, CmdArgs, HaveEntry, HaveBase,
                              HaveSubsystem, HaveDynamicBase,
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
  if (!HaveEntry) {
    if (IsDLL || HaveDLL)
      // CeGCC LINK_SPEC: %{shared|mdll: -e DllMainCRTStartup}
      CmdArgs.push_back("/entry:DllMainCRTStartup");
    else
      // mingwrt's crt3.o defines exactly one EXE entry,
      // WinMainCRTStartup(hInst, hPrev, lpCmdLine, nShow) - the CE loader
      // contract.  Programs with main() get the WinMain adapter from
      // winmain_ce.o in libmingw32.a; mainCRTStartup does not exist.
      CmdArgs.push_back("/entry:WinMainCRTStartup");
  }
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
  // The sysroot is a stock CeGCC install tree: the unmodified
  // kagurasumusun/mingwrt and kagurasumusun/w32api trees are built by
  // utils/wince/build-wince-sysroot.sh with this very toolchain and staged
  // as <sysroot>/{include,lib}, exactly as they would be under real
  // arm-mingw32ce binutils/gcc.
  CmdArgs.push_back(Args.MakeArgString(Twine("/libpath:") + LibDir));

  const bool WantThreads =
      Args.hasArg(options::OPT_mthreads) ||
      Args.hasFlag(options::OPT_pthread, options::OPT_no_pthread, false);
  // CeGCC LIB_SPEC/STARTFILE_SPEC: %{pg:-lgmon} + %{pg:gcrt3%O%s}.  gcrt3.o
  // is mingwrt's crt3.c wrapped with the profiler lifecycle; libgmon.a is
  // the user-mode sampling profiler (wince-sysroot/gmon).
  const bool WantProfiling = Args.hasArg(options::OPT_pg);

  if (!Args.hasArg(options::OPT_nostdlib) &&
      !Args.hasArg(options::OPT_nostartfiles)) {
    // CeGCC STARTFILE_SPEC:
    //   %{shared|mdll:dllcrt3%O%s} %{!shared:%{!mdll:crt3%O%s}} %{pg:gcrt3%O%s}
    // Both are built by mingwrt itself (CRT0S for *-mingw32ce hosts); the
    // main()<->WinMain() glue is mingwrt's own winmain_ce.o inside
    // libmingw32.a. Must precede all user objects.
    const char *StartFile =
        IsDLL || HaveDLL ? "dllcrt3.o" : (WantProfiling ? "gcrt3.o" : "crt3.o");
    SmallString<128> Crt(LibDir);
    llvm::sys::path::append(Crt, StartFile);
    CmdArgs.push_back(Args.MakeArgString(Crt));
  }

  if (!Args.hasArg(options::OPT_nostdlib) &&
      !Args.hasArg(options::OPT_nodefaultlibs)) {
    // Ordering follows the CeGCC LIBGCC_SPEC + LIB_SPEC link line
    //   %{mthreads:-lmingwthrd} -lmingw32 -lgcc -lceoldname -lmingwex -lcoredll
    // with libgcc replaced by compiler-rt builtins.  -mthreads/-pthread
    // additionally pulls the pthreads4w static library (libpthread.a,
    // built by the sysroot stage from wince-sysroot/pthread-win32), which
    // also serves as libc++'s threading API on this target.
    if (WantThreads)
      addWinCELibrary(Args, CmdArgs, LibDir, "mingwthrd");
    if (WantThreads)
      addWinCELibrary(Args, CmdArgs, LibDir, "pthread");
    addWinCELibrary(Args, CmdArgs, LibDir, "mingw32");
    addCompilerRTBuiltins(TC, Args, CmdArgs, LibDir);
    addWinCELibrary(Args, CmdArgs, LibDir, "ceoldname");
    addWinCELibrary(Args, CmdArgs, LibDir, "mingwex");
    if (WantProfiling)
      addWinCELibrary(Args, CmdArgs, LibDir, "gmon");
    // COREDLL import library by OS generation: coredll6.a carries the
    // CE 6.0-only exports (CeGetThreadPriority, FindFirstDevice, ...),
    // coredll.a the CE 5.0 surface.  The deployment default is CE 6.0;
    // a versioned triple (arm-pc-wince5.0) selects the CE 5.0 surface.
    llvm::VersionTuple OSVer = TC.getTriple().getOSVersion();
    StringRef CoreDll = "coredll6";
    if (OSVer.getMajor() && OSVer.getMajor() < 6)
      CoreDll = "coredll";
    addWinCELibrary(Args, CmdArgs, LibDir, CoreDll);
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
