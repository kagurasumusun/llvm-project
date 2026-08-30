//===- ARMCOFFMasmParser.cpp - COFF armasm (MASM-style) Parser for ARM ----===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM
// Exceptions.  See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// ARM dialect of the MASM-family parser: the Microsoft `armasm` syntax used
// by Windows CE Platform Builder sources (AREA/PROC/ENDP/EXPORT/IMPORT,
// DCD/DCB/DCW/DCQ/DCFS/DCFD data, SPACE/FILL, EQU, the &hex / %binary /
// n_xxxx integer literals, ';' comments, and labels written in front of a
// directive instead of with a trailing ':').  This is an MCAsmParserExtension
// registered on top of the ARM AsmParser, mirroring COFFMasmParser for x86;
// the instruction mnemonics themselves are parsed by the regular
// ARMAsmParser.
//
// Still missing for full armasm: the unaligned DCFU/DCFSU/DCFDU/DCQU
// variants, GBLA/GBLL/GBLS variables with SETA/SETL/SETS, MACRO/MEND and
// IF/ELSE/ENDIF.  See utils/wince/README.md.
//
//===----------------------------------------------------------------------===//

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
  bool parseDirectiveDCD(StringRef, SMLoc);
  bool parseDirectiveDCW(StringRef, SMLoc);
  bool parseDirectiveDCB(StringRef, SMLoc);
  bool parseDirectiveDCQ(StringRef, SMLoc);
  bool parseDirectiveDCF(StringRef, SMLoc, bool IsDouble);
  bool parseDirectiveDCFS(StringRef, SMLoc);
  bool parseDirectiveDCFD(StringRef, SMLoc);
  bool parseDirectiveSpace(StringRef, SMLoc);
  bool parseDirectiveFill(StringRef, SMLoc);
  bool parseDirectiveEqu(StringRef, SMLoc);
  /// armasm's "name PROC" / "name DCD 1" define `name` right there, but the
  /// label in front of ENDP/ENDFUNC and EQU names an entity defined
  /// elsewhere, so it must not be emitted a second time.
  bool emitMasmLabel(MCSymbol *Sym, SMLoc Loc, StringRef Directive) override;

  bool IgnoreDirective(StringRef, SMLoc) {
    while (!getLexer().is(AsmToken::EndOfStatement))
      Lex();
    return false;
  }

  /// The most recent PROC symbol, for ENDP's implicit .size framing.
  SmallVector<StringRef, 1> CurrentProcedures;

  /// Names declared with EXPORT/GLOBAL.  armasm sources export a procedure
  /// with "EXPORT name" followed by "name PROC", so PROC must not turn the
  /// symbol back into a local one.
  StringSet<> ExportedNames;

  void Initialize(MCAsmParser &Parser) override {
    // Call the base implementation.
    MCAsmParserExtension::Initialize(Parser);

    // armasm structural directives (case-insensitive, as armasm accepts
    // any case).
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

    // armasm data definition and reservation.
    addDirectiveHandler<&ARMCOFFMasmParser::parseDirectiveDCD>("dcd");
    addDirectiveHandler<&ARMCOFFMasmParser::parseDirectiveDCW>("dcw");
    addDirectiveHandler<&ARMCOFFMasmParser::parseDirectiveDCB>("dcb");
    addDirectiveHandler<&ARMCOFFMasmParser::parseDirectiveDCQ>("dcq");
    addDirectiveHandler<&ARMCOFFMasmParser::parseDirectiveDCFS>("dcfs");
    addDirectiveHandler<&ARMCOFFMasmParser::parseDirectiveDCFD>("dcfd");
    addDirectiveHandler<&ARMCOFFMasmParser::parseDirectiveSpace>("space");
    addDirectiveHandler<&ARMCOFFMasmParser::parseDirectiveFill>("fill");
    addDirectiveHandler<&ARMCOFFMasmParser::parseDirectiveEqu>("equ");

    // armasm directives with no object-file effect.
    for (const char *D :
         {"preserve8", "require8", "require", "keep", "nocrossref", "nofp",
          "rout", "opt", "ttl", "subt", "code32", "code16", "arm", "thumb",
          "thumbx"})
      addDirectiveHandler<&ARMCOFFMasmParser::IgnoreDirective>(D);

    // Statements in this dialect are "name PROC" / "name DCD 1": a dotless
    // identifier in front of one of the directives above is a label, not a
    // mnemonic.  Registering for that syntax is what makes it reachable --
    // it stays off for every other dialect.
    getParser().setMasmLabelExtension(this);

    // armasm comments with ';'.  The target's own comment string is '@',
    // which is what GNU-syntax ARM assembly uses, so this cannot be folded
    // into MCAsmInfo::getCommentString().
    getLexer().setSemicolonComments(true);

    // armasm integer literals: &FF [hex], %1010 [binary] and n_xxxx
    // [base n].  '0x' and plain decimal keep working as they always did.
    getLexer().setLexArmasmIntegers(true);
  }

public:
  ARMCOFFMasmParser() = default;
};

} // end anonymous namespace

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

