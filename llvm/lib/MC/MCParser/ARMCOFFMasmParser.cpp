//===- ARMCOFFMasmParser.cpp - COFF armasm (MASM-style) Parser for ARM ----===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM
// Exceptions.  See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// ARM dialect of the MASM-family parsers: the Microsoft `armasm` syntax used
// by Windows CE Platform Builder sources (AREA/PROC/ENDP/EXPORT/IMPORT,
// DCD/DCB/DCW/DCQ/DCFS/DCFD data, SPACE/FILL, EQU, the &hex / %binary /
// n_xxxx integer literals, ';' comments, and labels written in front of a
// directive instead of with a trailing ':').  This is an MCAsmParserExtension
// attached to MasmParser (the statement parser behind llvm-ml), mirroring
// COFFMasmParser for x86: MasmParser natively implements the MASM-family
// statement forms - "NAME <directive>" infix, EQU, IF/ELSEIF/ELSE/ENDIF,
// IFDEF/IFNDEF - and the integer data definitions (DB/DW/DD/DQ/REAL4/REAL8),
// so armasm reaches them through addAliasForDirective; what this extension
// adds on top is what MASM has no spelling for (AREA's COFF sections, the
// COFF PROC/ENDP/EXPORT/IMPORT framing, DCB strings, DCFS/DCFD narrowing,
// SPACE/FILL, GBLA/LCLA declarations, SETA/SETL assignment).  The
// instruction mnemonics themselves are parsed by the regular ARMAsmParser.
//
// Still missing for full armasm: the macro processor (MACRO/MEND,
// WHILE/WEND, GET/INCLUDE/LTORG are diagnosed by name, never silently
// ignored - MasmParser has a MASM macro processor, but armasm's macro
// syntax is not MASM's) and the SETS/SETB spellings plus the literal
// "IF :DEF:" form (IFDEF/IFNDEF are the spellings that work).  See
// utils/wince/README.md.
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
  bool parseDirectiveDCB(StringRef, SMLoc);
  bool parseDirectiveDCF(StringRef, SMLoc, bool IsDouble);
  bool parseDirectiveDCFS(StringRef, SMLoc);
  bool parseDirectiveDCFD(StringRef, SMLoc);
  bool parseDirectiveDCFSU(StringRef, SMLoc);
  bool parseDirectiveDCFDU(StringRef, SMLoc);
  bool parseDirectiveDCFU(StringRef, SMLoc);
  bool parseDirectiveSpace(StringRef, SMLoc);
  bool parseDirectiveFill(StringRef, SMLoc);
  /// GBLA/GBLL/GBLS and LCLA/LCLL/LCLS: armasm's variable storage.  Declaring
  /// is exactly "name = 0", which is what makes the name readable from any
  /// expression (SETA below, a DCD operand, an IF condition).
  bool parseDirectiveVarDecl(StringRef, SMLoc);
  /// SETA/SETL: armasm's variable assignment.  The name was declared with
  /// GBLA/GBLL/LCLA/LCLL, which made it an ordinary redefinable symbol;
  /// SETA/SETL give it a new value.
  bool parseDirectiveSet(StringRef, SMLoc);
  /// WHILE/WEND/MACRO/MEND/GET/INCLUDE: armasm's macro processor.
  bool parseDirectiveNeedsMacroPass(StringRef, SMLoc);

  /// True when the current identifier token is the NAME that introduces a
  /// "NAME <directive> operands" statement rather than the directive's
  /// first operand.  MasmParser hands the infix form to the directive with
  /// NAME un-lexed back as the current token; the two situations are told
  /// apart by what follows the identifier - a NAME is followed by the
  /// operands, an operand by the end of the statement or the comma before
  /// the next one.  Only the directives whose statement-start form begins
  /// with a value need this look-around (DCB/DCFS/DCFD/SPACE/FILL); for
  /// PROC/ENDP and GBLA the current identifier is the name in every form
  /// the directive accepts.
  bool tokenIsInfixName() {
    // A Lex/UnLex round-trip, not peekTokens: MasmParser hands the infix
    // form to the directive with the NAME pushed back through
    // AsmLexer::UnLex, and peekTokens lexes from the raw buffer - it never
    // sees the un-lexed token, so the peek would read what follows the
    // first operand instead of what follows the NAME.
    AsmToken T = Lex();
    getLexer().UnLex(T);
    return !T.is(AsmToken::EndOfStatement) && !T.is(AsmToken::Comma);
  }

  /// Emit the leading NAME of the current "NAME <directive>" statement as
  /// its label.  Does nothing when the statement began with the directive.
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

    // The integer data forms are MASM's DB/DW/DD/DQ with armasm names: DCB
    // is one byte, DCW two, DCD four, DCQ eight, and the 'U' spellings
    // (DCWU/DCDU/DCQU - the forms armasm allows at an unaligned address)
    // emit the same bytes, because LLVM MC never aligns a data emission in
    // the first place.  Aliasing them onto the generic directives is what
    // makes both "DCD 1" and "name DCD 1" work with no code of our own:
    // MasmParser dispatches the infix form (the name in front of the
    // directive) to the same directive table, where these land on
    // parseDirectiveNamedValue.  EQU and the conditionals (IF/ELSEIF/ELSE/
    // ENDIF/IFDEF/IFNDEF) are MASM directives with armasm-compatible
    // semantics already, so they need no registration at all.  DCB
    // (strings) and the DCFS/DCFD float forms stay real handlers below:
    // MASM's value parsing has no string form and no single-precision
    // narrowing.
    for (const char *D : {"dcd", "dcdu"})
      getParser().addAliasForDirective(D, "dd");
    for (const char *D : {"dcw", "dcwu"})
      getParser().addAliasForDirective(D, "dw");
    for (const char *D : {"dcq", "dcqu"})
      getParser().addAliasForDirective(D, "dq");

    // armasm data definition and reservation that MASM has no spelling for.
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

    // armasm's macro processor has no counterpart in a one-pass assembler,
    // and silently dropping the statements - which is what the no-op handler
    // below would do - would assemble a program the source does not describe.
    for (const char *D :
         {"while", "wend", "macro", "mend", "get", "include", "ltorg"})
      addDirectiveHandler<&ARMCOFFMasmParser::parseDirectiveNeedsMacroPass>(D);

    // armasm directives with no object-file effect (these really are
    // annotations: they cannot change the emitted bytes).
    for (const char *D :
         {"preserve8", "require8", "require", "keep", "nocrossref", "nofp",
          "rout", "opt", "ttl", "subt", "code32", "code16", "arm", "thumb",
          "thumbx"})
      addDirectiveHandler<&ARMCOFFMasmParser::IgnoreDirective>(D);

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

