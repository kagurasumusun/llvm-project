//===- TypedPointerAnalysis.cpp - typed-pointer bitcode support -----------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "TypedPointerAnalysis.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/GlobalAlias.h"
#include "llvm/IR/GlobalIFunc.h"
#include "llvm/IR/GlobalVariable.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Operator.h"
#include "llvm/IR/TypedPointerType.h"

using namespace llvm;

/// Maximum recursion depth when chasing pointer values through
/// bitcast/phi/select chains while collecting pointee evidence.
static constexpr unsigned MaxEvidenceDepth = 8;

TypedPointerAnalysis::TypedPointerAnalysis(const Module &M)
    : M(M), Ctx(M.getContext()) {
  I8Ty = Type::getInt8Ty(Ctx);
  I8PtrTy = TypedPointerType::get(I8Ty, 0);
  SynthTypes.insert(cast<TypedPointerType>(I8PtrTy));
}

Type *TypedPointerAnalysis::makeTyped(Type *Pointee, unsigned AddrSpace) {
  if (!PointerType::isValidElementType(Pointee))
    Pointee = I8Ty;
  Type *TP = TypedPointerType::get(Pointee, AddrSpace);
  SynthTypes.insert(TP);
  return TP;
}

//===----------------------------------------------------------------------===//
// Structural helpers
//===----------------------------------------------------------------------===//

bool TypedPointerAnalysis::containsPointerTypes(Type *Ty) {
  if (SynthTypes.contains(Ty))
    return true;
  SmallVector<Type *, 8> Worklist;
  DenseSet<Type *> Visited;
  Worklist.push_back(Ty);
  while (!Worklist.empty()) {
    Type *T = Worklist.pop_back_val();
    if (!Visited.insert(T).second)
      continue;
    if (auto It = HasPtrMemo.find(T); It != HasPtrMemo.end()) {
      if (It->second)
        return true;
      continue;
    }
    if (SynthTypes.contains(T) || T->isPointerTy()) {
      HasPtrMemo[Ty] = true;
      return true;
    }
    if (auto *FT = dyn_cast<FunctionType>(T)) {
      Worklist.push_back(FT->getReturnType());
      for (Type *PT : FT->params())
        Worklist.push_back(PT);
      continue;
    }
    if (auto *ST = dyn_cast<StructType>(T)) {
      if (!ST->isOpaque())
        for (Type *ET : ST->elements())
          Worklist.push_back(ET);
      continue;
    }
    if (auto *AT = dyn_cast<ArrayType>(T)) {
      Worklist.push_back(AT->getElementType());
      continue;
    }
    if (auto *VT = dyn_cast<VectorType>(T)) {
      Worklist.push_back(VT->getElementType());
      continue;
    }
    HasPtrMemo[T] = false;
  }
  HasPtrMemo[Ty] = false;
  return false;
}

Type *TypedPointerAnalysis::joinEvidence(ArrayRef<Type *> Ev) const {
  Type *Best = nullptr;
  for (Type *T : Ev) {
    if (!T)
      continue;
    if (!Best) {
      Best = T;
      continue;
    }
    if (T != Best)
      return I8Ty; // conflicting evidence
  }
  return Best ? Best : I8Ty;
}

//===----------------------------------------------------------------------===//
// Pointee resolution
//===----------------------------------------------------------------------===//

