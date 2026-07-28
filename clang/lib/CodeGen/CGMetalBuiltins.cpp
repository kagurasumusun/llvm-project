//===--- CGMetalBuiltins.cpp - Emit LLVM IR for AIR builtins --------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// Lowering for __air_* builtin functions used by the Metal stdlib.
//
// This is a CLEANROOM implementation:
//   - __air_* is our own naming (not Apple's __metal_*)
//   - Maps directly to LLVM intrinsics (not through Apple's naming)
//   - LLVM intrinsics are part of the LLVM project, not Apple's code
//   - The air.* intrinsics are the AIR ABI (public specification)
//
// Categories:
//   1. Math intrinsics → LLVM intrinsics (sin, cos, sqrt, etc.)
//   2. Integer intrinsics → LLVM intrinsics (abs, clz, etc.)
//   3. Texture/atomic/simd/raytracing → external calls (resolved by runtime)
//
// Reference: metal-info builtin_to_air_map.v2.csv (686 builtins)
//   https://github.com/kagurasumusun/metal-info
//
//===----------------------------------------------------------------------===//

#include "CodeGenFunction.h"
#include "clang/Basic/Builtins.h"
#include "llvm/IR/Intrinsics.h"

using namespace clang;
using namespace CodeGen;
using namespace llvm;

namespace {

/// Map __air_* math function names to LLVM intrinsics.
/// These are LLVM intrinsics (part of the LLVM project, not Apple code).
static llvm::Intrinsic::ID getMathIntrinsic(StringRef Name) {
  return llvm::StringSwitch<llvm::Intrinsic::ID>(Name)
      // Trigonometric
      .Case("__air_sin", llvm::Intrinsic::sin)
      .Case("__air_cos", llvm::Intrinsic::cos)
      .Case("__air_tan", llvm::Intrinsic::tan)
      .Case("__air_asin", llvm::Intrinsic::asin)
      .Case("__air_acos", llvm::Intrinsic::acos)
      .Case("__air_atan", llvm::Intrinsic::atan)
      .Case("__air_atan2", llvm::Intrinsic::atan2)
      // Hyperbolic
      .Case("__air_sinh", llvm::Intrinsic::sinh)
      .Case("__air_cosh", llvm::Intrinsic::cosh)
      .Case("__air_tanh", llvm::Intrinsic::tanh)
      // Exponential / logarithmic
      .Case("__air_exp", llvm::Intrinsic::exp)
      .Case("__air_exp2", llvm::Intrinsic::exp2)
      .Case("__air_exp10", llvm::Intrinsic::exp10)
      .Case("__air_log", llvm::Intrinsic::log)
      .Case("__air_log2", llvm::Intrinsic::log2)
      .Case("__air_log10", llvm::Intrinsic::log10)
      // Power / root
      .Case("__air_sqrt", llvm::Intrinsic::sqrt)
      .Case("__air_pow", llvm::Intrinsic::pow)
      .Case("__air_powr", llvm::Intrinsic::pow)
      // Rounding / absolute
      .Case("__air_fabs", llvm::Intrinsic::fabs)
      .Case("__air_floor", llvm::Intrinsic::floor)
      .Case("__air_ceil", llvm::Intrinsic::ceil)
      .Case("__air_trunc", llvm::Intrinsic::trunc)
      .Case("__air_rint", llvm::Intrinsic::rint)
      .Case("__air_round", llvm::Intrinsic::round)
      .Case("__air_nearbyint", llvm::Intrinsic::nearbyint)
      // FMA / copysign
      .Case("__air_fma", llvm::Intrinsic::fma)
      .Case("__air_fmuladd", llvm::Intrinsic::fmuladd)
      .Case("__air_copysign", llvm::Intrinsic::copysign)
      // Min/max
      .Case("__air_fmax", llvm::Intrinsic::maxnum)
      .Case("__air_fmin", llvm::Intrinsic::minnum)
      .Case("__air_fmax3", llvm::Intrinsic::maxnum)
      .Case("__air_fmin3", llvm::Intrinsic::minnum)
      .Default(llvm::Intrinsic::not_intrinsic);
}

/// Map __air_* integer function names to LLVM intrinsics.
static llvm::Intrinsic::ID getIntIntrinsic(StringRef Name) {
  return llvm::StringSwitch<llvm::Intrinsic::ID>(Name)
      // Bit operations
      .Case("__air_abs", llvm::Intrinsic::abs)
      .Case("__air_clz", llvm::Intrinsic::ctlz)
      .Case("__air_ctz", llvm::Intrinsic::cttz)
      .Case("__air_popcount", llvm::Intrinsic::ctpop)
      .Case("__air_reverse_bits", llvm::Intrinsic::bitreverse)
      // Saturating arithmetic
      .Case("__air_addsat", llvm::Intrinsic::sadd_sat)
      .Case("__air_subsat", llvm::Intrinsic::ssub_sat)
      // Multiply high
      .Case("__air_mulhi", llvm::Intrinsic::smul_fix)
      .Case("__air_absdiff", llvm::Intrinsic::abs)
      // FMA
      .Case("__air_fma", llvm::Intrinsic::fma)
      .Case("__air_fmuladd", llvm::Intrinsic::fmuladd)
      .Default(llvm::Intrinsic::not_intrinsic);
}

/// Map __air_* derivative function names to LLVM intrinsics.
///
/// Note: there is no llvm::Intrinsic::dx_fwidth in current LLVM; fwidth is
/// synthesized below as fabs(dpdx(x)) + fabs(dpdy(x)), following the DirectX
/// definition which Metal matches for pixel derivatives.
static llvm::Intrinsic::ID getDerivativeIntrinsic(StringRef Name) {
  return llvm::StringSwitch<llvm::Intrinsic::ID>(Name)
      .Case("__air_dfdx", llvm::Intrinsic::dx_dpdx)
      .Case("__air_dfdy", llvm::Intrinsic::dx_dpdy)
      .Default(llvm::Intrinsic::not_intrinsic);
}

} // namespace

