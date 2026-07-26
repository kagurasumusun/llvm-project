//===- AIRPrepare.cpp - Prepare LLVM Module for AIR bitcode encoding ------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// Strips attributes that Apple's Metal runtime (metalfe 32023.883) does not
// understand, and inserts no-op bitcasts for typed pointer reconstruction.
//
// Attribute whitelist is derived from real-device golden IR analysis:
//   https://github.com/kagurasumusun/metal-info
//   IR_GROUND_TRUTH.md §6.4 — observed function attributes in metalfe output
//
// metalfe 32023.883 is based on ~LLVM 15/16.  The highest observed
// ATTR_KIND in golden IR is NOSYNC (63).  Everything above that is
// stripped to prevent "Unknown attribute kind (N)" errors.
//
// IMPORTANT: This is a cleanroom implementation.  The attribute list is
// derived solely from empirical observation of metalfe output, not from
// Apple source code.
//
//===----------------------------------------------------------------------===//

#include "AIRWriter/AIRPointerTypeAnalysis.h"
#include "llvm/Target/AIR/AIRPrepare.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/IR/AttributeMask.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/Module.h"
#include "llvm/Pass.h"

using namespace llvm;
using namespace llvm::air;

namespace {

/// Returns true if \p Attr is one that Apple's Metal runtime accepts.
///
/// Derived from metalfe 32023.883 golden IR analysis (IR_GROUND_TRUTH.md §6.4).
/// The highest observed ATTR_KIND is NOSYNC (63).  We whitelist every kind up
/// to that value that makes sense for GPU code, plus a handful of structural
/// attributes that appear in standard LLVM IR and are silently ignored by
/// Metal's reader (e.g. Alignment).
///
/// IMPORTANT: This whitelist is based on empirical observation of metalfe
/// output, not on Apple source code.  It is a cleanroom implementation.
constexpr bool isValidForAIR(Attribute::AttrKind Attr) {
  switch (Attr) {
  // ---- Structural / layout ----
  case Attribute::None:
  case Attribute::Alignment:
  case Attribute::StackAlignment:
  case Attribute::ByVal:
  case Attribute::StructRet:
  case Attribute::InAlloca:
  case Attribute::ByRef:
  case Attribute::Returned:
  case Attribute::InReg:

  // ---- Inlining ----
  case Attribute::AlwaysInline:
  case Attribute::InlineHint:
  case Attribute::NoInline:
  case Attribute::MinSize:
  case Attribute::OptimizeForSize:
  case Attribute::OptimizeNone:

  // ---- Control flow / side-effect ----
  case Attribute::NoReturn:
  case Attribute::NoUnwind:
  case Attribute::Convergent:
  case Attribute::NoDuplicate:
  case Attribute::NoRecurse:
  case Attribute::WillReturn:
  case Attribute::NoFree:
  case Attribute::NoSync:
  case Attribute::MustProgress:

  // ---- Memory effects ----
  case Attribute::ReadNone:   // 20 — observed in golden
  case Attribute::ReadOnly:   // 21
  case Attribute::WriteOnly:  // 52
  case Attribute::ArgMemOnly: // 45 — observed in golden
  case Attribute::InaccessibleMemOnly:
  case Attribute::InaccessibleMemOrArgMemOnly:

  // ---- Pointer / value ----
  case Attribute::NonNull:
  case Attribute::Dereferenceable:
  case Attribute::DereferenceableOrNull:
  case Attribute::NoAlias:
  case Attribute::NoCapture:
  case Attribute::NoBuiltin:
  case Attribute::ImmArg:
  case Attribute::NounDef:
  case Attribute::NullPointerIsValid:
  case Attribute::NoCfCheck:

  // ---- Sign / extension ----
  case Attribute::SExt:
  case Attribute::ZExt:

  // ---- ABI / calling convention ----
  case Attribute::UWTable:
  case Attribute::Naked:
  case Attribute::NoImplicitFloat:
  case Attribute::Cold:
  case Attribute::Hot:
  case Attribute::Builtin:
  case Attribute::NoProfile:
  case Attribute::Speculatable:
  case Attribute::StrictFP:

  // ---- Security ----
  case Attribute::SafeStack:
  case Attribute::ShadowCallStack:
  case Attribute::SanitizeAddress:
  case Attribute::SanitizeThread:
  case Attribute::SanitizeMemory:
    return true;

  default:
    // Strip everything else — including:
    //   MEMORY (86), NOFPCLASS (87), OPTIMIZE_FOR_DEBUGGING (88),
    //   WRITABLE (89), CORO_ONLY_DESTROY_WHEN_COMPLETE (90),
    //   DEAD_ON_UNWIND (91), RANGE (92), and all subsequent.
    //
    // These are LLVM >=17 additions that Apple's metalfe 32023.883
    // (based on ~LLVM 15/16) does not understand.
    return false;
  }
}

/// String attribute keys emitted by Apple's metalfe 32023.883.
/// Derived from golden IR analysis (IR_GROUND_TRUTH.md §6.4).
static bool isMetalStringAttrValid(StringRef Key) {
  // From golden IR §6.4:
  // - "approx-func-fp-math"="true" (fast-math mode)
  // - "unsafe-fp-math"="true" (fast-math mode)
  // - "no-builtins" (builtin suppression)
  // - "frame-pointer" (frame pointer control)
  // - "air-buffer-no-alias" (buffer alias hint)
  return Key == "approx-func-fp-math" ||
         Key == "correctly-rounded-divide-sqrt-fp-math" ||
         Key == "frame-pointer" ||
         Key == "less-precise-fpmad" ||
         Key == "no-infs-fp-math" ||
         Key == "no-nans-fp-math" ||
         Key == "no-signed-zeros-fp-math" ||
         Key == "no-trapping-math" ||
         Key == "stack-protector-buffer-size" ||
         Key == "target-cpu" ||
         Key == "target-features" ||
         Key == "unsafe-fp-math" ||
         Key == "use-soft-float" ||
         Key == "no-builtins" ||
         // AIR-specific string attributes observed in golden IR:
         Key == "air-buffer-no-alias";
}

static void stripUnsupportedAttributes(Module &M) {
  // Build mask of unsupported enum attributes.
  AttributeMask AttrMask;
  for (unsigned I = Attribute::None; I != Attribute::EndAttrKinds; ++I) {
    auto Kind = static_cast<Attribute::AttrKind>(I);
    if (!isValidForAIR(Kind))
      AttrMask.addAttribute(Kind);
  }

  auto stripUnknownStringAttrs = [](AttributeList &AL) -> AttributeList {
    AttributeSet Clean;
    for (unsigned Idx : AL.indexes()) {
      AttributeSet AS = AL.getAttributes(Idx);
      AttributeSet NewAS;
      for (auto It = AS.begin(), End = AS.end(); It != End; ++It) {
        if (It->isStringAttribute()) {
          if (isMetalStringAttrValid(It->getKindAsString()))
            NewAS = NewAS.addAttribute(It->getContext(), *It);
        } else {
          NewAS = NewAS.addAttribute(It->getContext(), *It);
        }
      }
      if (NewAS.hasAttributes())
        Clean = Clean.addAttributesAtIndex(AL.getContext(), Idx, NewAS);
    }
    return Clean;
  };

  for (auto &F : M.functions()) {
    F.removeFnAttrs(AttrMask);
    F.removeRetAttrs(AttrMask);
    for (size_t Idx = 0, End = F.arg_size(); Idx < End; ++Idx)
      F.removeParamAttrs(Idx, AttrMask);
    F.setAttributes(stripUnknownStringAttrs(F.getAttributes()));

    for (auto &BB : F) {
      for (auto &I : BB) {
        if (auto *CB = dyn_cast<CallBase>(&I)) {
          CB->removeFnAttrs(AttrMask);
          CB->removeRetAttrs(AttrMask);
          for (size_t Idx = 0, End = CB->arg_size(); Idx < End; ++Idx)
            CB->removeParamAttrs(Idx, AttrMask);
          CB->setAttributes(stripUnknownStringAttrs(CB->getAttributes()));
        }
      }
    }
  }
}

static Value *maybeGenerateBitcast(IRBuilder<> &Builder,
                                   PointerTypeMap &PointerTypes,
                                   Instruction &Inst, Value *Operand,
                                   Type *Ty) {
  auto It = PointerTypes.find(Operand);
  if (It != PointerTypes.end()) {
    auto *OpTy = cast<TypedPointerType>(It->second)->getElementType();
    if (OpTy == Ty)
      return nullptr;
  }

  Type *ValTy = Operand->getType();
  if (auto *GlobalVar = dyn_cast<GlobalVariable>(Operand))
    ValTy = GlobalVar->getValueType();
  if (auto *AI = dyn_cast<AllocaInst>(Operand))
    ValTy = AI->getAllocatedType();

  if (auto *ArrTy = dyn_cast<ArrayType>(ValTy)) {
    Type *ElTy = ArrTy->getElementType();
    if (ElTy == Ty)
      return nullptr;
  }

  if (ConstantExpr *GEPInstr = dyn_cast<ConstantExpr>(Operand)) {
    while (GEPInstr->getOpcode() == Instruction::GetElementPtr) {
      Value *OpArg = GEPInstr->getOperand(0);
      if (ConstantExpr *NewGEPInstr = dyn_cast<ConstantExpr>(OpArg)) {
        GEPInstr = NewGEPInstr;
        continue;
      }
      if (auto *GlobalVar = dyn_cast<GlobalVariable>(OpArg))
        ValTy = GlobalVar->getValueType();
      if (auto *AI = dyn_cast<AllocaInst>(Operand))
        ValTy = AI->getAllocatedType();
      if (auto *ArrTy = dyn_cast<ArrayType>(ValTy)) {
        Type *ElTy = ArrTy->getElementType();
        if (ElTy == Ty)
          return nullptr;
      }
      break;
    }
  }

  Builder.SetInsertPoint(&Inst);
  PointerType *PtrTy = cast<PointerType>(Operand->getType());
  return Builder.Insert(
      CastInst::Create(Instruction::BitCast, Operand,
                       Builder.getPtrTy(PtrTy->getPointerAddressSpace())));
}

class AIRPrepareModule : public ModulePass {
public:
  static char ID;
  AIRPrepareModule() : ModulePass(ID) {}