void TypedPointerAnalysis::collectUseEvidence(const Value *V,
                                              SmallVectorImpl<Type *> &Out,
                                              unsigned Depth) {
  if (Depth > MaxEvidenceDepth)
    return;

  for (const Use &U : V->uses()) {
    const User *Usr = U.getUser();

    if (const auto *LI = dyn_cast<LoadInst>(Usr)) {
      if (LI->getPointerOperand() == V)
        Out.push_back(remap(LI->getType(), LI));
      continue;
    }
    if (const auto *SI = dyn_cast<StoreInst>(Usr)) {
      if (SI->getPointerOperand() == V) {
        const Value *Val = SI->getValueOperand();
        Out.push_back(remap(Val->getType(), Val));
      }
      continue;
    }
    if (const auto *GEP = dyn_cast<GEPOperator>(Usr)) {
      if (GEP->getPointerOperand() == V)
        Out.push_back(remapAggregate(
            const_cast<GEPOperator *>(GEP)->getSourceElementType()));
      continue;
    }
    if (const auto *RMW = dyn_cast<AtomicRMWInst>(Usr)) {
      if (RMW->getPointerOperand() == V) {
        const Value *Val = RMW->getValOperand();
        Out.push_back(remap(Val->getType(), Val));
      }
      continue;
    }
    if (const auto *CXI = dyn_cast<AtomicCmpXchgInst>(Usr)) {
      if (CXI->getPointerOperand() == V) {
        const Value *Val = CXI->getCompareOperand();
        Out.push_back(remap(Val->getType(), Val));
      }
      continue;
    }
    if (const auto *CB = dyn_cast<CallBase>(Usr)) {
      if (CB->isCallee(&U))
        continue; // V is the callee; not a pointer-slot use.
      if (!CB->isArgOperand(&U))
        continue; // Operand bundle inputs (gc-live, deopt, ...) carry no
                  // pointee evidence for the called signature.
      unsigned ArgNo = CB->getArgOperandNo(&U);
      const Function *G =
          dyn_cast<Function>(CB->getCalledOperand()->stripPointerCasts());
      if (G && ArgNo < G->getFunctionType()->getNumParams())
        Out.push_back(resolveParamPointee(G, ArgNo));
      continue;
    }
    if (const auto *RI = dyn_cast<ReturnInst>(Usr)) {
      Out.push_back(resolveRetPointee(RI->getFunction()));
      continue;
    }
    if (isa<BitCastInst>(Usr) || isa<AddrSpaceCastInst>(Usr) ||
        isa<PHINode>(Usr) || isa<SelectInst>(Usr) || isa<FreezeInst>(Usr)) {
      Out.push_back(resolvePointee(cast<Value>(Usr)));
      continue;
    }
    if (const auto *CE = dyn_cast<ConstantExpr>(Usr)) {
      if (CE->getOpcode() == Instruction::BitCast ||
          CE->getOpcode() == Instruction::AddrSpaceCast)
        Out.push_back(resolvePointee(CE));
      continue;
    }
    // Anything else (icmp, inttoptr, insertvalue, ...) gives no evidence.
  }
}

Type *TypedPointerAnalysis::resolvePointee(const Value *V) {
  if (auto It = PointeeMemo.find(V); It != PointeeMemo.end())
    return It->second;
  if (!PointeeInProgress.insert(V).second)
    return fallbackPointee(); // recursive reference: break with i8.

  Type *R = nullptr;

  if (const auto *GV = dyn_cast<GlobalVariable>(V)) {
    Type *VT = GV->getValueType();
    if (VT->isPointerTy() && GV->hasInitializer())
      // The value held by the global is itself a pointer: its pointee is
      // whatever the initializer points at.
      R = resolvePointee(GV->getInitializer());
    else
      R = remap(VT, GV);
  } else if (const auto *F = dyn_cast<Function>(V)) {
    R = synthesizeFunctionType(F);
  } else if (const auto *GA = dyn_cast<GlobalAlias>(V)) {
    if (const GlobalObject *GO = GA->getAliaseeObject())
      R = resolvePointee(GO);
    else
      R = remapAggregate(const_cast<GlobalAlias *>(GA)->getValueType());
  } else if (const auto *GIF = dyn_cast<GlobalIFunc>(V)) {
    R = remapAggregate(const_cast<GlobalIFunc *>(GIF)->getValueType());
  } else if (const auto *AI = dyn_cast<AllocaInst>(V)) {
    Type *ET = AI->getAllocatedType();
    if (ET->isPointerTy()) {
      // Slot holds a pointer: resolve via uses.
      SmallVector<Type *, 8> Ev;
      collectUseEvidence(V, Ev, 0);
      R = joinEvidence(Ev);
    } else {
      R = remapAggregate(ET);
    }
  } else if (const auto *GEP = dyn_cast<GEPOperator>(V)) {
    Type *TermET = computeGEPTerminalElementType(GEP);
    if (TermET->isPointerTy()) {
      // The GEP result points at a pointer slot: what that pointer itself
      // points at is only visible through uses.
      SmallVector<Type *, 8> Ev;
      collectUseEvidence(V, Ev, 0);
      R = joinEvidence(Ev);
    } else {
      R = remapAggregate(TermET);
    }
  } else if (const auto *CE = dyn_cast<ConstantExpr>(V)) {
    switch (CE->getOpcode()) {
    case Instruction::BitCast:
    case Instruction::AddrSpaceCast:
      R = resolvePointee(CE->getOperand(0));
      break;
    case Instruction::IntToPtr:
      R = I8Ty;
      break;
    default: {
      SmallVector<Type *, 8> Ev;
      collectUseEvidence(V, Ev, 0);
      R = joinEvidence(Ev);
      break;
    }
    }
  } else if (isa<ConstantPointerNull>(V) || isa<UndefValue>(V) ||
             isa<BlockAddress>(V) || isa<DSOLocalEquivalent>(V)) {
    R = I8Ty;
  } else if (isa<Constant>(V)) {
    // Anything left over (ConstantPtrAuth etc.) gets the fallback treatment.
    R = I8Ty;
  } else if (const auto *CB = dyn_cast<CallBase>(V)) {
    if (const Function *G =
            dyn_cast<Function>(CB->getCalledOperand()->stripPointerCasts()))
      R = resolveRetPointee(G);
    else {
      SmallVector<Type *, 8> Ev;
      collectUseEvidence(V, Ev, 0);
      R = joinEvidence(Ev);
    }
  } else if (const auto *A = dyn_cast<Argument>(V)) {
    R = resolveParamPointee(A->getParent(), A->getArgNo());
  } else {
    // Generic instructions (loads, phis, selects, extracts, ...): the pointee
    // is visible only through uses.
    SmallVector<Type *, 8> Ev;
    collectUseEvidence(V, Ev, 0);
    R = joinEvidence(Ev);
  }

  PointeeInProgress.erase(V);
  PointeeMemo[V] = R ? R : I8Ty;
  return PointeeMemo[V];
}