RValue CodeGenFunction::EmitMetalBuiltinExpr(const CallExpr *E,
                                             StringRef BuiltinName) {
  SmallVector<Value *, 8> Args;
  for (const Expr *Arg : E->arguments())
    Args.push_back(EmitScalarExpr(Arg));

  // Handle math intrinsics
  if (llvm::Intrinsic::ID IID = getMathIntrinsic(BuiltinName)) {
    Function *Fn = CGM.getIntrinsic(IID, Args[0]->getType());
    return RValue::get(Builder.CreateCall(Fn, Args));
  }

  // Handle integer intrinsics
  if (llvm::Intrinsic::ID IID = getIntIntrinsic(BuiltinName)) {
    // Some intrinsics need extra arguments
    if (IID == llvm::Intrinsic::ctlz || IID == llvm::Intrinsic::cttz)
      Args.push_back(Builder.getFalse()); // is_zero_undef
    if (IID == llvm::Intrinsic::abs)
      Args.push_back(Builder.getFalse()); // is_int_min_poison

    Function *Fn = CGM.getIntrinsic(IID, Args[0]->getType());
    return RValue::get(Builder.CreateCall(Fn, Args));
  }

  // Handle derivative intrinsics
  if (llvm::Intrinsic::ID IID = getDerivativeIntrinsic(BuiltinName)) {
    Function *Fn = CGM.getIntrinsic(IID, Args[0]->getType());
    return RValue::get(Builder.CreateCall(Fn, Args));
  }

  // fwidth(x) = fabs(dfdx(x)) + fabs(dfdy(x))
  if (BuiltinName == "__air_fwidth") {
    llvm::Type *Ty = Args[0]->getType();
    Value *DX = Builder.CreateCall(
        CGM.getIntrinsic(llvm::Intrinsic::dx_dpdx, Ty), Args[0]);
    Value *DY = Builder.CreateCall(
        CGM.getIntrinsic(llvm::Intrinsic::dx_dpdy, Ty), Args[0]);
    Value *AbsDX =
        Builder.CreateCall(CGM.getIntrinsic(llvm::Intrinsic::fabs, Ty), DX);
    Value *AbsDY =
        Builder.CreateCall(CGM.getIntrinsic(llvm::Intrinsic::fabs, Ty), DY);
    return RValue::get(Builder.CreateFAdd(AbsDX, AbsDY));
  }

  // All other builtins (texture, atomic, simd, raytracing, mesh, etc.)
  // are emitted as external function calls.  They are resolved by the Metal
  // runtime at metallib link time.
  //
  // The AIR bitcode encodes these as:
  //   call <return_type> @__air_<name>(<args>)
  //
  // The runtime libraries provide the implementations that lower to the
  // corresponding air.* intrinsics.

  SmallVector<llvm::Type *, 8> ArgTypes;
  for (Value *Arg : Args)
    ArgTypes.push_back(Arg->getType());

  llvm::FunctionType *FnTy = llvm::FunctionType::get(
      ConvertType(E->getType()), ArgTypes, /*isVarArg=*/false);

  FunctionCallee Callee = CGM.getModule().getOrInsertFunction(
      BuiltinName.str(), FnTy);

  return RValue::get(Builder.CreateCall(Callee, Args));
}
