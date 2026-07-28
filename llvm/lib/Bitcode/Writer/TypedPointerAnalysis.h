//===- TypedPointerAnalysis.h - typed-pointer bitcode support ---*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// Reconstruction of per-value pointer pointee types for legacy typed-pointer
// bitcode emission (LLVM 16 era TYPE_CODE_POINTER format).
//
// LLVM IR is opaque-pointer only.  Old bitcode consumers (e.g. Apple Metal's
// metalfe) require each pointer in the bitcode to carry an explicit pointee
// type.  This analysis rebuilds that information from the (still pointee
// typed) instructions: allocas know their allocated type, GEPs know their
// source element type, loads/stores transfer types between memory and SSA
// values, callsites and function bodies constrain signatures, and so on.
// Values whose pointee cannot be recovered uniquely fall back to i8 -- the
// same fallback philosophy LLVM itself used during the opaque-pointer
// transition (see the OpaquePointersUpgrade handling in LLVM 15/16).
//
// All types produced by this analysis are synthesized types owned by the
// module's LLVMContext (literal StructType/ArrayType/VectorType/FunctionType
// plus TypedPointerType).  They are never attached to the module; they exist
// purely so the bitcode writer can hand IDs to type records.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_BITCODE_WRITER_TYPEDPOINTERANALYSIS_H
#define LLVM_LIB_BITCODE_WRITER_TYPEDPOINTERANALYSIS_H

#include "llvm/ADT/ArrayRef.h"
#include "llvm/ADT/DenseMap.h"
#include "llvm/ADT/DenseSet.h"
#include "llvm/ADT/SmallPtrSet.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/ADT/StringRef.h"
#include <deque>
#include <string>
#include <utility>

namespace llvm {

class Function;
class FunctionType;
class GEPOperator;
class LLVMContext;
class Module;
class PointerType;
class StructType;
class Type;
class Value;

/// Maps values/types of an opaque-pointer module to the types that should be
/// emitted in legacy typed-pointer bitcode.
class TypedPointerAnalysis {
public:
  explicit TypedPointerAnalysis(const Module &M);

  /// Map the module type \p Ty to its emitted (typed-pointer) form.
  /// \p V is the value \p Ty was taken from, or nullptr for standalone /
  /// structural references (aggregate element types, GEP source element
  /// types, ...).
  Type *remap(Type *Ty, const Value *V);

  /// If \p Ty is a synthesized identified struct produced by this analysis,
  /// return the name the bitcode writer must emit for it (the original LLVM
  /// IR struct name).  Otherwise returns \p Ty 's own name (or empty).
  StringRef getEmissionName(Type *Ty) const;

  /// True if \p Ty was synthesized by this analysis.
  bool isSynthesizedType(Type *Ty) const { return SynthTypes.contains(Ty); }

private:
  const Module &M;
  LLVMContext &Ctx;

  Type *I8Ty;    // i8, ultimate element type.
  Type *I8PtrTy; // TypedPointerType i8 addrspace(0), the ultimate fallback.

  /// Types produced by this analysis (idempotence set).
  SmallPtrSet<Type *, 32> SynthTypes;

  /// Per-(type,value) remap memo for pointer-ish translations.
  DenseMap<std::pair<Type *, const Value *>, Type *> RemapMemo;

  /// Aggregate (value-independent) translations.
  DenseMap<Type *, Type *> AggMemo;

  /// Synthesized identified structs: original -> new, plus emission names.
  DenseMap<StructType *, StructType *> StructMemo;
  DenseMap<Type *, std::string *> EmissionNames;

  /// Name storage backing EmissionNames.
  std::deque<std::string> NameStorage;

  /// Per-function signature synthesis memos.
  DenseMap<const Function *, FunctionType *> FnTyMemo;
  DenseMap<std::pair<const Function *, unsigned>, Type *> ParamMemo;
  DenseMap<const Function *, Type *> RetMemo;

  /// Per-value pointee resolution memo (value -> remapped pointee type).
  DenseMap<const Value *, Type *> PointeeMemo;

  /// In-progress guards to break recursive resolution.
  DenseSet<const Value *> PointeeInProgress;
  DenseSet<const Function *> FnTyInProgress;
  DenseSet<std::pair<const Function *, unsigned>> ParamInProgress;
  DenseSet<const Function *> RetInProgress;

  /// Struct field pointee candidates, gathered from GEPs and initializers.
  /// Key: (struct type, field index). A single distinct candidate wins;
  /// multiple or none falls back to i8.
  DenseMap<std::pair<Type *, unsigned>, SmallVector<Type *, 4>> FieldCands;
  DenseMap<std::pair<Type *, unsigned>, Type *> FieldMemo;
  bool FieldEvidenceGathered = false;

  /// contains-pointer memo for the fast path.
  DenseMap<Type *, bool> HasPtrMemo;

  // ---- structural helpers ----
  bool containsPointerTypes(Type *Ty);
  Type *fallbackPointee() const { return I8Ty; }
  Type *makeTyped(Type *Pointee, unsigned AddrSpace);

  // ---- synthesis ----
  Type *remapPointer(PointerType *PT, const Value *V);
  Type *remapFunctionType(FunctionType *FT, const Value *V);
  Type *remapStruct(StructType *ST);
  Type *remapAggregate(Type *Ty);

  /// Synthesize (and memoize) the typed form of function \p F 's signature.
  FunctionType *synthesizeFunctionType(const Function *F);

  // ---- pointee resolution ----
  Type *resolvePointee(const Value *V);
  Type *resolveParamPointee(const Function *F, unsigned ArgNo);
  Type *resolveRetPointee(const Function *F);
  Type *resolveFieldPointee(Type *StructTy, unsigned Idx);

  /// Collect pointee evidence from all uses of pointer value \p V.
  void collectUseEvidence(const Value *V, SmallVectorImpl<Type *> &Out,
                          unsigned Depth);

  /// Join evidence to a single type (all-equal wins, else i8 fallback).
  Type *joinEvidence(ArrayRef<Type *> Ev) const;

  /// Compute the element type a constant GEP with the given source element
  /// type and operands points at.
  Type *computeGEPTerminalElementType(const GEPOperator *GEP) const;

  void gatherStructFieldEvidence();
};

} // end namespace llvm

#endif // LLVM_LIB_BITCODE_WRITER_TYPEDPOINTERANALYSIS_H
