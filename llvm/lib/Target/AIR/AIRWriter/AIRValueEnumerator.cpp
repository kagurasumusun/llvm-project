//===- AIRValueEnumerator.cpp - Number values and types for AIR bitcode ---===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This implements the ValueEnumerator class for AIR (Metal IR) bitcode.
// Forked from the DXIL ValueEnumerator.
//
//===----------------------------------------------------------------------===//

#include "AIRValueEnumerator.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/Config/llvm-config.h"
#include "llvm/IR/Argument.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Constant.h"
#include "llvm/IR/DebugInfoMetadata.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/GlobalAlias.h"
#include "llvm/IR/GlobalIFunc.h"
#include "llvm/IR/GlobalObject.h"
#include "llvm/IR/GlobalValue.h"
#include "llvm/IR/GlobalVariable.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Metadata.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Operator.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/TypedPointerType.h"
#include "llvm/IR/Use.h"
#include "llvm/IR/User.h"
#include "llvm/IR/Value.h"
#include "llvm/IR/ValueSymbolTable.h"
#include "llvm/Support/Casting.h"
#include "llvm/Support/Compiler.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/MathExtras.h"
#include "llvm/Support/raw_ostream.h"
#include <algorithm>
#include <cstddef>
#include <iterator>
#include <tuple>

using namespace llvm;
using namespace llvm::air;

namespace {

struct OrderMap {
  DenseMap<const Value *, std::pair<unsigned, bool>> IDs;
  unsigned LastGlobalConstantID = 0;
  unsigned LastGlobalValueID = 0;

  OrderMap() = default;

  bool isGlobalConstant(unsigned ID) const {
    return ID <= LastGlobalConstantID;
  }

  bool isGlobalValue(unsigned ID) const {
    return ID <= LastGlobalValueID && !isGlobalConstant(ID);
  }

  unsigned size() const { return IDs.size(); }
  std::pair<unsigned, bool> &operator[](const Value *V) { return IDs[V]; }

  std::pair<unsigned, bool> lookup(const Value *V) const {
    return IDs.lookup(V);
  }

  void index(const Value *V) {
    unsigned ID = IDs.size() + 1;
    IDs[V].first = ID;
  }
};

} // end anonymous namespace

static void orderValue(const Value *V, OrderMap &OM) {
  if (OM.lookup(V).first)
    return;

  if (const Constant *C = dyn_cast<Constant>(V)) {
    if (C->getNumOperands() && !isa<GlobalValue>(C)) {
      for (const Value *Op : C->operands())
        if (!isa<BasicBlock>(Op) && !isa<GlobalValue>(Op))
          orderValue(Op, OM);
      if (auto *CE = dyn_cast<ConstantExpr>(C))
        if (CE->getOpcode() == Instruction::ShuffleVector)
          orderValue(CE->getShuffleMaskForBitcode(), OM);
    }
  }

  OM.index(V);
}

