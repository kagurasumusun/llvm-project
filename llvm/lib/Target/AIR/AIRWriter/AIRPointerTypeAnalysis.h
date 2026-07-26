//===- AIRPointerTypeAnalysis.h - PointerType analysis for AIR --*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// An analysis pass to assign types to opaque pointers for AIR (Metal IR)
// bitcode emission.  Modelled after the DirectX PointerTypeAnalysis.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_TARGET_AIR_AIRPOINTERTYPEANALYSIS_H
#define LLVM_TARGET_AIR_AIRPOINTERTYPEANALYSIS_H

#include "llvm/ADT/DenseMap.h"
#include "llvm/IR/TypedPointerType.h"
#include "llvm/IR/Value.h"

namespace llvm {

class Module;
class Type;

namespace air {

// Maps opaque-pointer Values to their TypedPointerType (or, for non-pointer
// types, the reconstructed Type that contains typed pointers).
using PointerTypeMap = DenseMap<const Value *, Type *>;

/// Compute the \c PointerTypeMap for the module \p M.
PointerTypeMap computePointerTypeMap(const Module &M);

} // end namespace air
} // end namespace llvm

#endif // LLVM_TARGET_AIR_AIRPOINTERTYPEANALYSIS_H
