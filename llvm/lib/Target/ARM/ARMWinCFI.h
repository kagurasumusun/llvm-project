//===-- ARMWinCFI.h - Windows unwind information for ARM --------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// Predicates that decide whether a function is described by the Windows unwind
// data of a PE image or by the ARM EHABI tables.
//
//===----------------------------------------------------------------------===//

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

/// Tests whether the function is described by the Windows unwind data of a PE
/// image, rather than by the ARM EHABI tables.  That is what the target's
/// assembler info asks for on the desktop Windows on ARM releases; a Windows CE
/// ARM image carries EHABI tables instead, and only the functions that are
/// given a Microsoft SEH personality take the Windows form.
inline bool functionUsesWinCFI(const MachineFunction &MF) {
  const MCAsmInfo *MAI = MF.getTarget().getMCAsmInfo();
  if (MAI->usesWindowsCFI())
    return true;

  if (!MF.getTarget().getTargetTriple().isOSWindowsCE() ||
      MAI->getExceptionHandlingType() != ExceptionHandling::ARM)
    return false;

  const Function &F = MF.getFunction();
  if (!F.hasPersonalityFn())
    return false;
  EHPersonality Personality = classifyEHPersonality(F.getPersonalityFn());
  return Personality == EHPersonality::MSVC_TableSEH ||
         Personality == EHPersonality::MSVC_X86SEH;
}

/// Tests whether the function keeps a frame on the stack that unwind data can
/// describe.  The GHC calling convention does not, so it is the one left out.
inline bool functionHasUnwindableFrame(const MachineFunction &MF) {
  return MF.getFunction().getCallingConv() != CallingConv::GHC;
}

/// Tests whether the function's frame has to be described by the unwind data
/// its image carries.  A Windows CE image describes every frame that can be
/// described, whatever form its data takes; elsewhere it is the Windows unwind
/// data that asks for one.
inline bool functionNeedsWinCFIFrame(const MachineFunction &MF) {
  if (MF.getTarget().getTargetTriple().isOSWindowsCE())
    return functionHasUnwindableFrame(MF);
  return functionUsesWinCFI(MF) && MF.getFunction().needsUnwindTableEntry();
}

}

#endif
