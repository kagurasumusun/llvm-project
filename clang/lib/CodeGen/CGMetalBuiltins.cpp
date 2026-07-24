//===--- CGMetalBuiltins.cpp - Emit LLVM IR for Metal builtins ------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file implements lowering for __metal_* builtin functions.
//
//===----------------------------------------------------------------------===//

#include "CodeGenFunction.h"
#include "clang/Basic/Builtins.h"

using namespace clang;
using namespace CodeGen;

namespace {

/// Map __metal_* math function names to LLVM intrinsics.
static Intrinsic::ID getMathIntrinsic(StringRef Name) {
  return llvm::StringSwitch<Intrinsic::ID>(Name)
      .Case("__metal_sin", Intrinsic::sin)
      .Case("__metal_cos", Intrinsic::cos)
      .Case("__metal_tan", Intrinsic::tan)
      .Case("__metal_asin", Intrinsic::asin)
      .Case("__metal_acos", Intrinsic::acos)
      .Case("__metal_atan", Intrinsic::atan)
      .Case("__metal_atan2", Intrinsic::atan2)
      .Case("__metal_sinh", Intrinsic::sinh)
      .Case("__metal_cosh", Intrinsic::cosh)
      .Case("__metal_tanh", Intrinsic::tanh)
      .Case("__metal_exp", Intrinsic::exp)
      .Case("__metal_exp2", Intrinsic::exp2)
      .Case("__metal_exp10", Intrinsic::exp10)
      .Case("__metal_log", Intrinsic::log)
      .Case("__metal_log2", Intrinsic::log2)
      .Case("__metal_log10", Intrinsic::log10)
      .Case("__metal_sqrt", Intrinsic::sqrt)
      .Case("__metal_pow", Intrinsic::pow)
      .Case("__metal_fabs", Intrinsic::fabs)
      .Case("__metal_floor", Intrinsic::floor)
      .Case("__metal_ceil", Intrinsic::ceil)
      .Case("__metal_trunc", Intrinsic::trunc)
      .Case("__metal_rint", Intrinsic::rint)
      .Case("__metal_round", Intrinsic::round)
      .Case("__metal_fma", Intrinsic::fma)
      .Case("__metal_fmuladd", Intrinsic::fmuladd)
      .Case("__metal_fmax", Intrinsic::maxnum)
      .Case("__metal_fmin", Intrinsic::minnum)
      .Case("__metal_copysign", Intrinsic::copysign)
      .Default(Intrinsic::not_intrinsic);
}

/// Map __metal_* integer function names to LLVM intrinsics.
static Intrinsic::ID getIntIntrinsic(StringRef Name) {
  return llvm::StringSwitch<Intrinsic::ID>(Name)
      .Case("__metal_abs", Intrinsic::abs)
      .Case("__metal_clz", Intrinsic::ctlz)
      .Case("__metal_ctz", Intrinsic::cttz)
      .Case("__metal_popcount", Intrinsic::ctpop)
      .Case("__metal_addsat", Intrinsic::sadd_sat)
      .Case("__metal_subsat", Intrinsic::ssub_sat)
      .Default(Intrinsic::not_intrinsic);
}

} // namespace

RValue CodeGenFunction::EmitMetalBuiltinExpr(const CallExpr *E,
                                             StringRef BuiltinName) {
  // Handle math functions
  if (Intrinsic::ID IID = getMathIntrinsic(BuiltinName)) {
    SmallVector<Value *, 4> Args;
    for (const Expr *Arg : E->arguments()) {
      Args.push_back(EmitScalarExpr(Arg));
    }

    Function *Fn = CGM.getIntrinsic(IID, Args[0]->getType());
    return RValue::get(Builder.CreateCall(Fn, Args));
  }

  // Handle integer functions
  if (Intrinsic::ID IID = getIntIntrinsic(BuiltinName)) {
    SmallVector<Value *, 4> Args;
    for (const Expr *Arg : E->arguments()) {
      Args.push_back(EmitScalarExpr(Arg));
    }

    // Some intrinsics need an extra boolean argument
    if (IID == Intrinsic::ctlz || IID == Intrinsic::cttz) {
      Args.push_back(Builder.getFalse()); // is_zero_undef
    }

    Function *Fn = CGM.getIntrinsic(IID, Args[0]->getType());
    return RValue::get(Builder.CreateCall(Fn, Args));
  }

  // Handle SIMD shuffle functions
  if (BuiltinName.startswith("__metal_simd_shuffle")) {
    Value *Val = EmitScalarExpr(E->getArg(0));
    Value *Idx = EmitScalarExpr(E->getArg(1));
    
    // Map to appropriate shuffle intrinsic
    Intrinsic::ID IID;
    if (BuiltinName == "__metal_simd_shuffle")
      IID = Intrinsic::matrix_simd_shuffle;
    else if (BuiltinName == "__metal_simd_shuffle_down")
      IID = Intrinsic::matrix_simd_shuffle_down;
    else if (BuiltinName == "__metal_simd_shuffle_up")
      IID = Intrinsic::matrix_simd_shuffle_up;
    else
      IID = Intrinsic::matrix_simd_shuffle_xor;
    
    Function *Fn = CGM.getIntrinsic(IID, Val->getType());
    return RValue::get(Builder.CreateCall(Fn, {Val, Idx}));
  }

  // Handle atomic functions - emit as external function calls
  if (BuiltinName.startswith("__metal_atomic_")) {
    SmallVector<Value *, 8> Args;
    for (const Expr *Arg : E->arguments()) {
      Args.push_back(EmitScalarExpr(Arg));
    }

    // Create external function declaration
    llvm::FunctionType *FnTy = llvm::FunctionType::get(
        ConvertType(E->getType()),
        {Args[0]->getType()},
        /*isVarArg=*/false);
    
    FunctionCallee Callee = CGM.getModule().getOrInsertFunction(
        BuiltinName.str(), FnTy);
    
    return RValue::get(Builder.CreateCall(Callee, Args));
  }

  // Fallback: emit as external function call
  SmallVector<Value *, 8> Args;
  for (const Expr *Arg : E->arguments()) {
    Args.push_back(EmitScalarExpr(Arg));
  }

  llvm::FunctionType *FnTy = llvm::FunctionType::get(
      ConvertType(E->getType()),
      {Args.begin(), Args.end()},
      /*isVarArg=*/false);
  
  FunctionCallee Callee = CGM.getModule().getOrInsertFunction(
      BuiltinName.str(), FnTy);
  
  return RValue::get(Builder.CreateCall(Callee, Args));
}
