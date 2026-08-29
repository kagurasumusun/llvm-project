//===-- ARMWinCOFFStreamer.cpp - ARM Target WinCOFF Streamer ----*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "ARMMCTargetDesc.h"
#include "ARMUnwindOpAsm.h"
#include "llvm/Support/ARMEHABI.h"
#include "llvm/BinaryFormat/COFF.h"
#include "llvm/MC/MCAsmBackend.h"
#include "llvm/MC/MCAssembler.h"
#include "llvm/MC/MCCodeEmitter.h"
#include "llvm/MC/MCContext.h"
#include "llvm/MC/MCObjectWriter.h"
#include "llvm/MC/MCRegisterInfo.h"
#include "llvm/MC/MCSectionCOFF.h"
#include "llvm/MC/MCWin64EH.h"
#include "llvm/MC/MCWinCOFFStreamer.h"
#include "llvm/TargetParser/Triple.h"

using namespace llvm;

namespace {
class ARMWinCOFFStreamer : public MCWinCOFFStreamer {
  Win64EH::ARMUnwindEmitter EHStreamer;

public:
  ARMWinCOFFStreamer(MCContext &C, std::unique_ptr<MCAsmBackend> AB,
                     std::unique_ptr<MCCodeEmitter> CE,
                     std::unique_ptr<MCObjectWriter> OW)
      : MCWinCOFFStreamer(C, std::move(AB), std::move(CE), std::move(OW)) {
    EHABIReset();
  }

  void emitWinEHHandlerData(SMLoc Loc) override;
  void emitWindowsUnwindTables() override;
  void emitWindowsUnwindTables(WinEH::FrameInfo *Frame) override;

  void finishImpl() override;

  //===----------------------------------------------------------===//
  // Windows CE compressed WinEH (.seh) unwinding.  CE does not use the
  // desktop ARM .xdata/RUNTIME_FUNCTION layout: the exception data
  // directory points at an array of 8-byte IMAGE_CE_RUNTIME_FUNCTION_ENTRY
  // records, and the personality routine pointer + handler data live in the
  // 8 bytes immediately preceding the function's first instruction.  See
  // utils/wince/WINEH-ABI-FACTS.md and the CE6 kernel
  // (PRIVATE/.../CORE/DLL/exdsptch.c).
  //===----------------------------------------------------------===//

  // True when .seh frames must be encoded in the CE compressed format.
  bool isCEEH() const {
    return getContext().getTargetTriple().isWindowsCE();
  }

  void CEEmitUnwindInfo(WinEH::FrameInfo *Frame);

  //===----------------------------------------------------------===//
  // ARM EHABI (Windows CE) unwinding: state machine over .ARM.exidx /
  // .ARM.extab COFF sections, driven by the ARMTargetStreamer EHABI
  // directives (.fnstart ... .fnend and friends).  Entries hold absolute
  // addresses (IMAGE_REL_ARM_ADDR32) rather than ELF PREL31 offsets; the
  // toolchain's libunwind reads the table accordingly.
  //===----------------------------------------------------------===//

  bool isEHABI() const {
    return getContext().getTargetTriple().isWindowsCE();
  }

  void EHABIReset();
  void SwitchToExIdxSection(const MCSymbol &FnStart);
  void SwitchToExTabSection(const MCSymbol &FnStart);
  void FlushPendingOffset();
  void FlushUnwindOpcodes(bool NoHandlerData);

  void EHABIemitFnStart();
  void EHABIemitFnEnd();
  void EHABIemitCantUnwind();
  void EHABIemitPersonality(const MCSymbol *Per);
  void EHABIemitPersonalityIndex(unsigned Index);
  void EHABIemitHandlerData();
  void EHABIemitSetFP(MCRegister NewFPReg, MCRegister NewSPReg,
                      int64_t Offset);
  void EHABIemitMovSP(MCRegister Reg, int64_t Offset);
  void EHABIemitPad(int64_t Offset);
  void EHABIemitRegSave(const SmallVectorImpl<MCRegister> &RegList,
                        bool IsVector);
  void EHABIemitUnwindRaw(int64_t Offset,
                          const SmallVectorImpl<uint8_t> &Ops);

