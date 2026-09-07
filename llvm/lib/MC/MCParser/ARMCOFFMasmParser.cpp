#include "llvm/ADT/APFloat.h"
#include "llvm/ADT/StringSet.h"
#include "llvm/ADT/StringSwitch.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/BinaryFormat/COFF.h"
#include "llvm/MC/MCContext.h"
#include "llvm/MC/MCExpr.h"
#include "llvm/MC/MCParser/MCAsmParserExtension.h"
#include "llvm/MC/MCSectionCOFF.h"
#include "llvm/MC/MCDirectives.h"
#include "llvm/MC/MCStreamer.h"
#include "llvm/MC/MCSymbolCOFF.h"
#include "llvm/Support/SMLoc.h"
#include <cstdint>

using namespace llvm;

namespace {

class ARMCOFFMasmParser : public MCAsmParserExtension {
  template <bool (ARMCOFFMasmParser::*HandlerMethod)(StringRef, SMLoc)>
  void addDirectiveHandler(StringRef Directive) {
    MCAsmParser::ExtensionDirectiveHandler Handler =
        std::make_pair(this,
                       HandleDirective<ARMCOFFMasmParser, HandlerMethod>);
    getParser().addDirectiveHandler(Directive, Handler);
  }

  bool parseSectionSwitch(StringRef SectionName, unsigned Characteristics);
  bool parseSectionSwitch(StringRef SectionName, unsigned Characteristics,
                          StringRef COMDATSymName, COFF::COMDATType Type,
                          Align Alignment);

  bool parseDirectiveArea(StringRef, SMLoc);
  bool parseDirectiveProc(StringRef, SMLoc);
  bool parseDirectiveEndProc(StringRef, SMLoc);
  bool parseDirectiveEnd(StringRef, SMLoc);
  bool parseDirectiveExport(StringRef, SMLoc);
  bool parseDirectiveImport(StringRef, SMLoc);
  bool parseDirectiveExportAS(StringRef, SMLoc);
  bool parseDirectiveAlign(StringRef, SMLoc);
  bool parseDirectiveEntry(StringRef, SMLoc);
  bool parseDirectiveDataValue(StringRef, SMLoc, unsigned Size);
  bool parseDirectiveDCB(StringRef, SMLoc);
  bool parseDirectiveDCF(StringRef, SMLoc, bool IsDouble);
  bool parseDirectiveDCFS(StringRef, SMLoc);
  bool parseDirectiveDCFD(StringRef, SMLoc);
  bool parseDirectiveDCFSU(StringRef, SMLoc);
  bool parseDirectiveDCFDU(StringRef, SMLoc);
  bool parseDirectiveDCFU(StringRef, SMLoc);
  bool parseDirectiveSpace(StringRef, SMLoc);
  bool parseDirectiveFill(StringRef, SMLoc);
  bool parseDirectiveVarDecl(StringRef, SMLoc);
  bool parseDirectiveSet(StringRef, SMLoc);
  bool parseDirectiveNeedsMacroPass(StringRef, SMLoc);

  bool tokenIsInfixName() {
    AsmToken Name = getLexer().getTok();
    Lex();
    getLexer().UnLex(Name);
    return !getLexer().is(AsmToken::EndOfStatement) &&
           !getLexer().is(AsmToken::Comma);
  }

  bool emitInfixLabel(SMLoc Loc) {
    if (!getLexer().is(AsmToken::Identifier) || !tokenIsInfixName())
      return false;
    MCSymbol *Sym;
    if (getParser().parseSymbol(Sym))
      return Error(Loc, "expected identifier");
    getStreamer().emitLabel(Sym, Loc);
    return false;
  }

  bool IgnoreDirective(StringRef, SMLoc) {
    while (!getLexer().is(AsmToken::EndOfStatement))
      Lex();
    return false;
  }

  SmallVector<StringRef, 1> CurrentProcedures;

  StringSet<> ExportedNames;