static OrderMap orderModule(const Module &M) {
  OrderMap OM;

  for (const GlobalVariable &G : M.globals())
    if (G.hasInitializer())
      if (!isa<GlobalValue>(G.getInitializer()))
        orderValue(G.getInitializer(), OM);
  for (const GlobalAlias &A : M.aliases())
    if (!isa<GlobalValue>(A.getAliasee()))
      orderValue(A.getAliasee(), OM);
  for (const GlobalIFunc &I : M.ifuncs())
    if (!isa<GlobalValue>(I.getResolver()))
      orderValue(I.getResolver(), OM);
  for (const Function &F : M) {
    for (const Use &U : F.operands())
      if (!isa<GlobalValue>(U.get()))
        orderValue(U.get(), OM);
  }

  auto orderConstantValue = [&OM](const Value *V) {
    if ((isa<Constant>(V) && !isa<GlobalValue>(V)) || isa<InlineAsm>(V))
      orderValue(V, OM);
  };
  for (const Function &F : M) {
    if (F.isDeclaration())
      continue;
    for (const BasicBlock &BB : F)
      for (const Instruction &I : BB)
        for (const Value *V : I.operands()) {
          if (const auto *MAV = dyn_cast<MetadataAsValue>(V)) {
            if (const auto *VAM =
                    dyn_cast<ValueAsMetadata>(MAV->getMetadata())) {
              orderConstantValue(VAM->getValue());
            } else if (const auto *AL =
                           dyn_cast<DIArgList>(MAV->getMetadata())) {
              for (const auto *VAM : AL->getArgs())
                orderConstantValue(VAM->getValue());
            }
          }
        }
  }
  OM.LastGlobalConstantID = OM.size();

  for (const Function &F : M)
    orderValue(&F, OM);
  for (const GlobalAlias &A : M.aliases())
    orderValue(&A, OM);
  for (const GlobalIFunc &I : M.ifuncs())
    orderValue(&I, OM);
  for (const GlobalVariable &G : M.globals())
    orderValue(&G, OM);
  OM.LastGlobalValueID = OM.size();

  for (const Function &F : M) {
    if (F.isDeclaration())
      continue;
    for (const BasicBlock &BB : F)
      orderValue(&BB, OM);
    for (const Argument &A : F.args())
      orderValue(&A, OM);
    for (const BasicBlock &BB : F)
      for (const Instruction &I : BB) {
        for (const Value *Op : I.operands())
          if ((isa<Constant>(*Op) && !isa<GlobalValue>(*Op)) ||
              isa<InlineAsm>(*Op))
            orderValue(Op, OM);
        if (auto *SVI = dyn_cast<ShuffleVectorInst>(&I))
          orderValue(SVI->getShuffleMaskForBitcode(), OM);
        if (auto *SI = dyn_cast<SwitchInst>(&I)) {
          for (const auto &Case : SI->cases())
            orderValue(Case.getCaseValue(), OM);
        }
      }
    for (const BasicBlock &BB : F)
      for (const Instruction &I : BB)
        orderValue(&I, OM);
  }
  return OM;
}

static void predictValueUseListOrderImpl(const Value *V, const Function *F,
                                         unsigned ID, const OrderMap &OM,
                                         UseListOrderStack &Stack) {
  using Entry = std::pair<const Use *, unsigned>;
  SmallVector<Entry, 64> List;
  for (const Use &U : V->uses())
    if (OM.lookup(U.getUser()).first)
      List.push_back(std::make_pair(&U, List.size()));

  if (List.size() < 2)
    return;

  bool IsGlobalValue = OM.isGlobalValue(ID);
  llvm::sort(List, [&](const Entry &L, const Entry &R) {
    const Use *LU = L.first;
    const Use *RU = R.first;
    if (LU == RU)
      return false;

    auto LID = OM.lookup(LU->getUser()).first;
    auto RID = OM.lookup(RU->getUser()).first;

    if (OM.isGlobalValue(LID) && OM.isGlobalValue(RID)) {
      if (LID == RID)
        return LU->getOperandNo() > RU->getOperandNo();
      return LID < RID;
    }

    if (LID < RID) {
      if (RID <= ID)
        if (!IsGlobalValue)
          return true;
      return false;
    }
    if (RID < LID) {
      if (LID <= ID)
        if (!IsGlobalValue)
          return false;
      return true;
    }

    if (LID <= ID)
      if (!IsGlobalValue)
        return LU->getOperandNo() < RU->getOperandNo();
    return LU->getOperandNo() > RU->getOperandNo();
  });

  if (llvm::is_sorted(List, llvm::less_second()))
    return;

  Stack.emplace_back(V, F, List.size());
  assert(List.size() == Stack.back().Shuffle.size() && "Wrong size");
  for (size_t I = 0, E = List.size(); I != E; ++I)
    Stack.back().Shuffle[I] = List[I].second;
}