  MCSymbol *EHFnStart = nullptr;
  MCSymbol *EHExTab = nullptr;
  const MCSymbol *EHPersonality = nullptr;
  unsigned EHPersonalityIndex = ARM::EHABI::NUM_PERSONALITY_INDEX;
  MCRegister EHFPReg;
  int64_t EHFPOffset = 0;
  int64_t EHSPOffset = 0;
  int64_t EHPendingOffset = 0;
  bool EHUsedFP = false;
  bool EHCantUnwind = false;
  SmallVector<uint8_t, 16> Opcodes;
  llvm::UnwindOpcodeAssembler UnwindOpAsm;
};


//===----------------------------------------------------------------------===//
// ARM EHABI (Windows CE) state machine, adapted from ARMELFStreamer.
// Differences vs. the ELF streamer:
//  - .ARM.exidx / .ARM.extab are plain COFF initialized-data sections.
//  - table entries hold absolute addresses (ADDR32) instead of PREL31.
//===----------------------------------------------------------------------===//

void ARMWinCOFFStreamer::EHABIReset() {
  EHExTab = nullptr;
  EHFnStart = nullptr;
  EHPersonality = nullptr;
  EHPersonalityIndex = ARM::EHABI::NUM_PERSONALITY_INDEX;
  EHFPReg = ARM::SP;
  EHFPOffset = 0;
  EHSPOffset = 0;
  EHPendingOffset = 0;
  EHUsedFP = false;
  EHCantUnwind = false;

  Opcodes.clear();
  UnwindOpAsm.Reset();
}

void ARMWinCOFFStreamer::SwitchToExIdxSection(const MCSymbol &FnStart) {
  MCSectionCOFF *ExIdx = getContext().getCOFFSection(
      ".ARM.exidx", COFF::IMAGE_SCN_CNT_INITIALIZED_DATA |
                        COFF::IMAGE_SCN_MEM_READ);
  switchSection(ExIdx);
  emitValueToAlignment(Align(4), 0, 1, 0);
}

void ARMWinCOFFStreamer::SwitchToExTabSection(const MCSymbol &FnStart) {
  MCSectionCOFF *ExTab = getContext().getCOFFSection(
      ".ARM.extab", COFF::IMAGE_SCN_CNT_INITIALIZED_DATA |
                        COFF::IMAGE_SCN_MEM_READ);
  switchSection(ExTab);
  emitValueToAlignment(Align(4), 0, 1, 0);
}

void ARMWinCOFFStreamer::EHABIemitFnStart() {
  assert(!EHFnStart && ".fnstart must not nest");
  EHFnStart = getContext().createTempSymbol();
  emitLabel(EHFnStart);
}

void ARMWinCOFFStreamer::EHABIemitFnEnd() {
  assert(EHFnStart && ".fnstart must precede .fnend");

  // Emit unwind opcodes if there is no .handlerdata directive.
  if (!EHExTab && !EHCantUnwind)
    FlushUnwindOpcodes(true);

  // Emit the exception index table entry.
  SwitchToExIdxSection(*EHFnStart);

  const MCSymbolRefExpr *FnStartRef =
      MCSymbolRefExpr::create(EHFnStart, getContext());
  emitValue(FnStartRef, 4);

  if (EHCantUnwind) {
    emitInt32(ARM::EHABI::EXIDX_CANTUNWIND);
  } else if (EHExTab) {
    // Emit a reference to the unwind opcodes in the ".ARM.extab" section.
    const MCSymbolRefExpr *ExTabEntryRef =
        MCSymbolRefExpr::create(EHExTab, getContext());
    emitValue(ExTabEntryRef, 4);
  } else {
    // Compact model 0: unwind opcodes live in the second word of the
    // exidx entry.
    assert(EHPersonalityIndex == ARM::EHABI::AEABI_UNWIND_CPP_PR0 &&
           "Compact model must use __aeabi_unwind_cpp_pr0 as personality");
    assert(Opcodes.size() == 4u &&
           "Unwind opcode size for __aeabi_unwind_cpp_pr0 must be 4");
    uint64_t Intval = Opcodes[0] | Opcodes[1] << 8 | Opcodes[2] << 16 |
                      Opcodes[3] << 24;
    emitIntValue(Intval, Opcodes.size());
  }

  // Switch back to the section containing FnStart.
  switchSection(&EHFnStart->getSection());

  EHABIReset();
}

void ARMWinCOFFStreamer::EHABIemitCantUnwind() { EHCantUnwind = true; }

void ARMWinCOFFStreamer::EHABIemitPersonality(const MCSymbol *Per) {
  EHPersonality = Per;
  UnwindOpAsm.setPersonality(Per);
}

void ARMWinCOFFStreamer::EHABIemitPersonalityIndex(unsigned Index) {
  assert(Index < ARM::EHABI::NUM_PERSONALITY_INDEX && "invalid index");
  EHPersonalityIndex = Index;
}

void ARMWinCOFFStreamer::EHABIemitHandlerData() { FlushUnwindOpcodes(false); }

void ARMWinCOFFStreamer::EHABIemitSetFP(MCRegister NewFPReg,
                                        MCRegister NewSPReg,
                                        int64_t Offset) {
  assert((NewSPReg == ARM::SP) && "the operand of .setfp must be sp");
  EHUsedFP = true;
  EHFPReg = NewFPReg;
  if (NewSPReg == ARM::SP)
    EHFPOffset = EHSPOffset + Offset;
}

void ARMWinCOFFStreamer::EHABIemitMovSP(MCRegister Reg, int64_t Offset) {
  assert((Reg != ARM::SP && Reg != ARM::PC) &&
         "the operand of .movsp cannot be either sp or pc");
  assert(EHFPReg == ARM::SP && "current FP must be SP");

  FlushPendingOffset();

  EHFPReg = Reg;
  EHFPOffset = EHSPOffset + Offset;

  const MCRegisterInfo *MRI = getContext().getRegisterInfo();
  UnwindOpAsm.EmitSetSP(MRI->getEncodingValue(EHFPReg));
}

void ARMWinCOFFStreamer::EHABIemitPad(int64_t Offset) {
  // Track the change of the $sp offset; delayed until .save/.handlerdata.
  EHSPOffset -= Offset;
  EHPendingOffset -= Offset;
}

void ARMWinCOFFStreamer::FlushPendingOffset() {
  if (EHPendingOffset != 0) {
    UnwindOpAsm.EmitSPOffset(-EHPendingOffset);
    EHPendingOffset = 0;
  }
}

void ARMWinCOFFStreamer::FlushUnwindOpcodes(bool NoHandlerData) {
  if (EHUsedFP) {
    const MCRegisterInfo *MRI = getContext().getRegisterInfo();
    int64_t LastRegSaveSPOffset = EHSPOffset - EHPendingOffset;
    UnwindOpAsm.EmitSPOffset(LastRegSaveSPOffset - EHFPOffset);
    UnwindOpAsm.EmitSetSP(MRI->getEncodingValue(EHFPReg));
  } else {
    FlushPendingOffset();
  }

  UnwindOpAsm.Finalize(EHPersonalityIndex, Opcodes);

  if (NoHandlerData && EHPersonalityIndex == ARM::EHABI::AEABI_UNWIND_CPP_PR0)
    return;

  SwitchToExTabSection(*EHFnStart);

  assert(!EHExTab);
  EHExTab = getContext().createTempSymbol();
  emitLabel(EHExTab);

  if (EHPersonality) {
    const MCSymbolRefExpr *PersonalityRef =
        MCSymbolRefExpr::create(EHPersonality, getContext());
    emitValue(PersonalityRef, 4);
  }

  assert((Opcodes.size() % 4) == 0 && "unwind opcode size must be multiple of 4");
  for (unsigned I = 0; I != Opcodes.size(); I += 4) {
    uint64_t Intval = Opcodes[I] | Opcodes[I + 1] << 8 | Opcodes[I + 2] << 16 |
                      Opcodes[I + 3] << 24;
    emitInt32(Intval);
  }

  if (NoHandlerData && !EHPersonality)
    emitInt32(0);
}

void ARMWinCOFFStreamer::EHABIemitRegSave(
    const SmallVectorImpl<MCRegister> &RegList, bool IsVector) {
  uint32_t Mask = 0;
  unsigned Count = 0;
  const MCRegisterInfo &MRI = *getContext().getRegisterInfo();

  for (MCRegister Reg : RegList) {
    if (Reg == ARM::RA_AUTH_CODE)
      continue; // RA PAC unsupported on WinCE (no PAC hardware)
    unsigned RegEnc = MRI.getEncodingValue(Reg);
    assert(RegEnc < (IsVector ? 32U : 16U) && "Register out of range");
    unsigned Bit = (1u << RegEnc);
    if ((Mask & Bit) == 0) {
      Mask |= Bit;
      ++Count;
    }
  }

  if (Count) {
    EHSPOffset -= Count * (IsVector ? 8 : 4);
    FlushPendingOffset();
    if (IsVector)
      UnwindOpAsm.EmitVFPRegSave(Mask);
    else
      UnwindOpAsm.EmitRegSave(Mask);
  }
}

void ARMWinCOFFStreamer::EHABIemitUnwindRaw(
    int64_t Offset, const SmallVectorImpl<uint8_t> &Ops) {
  FlushPendingOffset();
  EHSPOffset = EHSPOffset - Offset;
  UnwindOpAsm.EmitRaw(Ops);
}

//===----------------------------------------------------------------------===//
// Windows CE compressed WinEH emission
//===----------------------------------------------------------------------===//

namespace {
// Bit layout of the second word of an IMAGE_CE_RUNTIME_FUNCTION_ENTRY
// (little-endian), matching the CE kernel's PDATA bitfield:
//   PrologLen:8 | FuncLen:22 | ThirtyTwoBit:1 | ExceptionFlag:1
constexpr uint32_t CE_PDATA_PROLOG_LEN_SHIFT = 0;
constexpr uint32_t CE_PDATA_FUNC_LEN_SHIFT = 8;
constexpr uint32_t CE_PDATA_THIRTY_TWO_BIT = 0x40000000u;
constexpr uint32_t CE_PDATA_EXCEPTION_FLAG = 0x80000000u;
constexpr uint32_t CE_PDATA_LEN_MASK = 0x003FFFFFu; // 22-bit FuncLen field
} // namespace

/// Emit a function's Windows CE compressed .pdata entry.
///
/// The CE kernel's RtlLookupFunctionEntry decodes 8-byte
/// IMAGE_CE_RUNTIME_FUNCTION_ENTRY records (see ce600 PRIVATE/.../CORE/DLL/
/// exdsptch.c):
///   word0 = pFuncStart (absolute VA; thumb bit preserved)
///   word1 = PrologLen:8 | FuncLen:22 | ThirtyTwoBit:1 | ExceptionFlag:1
///
/// FuncLen/PrologLen are lengths in instructions (ARM=4B / Thumb=2B) and are
/// only known at link time, so the object file carries a 16-byte-per-function
/// intermediate form that lets the length relocations attach to their own
/// words:
///   [0] pFuncStart (ADDR32 = absolute VA)
///   [4] flags word (PrologLen:8 | FuncLen:22 | ThirtyTwoBit | ExceptionFlag)
///   [8] FUNCLEN pseudo reloc slot -> function end symbol
///   [12] PROLOG pseudo reloc slot -> prologue end symbol
/// lld resolves the internal pseudo relocations
/// (IMAGE_REL_ARM_CE_PDATA_FUNCLEN/PROLOG) into the flags word, drops them,
/// and compacts each record to the final 8-byte layout
/// (Writer::sortCEExceptionTable).
///
/// The PDATA_EH pair (personality VA + handler-data VA) is NOT emitted here:
/// the compiler places the SEH scope table followed by the pair immediately
/// before the function's first instruction (ARMAsmPrinter), which keeps the
/// pair in the 8 bytes right before pFuncStart exactly where the CE kernel
/// reads it -- unlike a shared section, this stays correct for any number of
/// SEH functions in a module.
///
/// CE needs no .xdata unwind codes: the OS unwinder (unwind.c) reverse-
/// executes the prolog machine code, so only function/prologue extents and
/// the handler location are recorded.
void ARMWinCOFFStreamer::CEEmitUnwindInfo(WinEH::FrameInfo *Frame) {
  if (!Frame || Frame->CEEmitted)
    return;
  if (Frame->empty()) {
    Frame->EmitAttempted = true;
    return;
  }
  Frame->CEEmitted = true;

  MCContext &Ctx = getContext();

  const bool HasHandler = Frame->HandlesExceptions && Frame->ExceptionHandler;
  const bool IsThumb = Frame->Function &&
                       getAssembler().isThumbFunc(Frame->Function);

  // A CE .pdata entry records the function and prologue extents; the OS
  // unwinder restores the frame by re-executing the prologue machine code
  // between them in reverse, so a missing end label would yield a corrupt
  // entry. Fail loudly instead of emitting one.
  const MCSymbol *FuncEnd = Frame->FuncletOrFuncEnd ? Frame->FuncletOrFuncEnd
                                                    : Frame->End;
  if (!FuncEnd || !Frame->PrologEnd) {
    StringRef FnName = Frame->Function ? Frame->Function->getName()
                                       : StringRef("<unknown>");
    getContext().reportError(
        SMLoc(), "CE unwind info for '" + Twine(FnName) +
                     "' requires .seh_endprologue and .seh_endproc");
    return;
  }

  MCSectionCOFF *PData = Ctx.getCOFFSection(
      ".pdata", COFF::IMAGE_SCN_CNT_INITIALIZED_DATA |
                    COFF::IMAGE_SCN_MEM_READ | COFF::IMAGE_SCN_MEM_DISCARDABLE);
  switchSection(PData);
  emitValueToAlignment(Align(4));

  // word0: pFuncStart (absolute VA; thumb bit preserved for thumb funcs).
  emitValue(MCSymbolRefExpr::create(Frame->Begin, Ctx), 4);

  // word1: static flags. The FuncLen / PrologLen bitfields (in instructions)
  // are linker-filled from the symbols that follow, emitted as internal
  // pseudo-relocations carrying the CE specifier; lld computes the lengths
  // relative to word0's pFuncStart, patches the bitfields, and discards
  // these relocations. Each pseudo reloc occupies its own 4-byte slot in the
  // object; lld recognises and removes the slots so the final entry stays
  // 8 bytes.
  uint32_t Static = 0;
  if (!IsThumb)
    Static |= CE_PDATA_THIRTY_TWO_BIT; // ARM (32-bit instructions)
  if (HasHandler)
    Static |= CE_PDATA_EXCEPTION_FLAG;
  emitIntValue(Static, 4);
  emitValue(MCSymbolRefExpr::create(
                FuncEnd, MCSymbolRefExpr::VK_COFF_CE_PDATA_FUNCLEN, Ctx),
            4);
  emitValue(MCSymbolRefExpr::create(
                Frame->PrologEnd, MCSymbolRefExpr::VK_COFF_CE_PDATA_PROLOG,
                Ctx),
            4);

  // Return to the function's text section so later emission is unaffected.
  switchSection(Frame->TextSection);
}

void ARMWinCOFFStreamer::emitWinEHHandlerData(SMLoc Loc) {
  MCStreamer::emitWinEHHandlerData(Loc);

  if (isCEEH()) {
    // Windows CE has no .xdata records: the SEH scope table and the in-text
    // PDATA_EH pair are emitted by the compiler immediately before the
    // function body (ARMAsmPrinter::emitFunctionEntryLabel), so there is
    // nothing to emit here and no section to switch to. Hand-written
    // assembly using .seh_handlerdata must place its own table + pair before
    // the function; the .pdata entry itself is still produced by
    // CEEmitUnwindInfo on .seh_endproc.
    return;
  }

  // We have to emit the unwind info now, because this directive
  // actually switches to the .xdata section!
  EHStreamer.EmitUnwindInfo(*this, getCurrentWinFrameInfo(),
                            /* HandlerData = */ true);
}

void ARMWinCOFFStreamer::emitWindowsUnwindTables(WinEH::FrameInfo *Frame) {
  if (isCEEH())
    CEEmitUnwindInfo(Frame);
  else
    EHStreamer.EmitUnwindInfo(*this, Frame, /* HandlerData = */ false);
}

void ARMWinCOFFStreamer::emitWindowsUnwindTables() {
  if (!getNumWinFrameInfos())
    return;
  if (isCEEH()) {
    for (const auto &CFI : getWinFrameInfos())
      CEEmitUnwindInfo(CFI.get());
    return;
  }
  EHStreamer.Emit(*this);
}

void ARMWinCOFFStreamer::finishImpl() {
  emitFrames();
  emitWindowsUnwindTables();

  MCWinCOFFStreamer::finishImpl();
}
}

