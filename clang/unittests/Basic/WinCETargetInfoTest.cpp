#include "clang/Basic/Diagnostic.h"
#include "clang/Basic/DiagnosticOptions.h"
#include "clang/Basic/LangOptions.h"
#include "clang/Basic/MacroBuilder.h"
#include "clang/Basic/TargetInfo.h"
#include "clang/Basic/TargetOptions.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/Support/raw_ostream.h"
#include "gtest/gtest.h"
#include <map>
#include <memory>
#include <string>
#include <vector>

using namespace clang;

namespace {

using Definitions = std::map<std::string, std::vector<std::string>>;

Definitions getWinCEDefinitions(llvm::StringRef Triple, bool MSVCCompat,
                               bool GNUMode, llvm::StringRef CPU = "") {
  DiagnosticOptions DiagOpts;
  DiagnosticsEngine Diags(DiagnosticIDs::create(), DiagOpts,
                          new IgnoringDiagConsumer());
  TargetOptions TargetOpts;
  TargetOpts.Triple = Triple.str();
  TargetOpts.CPU = CPU.str();
  std::unique_ptr<TargetInfo> Target(
      TargetInfo::CreateTargetInfo(Diags, TargetOpts));
  EXPECT_TRUE(Target);
  EXPECT_FALSE(Diags.hasErrorOccurred());
  if (!Target)
    return {};

  LangOptions LangOpts;
  LangOpts.MSVCCompat = MSVCCompat;
  LangOpts.MicrosoftExt = MSVCCompat;
  LangOpts.GNUMode = GNUMode;
  std::string Text;
  llvm::raw_string_ostream OS(Text);
  MacroBuilder Builder(OS);
  Target->getTargetDefines(LangOpts, Builder);

  Definitions Result;
  llvm::SmallVector<llvm::StringRef, 128> Lines;
  llvm::StringRef(Text).split(Lines, '\n');
  for (llvm::StringRef Line : Lines) {
    if (!Line.consume_front("#define "))
      continue;
    auto [Name, Value] = Line.split(' ');
    Result[Name.str()].push_back(Value.str());
  }
  return Result;
}

TEST(WinCETargetInfoTest, SinglePlatformDefinition) {
  for (const char *Triple : {"arm-unknown-wince", "thumb-unknown-wince",
                             "i386-unknown-wince", "arm-unknown-wince-gnu",
                             "i386-unknown-wince-gnu"}) {
    for (bool MSVC : {false, true}) {
      SCOPED_TRACE(Triple);
      SCOPED_TRACE(MSVC);
      Definitions Macros = getWinCEDefinitions(Triple, MSVC, true);
      for (const char *Name : {"_WIN32", "_WIN32_WCE", "UNDER_CE", "WINCE",
                               "__WINCE__", "__MINGW32CE__", "__MINGW32__",
                               "__CEGCC_VERSION__", "__COREDLL__", "WIN32",
                               "WINNT", "_UNICODE", "UNICODE"}) {
        SCOPED_TRACE(Name);
        EXPECT_EQ(1u, Macros[Name].size());
      }
      EXPECT_EQ(0u, Macros.count("__MSVCRT__"));
      if (llvm::StringRef(Triple).starts_with("i386")) {
        EXPECT_EQ(std::vector<std::string>{"__attribute__((__cdecl__))"},
                  Macros["__stdcall"]);
        EXPECT_EQ(0u, Macros.count("_M_ARM"));
      } else {
        // A bare "arm" or "thumb" triple says nothing about the architecture,
        // so this is the architecture default of LLVM's ARM target, and _M_ARM
        // states that rather than a CPU the OS would have picked.
        EXPECT_EQ(std::vector<std::string>{"4"}, Macros["_M_ARM"]);
        EXPECT_EQ(0u, Macros.count("_M_ARM_NT"));
        EXPECT_EQ(0u, Macros.count("_M_IX86_FP"));
      }
    }
  }
}

// Which architecture the triple names is what decides the predefined version,
// with no help from the OS, and -mcpu= overrides the triple as it does for
// every other ARM target.
TEST(WinCETargetInfoTest, CPUAndVersionSelection) {
  EXPECT_EQ(std::vector<std::string>{"4"},
            getWinCEDefinitions("armv4t-unknown-wince", true, true)["_M_ARM"]);
  EXPECT_EQ(std::vector<std::string>{"7"},
            getWinCEDefinitions("thumbv7-unknown-wince", true, true)["_M_ARM"]);
  EXPECT_EQ(std::vector<std::string>{"5"},
            getWinCEDefinitions("armv5tej-unknown-wince", true, true)
                ["_M_ARM"]);
  EXPECT_EQ(std::vector<std::string>{"7"},
            getWinCEDefinitions("arm-unknown-wince", true, true,
                                "cortex-a8")["_M_ARM"]);
  EXPECT_EQ(std::vector<std::string>{"1312"},
            getWinCEDefinitions("arm-unknown-windowsce5.2", true, true)
                ["_WIN32_WCE"]);
}

}