static void predictValueUseListOrder(const Value *V, const Function *F,
                                     OrderMap &OM, UseListOrderStack &Stack) {
  auto &IDPair = OM[V];
  assert(IDPair.first && "Unmapped value");
  if (IDPair.second)
    return;

  IDPair.second = true;
  if (!V->use_empty() &&
      !isa<BasicBlock>(V) &&
      (isa<Constant>(V) || isa<InlineAsm>(V) || V->getType()->isVoidTy()))
    predictValueUseListOrderImpl(V, F, IDPair.first, OM, Stack);
}

/// Construct a ValueEnumerator from a Module M, enumerating all types and
/// values before returning.
ValueEnumerator::ValueEnumerator(const Module &M, Type *PrefixType) {
  // Enumerate the prefix type (typically i8* typed pointer).
  if (PrefixType)
    EnumerateType(PrefixType);

  // Enumerate types used by module-level metadata.
  for (const NamedMDNode &N : M.named_metadata())
    for (unsigned I = 0, E = N.getNumOperands(); I != E; ++I)
      EnumerateMetadata(nullptr, N.getOperand(I));

  // Enumerate the types used by global variables.
  for (const GlobalVariable &G : M.globals()) {
    EnumerateType(G.getValueType());
    // Enumerate types used by ByVal/StructRet/etc attributes.
    if (G.hasAttribute(Attribute::ByVal))
      EnumerateType(G.getParamByValType());
  }

  // Enumerate types used by aliases.
  for (const GlobalAlias &A : M.aliases())
    EnumerateType(A.getValueType());

  // Enumerate types used by functions and arguments.
  for (const Function &F : M) {
    EnumerateType(F.getReturnType());
    for (const Argument &A : F.args())
      EnumerateType(A.getType());
  }

  // Enumerate the operand types of constants that are used as initializers in
  // globals. Also, enumerate types used in metadata attachments.
  for (const GlobalVariable &G : M.globals()) {
    if (G.hasInitializer())
      EnumerateOperandType(G.getInitializer());
  }
  for (const GlobalAlias &A : M.aliases())
    if (!isa<GlobalValue>(A.getAliasee()))
      EnumerateOperandType(A.getAliasee());
  for (const GlobalIFunc &I : M.ifuncs())
    if (!isa<GlobalValue>(I.getResolver()))
      EnumerateOperandType(I.getResolver());
  for (const Function &F : M) {
    for (const Use &U : F.operands())
      if (!isa<GlobalValue>(U.get()))
        EnumerateOperandType(U.get());
    for (const BasicBlock &BB : F)
      for (const Instruction &I : BB)
        for (const Value *V : I.operands())
          if ((isa<Constant>(*V) && !isa<GlobalValue>(*V)) ||
              isa<InlineAsm>(*V))
            EnumerateOperandType(V);
  }

  OrderMap OM = orderModule(M);

  // Enumerate the global values.
  for (const GlobalVariable &G : M.globals())
    EnumerateValue(&G);
  for (const GlobalAlias &A : M.aliases())
    EnumerateValue(&A);
  for (const GlobalIFunc &I : M.ifuncs())
    EnumerateValue(&I);
  for (const Function &F : M)
    EnumerateValue(&F);

  // Enumerate the metadata.
  EnumerateNamedMetadata(M);

  // Maintain the use-list order for each value.
  for (const GlobalVariable &G : M.globals())
    predictValueUseListOrder(&G, nullptr, OM, UseListOrders);
  for (const GlobalAlias &A : M.aliases())
    predictValueUseListOrder(&A, nullptr, OM, UseListOrders);
  for (const GlobalIFunc &I : M.ifuncs())
    predictValueUseListOrder(&I, nullptr, OM, UseListOrders);
  for (const Function &F : M)
    predictValueUseListOrder(&F, nullptr, OM, UseListOrders);

  // Finally, do the metadata operands.
  for (const NamedMDNode &N : M.named_metadata())
    for (unsigned I = 0, E = N.getNumOperands(); I != E; ++I)
      EnumerateMetadata(nullptr, N.getOperand(I));

  // Enumerate the comdat selection information.
  for (const GlobalVariable &G : M.globals())
    if (const Comdat *C = G.getComdat())
      Comdats.insert(C);
  for (const Function &F : M)
    if (const Comdat *C = F.getComdat())
      Comdats.insert(C);

  // Enumerate the attribute lists.
  for (const Function &F : M)
    EnumerateAttributes(F.getAttributes());
}