  void Initialize(MCAsmParser &Parser) override {
    MCAsmParserExtension::Initialize(Parser);

    addDirectiveHandler<&ARMCOFFMasmParser::parseDirectiveArea>("area");
    addDirectiveHandler<&ARMCOFFMasmParser::parseDirectiveProc>("proc");
    addDirectiveHandler<&ARMCOFFMasmParser::parseDirectiveEndProc>("endp");
    addDirectiveHandler<&ARMCOFFMasmParser::parseDirectiveProc>("func");
    addDirectiveHandler<&ARMCOFFMasmParser::parseDirectiveEndProc>(
        "endfunc");
    addDirectiveHandler<&ARMCOFFMasmParser::parseDirectiveEnd>("end");
    addDirectiveHandler<&ARMCOFFMasmParser::parseDirectiveExport>("export");
    addDirectiveHandler<&ARMCOFFMasmParser::parseDirectiveExport>("global");
    addDirectiveHandler<&ARMCOFFMasmParser::parseDirectiveImport>("import");
    addDirectiveHandler<&ARMCOFFMasmParser::parseDirectiveImport>("extern");
    addDirectiveHandler<&ARMCOFFMasmParser::parseDirectiveExportAS>(
        "exportas");
    addDirectiveHandler<&ARMCOFFMasmParser::parseDirectiveAlign>("align");
    addDirectiveHandler<&ARMCOFFMasmParser::parseDirectiveEntry>("entry");

    for (const char *D : {"dcd", "dcdu"})
      getParser().addAliasForDirective(D, "dd");
    for (const char *D : {"dcw", "dcwu"})
      getParser().addAliasForDirective(D, "dw");
    for (const char *D : {"dcq", "dcqu"})
      getParser().addAliasForDirective(D, "dq");

    addDirectiveHandler<&ARMCOFFMasmParser::parseDirectiveDCB>("dcb");
    addDirectiveHandler<&ARMCOFFMasmParser::parseDirectiveDCB>("dcbu");
    addDirectiveHandler<&ARMCOFFMasmParser::parseDirectiveDCFS>("dcfs");
    addDirectiveHandler<&ARMCOFFMasmParser::parseDirectiveDCFD>("dcfd");
    addDirectiveHandler<&ARMCOFFMasmParser::parseDirectiveDCFU>("dcfu");
    addDirectiveHandler<&ARMCOFFMasmParser::parseDirectiveDCFSU>("dcfsu");
    addDirectiveHandler<&ARMCOFFMasmParser::parseDirectiveDCFDU>("dcfdu");
    addDirectiveHandler<&ARMCOFFMasmParser::parseDirectiveSpace>("space");
    addDirectiveHandler<&ARMCOFFMasmParser::parseDirectiveFill>("fill");
    for (const char *D : {"gbla", "gbll", "gbls", "lcla", "lcll", "lcls"})
      addDirectiveHandler<&ARMCOFFMasmParser::parseDirectiveVarDecl>(D);
    addDirectiveHandler<&ARMCOFFMasmParser::parseDirectiveSet>("seta");
    addDirectiveHandler<&ARMCOFFMasmParser::parseDirectiveSet>("setl");

    for (const char *D :
         {"while", "wend", "macro", "mend", "get", "include", "ltorg"})
      addDirectiveHandler<&ARMCOFFMasmParser::parseDirectiveNeedsMacroPass>(D);

    for (const char *D :
         {"preserve8", "require8", "require", "keep", "nocrossref", "nofp",
          "rout", "opt", "ttl", "subt", "code32", "code16", "arm", "thumb",
          "thumbx"})
      addDirectiveHandler<&ARMCOFFMasmParser::IgnoreDirective>(D);

    getLexer().setSemicolonComments(true);

    getLexer().setLexArmasmIntegers(true);
  }

public:
  ARMCOFFMasmParser() = default;
};

bool ARMCOFFMasmParser::parseDirectiveVarDecl(StringRef Directive, SMLoc Loc) {
  MCSymbol *Sym = nullptr;
  if (getLexer().is(AsmToken::Identifier)) {
    if (getParser().parseSymbol(Sym))
      return Error(Loc, "expected a name after " + Directive);
  } else
    return Error(Loc, "expected a name after " + Directive);

  while (getLexer().isNot(AsmToken::EndOfStatement))
    Lex();

  getStreamer().emitAssignment(Sym, MCConstantExpr::create(0, getContext()));
  Sym->setRedefinable(true);
  return false;
}

bool ARMCOFFMasmParser::parseDirectiveNeedsMacroPass(StringRef Directive,
                                                     SMLoc Loc) {
  while (getLexer().isNot(AsmToken::EndOfStatement))
    Lex();
  return Error(Loc, "'" + Directive + "' is armasm's macro processor, which "
                                    "llvm-mc does not implement; translate the "
                                    "file with "
                                    "cellvm-build:armasm/armasm-convert.py");
}
}

bool ARMCOFFMasmParser::parseSectionSwitch(StringRef SectionName,
                                           unsigned Characteristics) {
  return parseSectionSwitch(SectionName, Characteristics, "",
                            (COFF::COMDATType)0, Align(4));
}

