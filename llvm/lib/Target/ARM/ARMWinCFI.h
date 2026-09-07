#ifndef LLVM_LIB_TARGET_ARM_ARMWINCFI_H
#define LLVM_LIB_TARGET_ARM_ARMWINCFI_H

#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/IR/CallingConv.h"
#include "llvm/IR/EHPersonalities.h"
#include "llvm/IR/Function.h"
#include "llvm/MC/MCAsmInfo.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/TargetParser/Triple.h"

namespace llvm {

inline bool functionUsesWinCFI(const MachineFunction &MF) {
  const MCAsmInfo *MAI = MF.getTarget().getMCAsmInfo();
  if (MAI->usesWindowsCFI())
    return true;

  if (!MF.getTarget().getTargetTriple().isWindowsCE())
    return false;
  if (MAI->getExceptionHandlingType() != ExceptionHandling::ARM)
    return false;

  const Function &F = MF.getFunction();
  if (!F.hasPersonalityFn())
    return false;
  EHPersonality Personality = classifyEHPersonality(F.getPersonalityFn());
  return Personality == EHPersonality::MSVC_TableSEH ||
         Personality == EHPersonality::MSVC_X86SEH;
}

inline bool functionNeedsWinCFIFrame(const MachineFunction &MF) {
  if (!MF.getTarget().getTargetTriple().isWindowsCE())
    return functionUsesWinCFI(MF);
  return MF.getFunction().getCallingConv() != CallingConv::GHC;
}

}

#endif