  bool runOnModule(Module &M) override {
    PointerTypeMap PointerTypes = computePointerTypeMap(M);

    // 1. Strip unsupported attributes.
    stripUnsupportedAttributes(M);

    // 2. Insert no-op bitcasts for typed pointer reconstruction.
    for (auto &F : M.functions()) {
      for (auto &BB : F) {
        IRBuilder<> Builder(&BB);
        for (auto &I : make_early_inc_range(BB)) {
          if (isa<CallBase>(&I))
            continue;

          if (auto *LI = dyn_cast<LoadInst>(&I)) {
            if (Value *BC = maybeGenerateBitcast(
                    Builder, PointerTypes, I, LI->getPointerOperand(),
                    LI->getType())) {
              LI->replaceAllUsesWith(Builder.CreateLoad(LI->getType(), BC));
              LI->eraseFromParent();
            }
            continue;
          }
          if (auto *SI = dyn_cast<StoreInst>(&I)) {
            if (Value *BC = maybeGenerateBitcast(
                    Builder, PointerTypes, I, SI->getPointerOperand(),
                    SI->getValueOperand()->getType())) {
              SI->replaceAllUsesWith(
                  Builder.CreateStore(SI->getValueOperand(), BC));
              SI->eraseFromParent();
            }
            continue;
          }
          if (auto *GEP = dyn_cast<GetElementPtrInst>(&I)) {
            if (Value *BC = maybeGenerateBitcast(
                    Builder, PointerTypes, I, GEP->getPointerOperand(),
                    GEP->getSourceElementType()))
              GEP->setOperand(0, BC);
            continue;
          }
        }
      }
    }
    return true;
  }

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.setPreservesAll();
  }
};

char AIRPrepareModule::ID = 0;

} // end anonymous namespace

ModulePass *llvm::createAIRPrepareModulePass() {
  return new AIRPrepareModule();
}