bool ARMCOFFMasmParser::parseDirectiveVarDecl(StringRef Directive, SMLoc Loc) {
  // Both spellings occur in CE sources: "count GBLA 4" (the infix form;
  // MasmParser hands it to us with the name un-lexed back as the current
  // token) and "GBLA count" (statement start).  In both the current token
  // is the name, so read it the same way.
  MCSymbol *Sym = nullptr;
  if (getLexer().is(AsmToken::Identifier)) {
    if (getParser().parseSymbol(Sym))
      return Error(Loc, "expected a name after " + Directive);
  } else
    return Error(Loc, "expected a name after " + Directive);

  // A trailing ",N" is the element count of an array.  The elements are
  // separate armasm variables written NAME!n, which the expression grammar
  // cannot spell, so only the count is consumed here: the line is read
  // correctly and the scalar exists, which is what driver sources use.
  while (getLexer().isNot(AsmToken::EndOfStatement))
    Lex();

  getStreamer().emitAssignment(Sym, MCConstantExpr::create(0, getContext()));
  // armasm variables are reassigned by SETA; only a symbol marked
  // redefinable survives the check the generic assignment path applies.
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
/// armasm spells this "NAME PROC" (the infix form; MasmParser hands us NAME
/// as the current token).  "PROC NAME" is accepted as well so the directive
/// also works on its own.
bool ARMCOFFMasmParser::parseDirectiveProc(StringRef Directive, SMLoc Loc) {
  // "NAME PROC" (the infix form: MasmParser un-lexes NAME back as the
  // current token) and "PROC NAME" both present the name as the current
  // token; a bare "PROC" has none.
  MCSymbol *Sym = nullptr;
  if (getLexer().is(AsmToken::Identifier))
    if (getParser().parseSymbol(Sym))
      return Error(Loc, "expected identifier for procedure");
  if (!Sym)
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
  // Under MasmParser no machinery has emitted the NAME label for us (the
  // infix form only un-lexes it back), so "NAME PROC" defines it here -
  // and "PROC NAME" did too.
  getStreamer().emitLabel(Sym, Loc);
  CurrentProcedures.push_back(Sym->getName());
  return false;
}

/// ENDP [name]
///
/// armasm spells this "NAME ENDP" (the infix form).  The name is checked
/// against the procedure that is open, not emitted again: it already got
/// its label from "NAME PROC".
bool ARMCOFFMasmParser::parseDirectiveEndProc(StringRef Directive,
                                              SMLoc Loc) {
  // "NAME ENDP" (infix) and "ENDP NAME" both put the name in the current
  // token; "ENDP" alone matches the most recent open PROC.
  MCSymbol *Sym = nullptr;
  if (getLexer().is(AsmToken::Identifier)) {
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
  // llvm-readobj / the linker never see them.  MCSA_Global registers the
  // symbol; the .def/.scl/.type/.endef sequence is what gas .extern uses
  // to force a COFF symbol-table row for an undefined external.
  getStreamer().emitSymbolAttribute(Sym, MCSA_Global);
  getStreamer().beginCOFFSymbolDef(Sym);
  getStreamer().emitCOFFSymbolStorageClass(COFF::IMAGE_SYM_CLASS_EXTERNAL);
  getStreamer().emitCOFFSymbolType(0);
  getStreamer().endCOFFSymbolDef();
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
  // "NAME DCB \"x\", 0": the leading identifier is the label the values
  // follow (see tokenIsInfixName for how it is told from an operand).
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

/// DCFS/DCFD expression{, expression} - single/double precision data.
///
/// AsmParser reads every floating point literal as an IEEE double, which is
/// exactly what DCFD wants; DCFS narrows the value to single precision here
/// rather than emitting the double bit pattern truncated to four bytes.
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

/// SPACE expression - reserve that many zero bytes.
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

/// FILL expression{, value} - reserve that many bytes, filled with `value`.
///
/// armasm's third operand, the element size, is not supported.
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

/// NAME SETA expression / NAME SETL expression
///
/// armasm reassigns its variables with SETA (numeric) and SETL (logical).
/// The name was introduced by GBLA/GBLL/LCLA/LCLL, which made it an ordinary
/// redefinable symbol; giving it a new value is what an assignment to a
/// redefinable symbol does, so it stays readable from every expression -
/// another SETA's right-hand side included.
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
// Registered from the ARM target's MC setup; consumed by llvm-mc/llvm-ml
// style drivers for the arm-pc-wince (and any arm-*-win32*) triples.
MCAsmParserExtension *createARMCOFFMasmParser() {
  return new ARMCOFFMasmParser;
}
} // end namespace llvm
