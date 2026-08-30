//===- MCAsmParserExtension.cpp - Asm Parser Hooks ------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "llvm/MC/MCParser/MCAsmParserExtension.h"
#include "llvm/MC/MCContext.h"
#include "llvm/MC/MCExpr.h"
#include "llvm/MC/MCParser/AsmLexer.h"
#include "llvm/MC/MCParser/MCTargetAsmParser.h"
#include "llvm/MC/MCStreamer.h"
#include "llvm/MC/MCSymbol.h"

using namespace llvm;

MCAsmParserExtension::MCAsmParserExtension() = default;

MCAsmParserExtension::~MCAsmParserExtension() = default;

void MCAsmParserExtension::Initialize(MCAsmParser &Parser) {
  this->Parser = &Parser;
}

bool MCAsmParserExtension::emitMasmLabel(MCSymbol *Sym, SMLoc Loc,
                                         StringRef /*Directive*/) {
  // The plain MASM-family reading: "name DCD 1" defines `name` right here.
  // (AsmParser has already made sure the following directive, if any, is one
  // this extension registered.)  A name that is already a defined symbol
  // (armasm "name ENDP" after "name PROC") must not be emitted again.
  if (Sym->isDefined())
    return false;
  getParser().getTargetParser().doBeforeLabelEmit(Sym, Loc);
  getStreamer().emitLabel(Sym, Loc);
  getParser().getTargetParser().onLabelParsed(Sym);
  return false;
}

/// parseDirectiveCGProfile
///  ::= .cg_profile identifier, identifier, <number>
bool MCAsmParserExtension::parseDirectiveCGProfile(StringRef, SMLoc) {
  StringRef From;
  SMLoc FromLoc = getLexer().getLoc();
  if (getParser().parseIdentifier(From))
    return TokError("expected identifier in directive");

  if (getLexer().isNot(AsmToken::Comma))
    return TokError("expected a comma");
  Lex();

  StringRef To;
  SMLoc ToLoc = getLexer().getLoc();
  if (getParser().parseIdentifier(To))
    return TokError("expected identifier in directive");

  if (getLexer().isNot(AsmToken::Comma))
    return TokError("expected a comma");
  Lex();

  int64_t Count;
  if (getParser().parseIntToken(Count))
    return true;

  if (getLexer().isNot(AsmToken::EndOfStatement))
    return TokError("unexpected token in directive");

  MCSymbol *FromSym = getContext().parseSymbol(From);
  MCSymbol *ToSym = getContext().parseSymbol(To);

  getStreamer().emitCGProfileEntry(
      MCSymbolRefExpr::create(FromSym, getContext(), FromLoc),
      MCSymbolRefExpr::create(ToSym, getContext(), ToLoc), Count);
  return false;
}

bool MCAsmParserExtension::maybeParseUniqueID(int64_t &UniqueID) {
  AsmLexer &L = getLexer();
  if (L.isNot(AsmToken::Comma))
    return false;
  Lex();
  StringRef UniqueStr;
  if (getParser().parseIdentifier(UniqueStr))
    return TokError("expected identifier");
  if (UniqueStr != "unique")
    return TokError("expected 'unique'");
  if (L.isNot(AsmToken::Comma))
    return TokError("expected commma");
  Lex();
  if (getParser().parseAbsoluteExpression(UniqueID))
    return true;
  if (UniqueID < 0)
    return TokError("unique id must be positive");
  if (!isUInt<32>(UniqueID) || UniqueID == ~0U)
    return TokError("unique id is too large");
  return false;
}