MCStreamer *
llvm::createARMWinCOFFStreamer(MCContext &Context,
                               std::unique_ptr<MCAsmBackend> &&MAB,
                               std::unique_ptr<MCObjectWriter> &&OW,
                               std::unique_ptr<MCCodeEmitter> &&Emitter) {
  return new ARMWinCOFFStreamer(Context, std::move(MAB), std::move(Emitter),
                                std::move(OW));
}

namespace {
class ARMTargetWinCOFFStreamer : public llvm::ARMTargetStreamer {
public:
  ARMTargetWinCOFFStreamer(llvm::MCStreamer &S) : ARMTargetStreamer(S) {}

  ARMWinCOFFStreamer &getStreamer() {
    return static_cast<ARMWinCOFFStreamer &>(Streamer);
  }
  void emitThumbFunc(MCSymbol *Symbol) override;

  //===------------------------------------------------------------===//
  // ARM EHABI directives (Windows CE): route to the streamer's EHABI
  // state machine; on desktop Windows these directives are not used,
  // and the base-class (text/unsupported) behavior is kept.
  //===------------------------------------------------------------===//
  void emitFnStart() override;
  void emitFnEnd() override;
  void emitCantUnwind() override;
  void emitPersonality(const MCSymbol *Personality) override;
  void emitPersonalityIndex(unsigned Index) override;
  void emitHandlerData() override;
  void emitSetFP(MCRegister FpReg, MCRegister SpReg,
                 int64_t Offset = 0) override;
  void emitMovSP(MCRegister Reg, int64_t Offset = 0) override;
  void emitPad(int64_t Offset) override;
  void emitRegSave(const SmallVectorImpl<MCRegister> &RegList,
                   bool isVector) override;
  void emitUnwindRaw(int64_t Offset,
                     const SmallVectorImpl<uint8_t> &Opcodes) override;