unsigned ValueEnumerator::getValueID(const Value *V) const {
  ValueMapType::const_iterator I = ValueMap.find(V);
  assert(I != ValueMap.end() && "Value not in slotcalculator!");
  return I->second - 1;
}

unsigned ValueEnumerator::getInstructionID(const Instruction *I) const {
  InstructionMapType::const_iterator II = InstructionMap.find(I);
  assert(II != InstructionMap.end() && "Instruction not in slotcalculator!");
  return II->second;
}

void ValueEnumerator::setInstructionID(const Instruction *I) {
  assert(InstructionMap.count(I) == 0 && "Instruction already set!");
  InstructionMap[I] = ++InstructionCount;
}

static DenseMap<const Metadata *, unsigned>
_MDsToMap(const std::vector<const Metadata *> &MDs) {
  DenseMap<const Metadata *, unsigned> Map;
  for (unsigned I = 0, E = MDs.size(); I != E; ++I)
    Map[MDs[I]] = I + 1;
  return Map;
}

void ValueEnumerator::organizeMetadata() {
  // Partition metadata into distinct and non-distinct sets.
  auto Partition = std::stable_partition(
      FunctionMDs.begin(), FunctionMDs.end(),
      [](const Metadata *MD) { return !isa<MDNode>(MD) || !cast<MDNode>(MD)->isResolved(); });

  // Append non-distinct after distinct.
  auto DistinctSize = Partition - FunctionMDs.begin();
  std::vector<const Metadata *> NonDistinct(Partition, FunctionMDs.end());
  FunctionMDs.erase(Partition, FunctionMDs.end());
  FunctionMDs.insert(FunctionMDs.end(), NonDistinct.begin(), NonDistinct.end());
  NumMDStrings = 0;
}

void ValueEnumerator::dropFunctionFromMetadata(
    MetadataMapType::value_type &FirstMD) {
  FirstMD.second.F = 0;
}

void ValueEnumerator::incorporateFunctionMetadata(const Function &F) {
  NumModuleMDs = MDs.size();

  auto R = FunctionMDInfo.lookup(getValueID(&F) + 1);
  NumMDStrings = R.NumStrings;
  MDs.insert(MDs.end(), FunctionMDs.begin() + R.First,
             FunctionMDs.begin() + R.Last);
}

void ValueEnumerator::EnumerateValue(const Value *V) {
  assert(!V->getType()->isVoidTy() && "Can't insert void values!");
  assert(!isa<MetadataAsValue>(V) && "EnumerateValue doesn't handle Metadata!");

  unsigned &ValueID = ValueMap[V];
  if (ValueID) {
    Values[ValueID - 1].second++;
    return;
  }

  if (auto *GO = dyn_cast<GlobalObject>(V))
    if (const Comdat *C = GO->getComdat())
      Comdats.insert(C);

  EnumerateType(V->getType());

  if (const Constant *C = dyn_cast<Constant>(V)) {
    if (isa<GlobalValue>(C)) {
      // Initializers for globals are handled explicitly elsewhere.
    } else if (C->getNumOperands()) {
      for (User::const_op_iterator I = C->op_begin(), E = C->op_end(); I != E;
           ++I)
        if (!isa<BasicBlock>(*I))
          EnumerateValue(*I);
      if (auto *CE = dyn_cast<ConstantExpr>(C)) {
        if (CE->getOpcode() == Instruction::ShuffleVector)
          EnumerateValue(CE->getShuffleMaskForBitcode());
        if (auto *GEP = dyn_cast<GEPOperator>(CE))
          EnumerateType(GEP->getSourceElementType());
      }

      Values.push_back(std::make_pair(V, 1U));
      ValueMap[V] = Values.size();
      return;
    }
  }

  Values.push_back(std::make_pair(V, 1U));
  ValueID = Values.size();
}