bool ARMCOFFMasmParser::parseSectionSwitch(
    StringRef SectionName, unsigned Characteristics, StringRef COMDATSymName,
    COFF::COMDATType Type, Align Alignment) {
  if (getLexer().isNot(AsmToken::EndOfStatement))
    return TokError("unexpected token in section switching directive");
  Lex();

  MCSection *Section = getContext().getCOFFSection(
      SectionName, Characteristics, COMDATSymName, Type);
  Section->setAlignment(Alignment);
  getStreamer().switchSection(Section);
  return false;
}

bool ARMCOFFMasmParser::parseDirectiveArea(StringRef Directive, SMLoc Loc) {
  std::string NameStorage;
  StringRef Name;
  if (getLexer().is(AsmToken::Pipe)) {
    Lex();
    while (getLexer().isNot(AsmToken::Pipe)) {
      if (getLexer().is(AsmToken::EndOfStatement) || getLexer().is(AsmToken::Eof))
        return Error(Loc, "unterminated section name in AREA (missing '|')");
      NameStorage += getTok().getString();
      Lex();
    }
    Lex();
    Name = NameStorage;
  } else {
    if (getParser().parseIdentifier(Name))
      return Error(Loc, "expected section name after AREA");
    while (Name.ends_with("|"))
      Name = Name.drop_back();
  }
  Name = Name.trim();

  unsigned Characteristics = 0;
  Align Alignment = Align(4);
  bool IsCode = false, IsData = false, NoInit = false, ReadOnly = false;

  while (getLexer().isNot(AsmToken::EndOfStatement)) {
    if (!getTok().is(AsmToken::Identifier) &&
        !getTok().is(AsmToken::Equal)) {
      Lex();
      continue;
    }
    if (getTok().is(AsmToken::Equal)) {
      Lex();
      int64_t Val = 0;
      if (getParser().parseIntToken(Val,
                                    "expected integer after ALIGN="))
        return true;
      Alignment = Align(std::min<uint64_t>(
          UINT64_C(1) << std::min<int64_t>(Val, 12), UINT64_C(8192)));
      continue;
    }
    StringRef Kw = getTok().getIdentifier();
    Lex();
    if (Kw.equals_insensitive("CODE")) {
      IsCode = true;
    } else if (Kw.equals_insensitive("DATA")) {
      IsData = true;
    } else if (Kw.equals_insensitive("READONLY")) {
      ReadOnly = true;
    } else if (Kw.equals_insensitive("READWRITE")) {
      IsData = true;
    } else if (Kw.equals_insensitive("NOINIT") ||
               Kw.equals_insensitive("UNINIT")) {
      NoInit = true;
    }
  }

  if (IsCode)
    Characteristics = COFF::IMAGE_SCN_CNT_CODE | COFF::IMAGE_SCN_MEM_EXECUTE |
                      COFF::IMAGE_SCN_MEM_READ;
  else if (NoInit)
    Characteristics = COFF::IMAGE_SCN_CNT_UNINITIALIZED_DATA |
                      COFF::IMAGE_SCN_MEM_READ | COFF::IMAGE_SCN_MEM_WRITE;
  else {
    Characteristics = COFF::IMAGE_SCN_CNT_INITIALIZED_DATA |
                      COFF::IMAGE_SCN_MEM_READ;
    if (!ReadOnly)
      Characteristics |= COFF::IMAGE_SCN_MEM_WRITE;
  }

  std::string SectionName = Name.str();
  if (SectionName.empty() || SectionName[0] != '.') {
    if (IsCode)
      SectionName = ".text." + SectionName;
    else
      SectionName = ".data." + SectionName;
  }

  MCSection *Section = getContext().getCOFFSection(
      SectionName, Characteristics, "", (COFF::COMDATType)0);
  Section->setAlignment(Alignment);
  getStreamer().switchSection(Section);
  return false;
}

bool ARMCOFFMasmParser::parseDirectiveProc(StringRef Directive, SMLoc Loc) {
  MCSymbol *Sym = nullptr;
  if (getLexer().is(AsmToken::Identifier))
    if (getParser().parseSymbol(Sym))
      return Error(Loc, "expected identifier for procedure");
  if (!Sym)
    return Error(Loc, "expected identifier for procedure");

  while (getLexer().isNot(AsmToken::EndOfStatement))
    Lex();

  auto *COFFSym = static_cast<MCSymbolCOFF *>(Sym);
  if (!ExportedNames.count(Sym->getName()))
    COFFSym->setExternal(false);
  COFFSym->setType(COFF::IMAGE_SYM_DTYPE_FUNCTION
                   << COFF::SCT_COMPLEX_TYPE_SHIFT);
  getStreamer().emitLabel(Sym, Loc);
  CurrentProcedures.push_back(Sym->getName());
  return false;
}