Type *TypedPointerAnalysis::resolveParamPointee(const Function *F,
                                                unsigned ArgNo) {
  auto Key = std::make_pair(F, ArgNo);
  if (auto It = ParamMemo.find(Key); It != ParamMemo.end())
    return It->second;
  if (!ParamInProgress.insert(Key).second)
    return fallbackPointee();

  SmallVector<Type *, 8> Ev;
  // Evidence from the function body.
  Function *MutF = const_cast<Function *>(F);
  if (Argument *A = MutF->getArg(ArgNo))
    collectUseEvidence(A, Ev, 0);

  // Evidence from direct callers.
  for (const Use &U : MutF->uses()) {
    const auto *CB = dyn_cast<CallBase>(U.getUser());
    if (!CB)
      continue;
    if (dyn_cast<Function>(CB->getCalledOperand()->stripPointerCasts()) != F)
      continue;
    if (ArgNo >= CB->arg_size())
      continue;
    const Value *ArgVal = CB->getArgOperand(ArgNo);
    if (!ArgVal->getType()->isPointerTy())
      continue;
    Ev.push_back(resolvePointee(ArgVal));
  }

  Type *R = joinEvidence(Ev);
  ParamInProgress.erase(Key);
  ParamMemo[Key] = R;
  return R;
}

Type *TypedPointerAnalysis::resolveRetPointee(const Function *F) {
  if (auto It = RetMemo.find(F); It != RetMemo.end())
    return It->second;
  if (!RetInProgress.insert(const_cast<Function *>(F)).second)
    return fallbackPointee();

  SmallVector<Type *, 8> Ev;
  if (!F->isDeclaration()) {
    for (const BasicBlock &BB : *F)
      if (const auto *RI = dyn_cast<ReturnInst>(BB.getTerminator()))
        if (const Value *RV = RI->getReturnValue())
          if (RV->getType()->isPointerTy())
            Ev.push_back(resolvePointee(RV));
  }
  for (const Use &U : const_cast<Function *>(F)->uses()) {
    const auto *CB = dyn_cast<CallBase>(U.getUser());
    if (!CB)
      continue;
    collectUseEvidence(CB, Ev, 0);
  }

  Type *R = joinEvidence(Ev);
  RetInProgress.erase(const_cast<Function *>(F));
  RetMemo[F] = R;
  return R;
}

//===----------------------------------------------------------------------===//
// Struct field resolution
//===----------------------------------------------------------------------===//