void ValueEnumerator::EnumerateType(Type *Ty) {
  unsigned *TypeID = &TypeMap[Ty];

  if (*TypeID)
    return;

  if (StructType *STy = dyn_cast<StructType>(Ty))
    if (!STy->isLiteral())
      *TypeID = ~0U;

  for (Type *SubTy : Ty->subtypes())
    EnumerateType(SubTy);

  TypeID = &TypeMap[Ty];

  if (*TypeID && *TypeID != ~0U)
    return;

  Types.push_back(Ty);
  *TypeID = Types.size();
}

void ValueEnumerator::EnumerateOperandType(const Value *V) {
  EnumerateType(V->getType());

  assert(!isa<MetadataAsValue>(V) && "Unexpected metadata operand");

  const Constant *C = dyn_cast<Constant>(V);
  if (!C)
    return;

  if (ValueMap.count(C))
    return;

  for (const Value *Op : C->operands()) {
    if (isa<BasicBlock>(Op))
      continue;
    EnumerateOperandType(Op);
  }
  if (auto *CE = dyn_cast<ConstantExpr>(C)) {
    if (CE->getOpcode() == Instruction::ShuffleVector)
      EnumerateOperandType(CE->getShuffleMaskForBitcode());
    if (CE->getOpcode() == Instruction::GetElementPtr)
      EnumerateType(cast<GEPOperator>(CE)->getSourceElementType());
  }
}

void ValueEnumerator::EnumerateAttributes(AttributeList PAL) {
  if (PAL.isEmpty())
    return;

  unsigned &Entry = AttributeListMap[PAL];
  if (Entry == 0) {
    AttributeLists.push_back(PAL);
    Entry = AttributeLists.size();
  }

  for (unsigned i : PAL.indexes()) {
    AttributeSet AS = PAL.getAttributes(i);
    if (!AS.hasAttributes())
      continue;
    IndexAndAttrSet Pair = {i, AS};
    unsigned &Entry = AttributeGroupMap[Pair];
    if (Entry == 0) {
      AttributeGroups.push_back(Pair);
      Entry = AttributeGroups.size();

      for (Attribute Attr : AS) {
        if (Attr.isTypeAttribute())
          EnumerateType(Attr.getValueAsType());
      }
    }
  }
}