bool ARMCOFFMasmParser::parseDirectiveEndProc(StringRef Directive,
                                              SMLoc Loc) {
  MCSymbol *Sym = nullptr;
  if (getLexer().is(AsmToken::Identifier)) {
    if (getParser().parseSymbol(Sym))
      return Error(Loc, "expected identifier after ENDP");
  }
  while (getLexer().isNot(AsmToken::EndOfStatement))
    Lex();

  if (Sym) {
    if (CurrentProcedures.empty() ||
        CurrentProcedures.back() != Sym->getName())
      return Warning(Loc, "ENDP name does not match the open PROC");
    CurrentProcedures.pop_back();
  } else if (!CurrentProcedures.empty()) {
    CurrentProcedures.pop_back();
  }
  return false;
}

bool ARMCOFFMasmParser::parseDirectiveEnd(StringRef, SMLoc) {
  while (getLexer().isNot(AsmToken::EndOfStatement))
    Lex();
  return false;
}

bool ARMCOFFMasmParser::parseDirectiveExport(StringRef Directive, SMLoc Loc) {
  MCSymbol *Sym;
  if (getParser().parseSymbol(Sym))
    return Error(Loc, "expected identifier");
  auto *COFFSym = static_cast<MCSymbolCOFF *>(Sym);
  COFFSym->setExternal(true);
  ExportedNames.insert(Sym->getName());
  while (getLexer().isNot(AsmToken::EndOfStatement))
    Lex();
  return false;
}

bool ARMCOFFMasmParser::parseDirectiveImport(StringRef Directive, SMLoc Loc) {
  MCSymbol *Sym;
  if (getParser().parseSymbol(Sym))
    return Error(Loc, "expected identifier");
  auto *COFFSym = static_cast<MCSymbolCOFF *>(Sym);
  COFFSym->setExternal(true);
  getStreamer().emitSymbolAttribute(Sym, MCSA_Global);
  getStreamer().beginCOFFSymbolDef(Sym);
  getStreamer().emitCOFFSymbolStorageClass(COFF::IMAGE_SYM_CLASS_EXTERNAL);
  getStreamer().emitCOFFSymbolType(0);
  getStreamer().endCOFFSymbolDef();
  while (getLexer().isNot(AsmToken::EndOfStatement))
    Lex();
  return false;
}

bool ARMCOFFMasmParser::parseDirectiveExportAS(StringRef Directive,
                                               SMLoc Loc) {
  MCSymbol *Sym;
  if (getParser().parseSymbol(Sym))
    return Error(Loc, "expected identifier");
  if (getParser().parseComma())
    return true;
  MCSymbol *Target;
  if (getParser().parseSymbol(Target))
    return Error(Loc, "expected target identifier");
  getStreamer().emitLabel(Target, Loc);
  return false;
}

bool ARMCOFFMasmParser::parseDirectiveAlign(StringRef Directive, SMLoc Loc) {
  int64_t Val = 4;
  if (getLexer().is(AsmToken::Integer)) {
    if (getParser().parseIntToken(Val, "expected alignment value"))
      return true;
    if (Val > 0 && Val <= 3)
      Val = UINT64_C(1) << Val;
  } else if (getLexer().is(AsmToken::Equal)) {
    Lex();
    if (getParser().parseIntToken(Val, "expected integer after ALIGN="))
      return true;
    Val = UINT64_C(1) << std::min<int64_t>(Val, 12);
  }
  getStreamer().emitValueToAlignment(Align(std::min<uint64_t>(Val, 8192)));
  return false;
}

bool ARMCOFFMasmParser::parseDirectiveEntry(StringRef, SMLoc) {
  while (getLexer().isNot(AsmToken::EndOfStatement))
    Lex();
  return false;
}

bool ARMCOFFMasmParser::parseDirectiveDataValue(StringRef Directive, SMLoc Loc,
                                                unsigned Size) {
  if (emitInfixLabel(Loc))
    return true;
  if (getLexer().is(AsmToken::EndOfStatement))
    return Error(Loc, "expected expression after " + Directive);

  auto parseOp = [&]() -> bool {
    if (getParser().checkForValidSection())
      return true;
    SMLoc ExprLoc = getLexer().getLoc();
    if (Size == 1 && getLexer().is(AsmToken::String)) {
      StringRef Str = getTok().getStringContents();
      Lex();
      for (char C : Str)
        getStreamer().emitIntValue((unsigned char)C, 1);
      return false;
    }
    const MCExpr *Value;
    if (getParser().parseExpression(Value))
      return true;
    getStreamer().emitValue(Value, Size, ExprLoc);
    return false;
  };

  return parseMany(parseOp);
}

