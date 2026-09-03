//===-- CodeGen/AsmPrinter/ARMException.cpp - ARM EHABI Exception Impl ----===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file contains support for writing DWARF exception info into asm files.
//
//===----------------------------------------------------------------------===//

#include "DwarfException.h"
#include "llvm/ADT/Twine.h"
#include "llvm/CodeGen/AsmPrinter.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/IR/EHPersonalities.h"
#include "llvm/IR/Function.h"
#include "llvm/MC/MCAsmInfo.h"
#include "llvm/MC/MCContext.h"
#include "llvm/MC/MCStreamer.h"
using namespace llvm;

ARMException::ARMException(AsmPrinter *A) : EHStreamer(A) {}

ARMException::~ARMException() = default;

ARMTargetStreamer &ARMException::getTargetStreamer() {
  MCTargetStreamer &TS = *Asm->OutStreamer->getTargetStreamer();
  return static_cast<ARMTargetStreamer &>(TS);
}

void ARMException::beginFunction(const MachineFunction *MF) {
  // Windows CE SEH functions (MSVC __try/__except; the prologue carries
  // SEH_* pseudo-instructions inserted by ARMFrameLowering, which also sets
  // MF.hasWinCFI) build a WinCFI frame (.seh_proc). The ARMWinCOFFStreamer
  // encodes the frame into the compressed CE .pdata format plus an in-text
  // PDATA_EH pair placed by ARMAsmPrinter. See ARMWinCFI.h /
  // utils/wince/WINEH-ABI-FACTS.md 4d.
  if (MF->hasWinCFI()) {
    Asm->OutStreamer->emitWinCFIStartProc(Asm->CurrentFnSym);
    const Function &F = MF->getFunction();
    const Function *Per = F.hasPersonalityFn()
                              ? dyn_cast<Function>(
                                    F.getPersonalityFn()->stripPointerCasts())
                              : nullptr;
    EHPersonality Personality = classifyEHPersonality(Per);
    // Only the parent function (the one holding the SEH state machine) gets
    // a handler; funclets are handler bodies and must not claim an exception
    // handler of their own (mirrors WinException::beginFunclet).
    if (MF->hasEHFunclets() && Per) {
      Asm->OutStreamer->emitWinEHHandler(Asm->getSymbol(Per),
                                         /*Unwind=*/false,
                                         /*Except=*/true);
    } else if (F.hasPersonalityFn() &&
               Personality == EHPersonality::GNU_CXX) {
      // Windows CE C++ (Itanium) exceptions: the kernel dispatches a thrown
      // exception through the .pdata ExceptionFlag + the PDATA_EH pair to
      // __wince_cxx_frame_handler (libcxxabi), which runs the Itanium
      // search/cleanup on a cursor built from the fault CONTEXT and resumes
      // the unwind.  Claiming this handler is what makes the .pdata entry
      // carry ExceptionFlag=1 (see ARMWinCOFFStreamer::CEEmitUnwindInfo);
      // without it a C++ throw has no frame handler and the process dies.
      // The EHABI frame below (.personality __gxx_personality_v0) is a
      // separate mechanism (this toolchain's own unwinder) and is unchanged;
      // the two coexist, exactly as WINEH-ABI-FACTS.md 4i describes.
      Asm->OutStreamer->emitWinEHHandler(
          Asm->OutContext.getOrCreateSymbol("__wince_cxx_frame_handler"),
          /*Unwind=*/false,
          /*Except=*/true);
    }
    // No DWARF CFI for WinCFI frames (the prologue carries no CFI
    // instructions either; matches the WinException path on desktop).
    shouldEmitCFI = false;
    // On Windows CE every function carries BOTH unwind tables: on top of
    // the WinCFI frame above (.pdata, consumed by the kernel) also open
    // the ARM EHABI frame (.fnstart/.fnend, consumed by this toolchain's
    // own unwinder).  Without an .ARM.exidx entry for an SEH function, a
    // C++ exception propagating through it would hit the *previous*
    // function's entry in the (binary-searched) table and unwind with the
    // wrong opcodes.  endFunction closes the EHABI frame before the WinCFI
    // one.
    if (Asm->MAI->getExceptionHandlingType() == ExceptionHandling::ARM)
      getTargetStreamer().emitFnStart();
    return;
  }

  if (Asm->MAI->getExceptionHandlingType() == ExceptionHandling::ARM)
    getTargetStreamer().emitFnStart();
  // See if we need call frame info.
  AsmPrinter::CFISection CFISecType = Asm->getFunctionCFISectionType(*MF);
  assert(CFISecType != AsmPrinter::CFISection::EH &&
         "non-EH CFI not yet supported in prologue with EHABI lowering");

  if (CFISecType == AsmPrinter::CFISection::Debug) {
    if (!hasEmittedCFISections) {
      if (Asm->getModuleCFISectionType() == AsmPrinter::CFISection::Debug)
        Asm->OutStreamer->emitCFISections(false, true, false);
      hasEmittedCFISections = true;
    }

    shouldEmitCFI = true;
    Asm->OutStreamer->emitCFIStartProc(false);
  }
}

