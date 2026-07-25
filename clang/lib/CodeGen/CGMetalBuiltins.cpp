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
using namespace llvm;

namespace {

/// Map __metal_* math function names to LLVM intrinsics.
static llvm::Intrinsic::ID getMathIntrinsic(StringRef Name) {
  return llvm::StringSwitch<llvm::Intrinsic::ID>(Name)
      .Case("__metal_sin", llvm::Intrinsic::sin)
      .Case("__metal_cos", llvm::Intrinsic::cos)
      .Case("__metal_tan", llvm::Intrinsic::tan)
      .Case("__metal_asin", llvm::Intrinsic::asin)
      .Case("__metal_acos", llvm::Intrinsic::acos)
      .Case("__metal_atan", llvm::Intrinsic::atan)
      .Case("__metal_atan2", llvm::Intrinsic::atan2)
      .Case("__metal_sinh", llvm::Intrinsic::sinh)
      .Case("__metal_cosh", llvm::Intrinsic::cosh)
      .Case("__metal_tanh", llvm::Intrinsic::tanh)
      .Case("__metal_exp", llvm::Intrinsic::exp)
      .Case("__metal_exp2", llvm::Intrinsic::exp2)
      .Case("__metal_exp10", llvm::Intrinsic::exp10)
      .Case("__metal_log", llvm::Intrinsic::log)
      .Case("__metal_log2", llvm::Intrinsic::log2)
      .Case("__metal_log10", llvm::Intrinsic::log10)
      .Case("__metal_sqrt", llvm::Intrinsic::sqrt)
      .Case("__metal_pow", llvm::Intrinsic::pow)
      .Case("__metal_fabs", llvm::Intrinsic::fabs)
      .Case("__metal_floor", llvm::Intrinsic::floor)
      .Case("__metal_ceil", llvm::Intrinsic::ceil)
      .Case("__metal_trunc", llvm::Intrinsic::trunc)
      .Case("__metal_rint", llvm::Intrinsic::rint)
      .Case("__metal_round", llvm::Intrinsic::round)
      .Case("__metal_fma", llvm::Intrinsic::fma)
      .Case("__metal_fmuladd", llvm::Intrinsic::fmuladd)
      .Case("__metal_fmax", llvm::Intrinsic::maxnum)
      .Case("__metal_fmin", llvm::Intrinsic::minnum)
      .Case("__metal_copysign", llvm::Intrinsic::copysign)
      .Default(llvm::Intrinsic::not_intrinsic);
}

/// Map __metal_* integer function names to LLVM intrinsics.
static llvm::Intrinsic::ID getIntIntrinsic(StringRef Name) {
  return llvm::StringSwitch<llvm::Intrinsic::ID>(Name)
      .Case("__metal_abs", llvm::Intrinsic::abs)
      .Case("__metal_clz", llvm::Intrinsic::ctlz)
      .Case("__metal_ctz", llvm::Intrinsic::cttz)
      .Case("__metal_popcount", llvm::Intrinsic::ctpop)
      .Case("__metal_addsat", llvm::Intrinsic::sadd_sat)
      .Case("__metal_subsat", llvm::Intrinsic::ssub_sat)
      .Default(llvm::Intrinsic::not_intrinsic);
}

} // namespace

RValue CodeGenFunction::EmitMetalBuiltinExpr(const CallExpr *E,
                                             StringRef BuiltinName) {
  // Handle math functions
  if (llvm::Intrinsic::ID IID = getMathIntrinsic(BuiltinName)) {
    SmallVector<Value *, 4> Args;
    for (const Expr *Arg : E->arguments()) {
      Args.push_back(EmitScalarExpr(Arg));
    }

    Function *Fn = CGM.getIntrinsic(IID, Args[0]->getType());
    return RValue::get(Builder.CreateCall(Fn, Args));
  }

  // Handle integer functions
  if (llvm::Intrinsic::ID IID = getIntIntrinsic(BuiltinName)) {
    SmallVector<Value *, 4> Args;
    for (const Expr *Arg : E->arguments()) {
      Args.push_back(EmitScalarExpr(Arg));
    }

    // Some intrinsics need an extra boolean argument
    if (IID == llvm::Intrinsic::ctlz || IID == llvm::Intrinsic::cttz) {
      Args.push_back(Builder.getFalse()); // is_zero_undef
    }

    Function *Fn = CGM.getIntrinsic(IID, Args[0]->getType());
    return RValue::get(Builder.CreateCall(Fn, Args));
  }

  // Handle SIMD shuffle and atomic functions as external calls.
  // These are resolved by the Metal linker at metallib link time.

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