void TypedPointerAnalysis::gatherStructFieldEvidence() {
  if (FieldEvidenceGathered)
    return;
  FieldEvidenceGathered = true;

  auto AddCand = [this](Type *ST, unsigned Idx, Type *C) {
    if (!C)
      return;
    auto &Vec = FieldCands[std::make_pair(ST, Idx)];
    for (Type *T : Vec)
      if (T == C)
        return;
    Vec.push_back(C);
  };

  // Evidence 1: GEPs (instructions and constant expressions) whose source
  // element type is a struct reveal the pointee of pointer fields through
  // the uses of the GEP result.
  std::function<void(const GEPOperator *)> ScanGEP =
      [this, &AddCand](const GEPOperator *GEP) {
        auto *ST = dyn_cast<StructType>(GEP->getSourceElementType());
        if (!ST || ST->isOpaque())
          return;
        // Canonical struct-field GEP (LLVM 16 form):
        //   getelementptr %ST, ptr %base, i32 0, i32 FieldIdx
        // -- 3 operands, a zero leading index, then the field index.
        if (GEP->getNumOperands() != 3)
          return;
        const auto *Zero = dyn_cast<ConstantInt>(GEP->getOperand(1));
        const auto *CI = dyn_cast<ConstantInt>(GEP->getOperand(2));
        if (!Zero || !Zero->isZero() || !CI)
          return;
        unsigned Idx = CI->getZExtValue();
        if (Idx >= ST->getNumElements() ||
            !ST->getElementType(Idx)->isPointerTy())
          return;
        // resolvePointee() of the GEP gives the type of the *cell* the GEP
        // points at; for a pointer field that cell holds pointer values, so
        // the field candidate is the cell content type's element type --
        // the same level the initializer path records (resolvePointee() of
        // the stored value).
        Type *Cell = resolvePointee(GEP);
        if (auto *TP = dyn_cast_or_null<TypedPointerType>(Cell))
          AddCand(ST, Idx, TP->getElementType());
      };

  // Evidence 2: global initializers. A struct element initialized with a
  // concrete pointer constant (function, global) reveals that field's
  // pointee directly.
  std::function<void(Type *, const Constant *)> ScanConstant;
  ScanConstant = [&](Type *CtxTy, const Constant *C) {
    for (const Use &OpU : C->operands())
      if (const auto *GEP = dyn_cast<GEPOperator>(OpU.get()))
        ScanGEP(GEP);

    auto *ST = dyn_cast_or_null<StructType>(CtxTy);
    if (ST && !ST->isOpaque()) {
      for (unsigned I = 0, E = ST->getNumElements(); I != E; ++I) {
        if (I >= C->getNumOperands())
          break;
        const auto *Elt = dyn_cast<Constant>(C->getOperand(I));
        if (!Elt)
          continue;
        Type *ET = ST->getElementType(I);
        if (ET->isPointerTy() &&
            (isa<Function>(Elt) || isa<GlobalVariable>(Elt)))
          AddCand(ST, I, resolvePointee(Elt));
        else if (containsPointerTypes(ET))
          ScanConstant(ET, Elt);
      }
      return;
    }
    if (auto *AT = dyn_cast_or_null<ArrayType>(CtxTy)) {
      Type *ET = AT->getElementType();
      if (!containsPointerTypes(ET))
        return;
      for (const Use &OpU : C->operands())
        if (const auto *Elt = dyn_cast<Constant>(OpU.get()))
          ScanConstant(ET, Elt);
    }
  };

  for (const GlobalVariable &GV : M.globals())
    if (GV.hasInitializer())
      ScanConstant(GV.getValueType(), GV.getInitializer());

  for (const Function &F : M) {
    for (const BasicBlock &BB : F) {
      for (const Instruction &I : BB) {
        if (const auto *GEP = dyn_cast<GetElementPtrInst>(&I))
          ScanGEP(cast<GEPOperator>(GEP));
        for (const Use &Op : I.operands())
          if (const auto *GEP = dyn_cast<GEPOperator>(Op.get()))
            ScanGEP(GEP);
      }
    }
  }
}

Type *TypedPointerAnalysis::resolveFieldPointee(Type *StructTy, unsigned Idx) {
  gatherStructFieldEvidence();
  auto Key = std::make_pair(StructTy, Idx);
  if (auto It = FieldMemo.find(Key); It != FieldMemo.end())
    return It->second;
  Type *R = I8Ty;
  if (auto It = FieldCands.find(Key); It != FieldCands.end())
    R = joinEvidence(It->second);
  FieldMemo[Key] = R;
  return R;
}

Type *TypedPointerAnalysis::computeGEPTerminalElementType(
    const GEPOperator *GEP) const {
  return const_cast<GEPOperator *>(GEP)->getResultElementType();
}

//===----------------------------------------------------------------------===//
// Synthesis
//===----------------------------------------------------------------------===//

