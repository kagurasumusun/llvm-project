//===- ARMWinCFI.h - Mixed ARM EHABI / WinCFI dispatch for WinCE --------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM
// Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// On Windows CE, C++ exceptions use ARM EHABI (.ARM.exidx/.ARM.extab) via
// this toolchain's own libunwind/libc++abi, while OS-level SEH (__try) must
// interoperate with the CE kernel's own unwinder, which needs
// WinEH::EncodingType::CE .pdata (see utils/wince/WINEH-ABI-FACTS.md). The
// target forces ExceptionHandling::ARM (EHABI) target-wide -- MCAsmInfo has
// no notion of "per function" -- so this header exists to make that
// decision per MachineFunction instead, without touching
// MCAsmInfo::usesWindowsCFI()/ExceptionsType itself (which multiple
// existing, already-working EHABI code paths key off of directly; flipping
// it target-wide was checked and confirmed to silently disable all EHABI
// unwind-directive emission -- see WINEH-ABI-FACTS.md section 4c).
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

/// Returns true if MF's prologue/epilogue unwind info should be emitted as
/// WinCFI (.seh_* directives feeding WinEH::EncodingType::CE .pdata, see
/// utils/wince/WINEH-ABI-FACTS.md sections 4d/4f/4g)
/// rather than ARM EHABI (.save/.setfp/.pad, .fnstart/.fnend).
///
/// - On non-CE Windows ARM/ARM64 targets this is unconditionally true for
///   every function, matching the pre-existing behavior driven by
///   MCAsmInfo::usesWindowsCFI() (this function changes nothing there).
/// - On Windows CE, this is decided per function: only functions whose IR
///   personality classifies as an SEH personality (i.e. compiled from MS
///   `__try` -- see clang/lib/CodeGen/CGException.cpp's
///   getSEHPersonalityMSVC(), which is the only thing that currently
///   produces MSVC_TableSEH/MSVC_X86SEH personalities on this target)
///   return true here; every other CE function (in particular, every
///   function using EHABI-based C++ exceptions today, which is the
///   overwhelming majority) returns exactly what it did before this
///   function existed (false), so this is additive-only for existing CE
///   code -- no currently-reachable EHABI function changes behavior.
inline bool functionUsesWinCFI(const MachineFunction &MF) {
  const MCAsmInfo *MAI = MF.getTarget().getMCAsmInfo();
  if (MAI->usesWindowsCFI())
    return true;

  if (!MF.getTarget().getTargetTriple().isWindowsCE())
    return false;
  if (MAI->getExceptionHandlingType() != ExceptionHandling::ARM)
    return false; // Unexpected config for this target; stay conservative.

  const Function &F = MF.getFunction();
  if (!F.hasPersonalityFn())
    return false;
  EHPersonality Personality = classifyEHPersonality(F.getPersonalityFn());
  return Personality == EHPersonality::MSVC_TableSEH ||
         Personality == EHPersonality::MSVC_X86SEH;
}

/// Returns true if emitPrologue must open a WinCFI (.seh_proc) frame for MF,
/// i.e. whether the function must end up with a CE .pdata entry.
///
/// On Windows CE the kernel dispatches and unwinds EVERY exception through
/// the compressed .pdata table (RtlLookupFunctionEntry / RtlVirtualUnwind in
/// CORE/DLL/exdsptch.c + ARM/unwind.c): a function without a .pdata entry
/// cannot be unwound by the OS at all, and unwinding stops at the first
/// such frame. eVC++ therefore emits a .pdata entry for every function
/// (with ExceptionFlag set only for __try functions). Match that: every CE
/// function gets a WinCFI frame, and -- in addition, not instead -- the ARM
/// EHABI .ARM.exidx entry that the toolchain's own C++ unwinder (libunwind)
/// needs; ARMException emits both tables for such functions.
///
/// Exceptions:
///  - GHC calling convention: emitPrologue returns before setting
///    hasWinCFI, so a forced frame would reach .seh_endproc without
///    .seh_endprologue and fail MC validation. GHC functions have no
///    prologue/epilogue at all; they stay EHABI-only (as before).
///  - Naked functions never run emitPrologue (PrologEpilogInserter skips
///    them entirely), so they never get a WinCFI frame -- same as before.
///
/// Everywhere else (ARMNT and non-Windows targets) this returns exactly
/// what needsWinCFI used to return (functionUsesWinCFI), so no behavior
/// changes outside Windows CE.
inline bool functionNeedsWinCFIFrame(const MachineFunction &MF) {
  if (!MF.getTarget().getTargetTriple().isWindowsCE())
    return functionUsesWinCFI(MF);
  return MF.getFunction().getCallingConv() != CallingConv::GHC;
}

} // end namespace llvm

#endif // LLVM_LIB_TARGET_ARM_ARMWINCFI_H