  // The unwind codes on ARM Windows are documented at
  // https://docs.microsoft.com/en-us/cpp/build/arm-exception-handling
  void emitARMWinCFIAllocStack(unsigned Size, bool Wide) override;
  void emitARMWinCFISaveRegMask(unsigned Mask, bool Wide) override;
  void emitARMWinCFISaveSP(unsigned Reg) override;
  void emitARMWinCFISaveFRegs(unsigned First, unsigned Last) override;
  void emitARMWinCFISaveLR(unsigned Offset) override;
  void emitARMWinCFIPrologEnd(bool Fragment) override;
  void emitARMWinCFINop(bool Wide) override;
  void emitARMWinCFIEpilogStart(unsigned Condition) override;
  void emitARMWinCFIEpilogEnd() override;
  void emitARMWinCFICustom(unsigned Opcode) override;

private:
  void emitARMWinUnwindCode(unsigned UnwindCode, int Reg, int Offset);
};


//===----------------------------------------------------------------------===//
// ARM EHABI directive routing (Windows CE)
//===----------------------------------------------------------------------===//

void ARMTargetWinCOFFStreamer::emitFnStart() {
  ARMWinCOFFStreamer &S = getStreamer();
  if (S.isEHABI())
    S.EHABIemitFnStart();
}

void ARMTargetWinCOFFStreamer::emitFnEnd() {
  ARMWinCOFFStreamer &S = getStreamer();
  if (S.isEHABI())
    S.EHABIemitFnEnd();
}

void ARMTargetWinCOFFStreamer::emitCantUnwind() {
  ARMWinCOFFStreamer &S = getStreamer();
  if (S.isEHABI())
    S.EHABIemitCantUnwind();
}

void ARMTargetWinCOFFStreamer::emitPersonality(const MCSymbol *Personality) {
  ARMWinCOFFStreamer &S = getStreamer();
  if (S.isEHABI())
    S.EHABIemitPersonality(Personality);
}

void ARMTargetWinCOFFStreamer::emitPersonalityIndex(unsigned Index) {
  ARMWinCOFFStreamer &S = getStreamer();
  if (S.isEHABI())
    S.EHABIemitPersonalityIndex(Index);
}

void ARMTargetWinCOFFStreamer::emitHandlerData() {
  ARMWinCOFFStreamer &S = getStreamer();
  if (S.isEHABI())
    S.EHABIemitHandlerData();
}

void ARMTargetWinCOFFStreamer::emitSetFP(MCRegister FpReg, MCRegister SpReg,
                                         int64_t Offset) {
  ARMWinCOFFStreamer &S = getStreamer();
  if (S.isEHABI())
    S.EHABIemitSetFP(FpReg, SpReg, Offset);
}

void ARMTargetWinCOFFStreamer::emitMovSP(MCRegister Reg, int64_t Offset) {
  ARMWinCOFFStreamer &S = getStreamer();
  if (S.isEHABI())
    S.EHABIemitMovSP(Reg, Offset);
}

void ARMTargetWinCOFFStreamer::emitPad(int64_t Offset) {
  ARMWinCOFFStreamer &S = getStreamer();
  if (S.isEHABI())
    S.EHABIemitPad(Offset);
}

void ARMTargetWinCOFFStreamer::emitRegSave(
    const SmallVectorImpl<MCRegister> &RegList, bool isVector) {
  ARMWinCOFFStreamer &S = getStreamer();
  if (S.isEHABI())
    S.EHABIemitRegSave(RegList, isVector);
}

void ARMTargetWinCOFFStreamer::emitUnwindRaw(
    int64_t Offset, const SmallVectorImpl<uint8_t> &Opcodes) {
  ARMWinCOFFStreamer &S = getStreamer();
  if (S.isEHABI())
    S.EHABIemitUnwindRaw(Offset, Opcodes);
}

void ARMTargetWinCOFFStreamer::emitThumbFunc(MCSymbol *Symbol) {
  getStreamer().getAssembler().setIsThumbFunc(Symbol);
}

// Helper function to common out unwind code setup for those codes that can
// belong to both prolog and epilog.
void ARMTargetWinCOFFStreamer::emitARMWinUnwindCode(unsigned UnwindCode,
                                                    int Reg, int Offset) {
  auto &S = getStreamer();
  WinEH::FrameInfo *CurFrame = S.EnsureValidWinFrameInfo(SMLoc());
  if (!CurFrame)
    return;
  MCSymbol *Label = S.emitCFILabel();
  auto Inst = WinEH::Instruction(UnwindCode, Label, Reg, Offset);
  if (S.isInEpilogCFI())
    S.getCurrentWinEpilog()->Instructions.push_back(Inst);
  else
    CurFrame->Instructions.push_back(Inst);
}

void ARMTargetWinCOFFStreamer::emitARMWinCFIAllocStack(unsigned Size,
                                                       bool Wide) {
  unsigned Op = Win64EH::UOP_AllocSmall;
  if (!Wide) {
    if (Size / 4 > 0xffff)
      Op = Win64EH::UOP_AllocHuge;
    else if (Size / 4 > 0x7f)
      Op = Win64EH::UOP_AllocLarge;
  } else {
    Op = Win64EH::UOP_WideAllocMedium;
    if (Size / 4 > 0xffff)
      Op = Win64EH::UOP_WideAllocHuge;
    else if (Size / 4 > 0x3ff)
      Op = Win64EH::UOP_WideAllocLarge;
  }
  emitARMWinUnwindCode(Op, -1, Size);
}

void ARMTargetWinCOFFStreamer::emitARMWinCFISaveRegMask(unsigned Mask,
                                                        bool Wide) {
  assert(Mask != 0);
  int Lr = (Mask & 0x4000) ? 1 : 0;
  Mask &= ~0x4000;
  if (Wide)
    assert((Mask & ~0x1fff) == 0);
  else
    assert((Mask & ~0x00ff) == 0);
  if (Mask && ((Mask + (1 << 4)) & Mask) == 0) {
    if (Wide && (Mask & 0x1000) == 0 && (Mask & 0xff) == 0xf0) {
      // One continuous range from r4 to r8-r11
      for (int I = 11; I >= 8; I--) {
        if (Mask & (1 << I)) {
          emitARMWinUnwindCode(Win64EH::UOP_WideSaveRegsR4R11LR, I, Lr);
          return;
        }
      }
      // If it actually was from r4 to r4-r7, continue below.
    } else if (!Wide) {
      // One continuous range from r4 to r4-r7
      for (int I = 7; I >= 4; I--) {
        if (Mask & (1 << I)) {
          emitARMWinUnwindCode(Win64EH::UOP_SaveRegsR4R7LR, I, Lr);
          return;
        }
      }
      llvm_unreachable("logic error");
    }
  }
  Mask |= Lr << 14;
  if (Wide)
    emitARMWinUnwindCode(Win64EH::UOP_WideSaveRegMask, Mask, 0);
  else
    emitARMWinUnwindCode(Win64EH::UOP_SaveRegMask, Mask, 0);
}

void ARMTargetWinCOFFStreamer::emitARMWinCFISaveSP(unsigned Reg) {
  emitARMWinUnwindCode(Win64EH::UOP_SaveSP, Reg, 0);
}

void ARMTargetWinCOFFStreamer::emitARMWinCFISaveFRegs(unsigned First,
                                                      unsigned Last) {
  assert(First <= Last);
  assert(First >= 16 || Last < 16);
  assert(First <= 31 && Last <= 31);
  if (First == 8)
    emitARMWinUnwindCode(Win64EH::UOP_SaveFRegD8D15, Last, 0);
  else if (First <= 15)
    emitARMWinUnwindCode(Win64EH::UOP_SaveFRegD0D15, First, Last);
  else
    emitARMWinUnwindCode(Win64EH::UOP_SaveFRegD16D31, First, Last);
}

void ARMTargetWinCOFFStreamer::emitARMWinCFISaveLR(unsigned Offset) {
  emitARMWinUnwindCode(Win64EH::UOP_SaveLR, 0, Offset);
}

void ARMTargetWinCOFFStreamer::emitARMWinCFINop(bool Wide) {
  if (Wide)
    emitARMWinUnwindCode(Win64EH::UOP_WideNop, -1, 0);
  else
    emitARMWinUnwindCode(Win64EH::UOP_Nop, -1, 0);
}

void ARMTargetWinCOFFStreamer::emitARMWinCFIPrologEnd(bool Fragment) {
  auto &S = getStreamer();
  WinEH::FrameInfo *CurFrame = S.EnsureValidWinFrameInfo(SMLoc());
  if (!CurFrame)
    return;

  MCSymbol *Label = S.emitCFILabel();
  CurFrame->PrologEnd = Label;
  WinEH::Instruction Inst =
      WinEH::Instruction(Win64EH::UOP_End, /*Label=*/nullptr, -1, 0);
  auto it = CurFrame->Instructions.begin();
  CurFrame->Instructions.insert(it, Inst);
  CurFrame->Fragment = Fragment;
}

void ARMTargetWinCOFFStreamer::emitARMWinCFIEpilogStart(unsigned Condition) {
  auto &S = getStreamer();
  WinEH::FrameInfo *CurFrame = S.EnsureValidWinFrameInfo(SMLoc());
  if (!CurFrame)
    return;

  S.emitWinCFIBeginEpilogue();
  if (S.isInEpilogCFI()) {
    S.getCurrentWinEpilog()->Condition = Condition;
  }
}

void ARMTargetWinCOFFStreamer::emitARMWinCFIEpilogEnd() {
  auto &S = getStreamer();
  WinEH::FrameInfo *CurFrame = S.EnsureValidWinFrameInfo(SMLoc());
  if (!CurFrame)
    return;

  if (S.isInEpilogCFI()) {
    std::vector<WinEH::Instruction> &Epilog =
        S.getCurrentWinEpilog()->Instructions;

    unsigned UnwindCode = Win64EH::UOP_End;
    if (!Epilog.empty()) {
      WinEH::Instruction EndInstr = Epilog.back();
      if (EndInstr.Operation == Win64EH::UOP_Nop) {
        UnwindCode = Win64EH::UOP_EndNop;
        Epilog.pop_back();
      } else if (EndInstr.Operation == Win64EH::UOP_WideNop) {
        UnwindCode = Win64EH::UOP_WideEndNop;
        Epilog.pop_back();
      }
    }

    WinEH::Instruction Inst = WinEH::Instruction(UnwindCode, nullptr, -1, 0);
    S.getCurrentWinEpilog()->Instructions.push_back(Inst);
  }
  S.emitWinCFIEndEpilogue();
}

void ARMTargetWinCOFFStreamer::emitARMWinCFICustom(unsigned Opcode) {
  emitARMWinUnwindCode(Win64EH::UOP_Custom, 0, Opcode);
}

} // end anonymous namespace

MCTargetStreamer *llvm::createARMObjectTargetWinCOFFStreamer(MCStreamer &S) {
  return new ARMTargetWinCOFFStreamer(S);
}