void ValueEnumerator::incorporateFunction(const Function &F) {
  InstructionCount = 0;
  NumModuleValues = Values.size();

  incorporateFunctionMetadata(F);

  for (const auto &I : F.args()) {
    EnumerateValue(&I);
    if (I.hasAttribute(Attribute::ByVal))
      EnumerateType(I.getParamByValType());
    else if (I.hasAttribute(Attribute::StructRet))
      EnumerateType(I.getParamStructRetType());
    else if (I.hasAttribute(Attribute::ByRef))
      EnumerateType(I.getParamByRefType());
  }
  FirstFuncConstantID = Values.size();

  for (const BasicBlock &BB : F) {
    for (const Instruction &I : BB) {
      for (const Use &OI : I.operands()) {
        if ((isa<Constant>(OI) && !isa<GlobalValue>(OI)) || isa<InlineAsm>(OI))
          EnumerateValue(OI);
      }
      if (auto *SVI = dyn_cast<ShuffleVectorInst>(&I))
        EnumerateValue(SVI->getShuffleMaskForBitcode());
      if (auto *SI = dyn_cast<SwitchInst>(&I)) {
        for (const auto &Case : SI->cases())
          EnumerateValue(Case.getCaseValue());
      }
    }
    BasicBlocks.push_back(&BB);
    ValueMap[&BB] = BasicBlocks.size();
  }

  EnumerateAttributes(F.getAttributes());

  FirstInstID = Values.size();

  SmallVector<LocalAsMetadata *, 8> FnLocalMDVector;
  SmallVector<DIArgList *, 8> ArgListMDVector;
  for (const BasicBlock &BB : F) {
    for (const Instruction &I : BB) {
      for (const Use &OI : I.operands()) {
        if (auto *MD = dyn_cast<MetadataAsValue>(&OI)) {
          if (auto *Local = dyn_cast<LocalAsMetadata>(MD->getMetadata())) {
            FnLocalMDVector.push_back(Local);
          } else if (auto *ArgList = dyn_cast<DIArgList>(MD->getMetadata())) {
            ArgListMDVector.push_back(ArgList);
            for (ValueAsMetadata *VMD : ArgList->getArgs()) {
              if (auto *Local = dyn_cast<LocalAsMetadata>(VMD)) {
                FnLocalMDVector.push_back(Local);
              }
            }
          }
        }
      }

      if (!I.getType()->isVoidTy())
        EnumerateValue(&I);
    }
  }

  for (unsigned i = 0, e = FnLocalMDVector.size(); i != e; ++i) {
    assert(ValueMap.count(FnLocalMDVector[i]->getValue()) &&
           "Missing value for metadata operand");
    EnumerateFunctionLocalMetadata(F, FnLocalMDVector[i]);
  }
  for (const DIArgList *ArgList : ArgListMDVector)
    EnumerateFunctionLocalListMetadata(F, ArgList);
}

void ValueEnumerator::purgeFunction() {
  for (unsigned i = NumModuleValues, e = Values.size(); i != e; ++i)
    ValueMap.erase(Values[i].first);
  for (unsigned i = NumModuleMDs, e = MDs.size(); i != e; ++i)
    MetadataMap.erase(MDs[i]);
  for (const BasicBlock *BB : BasicBlocks)
    ValueMap.erase(BB);

  Values.resize(NumModuleValues);
  MDs.resize(NumModuleMDs);
  BasicBlocks.clear();
  NumMDStrings = 0;
}

static void IncorporateFunctionInfoGlobalBBIDs(
    const Function *F, DenseMap<const BasicBlock *, unsigned> &IDMap) {
  unsigned Counter = 0;
  for (const BasicBlock &BB : *F)
    IDMap[&BB] = ++Counter;
}

unsigned ValueEnumerator::getGlobalBasicBlockID(const BasicBlock *BB) const {
  unsigned &Idx = GlobalBasicBlockIDs[BB];
  if (Idx != 0)
    return Idx - 1;

  IncorporateFunctionInfoGlobalBBIDs(BB->getParent(), GlobalBasicBlockIDs);
  return getGlobalBasicBlockID(BB);
}

uint64_t ValueEnumerator::computeBitsRequiredForTypeIndices() const {
  return Log2_32_Ceil(getTypes().size() + 1);
}

unsigned ValueEnumerator::getComdatID(const Comdat *C) const {
  ComdatSetType::const_iterator I = Comdats.find(C);
  assert(I != Comdats.end() && "Comdat not in slotcalculator!");
  return I - Comdats.begin();
}

// Metadata enumeration is identical to the DXIL implementation.  We include a
// simplified subset here -- enough to support the metadata emitted by the
// Metal CodeGen path.

