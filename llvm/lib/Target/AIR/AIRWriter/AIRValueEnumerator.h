//===- AIRValueEnumerator.h - Number values and types for AIR bitcode -----===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This class gives values and types unique IDs for the AIR (Metal IR) bitcode
// writer.  Forked from the DXIL ValueEnumerator.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_TARGET_AIR_AIRVALUEENUMERATOR_H
#define LLVM_TARGET_AIR_AIRVALUEENUMERATOR_H

#include "llvm/ADT/ArrayRef.h"
#include "llvm/ADT/DenseMap.h"
#include "llvm/ADT/UniqueVector.h"
#include "llvm/IR/Attributes.h"
#include "llvm/IR/UseListOrder.h"
#include <cassert>
#include <cstdint>
#include <utility>
#include <vector>

namespace llvm {

class BasicBlock;
class Comdat;
class DIArgList;
class Function;
class Instruction;
class LocalAsMetadata;
class MDNode;
class Metadata;
class Module;
class NamedMDNode;
class raw_ostream;
class Type;
class Value;
class ValueSymbolTable;

namespace air {

class ValueEnumerator {
public:
  using TypeList = std::vector<Type *>;
  using ValueList = std::vector<std::pair<const Value *, unsigned>>;
  using IndexAndAttrSet = std::pair<unsigned, AttributeSet>;

  UseListOrderStack UseListOrders;

private:
  using TypeMapType = DenseMap<Type *, unsigned>;
  TypeMapType TypeMap;
  TypeList Types;

  using ValueMapType = DenseMap<const Value *, unsigned>;
  ValueMapType ValueMap;
  ValueList Values;

  using ComdatSetType = UniqueVector<const Comdat *>;
  ComdatSetType Comdats;

  std::vector<const Metadata *> MDs;
  std::vector<const Metadata *> FunctionMDs;

  struct MDIndex {
    unsigned F = 0;
    unsigned ID = 0;
    MDIndex() = default;
    explicit MDIndex(unsigned F) : F(F) {}
    bool hasDifferentFunction(unsigned NewF) const { return F && F != NewF; }
    const Metadata *get(ArrayRef<const Metadata *> MDs) const {
      assert(ID && "Expected non-zero ID");
      assert(ID <= MDs.size() && "Expected valid ID");
      return MDs[ID - 1];
    }
  };

  using MetadataMapType = DenseMap<const Metadata *, MDIndex>;
  MetadataMapType MetadataMap;

  struct MDRange {
    unsigned First = 0;
    unsigned Last = 0;
    unsigned NumStrings = 0;
    MDRange() = default;
    explicit MDRange(unsigned First) : First(First) {}
  };
  SmallDenseMap<unsigned, MDRange, 1> FunctionMDInfo;

  using AttributeGroupMapType = DenseMap<IndexAndAttrSet, unsigned>;
  AttributeGroupMapType AttributeGroupMap;
  std::vector<IndexAndAttrSet> AttributeGroups;

  using AttributeListMapType = DenseMap<AttributeList, unsigned>;
  AttributeListMapType AttributeListMap;
  std::vector<AttributeList> AttributeLists;

  mutable DenseMap<const BasicBlock *, unsigned> GlobalBasicBlockIDs;

  using InstructionMapType = DenseMap<const Instruction *, unsigned>;
  InstructionMapType InstructionMap;
  unsigned InstructionCount;

  std::vector<const BasicBlock *> BasicBlocks;

  unsigned NumModuleValues;
  unsigned NumModuleMDs = 0;
  unsigned NumMDStrings = 0;

  unsigned FirstFuncConstantID;
  unsigned FirstInstID;

public:
  ValueEnumerator(const Module &M, Type *PrefixType);
  ValueEnumerator(const ValueEnumerator &) = delete;
  ValueEnumerator &operator=(const ValueEnumerator &) = delete;

  void dump() const;
  void print(raw_ostream &OS, const ValueMapType &Map, const char *Name) const;
  void print(raw_ostream &OS, const MetadataMapType &Map,
             const char *Name) const;

  unsigned getValueID(const Value *V) const;

