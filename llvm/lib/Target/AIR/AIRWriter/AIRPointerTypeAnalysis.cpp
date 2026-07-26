//===- AIRPointerTypeAnalysis.cpp - PointerType analysis for AIR ----------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// Analysis pass to assign typed pointer types to opaque pointers in a Module
// targeting AIR (Metal IR).  Forked from DirectX's PointerTypeAnalysis and
// adapted for Metal address spaces.
//
//===----------------------------------------------------------------------===//

#include "AIRPointerTypeAnalysis.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/GlobalVariable.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Operator.h"

using namespace llvm;
using namespace llvm::air;

namespace {

Type *classifyFunctionType(const Function &F, PointerTypeMap &Map);

// Classifies the type of the value passed in by walking the value's users to
// find a typed instruction to materialize a type from.
Type *classifyPointerType(const Value *V, PointerTypeMap &Map) {
  assert(V->getType()->isPointerTy() &&
         "classifyPointerType called with non-pointer");

  if (const Function *F = dyn_cast<Function>(V))
    return classifyFunctionType(*F, Map);

  // Remove dead constant users from global variables.
  if (const GlobalVariable *GV = dyn_cast<GlobalVariable>(V))
    GV->removeDeadConstantUsers();

  auto It = Map.find(V);
  if (It != Map.end())
    return It->second;

  Type *PointeeTy = nullptr;
  if (auto *GEP = dyn_cast<GEPOperator>(V)) {
    if (!GEP->getResultElementType()->isPointerTy())
      PointeeTy = GEP->getResultElementType();
  } else if (auto *Inst = dyn_cast<AllocaInst>(V)) {
    PointeeTy = Inst->getAllocatedType();
  } else if (auto *GV = dyn_cast<GlobalVariable>(V)) {
    PointeeTy = GV->getValueType();
  }

  for (const auto *User : V->users()) {
    Type *NewPointeeTy = nullptr;
    if (const auto *Inst = dyn_cast<LoadInst>(User)) {
      NewPointeeTy = Inst->getType();
    } else if (const auto *Inst = dyn_cast<StoreInst>(User)) {
      NewPointeeTy = Inst->getValueOperand()->getType();
      if (NewPointeeTy->isPointerTy())
        continue;
    } else if (const auto *GEP = dyn_cast<GEPOperator>(User)) {
      NewPointeeTy = GEP->getSourceElementType();
    }
    if (NewPointeeTy) {
      if (NewPointeeTy->isPointerTy()) {
        PointeeTy = classifyPointerType(User, Map);
        break;
      }
      if (!PointeeTy)
        PointeeTy = NewPointeeTy;
      else if (PointeeTy != NewPointeeTy)
        PointeeTy = Type::getInt8Ty(V->getContext());
    }
  }
  // If we were unable to determine the pointee type, default to i8.
  if (!PointeeTy)
    PointeeTy = Type::getInt8Ty(V->getContext());

  auto *TypedPtrTy =
      TypedPointerType::get(PointeeTy, V->getType()->getPointerAddressSpace());
  Map[V] = TypedPtrTy;
  return TypedPtrTy;
}

Type *classifyFunctionType(const Function &F, PointerTypeMap &Map) {
  auto It = Map.find(&F);
  if (It != Map.end())
    return It->second;

  SmallVector<Type *, 8> NewArgs;
  Type *RetTy = F.getReturnType();
  LLVMContext &Ctx = F.getContext();
  if (RetTy->isPointerTy()) {
    RetTy = nullptr;
    for (const auto &B : F) {
      const auto *RetInst = dyn_cast_or_null<ReturnInst>(B.getTerminator());
      if (!RetInst)
        continue;

      Type *NewRetTy = classifyPointerType(RetInst->getReturnValue(), Map);
      if (!RetTy)
        RetTy = NewRetTy;
      else if (RetTy != NewRetTy)
        RetTy = TypedPointerType::get(
            Type::getInt8Ty(Ctx), F.getReturnType()->getPointerAddressSpace());
    }
    if (!RetTy)
      RetTy = TypedPointerType::get(
          Type::getInt8Ty(Ctx), F.getReturnType()->getPointerAddressSpace());
  }
  for (auto &A : F.args()) {
    Type *ArgTy = A.getType();
    if (ArgTy->isPointerTy())
      ArgTy = classifyPointerType(&A, Map);
    NewArgs.push_back(ArgTy);
  }
  auto *TypedPtrTy =
      TypedPointerType::get(FunctionType::get(RetTy, NewArgs, false), 0);
  Map[&F] = TypedPtrTy;
  return TypedPtrTy;
}

static Type *classifyConstantWithOpaquePtr(const Constant *C,
                                           PointerTypeMap &Map) {
  if (isa<ConstantPointerNull>(C))
    return TypedPointerType::get(Type::getInt8Ty(C->getContext()),
                                 C->getType()->getPointerAddressSpace());

  if (isa<ConstantData>(C))
    return C->getType();

  auto It = Map.find(C);
  if (It != Map.end())
    return It->second;

  if (const auto *F = dyn_cast<Function>(C))
    return classifyFunctionType(*F, Map);

  Type *Ty = C->getType();
  Type *TargetTy = nullptr;
  if (auto *CS = dyn_cast<ConstantStruct>(C)) {
    SmallVector<Type *> EltTys;
    for (unsigned int I = 0; I < CS->getNumOperands(); ++I) {
      const Constant *Elt = C->getAggregateElement(I);
      Type *EltTy = classifyConstantWithOpaquePtr(Elt, Map);
      EltTys.emplace_back(EltTy);
    }
    TargetTy = StructType::get(C->getContext(), EltTys);
  } else if (auto *CA = dyn_cast<ConstantAggregate>(C)) {
    Type *TargetEltTy = nullptr;
    for (auto &Elt : CA->operands()) {
      Type *EltTy = classifyConstantWithOpaquePtr(cast<Constant>(&Elt), Map);
      assert(TargetEltTy == EltTy || TargetEltTy == nullptr);
      TargetEltTy = EltTy;
    }
    if (auto *AT = dyn_cast<ArrayType>(Ty)) {
      TargetTy = ArrayType::get(TargetEltTy, AT->getNumElements());
    } else {
      auto *VT = cast<VectorType>(Ty);
      TargetTy = VectorType::get(TargetEltTy, VT);
    }
  }
  assert(TargetTy && "PointerTypeAnalysis failed to identify target type");

  if (TargetTy == Ty)
    return Ty;

  Map[C] = TargetTy;
  return TargetTy;
}

static void classifyGlobalCtorPointerType(const GlobalVariable &GV,
                                          PointerTypeMap &Map) {
  const auto *CA = cast<ConstantArray>(GV.getInitializer());
  Type *CtorArrayTy = classifyConstantWithOpaquePtr(CA, Map);
  Map[&GV] = TypedPointerType::get(CtorArrayTy,
                                   GV.getType()->getPointerAddressSpace());
}

} // anonymous namespace

PointerTypeMap llvm::air::computePointerTypeMap(const Module &M) {
  PointerTypeMap Map;
  for (auto &G : M.globals()) {
    if (G.getType()->isPointerTy())
      classifyPointerType(&G, Map);
    if (G.getName() == "llvm.global_ctors")
      classifyGlobalCtorPointerType(G, Map);
  }

  for (auto &F : M) {
    classifyFunctionType(F, Map);

    for (const auto &B : F) {
      for (const auto &I : B) {
        if (I.getType()->isPointerTy())
          classifyPointerType(&I, Map);
        for (const auto &O : I.operands())
          if (O.get()->getType()->isPointerTy())
            classifyPointerType(O.get(), Map);
      }
    }
  }
  return Map;
}