void ARMException::markFunctionEnd() {
  if (Asm->MF && Asm->MF->hasWinCFI())
    return;
  if (shouldEmitCFI)
    Asm->OutStreamer->emitCFIEndProc();
}

/// Emit the ARM EHABI closing directives for MF: the personality /
/// unwind-opcode handling followed by .fnend.  On Windows CE this also runs
/// for WinCFI (SEH) functions: their SEH personality (__C_specific_handler)
/// is NOT an EHABI personality -- the kernel dispatches SEH through .pdata
/// and PDATA_EH -- so the EHABI entry carries plain unwind opcodes only
/// (compact pr0/pr1), which is exactly what a C++ exception needs to unwind
/// through the frame without triggering any handler.
void ARMException::emitEHABIFunctionEnd(const MachineFunction *MF) {
  ARMTargetStreamer &ATS = getTargetStreamer();
  const Function &F = MF->getFunction();
  const Function *Per = nullptr;
  if (F.hasPersonalityFn())
    Per = dyn_cast<Function>(F.getPersonalityFn()->stripPointerCasts());
  EHPersonality Personality = classifyEHPersonality(Per);
  bool IsSEHPersonality = Personality == EHPersonality::MSVC_TableSEH ||
                          Personality == EHPersonality::MSVC_X86SEH;
  bool forceEmitPersonality =
    !IsSEHPersonality && F.hasPersonalityFn() &&
    !isNoOpWithoutInvoke(Personality) &&
    F.needsUnwindTableEntry();
  bool shouldEmitPersonality = forceEmitPersonality ||
    !MF->getLandingPads().empty();
  if (IsSEHPersonality) {
    // SEH function on Windows CE: unwind opcodes only -- never
    // .cantunwind (the frame must stay unwindable by the EHABI unwinder,
    // that is the whole point of its exidx entry) and never .handlerdata
    // (no EHABI personality / LSDA: SEH scope tables live in .text,
    // emitted by ARMAsmPrinter::emitCEHandlerData).
  } else if (!Asm->MF->getFunction().needsUnwindTableEntry() &&
             !shouldEmitPersonality) {
    ATS.emitCantUnwind();
  } else if (shouldEmitPersonality) {
    // Emit references to personality.
    if (Per) {
      MCSymbol *PerSym = Asm->getSymbol(Per);
      ATS.emitPersonality(PerSym);
    }

    // Emit .handlerdata directive.
    ATS.emitHandlerData();

    // Emit actual exception table
    emitExceptionTable();
  }

  ATS.emitFnEnd();
}

/// endFunction - Gather and emit post-function exception information.
///
void ARMException::endFunction(const MachineFunction *MF) {
  if (MF->hasWinCFI()) {
    // Windows CE: close the EHABI frame first (.fnend flushes the unwind
    // opcodes into .ARM.exidx and switches the streamer back to the
    // function's text section), then close the WinCFI frame (.seh_endproc),
    // which triggers the ARMWinCOFFStreamer to emit the compressed CE
    // .pdata entry from the accumulated frame info.
    if (Asm->MAI->getExceptionHandlingType() == ExceptionHandling::ARM)
      emitEHABIFunctionEnd(MF);
    Asm->OutStreamer->emitWinCFIEndProc();
    return;
  }

  if (Asm->MAI->getExceptionHandlingType() == ExceptionHandling::ARM)
    emitEHABIFunctionEnd(MF);
}

void ARMException::emitTypeInfos(unsigned TTypeEncoding,
                                 MCSymbol *TTBaseLabel) {
  const MachineFunction *MF = Asm->MF;
  const std::vector<const GlobalValue *> &TypeInfos = MF->getTypeInfos();
  const std::vector<unsigned> &FilterIds = MF->getFilterIds();

  bool VerboseAsm = Asm->OutStreamer->isVerboseAsm();

  int Entry = 0;
  // Emit the Catch TypeInfos.
  if (VerboseAsm && !TypeInfos.empty()) {
    Asm->OutStreamer->AddComment(">> Catch TypeInfos <<");
    Asm->OutStreamer->addBlankLine();
    Entry = TypeInfos.size();
  }

  for (const GlobalValue *GV : reverse(TypeInfos)) {
    if (VerboseAsm)
      Asm->OutStreamer->AddComment("TypeInfo " + Twine(Entry--));
    Asm->emitTTypeReference(GV, TTypeEncoding);
  }

  Asm->OutStreamer->emitLabel(TTBaseLabel);

  // Emit the Exception Specifications.
  if (VerboseAsm && !FilterIds.empty()) {
    Asm->OutStreamer->AddComment(">> Filter TypeInfos <<");
    Asm->OutStreamer->addBlankLine();
    Entry = 0;
  }
  for (std::vector<unsigned>::const_iterator
         I = FilterIds.begin(), E = FilterIds.end(); I < E; ++I) {
    unsigned TypeID = *I;
    if (VerboseAsm) {
      --Entry;
      if (TypeID != 0)
        Asm->OutStreamer->AddComment("FilterInfo " + Twine(Entry));
    }

    Asm->emitTTypeReference((TypeID == 0 ? nullptr : TypeInfos[TypeID - 1]),
                            TTypeEncoding);
  }
}