void ValueEnumerator::EnumerateMetadata(unsigned F, const Metadata *MD) {
  if (!MD)
    return;

  if (auto *N = dyn_cast<MDNode>(MD)) {
    if (N->isFunctionLocal()) {
      // Will be enumerated when the function is incorporated.
      return;
    }
  }

  if (MetadataMap.count(MD))
    return;

  if (auto *N = dyn_cast<MDNode>(MD)) {
    MetadataMap[MD] = MDIndex(F);
    for (unsigned I = 0, E = N->getNumOperands(); I != E; ++I)
      EnumerateMetadata(F, N->getOperand(I));
    FunctionMDs.push_back(MD);
    MetadataMap[MD].ID = FunctionMDs.size();
  } else if (auto *S = dyn_cast<MDString>(MD)) {
    MetadataMap[MD] = MDIndex(F);
    FunctionMDs.push_back(MD);
    MetadataMap[MD].ID = FunctionMDs.size();
  } else {
    MetadataMap[MD] = MDIndex(F);
    FunctionMDs.push_back(MD);
    MetadataMap[MD].ID = FunctionMDs.size();
  }
}

void ValueEnumerator::EnumerateMetadata(const Function *F, const Metadata *MD) {
  EnumerateMetadata(F ? getMetadataFunctionID(F) : 0, MD);
}

void ValueEnumerator::EnumerateFunctionLocalMetadata(
    const Function &F, const LocalAsMetadata *Local) {
  auto It = MetadataMap.find(Local);
  if (It != MetadataMap.end())
    return;

  EnumerateValue(Local->getValue());
  FunctionMDs.push_back(Local);
  MetadataMap[Local] = MDIndex(getMetadataFunctionID(&F));
  MetadataMap[Local].ID = FunctionMDs.size();
}

void ValueEnumerator::EnumerateFunctionLocalMetadata(
    unsigned F, const LocalAsMetadata *Local) {
  auto It = MetadataMap.find(Local);
  if (It != MetadataMap.end())
    return;

  EnumerateValue(Local->getValue());
  FunctionMDs.push_back(Local);
  MetadataMap[Local] = MDIndex(F);
  MetadataMap[Local].ID = FunctionMDs.size();
}

void ValueEnumerator::EnumerateFunctionLocalListMetadata(
    const Function &F, const DIArgList *ArgList) {
  EnumerateFunctionLocalListMetadata(getMetadataFunctionID(&F), ArgList);
}

void ValueEnumerator::EnumerateFunctionLocalListMetadata(
    unsigned F, const DIArgList *ArgList) {
  auto It = MetadataMap.find(ArgList);
  if (It != MetadataMap.end())
    return;

  for (auto *VAM : ArgList->getArgs())
    EnumerateFunctionLocalMetadata(F, VAM);

  FunctionMDs.push_back(ArgList);
  MetadataMap[ArgList] = MDIndex(F);
  MetadataMap[ArgList].ID = FunctionMDs.size();
}

void ValueEnumerator::EnumerateNamedMDNode(const NamedMDNode *NMD) {
  for (unsigned I = 0, E = NMD->getNumOperands(); I != E; ++I)
    EnumerateMetadata(nullptr, NMD->getOperand(I));
}

unsigned ValueEnumerator::getMetadataFunctionID(const Function *F) const {
  if (!F)
    return 0;
  return getValueID(F) + 1;
}

void ValueEnumerator::EnumerateNamedMetadata(const Module &M) {
  // Enumerate named metadata nodes.
  for (const NamedMDNode &N : M.named_metadata())
    EnumerateNamedMDNode(&N);

  organizeMetadata();
}

void ValueEnumerator::dump() const {
#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  print(dbgs(), ValueMap, "ValueIDs");
  print(dbgs(), MetadataMap, "MetadataIDs");
#endif
}

void ValueEnumerator::print(raw_ostream &OS, const ValueMapType &Map,
                            const char *Name) const {
  OS << Name << ":\n";
  for (const auto &I : Map)
    OS << "  " << I.second << ": " << *I.first << "\n";
}

void ValueEnumerator::print(raw_ostream &OS, const MetadataMapType &Map,
                            const char *Name) const {
  OS << Name << ":\n";
  for (const auto &I : Map)
    OS << "  " << I.second.ID << ": " << *I.first << "\n";
}