bool ARMCOFFMasmParser::parseDirectiveDCB(StringRef Directive, SMLoc Loc) {
  return parseDirectiveDataValue(Directive, Loc, 1);
}

bool ARMCOFFMasmParser::parseDirectiveDCF(StringRef Directive, SMLoc Loc,
                                          bool IsDouble) {
  if (emitInfixLabel(Loc))
    return true;
  if (getLexer().is(AsmToken::EndOfStatement))
    return Error(Loc, "expected expression after " + Directive);

  auto parseOp = [&]() -> bool {
    if (getParser().checkForValidSection())
      return true;
    SMLoc ExprLoc = getLexer().getLoc();
    if (!IsDouble && getLexer().is(AsmToken::Real)) {
      APFloat Val(APFloat::IEEEdouble(), getTok().getString());
      bool LosesInfo = false;
      (void)Val.convert(APFloat::IEEEsingle(), APFloat::rmNearestTiesToEven,
                        &LosesInfo);
      Lex();
      getStreamer().emitIntValue(Val.bitcastToAPInt().getZExtValue(), 4);
      return false;
    }
    const MCExpr *Value;
    if (getParser().parseExpression(Value))
      return true;
    getStreamer().emitValue(Value, IsDouble ? 8 : 4, ExprLoc);
    return false;
  };

  return parseMany(parseOp);
}

bool ARMCOFFMasmParser::parseDirectiveDCFS(StringRef Directive, SMLoc Loc) {
  return parseDirectiveDCF(Directive, Loc, false);
}

bool ARMCOFFMasmParser::parseDirectiveDCFD(StringRef Directive, SMLoc Loc) {
  return parseDirectiveDCF(Directive, Loc, true);
}

bool ARMCOFFMasmParser::parseDirectiveDCFSU(StringRef Directive, SMLoc Loc) {
  return parseDirectiveDCF(Directive, Loc, false);
}

bool ARMCOFFMasmParser::parseDirectiveDCFDU(StringRef Directive, SMLoc Loc) {
  return parseDirectiveDCF(Directive, Loc, true);
}

bool ARMCOFFMasmParser::parseDirectiveDCFU(StringRef Directive, SMLoc Loc) {
  return parseDirectiveDCF(Directive, Loc, false);
}

bool ARMCOFFMasmParser::parseDirectiveSpace(StringRef, SMLoc Loc) {
  if (emitInfixLabel(Loc))
    return true;
  int64_t NumBytes;
  if (getParser().parseAbsoluteExpression(NumBytes))
    return true;
  while (getLexer().isNot(AsmToken::EndOfStatement))
    Lex();
  if (NumBytes < 0)
    return Error(Loc, "invalid number of bytes");
  if (getParser().checkForValidSection())
    return true;
  getStreamer().emitZeros((uint64_t)NumBytes);
  return false;
}

bool ARMCOFFMasmParser::parseDirectiveFill(StringRef, SMLoc Loc) {
  if (emitInfixLabel(Loc))
    return true;
  int64_t NumBytes;
  if (getParser().parseAbsoluteExpression(NumBytes))
    return true;
  int64_t FillValue = 0;
  if (getLexer().is(AsmToken::Comma)) {
    Lex();
    if (getParser().parseAbsoluteExpression(FillValue))
      return true;
  }
  while (getLexer().isNot(AsmToken::EndOfStatement))
    Lex();
  if (NumBytes < 0)
    return Error(Loc, "invalid number of bytes");
  if (getParser().checkForValidSection())
    return true;
  getStreamer().emitFill((uint64_t)NumBytes, (uint8_t)FillValue);
  return false;
}

bool ARMCOFFMasmParser::parseDirectiveSet(StringRef Directive, SMLoc Loc) {
  if (!getLexer().is(AsmToken::Identifier))
    return Error(Loc, "expected a name in front of " + Directive +
                          ": 'NAME " + Directive + " value'");
  MCSymbol *Sym;
  if (getParser().parseSymbol(Sym))
    return Error(Loc, "expected a name in front of " + Directive);
  const MCExpr *Value;
  if (getParser().parseExpression(Value))
    return true;
  while (getLexer().isNot(AsmToken::EndOfStatement))
    Lex();
  getStreamer().emitAssignment(Sym, Value);
  Sym->setRedefinable(true);
  return false;
}

namespace llvm {
MCAsmParserExtension *createARMCOFFMasmParser() {
  return new ARMCOFFMasmParser;
}
}