  unsigned getMetadataID(const Metadata *MD) const {
    auto ID = getMetadataOrNullID(MD);
    assert(ID != 0 && "Metadata not in slotcalculator!");
    return ID - 1;
  }

  unsigned getMetadataOrNullID(const Metadata *MD) const {
    return MetadataMap.lookup(MD).ID;
  }

  unsigned numMDs() const { return MDs.size(); }

  unsigned getTypeID(Type *T) const {
    TypeMapType::const_iterator I = TypeMap.find(T);
    assert(I != TypeMap.end() && "Type not in ValueEnumerator!");
    return I->second - 1;
  }

  unsigned getInstructionID(const Instruction *I) const;
  void setInstructionID(const Instruction *I);

  unsigned getAttributeListID(AttributeList PAL) const {
    if (PAL.isEmpty())
      return 0;
    AttributeListMapType::const_iterator I = AttributeListMap.find(PAL);
    assert(I != AttributeListMap.end() && "AttributeList not in ValueEnumerator!");
    return I->second;
  }

  unsigned getAttributeGroupID(IndexAndAttrSet Group) const {
    if (!Group.second.hasAttributes())
      return 0;
    AttributeGroupMapType::const_iterator I = AttributeGroupMap.find(Group);
    assert(I != AttributeGroupMap.end() && "AttributeGroup not in ValueEnumerator!");
    return I->second;
  }

  void getFunctionConstantRange(unsigned &Start, unsigned &End) const {
    Start = FirstFuncConstantID;
    End = FirstInstID;
  }

  const ValueList &getValues() const { return Values; }

  bool hasMDs() const { return NumModuleMDs < MDs.size(); }

  ArrayRef<const Metadata *> getMDStrings() const {
    return ArrayRef(MDs).slice(NumModuleMDs, NumMDStrings);
  }

  ArrayRef<const Metadata *> getNonMDStrings() const {
    return ArrayRef(MDs).slice(NumModuleMDs).slice(NumMDStrings);
  }

  const TypeList &getTypes() const { return Types; }

  const std::vector<const BasicBlock *> &getBasicBlocks() const {
    return BasicBlocks;
  }

  const std::vector<AttributeList> &getAttributeLists() const {
    return AttributeLists;
  }

  const std::vector<IndexAndAttrSet> &getAttributeGroups() const {
    return AttributeGroups;
  }

  const ComdatSetType &getComdats() const { return Comdats; }
  unsigned getComdatID(const Comdat *C) const;

  unsigned getGlobalBasicBlockID(const BasicBlock *BB) const;

  void incorporateFunction(const Function &F);
  void purgeFunction();
  uint64_t computeBitsRequiredForTypeIndices() const;

  void EnumerateType(Type *T);

private:
  void organizeMetadata();
  void dropFunctionFromMetadata(MetadataMapType::value_type &FirstMD);
  void incorporateFunctionMetadata(const Function &F);
  const MDNode *enumerateMetadataImpl(unsigned F, const Metadata *MD);

  unsigned getMetadataFunctionID(const Function *F) const;

  void EnumerateMetadata(const Function *F, const Metadata *MD);
  void EnumerateMetadata(unsigned F, const Metadata *MD);

  void EnumerateFunctionLocalMetadata(const Function &F,
                                      const LocalAsMetadata *Local);
  void EnumerateFunctionLocalMetadata(unsigned F, const LocalAsMetadata *Local);
  void EnumerateFunctionLocalListMetadata(const Function &F,
                                          const DIArgList *ArgList);
  void EnumerateFunctionLocalListMetadata(unsigned F, const DIArgList *Arglist);
  void EnumerateNamedMDNode(const NamedMDNode *NMD);
  void EnumerateValue(const Value *V);
  void EnumerateOperandType(const Value *V);
  void EnumerateAttributes(AttributeList PAL);

  void EnumerateValueSymbolTable(const ValueSymbolTable &ST);
  void EnumerateNamedMetadata(const Module &M);
};

} // end namespace air
} // end namespace llvm

#endif // LLVM_TARGET_AIR_AIRVALUEENUMERATOR_H