FunctionType *
TypedPointerAnalysis::synthesizeFunctionType(const Function *F) {
  if (auto It = FnTyMemo.find(F); It != FnTyMemo.end())
    return It->second;
  FunctionType *FT = const_cast<Function *>(F)->getFunctionType();

  if (!FnTyInProgress.insert(const_cast<Function *>(F)).second) {
    // Recursive self-reference through a function pointer: emit the
    // all-fallback shape.
    SmallVector<Type *, 8> Params;
    for (Type *PT : FT->params())
      Params.push_back(
          PT->isPointerTy()
              ? makeTyped(I8Ty, cast<PointerType>(PT)->getAddressSpace())
              : PT);
    Type *RT = FT->getReturnType();
    Type *NewRT =
        RT->isPointerTy()
            ? makeTyped(I8Ty, cast<PointerType>(RT)->getAddressSpace())
            : RT;
    FunctionType *R = FunctionType::get(NewRT, Params, FT->isVarArg());
    SynthTypes.insert(cast<FunctionType>(R));
    return R;
  }

  SmallVector<Type *, 8> Params;
  for (unsigned I = 0, E = FT->getNumParams(); I != E; ++I) {
    Type *PT = FT->getParamType(I);
    if (PT->isPointerTy())
      Params.push_back(makeTyped(resolveParamPointee(F, I),
                                 cast<PointerType>(PT)->getAddressSpace()));
    else
      Params.push_back(remapAggregate(PT));
  }
  Type *RT = FT->getReturnType();
  Type *NewRT = RT->isPointerTy()
                    ? makeTyped(resolveRetPointee(F),
                                cast<PointerType>(RT)->getAddressSpace())
                    : remapAggregate(RT);

  FunctionType *NFT = FunctionType::get(NewRT, Params, FT->isVarArg());
  SynthTypes.insert(NFT);
  FnTyInProgress.erase(const_cast<Function *>(F));
  FnTyMemo[F] = NFT;
  return NFT;
}

Type *TypedPointerAnalysis::remapFunctionType(FunctionType *FT,
                                              const Value *V) {
  if (SynthTypes.contains(FT))
    return FT;

  auto Key = std::make_pair(static_cast<Type *>(FT), V);
  if (auto It = RemapMemo.find(Key); It != RemapMemo.end())
    return It->second;

  Type *R = nullptr;
  const Function *G = nullptr;
  if (V) {
    if (const auto *F = dyn_cast<Function>(V)) {
      if (const_cast<Function *>(F)->getFunctionType() == FT)
        G = F;
    } else if (const auto *CB = dyn_cast<CallBase>(V)) {
      G = dyn_cast<Function>(CB->getCalledOperand()->stripPointerCasts());
      if (G && G->getFunctionType() != FT)
        G = nullptr;
    }
  }

  if (G) {
    R = synthesizeFunctionType(G);
  } else {
    SmallVector<Type *, 8> Params;
    const CallBase *CB = V ? dyn_cast<CallBase>(V) : nullptr;
    for (unsigned I = 0, E = FT->getNumParams(); I != E; ++I) {
      Type *PT = FT->getParamType(I);
      if (!PT->isPointerTy()) {
        Params.push_back(remapAggregate(PT));
        continue;
      }
      Type *Pointee = I8Ty;
      // Indirect call: recover the parameter pointee from the passed argument.
      if (CB && I < CB->arg_size() && CB->getArgOperand(I)->getType() == PT)
        Pointee = resolvePointee(CB->getArgOperand(I));
      Params.push_back(
          makeTyped(Pointee, cast<PointerType>(PT)->getAddressSpace()));
    }
    Type *RT = FT->getReturnType();
    Type *NewRT;
    if (RT->isPointerTy()) {
      Type *Pointee = I8Ty;
      if (CB) {
        SmallVector<Type *, 8> Ev;
        collectUseEvidence(CB, Ev, 0);
        Pointee = joinEvidence(Ev);
      }
      NewRT =
          makeTyped(Pointee, cast<PointerType>(RT)->getAddressSpace());
    } else {
      NewRT = remapAggregate(RT);
    }
    R = FunctionType::get(NewRT, Params, FT->isVarArg());
    SynthTypes.insert(R);
  }

  RemapMemo[Key] = R;
  return R;
}

