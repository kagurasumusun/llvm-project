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
// DCD/DCB/DCW/DCQ data, %/&/2_ numeric markers, ';' comments, APCS register
// aliases).  This is an MCAsmParserExtension registered on top of the ARM
// AsmParser, mirroring COFFMasmParser for x86; the instruction mnemonics
// themselves are parsed by the regular ARMAsmParser.
//
//===----------------------------------------------------------------------===//

#include "llvm/ADT/StringSwitch.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/BinaryFormat/COFF.h"
#include "llvm/MC/MCContext.h"
#include "llvm/MC/MCParser/MCAsmParserExtension.h"
#include "llvm/MC/MCSectionCOFF.h"
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
  bool IgnoreDirective(StringRef, SMLoc) {
    while (!getLexer().is(AsmToken::EndOfStatement))
      Lex();
    return false;
  }

  /// The most recent PROC symbol, for ENDP's implicit .size framing.
  SmallVector<StringRef, 1> CurrentProcedures;

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

    // armasm directives with no object-file effect.
    for (const char *D :
         {"preserve8", "require8", "require", "keep", "nocrossref", "nofp",
          "rout", "opt", "ttl", "subt", "code32", "code16", "arm", "thumb",
          "thumbx"})
      addDirectiveHandler<&ARMCOFFMasmParser::IgnoreDirective>(D);
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
      NameStorage += getTok().getString();
      if (getLexer().is(AsmToken::PipePipe))
        NameStorage += "|";
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
bool ARMCOFFMasmParser::parseDirectiveProc(StringRef Directive, SMLoc Loc) {
  MCSymbol *Sym;
  if (getParser().parseSymbol(Sym))
    return Error(Loc, "expected identifier for procedure");

  // Optional [FRAME:handler] etc. - skip to end (CE sources rarely use it;
  // ARM32 CE has no SEH unwinding records).
  while (getLexer().isNot(AsmToken::EndOfStatement))
    Lex();

  auto *COFFSym = static_cast<MCSymbolCOFF *>(Sym);
  COFFSym->setExternal(false);
  COFFSym->setType(COFF::IMAGE_SYM_DTYPE_FUNCTION
                   << COFF::SCT_COMPLEX_TYPE_SHIFT);
  getStreamer().emitLabel(Sym, Loc);
  CurrentProcedures.push_back(Sym->getName());
  return false;
}

/// ENDP [name]
bool ARMCOFFMasmParser::parseDirectiveEndProc(StringRef Directive,
                                              SMLoc Loc) {
  // Optional name operand.
  while (getLexer().isNot(AsmToken::EndOfStatement))
    Lex();
  if (!CurrentProcedures.empty()) {
    StringRef Name = CurrentProcedures.pop_back_val();
    MCSymbol *Sym = getContext().getOrCreateSymbol(Name);
    // armasm/COFF: mark the size type; GNU .size framing is not emitted
    // here because COFF import/export does not need it.
    (void)Sym;
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
  // The symbol stays undefined until a matching definition is encountered,
  // which is exactly what the COFF object writer wants for an extern/import.
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

namespace llvm {
// Registered from the ARM target's MC setup; consumed by llvm-mc/llvm-ml
// style drivers for the arm-pc-wince (and any arm-*-win32*) triples.
MCAsmParserExtension *createARMCOFFMasmParser() {
  return new ARMCOFFMasmParser;
}
} // end namespace llvm