/// AREA name, attr{, attr...}
///   CODE -> .text   DATA + READWRITE -> .data
///   NOINIT/UNINIT -> .bss (nobits)     ALIGN=n -> 2^n alignment
///
/// The name may also be armasm's pipe form, |name|, which is how Platform
/// Builder spells the standard sections (|.text|, |.data|, ...).  Those
/// names start with a character the assembler lexer does not accept in an
/// identifier, so they are collected token by token between the pipes.
bool ARMCOFFMasmParser::parseDirectiveArea(StringRef Directive, SMLoc Loc) {
  std::string NameStorage;
  StringRef Name;
  if (getLexer().is(AsmToken::Pipe)) {
    Lex(); // the opening '|'
    while (getLexer().isNot(AsmToken::Pipe)) {
      if (getLexer().is(AsmToken::EndOfStatement) || getLexer().is(AsmToken::Eof))
        return Error(Loc, "unterminated section name in AREA (missing '|')");
      // Spaces are skipped by the lexer, so a section name containing one
      // would be joined without it; armasm section names are single words.
      // PipePipe ("||") contributes both characters, i.e. the source text.
      NameStorage += getTok().getString();
      Lex();
    }
    Lex(); // the closing '|'
    Name = NameStorage;
  } else {
    if (getParser().parseIdentifier(Name))
      return Error(Loc, "expected section name after AREA");
    // Some sources write the pipes without a leading '|' (trailing only).
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
      // ALIGN=2^k or ALIGN=n?  armasm: ALIGN=exp where the alignment is
      // the value itself if a power of two (documented as 2^exp by some
      // revisions; MS docs: "alignment expressed as 2^exp").  armasm
      // actually takes the byte alignment directly when it is a power of
      // two; Platform Builder sources use ALIGN=2 for 4 bytes, i.e. the
      // exponent form.
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

/// NAME PROC [FRAME [:handler]]
///
/// armasm spells this "NAME PROC": the name is the label in front of the
/// directive, which emitMasmLabel() has already emitted.  "PROC NAME" is
/// accepted as well so the directive also works on its own.
bool ARMCOFFMasmParser::parseDirectiveProc(StringRef Directive, SMLoc Loc) {
  MCSymbol *Sym = getParser().takeMasmLabel();
  bool LabelEmitted = Sym != nullptr;
  if (!Sym && getParser().parseSymbol(Sym))
    return Error(Loc, "expected identifier for procedure");

  // Optional [FRAME:handler] etc. - skip to end (CE sources rarely use it;
  // ARM32 CE has no SEH unwinding records).
  while (getLexer().isNot(AsmToken::EndOfStatement))
    Lex();

  auto *COFFSym = static_cast<MCSymbolCOFF *>(Sym);
  // A procedure is local unless the source exported it; "EXPORT name" comes
  // first in armasm and must survive the "name PROC" that follows.
  if (!ExportedNames.count(Sym->getName()))
    COFFSym->setExternal(false);
  COFFSym->setType(COFF::IMAGE_SYM_DTYPE_FUNCTION
                   << COFF::SCT_COMPLEX_TYPE_SHIFT);
  if (!LabelEmitted)
    getStreamer().emitLabel(Sym, Loc);
  CurrentProcedures.push_back(Sym->getName());
  return false;
}

/// ENDP [name]
///
/// armasm spells this "NAME ENDP", where the name is the label in front of
/// the directive and was deliberately not emitted again (see
/// emitMasmLabel()).  It is checked against the procedure that is open.
bool ARMCOFFMasmParser::parseDirectiveEndProc(StringRef Directive,
                                              SMLoc Loc) {
  MCSymbol *Sym = getParser().takeMasmLabel();
  if (!Sym && getLexer().isNot(AsmToken::EndOfStatement)) {
    if (getParser().parseSymbol(Sym))
      return Error(Loc, "expected identifier after ENDP");
  }
  // Optional trailing operands.
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

/// END - end of source; nothing to emit (parser stops at EOF).
bool ARMCOFFMasmParser::parseDirectiveEnd(StringRef, SMLoc) {
  while (getLexer().isNot(AsmToken::EndOfStatement))
    Lex();
  return false;
}

/// EXPORT / GLOBAL name [type]
bool ARMCOFFMasmParser::parseDirectiveExport(StringRef Directive, SMLoc Loc) {
  MCSymbol *Sym;
  if (getParser().parseSymbol(Sym))
    return Error(Loc, "expected identifier");
  auto *COFFSym = static_cast<MCSymbolCOFF *>(Sym);
  COFFSym->setExternal(true);
  ExportedNames.insert(Sym->getName());
  // armasm EXPORT with [DATA] / {arm} decorators: skip to end.
  while (getLexer().isNot(AsmToken::EndOfStatement))
    Lex();
  return false;
}

/// IMPORT / EXTERN name [type]
bool ARMCOFFMasmParser::parseDirectiveImport(StringRef Directive, SMLoc Loc) {
  MCSymbol *Sym;
  if (getParser().parseSymbol(Sym))
    return Error(Loc, "expected identifier");
  auto *COFFSym = static_cast<MCSymbolCOFF *>(Sym);
  COFFSym->setExternal(true);
  // Unused IMPORTs must still appear in the COFF symbol table; otherwise
  // llvm-readobj / the linker never see them.
  getStreamer().emitSymbolAttribute(Sym, MCSA_Global);
  while (getLexer().isNot(AsmToken::EndOfStatement))
    Lex();
  return false;
}

/// EXPORTAS name, target
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

/// ALIGN [=exp | exp] - armasm alignment is the exponent form (2^exp) when
/// used with '=', plain power-of-two bytes otherwise; PB sources use both.
bool ARMCOFFMasmParser::parseDirectiveAlign(StringRef Directive, SMLoc Loc) {
  int64_t Val = 4;
  if (getLexer().is(AsmToken::Integer)) {
    if (getParser().parseIntToken(Val, "expected alignment value"))
      return true;
    // Heuristic: PB sources write ALIGN=2 intending 4 bytes (exponent
    // form), or ALIGN 4 for 4 bytes.  Values of 0/1/2/3 are treated as the
    // exponent (2^val); >=4 as direct byte alignment.
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

/// ENTRY - mark an entry point label (no-op for object files on CE).
bool ARMCOFFMasmParser::parseDirectiveEntry(StringRef, SMLoc) {
  while (getLexer().isNot(AsmToken::EndOfStatement))
    Lex();
  return false;
}

/// DCD/DCW/DCB/DCQ expression{, expression} - 4/2/1/8-byte data.
///
/// armasm has no separate signed/unsigned forms and no alignment variants;
/// DCB additionally accepts a quoted string, as armasm does.
bool ARMCOFFMasmParser::parseDirectiveDataValue(StringRef Directive, SMLoc Loc,
                                                unsigned Size) {
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

bool ARMCOFFMasmParser::parseDirectiveDCD(StringRef Directive, SMLoc Loc) {
  return parseDirectiveDataValue(Directive, Loc, 4);
}

bool ARMCOFFMasmParser::parseDirectiveDCW(StringRef Directive, SMLoc Loc) {
  return parseDirectiveDataValue(Directive, Loc, 2);
}

bool ARMCOFFMasmParser::parseDirectiveDCB(StringRef Directive, SMLoc Loc) {
  return parseDirectiveDataValue(Directive, Loc, 1);
}

bool ARMCOFFMasmParser::parseDirectiveDCQ(StringRef Directive, SMLoc Loc) {
  return parseDirectiveDataValue(Directive, Loc, 8);
}

/// DCFS/DCFD expression{, expression} - single/double precision data.
///
/// AsmParser reads every floating point literal as an IEEE double, which is
/// exactly what DCFD wants; DCFS narrows the value to single precision here
/// rather than emitting the double bit pattern truncated to four bytes.
bool ARMCOFFMasmParser::parseDirectiveDCF(StringRef Directive, SMLoc Loc,
                                          bool IsDouble) {
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

/// SPACE expression - reserve that many zero bytes.
bool ARMCOFFMasmParser::parseDirectiveSpace(StringRef, SMLoc Loc) {
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

/// FILL expression{, value} - reserve that many bytes, filled with `value`.
///
/// armasm's third operand, the element size, is not supported.
bool ARMCOFFMasmParser::parseDirectiveFill(StringRef, SMLoc Loc) {
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

/// NAME EQU expression
///
/// EQU defines a symbolic constant, so the name in front of the directive is
/// deliberately not emitted as a label (see emitMasmLabel()).
bool ARMCOFFMasmParser::parseDirectiveEqu(StringRef Directive, SMLoc Loc) {
  MCSymbol *Sym = getParser().takeMasmLabel();
  if (!Sym)
    return Error(Loc, "EQU needs a name in front of it: 'NAME EQU value'");
  const MCExpr *Value;
  if (getParser().parseExpression(Value))
    return true;
  while (getLexer().isNot(AsmToken::EndOfStatement))
    Lex();
  getStreamer().emitAssignment(Sym, Value);
  return false;
}

bool ARMCOFFMasmParser::emitMasmLabel(MCSymbol *Sym, SMLoc Loc,
                                      StringRef Directive) {
  // ENDP/ENDFUNC close the procedure that "NAME PROC" opened and EQU defines
  // a symbolic constant: in both cases the label names something that is
  // already defined, so emitting it here would redefine the symbol.
  if (!Directive.empty() && (Directive.equals_insensitive("endp") ||
                             Directive.equals_insensitive("endfunc") ||
                             Directive.equals_insensitive("equ")))
    return false;
  return MCAsmParserExtension::emitMasmLabel(Sym, Loc, Directive);
}

namespace llvm {
// Registered from the ARM target's MC setup; consumed by llvm-mc/llvm-ml
// style drivers for the arm-pc-wince (and any arm-*-win32*) triples.
MCAsmParserExtension *createARMCOFFMasmParser() {
  return new ARMCOFFMasmParser;
}
} // end namespace llvm