Type *TypedPointerAnalysis::remapStruct(StructType *ST) {
  if (SynthTypes.contains(ST))
    return ST;
  if (auto It = StructMemo.find(ST); It != StructMemo.end())
    return It->second;

  if (!ST->isLiteral()) {
    // Identified struct: synthesize a new identified struct under a temporary
    // unique name and remember the name the writer must emit.  The new struct
    // is registered before its body is filled so that self-referential
    // structs resolve correctly.
    std::string OrigName;
    if (ST->hasName())
      OrigName = ST->getName().str();
    StructType *New = StructType::create(
        Ctx, OrigName.empty() ? "air.typed" : (OrigName + ".air.typed"));
    SynthTypes.insert(New);
    StructMemo[ST] = New;
    NameStorage.push_back(OrigName);
    EmissionNames[New] = &NameStorage.back();
    if (!ST->isOpaque()) {
      SmallVector<Type *, 8> Elems;
      for (unsigned I = 0, E = ST->getNumElements(); I != E; ++I) {
        Type *ET = ST->getElementType(I);
        if (ET->isPointerTy())
          Elems.push_back(makeTyped(resolveFieldPointee(ST, I),
                                    cast<PointerType>(ET)->getAddressSpace()));
        else
          Elems.push_back(remapAggregate(ET));
      }
      New->setBody(Elems, ST->isPacked());
    }
    return New;
  }

  // Literal struct: value-independent, uniqued rebuild.
  if (auto It = AggMemo.find(ST); It != AggMemo.end())
    return It->second;
  SmallVector<Type *, 8> Elems;
  for (unsigned I = 0, E = ST->getNumElements(); I != E; ++I) {
    Type *ET = ST->getElementType(I);
    if (ET->isPointerTy())
      Elems.push_back(makeTyped(resolveFieldPointee(ST, I),
                                cast<PointerType>(ET)->getAddressSpace()));
    else
      Elems.push_back(remapAggregate(ET));
  }
  StructType *New = StructType::get(Ctx, Elems, ST->isPacked());
  SynthTypes.insert(New);
  AggMemo[ST] = New;
  return New;
}

Type *TypedPointerAnalysis::remapAggregate(Type *Ty) {
  if (SynthTypes.contains(Ty))
    return Ty;
  if (auto *PT = dyn_cast<PointerType>(Ty))
    // A bare pointer with no value context (e.g. GEP source element type):
    // no pointee information stance, fall back to i8*.
    return makeTyped(I8Ty, PT->getAddressSpace());
  if (!containsPointerTypes(Ty))
    return Ty;
  if (auto It = AggMemo.find(Ty); It != AggMemo.end())
    return It->second;

  Type *R = Ty;
  if (auto *ST = dyn_cast<StructType>(Ty)) {
    R = remapStruct(ST);
  } else if (auto *AT = dyn_cast<ArrayType>(Ty)) {
    Type *ET = AT->getElementType();
    Type *NewET = remapAggregate(ET);
    if (NewET != ET) {
      R = ArrayType::get(NewET, AT->getNumElements());
      SynthTypes.insert(R);
    }
  } else if (auto *VT = dyn_cast<VectorType>(Ty)) {
    Type *ET = VT->getElementType();
    Type *NewET = remapAggregate(ET);
    if (NewET != ET) {
      R = VectorType::get(NewET, VT->getElementCount());
      SynthTypes.insert(R);
    }
  } else if (auto *FT = dyn_cast<FunctionType>(Ty)) {
    R = remapFunctionType(FT, nullptr);
  }

  AggMemo[Ty] = R;
  return R;
}

Type *TypedPointerAnalysis::remapPointer(PointerType *PT, const Value *V) {
  auto Key = std::make_pair(static_cast<Type *>(PT), V);
  if (auto It = RemapMemo.find(Key); It != RemapMemo.end())
    return It->second;

  Type *Pointee = V ? resolvePointee(V) : I8Ty;
  Type *R = makeTyped(Pointee, PT->getAddressSpace());
  RemapMemo[Key] = R;
  return R;
}

Type *TypedPointerAnalysis::remap(Type *Ty, const Value *V) {
  if (SynthTypes.contains(Ty) || isa<TypedPointerType>(Ty))
    return Ty;
  if (!containsPointerTypes(Ty))
    return Ty;

  if (auto *PT = dyn_cast<PointerType>(Ty))
    return remapPointer(PT, V);
  if (auto *FT = dyn_cast<FunctionType>(Ty))
    return remapFunctionType(FT, V);
  return remapAggregate(Ty);
}

StringRef TypedPointerAnalysis::getEmissionName(Type *Ty) const {
  if (auto It = EmissionNames.find(Ty); It != EmissionNames.end())
    return *It->second;
  if (auto *ST = dyn_cast<StructType>(Ty))
    return ST->getName();
  return StringRef();
}
