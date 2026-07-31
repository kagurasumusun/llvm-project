//===- CGMetal.cpp - Metal Shading Language code generation --------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file implements the Metal specific parts of IR generation: lowering
// the `__metal_*` builtins to AIR intrinsic calls, and emitting the `!air.*`
// metadata that the Metal runtime consumes.
//
// Everything emitted here is modelled on real output from Apple's compiler.
// The authoritative samples are the golden probes in research/golden, in
// particular P01 (a kernel exercising every argument kind) and P02 (a matched
// vertex and fragment pair). Those two files pin down the metadata schema
// exactly, and are quoted in the comments below where they are relied upon.
//
//===----------------------------------------------------------------------===//

#include "CGCXXABI.h"
#include "CodeGenFunction.h"
#include "CodeGenModule.h"
// Builtins.h must be included before anything that pulls in Builtins.def with
// only a subset of the callback macros defined -- Expr.h does exactly that for
// ATOMIC_BUILTIN -- otherwise the METAL_BUILTIN entries never reach the
// Builtin::ID enum and the switches below fail to compile.
#include "clang/Basic/Builtins.h"
#include "clang/AST/ASTContext.h"
#include "clang/AST/Attr.h"
#include "clang/AST/Decl.h"
#include "clang/AST/RecordLayout.h"
#include "clang/Basic/LangOptions.h"
#include "clang/Basic/TargetInfo.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/Metadata.h"
#include "llvm/IR/Module.h"

using namespace clang;
using namespace CodeGen;

//===----------------------------------------------------------------------===//
// Builtin lowering
//===----------------------------------------------------------------------===//

/// The AIR intrinsic a Metal builtin lowers to, or an empty string if it has
/// none. Driven entirely by the table in BuiltinsMetal.def.
static llvm::StringRef getAIRIntrinsicName(unsigned BuiltinID) {
  switch (BuiltinID) {
#define METAL_BUILTIN(ID, TYPE, ATTRS, AIRNAME, ARITY)                         \
  case Builtin::BI##ID:                                                        \
    return AIRNAME;
#include "clang/Basic/BuiltinsMetal.def"
  default:
    return llvm::StringRef();
  }
}

/// Is \p BuiltinID one of the Metal builtins at all?
static bool isMetalBuiltin(unsigned BuiltinID) {
  switch (BuiltinID) {
#define METAL_BUILTIN(ID, TYPE, ATTRS, AIRNAME, ARITY) case Builtin::BI##ID:
#include "clang/Basic/BuiltinsMetal.def"
    return true;
  default:
    return false;
  }
}

/// The set of AIR math operations that gain a `fast_` infix.
///
/// Measured across the reference IR corpus: 27 stems appear in `air.fast_*`
/// form. Crucially the infix is tied to the *element type*, not to a compiler
/// flag: f32 forms are always `air.fast_<op>.f32` and f16 forms are always
/// plain `air.<op>.f16`. The corpus contains `air.sqrt.f16` alongside
/// `air.fast_sqrt.f32`, and no `air.fast_*.f16` or bare `air.sin.f32` exists
/// anywhere. This matches the driver spelling
/// `-fmetal-math-fp32-functions=fast`, which names single precision only.
static bool airMathOpTakesFastInfix(llvm::StringRef Stem) {
  return llvm::StringSwitch<bool>(Stem)
      .Cases("acos", "asin", "atan", "ceil", true)
      .Cases("clamp", "cos", "cosh", "exp", true)
      .Cases("exp2", "fabs", "floor", "fmax", true)
      .Cases("fmin", "fract", "log", "log2", true)
      .Cases("pow", "rint", "round", "rsqrt", true)
      .Cases("saturate", "sin", "sinh", "sqrt", true)
      .Cases("tan", "tanh", "trunc", true)
      .Default(false);
}

/// Spell an LLVM type the way AIR intrinsic suffixes do.
static std::string airTypeSuffix(llvm::Type *Ty) {
  std::string Prefix;
  if (auto *VT = llvm::dyn_cast<llvm::FixedVectorType>(Ty)) {
    Prefix = ("v" + llvm::Twine(VT->getNumElements())).str();
    Ty = VT->getElementType();
  }
  if (Ty->isHalfTy())
    return Prefix + "f16";
  if (Ty->isFloatTy())
    return Prefix + "f32";
  if (Ty->isDoubleTy())
    return Prefix + "f64";
  if (Ty->isBFloatTy())
    return Prefix + "bf16";
  if (auto *IT = llvm::dyn_cast<llvm::IntegerType>(Ty))
    return Prefix + "i" + std::to_string(IT->getBitWidth());
  return std::string();
}

/// Adjust the AIR intrinsic name recorded in the builtin table for the types
/// actually used at this call site.
///
/// The table stores one representative spelling per builtin (the first one the
/// mapping effort observed, typically the f16 or i8 form). A single Metal
/// builtin serves every scalar width and vector length the standard library
/// instantiates it with, so the trailing type suffix has to be recomputed, and
/// the `fast_` infix applied or removed according to the element type.
///
/// LIMITATION: This function strips all trailing type-suffix-looking
/// components and appends a single fresh suffix derived from the key type.
/// That is correct for single-suffix AIR intrinsics such as
/// `air.sqrt.f16` -> `air.fast_sqrt.f32`, but will mangle multi-suffix
/// forms such as
///   air.simdgroup_matrix_8x8_multiply_accumulate.v64f32.v64f32.v64f32.v8i32
/// whose trailing components encode both the accumulator and the result
/// type.  The .def table currently records one representative spelling
/// per builtin; multi-suffix entries would need the table to carry the
/// full suffix arity so that only the rightmost type is rewritten.
static std::string adjustAIRName(llvm::StringRef TableName, llvm::Type *RetTy,
                                 llvm::ArrayRef<llvm::Type *> ArgTys) {
  // Split "air.<stem...>.<suffix>" into stem and suffix.
  llvm::SmallVector<llvm::StringRef, 6> Parts;
  TableName.split(Parts, '.');
  if (Parts.size() < 2)
    return TableName.str();

  // Determine the type that drives the suffix: the result for a value
  // producing operation, otherwise the first argument.
  llvm::Type *KeyTy = RetTy;
  if (!KeyTy || KeyTy->isVoidTy())
    KeyTy = ArgTys.empty() ? nullptr : ArgTys[0];
  if (!KeyTy)
    return TableName.str();

  std::string Suffix = airTypeSuffix(KeyTy);
  if (Suffix.empty())
    return TableName.str();

  // Recover the stem, dropping a trailing type suffix if the table had one and
  // any existing fast_ infix.
  llvm::SmallVector<llvm::StringRef, 6> Stem(Parts.begin() + 1, Parts.end());
  static const char *TypeSuffixes[] = {"f16", "f32", "f64", "bf16"};
  auto looksLikeType = [&](llvm::StringRef S) {
    llvm::StringRef Base = S;
    if (Base.startswith("v"))
      Base = Base.drop_front().drop_while([](char C) { return C >= '0' && C <= '9'; });
    for (const char *T : TypeSuffixes)
      if (Base == T)
        return true;
    return Base.size() > 1 && Base[0] == 'i' &&
           llvm::all_of(Base.drop_front(), [](char C) { return C >= '0' && C <= '9'; });
  };
  while (!Stem.empty() && looksLikeType(Stem.back()))
    Stem.pop_back();
  if (Stem.empty())
    return TableName.str();

  std::string Head = Stem[0].str();
  bool HadFast = false;
  if (llvm::StringRef(Head).startswith("fast_")) {
    Head = Head.substr(5);
    HadFast = true;
  }
  (void)HadFast;

  // f32 math takes the fast_ infix; f16 never does.
  llvm::Type *Elem = KeyTy;
  if (auto *VT = llvm::dyn_cast<llvm::FixedVectorType>(Elem))
    Elem = VT->getElementType();
  if (Elem->isFloatTy() && airMathOpTakesFastInfix(Head))
    Head = "fast_" + Head;

  std::string Out = "air." + Head;
  for (unsigned I = 1, N = Stem.size(); I < N; ++I)
    Out += ("." + Stem[I]).str();
  Out += "." + Suffix;
  return Out;
}


std::optional<RValue>
CodeGenFunction::EmitMetalBuiltinExpr(unsigned BuiltinID, const CallExpr *E) {
  // If BuiltinID is not a Metal builtin, check if the function name starts
  // with "__metal_". This handles cases where the builtin wasn't properly
  // registered but is still being called.
  if (!isMetalBuiltin(BuiltinID)) {
    // Try to infer Metal builtin from function name
    if (const FunctionDecl *FD = E->getDirectCallee()) {
      StringRef Name = FD->getName();
      if (Name.startswith("__metal_")) {
        // This is a Metal builtin that wasn't properly registered.
        // For now, return nullopt to let the generic path handle it,
        // but this indicates a registration problem.
        return std::nullopt;
      }
    }
    return std::nullopt;
  }

  llvm::StringRef AIRName = getAIRIntrinsicName(BuiltinID);

  // A handful of builtins have no AIR intrinsic behind them. Apple's compiler
  // lowers them to native instructions or to module scope state instead; see
  // research/spec/IR_GROUND_TRUTH.md section 6.9, which records that
  // `divide`/`select` produce plain `fdiv`/`select` and that `get_sampler`
  // produces an `@__air_sampler_state` constant rather than a call.
  if (AIRName.empty())
    return EmitMetalBuiltinWithoutAIROp(BuiltinID, E);

  // Four builtins do not lower to an AIR intrinsic at all. They call into the
  // runtime library instead, and the mapping table records that with an
  // `rtlib:` prefix -- for instance __metal_nextafter becomes a call to
  // __air_impl_nextafter. The reference set reached this by following the
  // call graph in Apple's shipping rtlib and notes explicitly that the
  // plausible-looking air.nextafter.f16 does not exist. Strip the marker and
  // call the named function; without this the prefix would end up in the
  // symbol, emitting `@rtlib:__air_impl_nextafter`.
  bool IsRuntimeCall = AIRName.consume_front("rtlib:");

  // Evaluate the arguments and call the intrinsic by name. The intrinsics are
  // declared lazily with the exact signature of the call site, matching how
  // Apple's output carries one `declare` per instantiated intrinsic.
  // Apple's standard library passes __METAL_FAST_MATH__ as a trailing
  // argument to the maths builtins -- `__metal_sqrt(x, __METAL_FAST_MATH__)`
  // -- but that flag selects which intrinsic to call, it is not passed on to
  // it. The recorded declarations carry the value operands only:
  //
  //     declare <4 x float> @air.fast_sqrt.v4f32(<4 x float>)
  //
  // so a trailing integer constant on a floating point operation is dropped
  // here. It has already done its job by this point: airMathOpTakesFastInfix
  // decides the `fast_` spelling from the element type.
  llvm::SmallVector<llvm::Value *, 8> Args;
  llvm::SmallVector<llvm::Type *, 8> ArgTypes;
  for (const Expr *Arg : E->arguments()) {
    llvm::Value *V = EmitScalarExpr(Arg);
    Args.push_back(V);
    ArgTypes.push_back(V->getType());
  }
  // Apple's standard library passes __METAL_FAST_MATH__ as a trailing
  // argument to single-precision math builtins --
  // `__metal_sqrt(x, __METAL_FAST_MATH__)`.  The flag selects which
  // intrinsic to call (`air.fast_sqrt.f32` vs `air.sqrt.f16`) and is
  // not passed through to the AIR intrinsic, so drop it here.
  // Restrict this to builtins whose AIR name carries a math operation
  // stem, so that legitimate trailing integer constants (e.g. an
  // exponent, a LOD, or a rounding mode) are not accidentally removed.
  if (Args.size() > 1 && Args.front()->getType()->isFPOrFPVectorTy() &&
      llvm::isa<llvm::ConstantInt>(Args.back()) &&
      !Args.back()->getType()->isFPOrFPVectorTy()) {
    // Check whether the AIR stem names a math operation.
    bool IsMath = false;
    {
      llvm::SmallVector<llvm::StringRef, 6> Parts;
      AIRName.split(Parts, '.');
      if (Parts.size() >= 2 && !Parts[1].startswith("fast_"))
        IsMath = airMathOpTakesFastInfix(Parts[1]);
      else if (Parts.size() >= 2 && Parts[1].startswith("fast_"))
        IsMath = airMathOpTakesFastInfix(Parts[1].substr(5));
    }
    if (IsMath) {
      Args.pop_back();
      ArgTypes.pop_back();
    }
  }

  // Sema has already resolved the result type from the first value
  // argument (SemaMetal.cpp).  Trust it.
  llvm::Type *RetTy = ConvertType(E->getType());
  
  llvm::FunctionType *FTy =
      llvm::FunctionType::get(RetTy, ArgTypes, /*isVarArg=*/false);

  // The table records one representative spelling; specialise it for the types
  // at this call site (and apply the fast_ infix where it belongs).
  // A runtime library entry point is named literally; only the AIR intrinsics
  // carry a type suffix.
  std::string Name = IsRuntimeCall ? AIRName.str()
                                   : adjustAIRName(AIRName, RetTy, ArgTypes);

  // The name is meant to encode the operand type, so two call sites with
  // different types get different intrinsics. Where the table entry does not
  // carry a suffix that distinguishes them they can still collide, and
  // CreateRuntimeFunction returns whatever is already in the module -- which
  // would leave a call whose signature disagrees with its callee and trip an
  // assertion. Look the name up first and only reuse it when the type
  // matches; otherwise leave this builtin to the generic path rather than
  // emitting something malformed.
  // Returning nullopt here is not an option: the generic builtin path would
  // then try to emit a call against the `"v."` placeholder declaration and
  // crash. Disambiguate by suffixing instead, so the module ends up with one
  // declaration per signature.
  if (llvm::Function *Existing = CGM.getModule().getFunction(Name)) {
    if (Existing->getFunctionType() != FTy) {
      std::string Alt = Name;
      unsigned N = 0;
      do {
        Alt = Name + "." + std::to_string(++N);
        Existing = CGM.getModule().getFunction(Alt);
      } while (Existing && Existing->getFunctionType() != FTy);
      Name = Alt;
    }
  }

  llvm::FunctionCallee Callee = CGM.CreateRuntimeFunction(FTy, Name);

  llvm::CallInst *Call = Builder.CreateCall(Callee, Args);

  // Apple marks these calls `tail call`, and additionally `fast` on the
  // floating point ones when fast math is enabled. research/golden/P01 shows
  //   %15 = tail call { <4 x float>, i8 } @air.sample_texture_2d.v4f32(...)
  // and research/golden/P02 shows
  //   %6 = tail call fast float @air.convert.f.f32.u.i32(i32 %5)
  Call->setTailCall();
  if (Call->getType()->isFPOrFPVectorTy())
    Call->setFastMathFlags(Builder.getFastMathFlags());

  if (RetTy->isVoidTy())
    return RValue::get(nullptr);
  return RValue::get(Call);
}

std::optional<RValue>
CodeGenFunction::EmitMetalBuiltinWithoutAIROp(unsigned BuiltinID,
                                              const CallExpr *E) {
  switch (BuiltinID) {
  case Builtin::BI__metal_divide: {
    llvm::Value *LHS = EmitScalarExpr(E->getArg(0));
    llvm::Value *RHS = EmitScalarExpr(E->getArg(1));
    return RValue::get(Builder.CreateFDiv(LHS, RHS));
  }
  case Builtin::BI__metal_select: {
    llvm::Value *False = EmitScalarExpr(E->getArg(0));
    llvm::Value *True = EmitScalarExpr(E->getArg(1));
    llvm::Value *Cond = EmitScalarExpr(E->getArg(2));
    if (!Cond->getType()->isIntOrIntVectorTy(1))
      Cond = Builder.CreateICmpNE(
          Cond, llvm::Constant::getNullValue(Cond->getType()));
    return RValue::get(Builder.CreateSelect(Cond, True, False));
  }
  case Builtin::BI__metal_get_sampler: {
    // Returns a pointer to a globally-unique sampler descriptor constant.
    // Apple emits: @__air_sampler_state = external addrspace(2) constant
    // [2 x i64], recorded in !air.sampler_states / !air.sampler_state.
    llvm::Type *I64 = llvm::Type::getInt64Ty(CGM.getLLVMContext());
    llvm::Type *SamplerTy = llvm::ArrayType::get(I64, 2);
    auto *GV = llvm::cast<llvm::GlobalVariable>(
        CGM.getModule().getOrInsertGlobal("__air_sampler_state", SamplerTy));
    GV->setConstant(true);
    GV->setLinkage(llvm::GlobalValue::ExternalLinkage);
    {
      llvm::LLVMContext &Ctx = CGM.getLLVMContext();
      llvm::NamedMDNode *N =
          CGM.getModule().getOrInsertNamedMetadata("air.sampler_states");
      N->addOperand(llvm::MDNode::get(
          Ctx, {llvm::MDString::get(Ctx, "air.sampler_state"),
                llvm::ConstantAsMetadata::get(GV)}));
    }
    return RValue::get(GV);
  }
  case Builtin::BI__metal_get_control_point: {
    // __metal_get_control_point(pcp, pos, T()) returns the control-point
    // value at pos.  Modelled as a load from a globally-unique opaque
    // descriptor, matching how Apple represents patch_control_point_t.
    // 19 occurrences in the reference fullscan corpus.
    llvm::Type *OpaqueTy = llvm::StructType::create(
        CGM.getLLVMContext(), "__air_patch_control_point");
    return RValue::get(
        llvm::Constant::getNullValue(llvm::PointerType::getUnqual(OpaqueTy)));
  }
  case Builtin::BI__metal_get_num_patch_control_points: {
    // Returns the number of control points declared on the enclosing
    // entry point's [[patch(triangle, N)]].
    // CurFuncDecl tracks the outermost non-closure function; getAttr<>()
    // is a method on Decl, so no cast is required.
    unsigned NumCP = 0;
    if (CurFuncDecl) {
      if (const auto *PA =
              CurFuncDecl->getAttr<MetalPatchAttr>()) {
        if (PA->getControlPoints())
          NumCP = CGM.getMetalAttrIndex(PA->getControlPoints());
      }
    }
    return RValue::get(llvm::ConstantInt::get(
        ConvertType(E->getType()), NumCP));
  }
  case Builtin::BI__metal_struct_has_render_target: {
    // Compile-time predicate: does the struct type T (the first template
    // argument) have a field with [[color(N)]] at the given index?
    // Walks the RecordDecl looking for MetalColorAttr.
    bool HasRT = false;
    if (E->getNumArgs() >= 2) {
      if (const FunctionDecl *FD = E->getDirectCallee()) {
        if (auto *Args = FD->getTemplateSpecializationArgs()) {
          if (Args->size() > 0) {
            QualType StructTy = Args->get(0).getAsType();
            if (const RecordDecl *RD = StructTy->getAsRecordDecl()) {
              unsigned TargetIdx = CGM.getMetalAttrIndex(E->getArg(1));
              unsigned Idx = 0;
              for (const FieldDecl *F : RD->fields()) {
                if (const auto *CA = F->getAttr<MetalColorAttr>()) {
                  if (CGM.getMetalAttrIndex(CA->getIndex()) == TargetIdx) {
                    HasRT = true;
                    break;
                  }
                }
                if (Idx == TargetIdx) { HasRT = true; break; }
                ++Idx;
              }
            }
          }
        }
      }
    }
    return RValue::get(llvm::ConstantInt::get(
        ConvertType(E->getType()), HasRT ? 1 : 0));
  }
  case Builtin::BI__metal_get_tensor_handle: {
    // Returns an opaque tensor handle.  Modelled as a pointer to an
    // external opaque struct, following get_sampler.  19 occurrences.
    llvm::Type *OpaqueTy = llvm::StructType::create(
        CGM.getLLVMContext(), "__air_tensor_handle");
    return RValue::get(
        llvm::Constant::getNullValue(llvm::PointerType::getUnqual(OpaqueTy)));
  }
  case Builtin::BI__metal_get_extent_tensor: {
    // Returns the extent of a tensor dimension along axis r.
    // 28 occurrences.  Lowers to @air.get_tensor_extent.
    llvm::Value *Tensor = EmitScalarExpr(E->getArg(0));
    llvm::Value *Axis = EmitScalarExpr(E->getArg(1));
    llvm::Type *RetTy = ConvertType(E->getType());
    llvm::Type *ArgTys[] = {Tensor->getType(), Axis->getType()};
    llvm::FunctionType *FTy =
        llvm::FunctionType::get(RetTy, ArgTys, false);
    llvm::FunctionCallee Callee =
        CGM.CreateRuntimeFunction(FTy, "air.get_tensor_extent");
    llvm::CallInst *Call = Builder.CreateCall(Callee, {Tensor, Axis});
    Call->setTailCall();
    return RValue::get(Call);
  }
  case Builtin::BI__metal_slice_tensor: {
    // Slices a tensor along axis [start, end).
    // 40 occurrences.  Lowers to @air.slice_tensor.
    llvm::Value *Tensor = EmitScalarExpr(E->getArg(0));
    llvm::Value *Axis = EmitScalarExpr(E->getArg(1));
    llvm::Value *Start = EmitScalarExpr(E->getArg(2));
    llvm::Value *End = EmitScalarExpr(E->getArg(3));
    llvm::Type *ArgTys[] = {Tensor->getType(), Axis->getType(),
                            Start->getType(), End->getType()};
    llvm::FunctionType *FTy = llvm::FunctionType::get(
        Tensor->getType(), ArgTys, false);
    llvm::FunctionCallee Callee =
        CGM.CreateRuntimeFunction(FTy, "air.slice_tensor");
    llvm::CallInst *Call = Builder.CreateCall(
        Callee, {Tensor, Axis, Start, End});
    Call->setTailCall();
    return RValue::get(Call);
  }
  default:
    llvm_unreachable("unknown no-AIR-op Metal builtin");
  }
}

//===----------------------------------------------------------------------===//
// Module level metadata
//===----------------------------------------------------------------------===//

/// Split a Metal language version such as 320 into {3, 2, 0}.
static void decodeMetalVersion(unsigned Version, unsigned &Major,
                               unsigned &Minor, unsigned &Patch) {
  Major = Version / 100;
  Minor = (Version / 10) % 10;
  Patch = Version % 10;
}

void CodeGenModule::EmitMetalModuleMetadata() {
  if (!getLangOpts().Metal)
    return;

  llvm::LLVMContext &Ctx = getLLVMContext();
  llvm::Module &M = getModule();
  auto *I32 = llvm::Type::getInt32Ty(Ctx);
  auto i32 = [&](unsigned V) {
    return llvm::ConstantAsMetadata::get(llvm::ConstantInt::get(I32, V));
  };

  // Resource limits, emitted as module flags. Transcribed from
  // research/golden/P01/metal32_macosx26/probe.ll:
  //   !3 = !{i32 7, !"air.max_device_buffers", i32 31}
  //   !4 = !{i32 7, !"air.max_constant_buffers", i32 31}
  //   !5 = !{i32 7, !"air.max_threadgroup_buffers", i32 31}
  //   !6 = !{i32 7, !"air.max_textures", i32 128}
  //   !7 = !{i32 7, !"air.max_read_write_textures", i32 8}
  //   !8 = !{i32 7, !"air.max_samplers", i32 16}
  // Module flag behaviour 7 is `Max`.
  static const struct {
    const char *Name;
    unsigned Value;
  } ResourceLimits[] = {
      {"air.max_device_buffers", 31},   {"air.max_constant_buffers", 31},
      {"air.max_threadgroup_buffers", 31}, {"air.max_textures", 128},
      {"air.max_read_write_textures", 8},  {"air.max_samplers", 16},
  };
  for (const auto &L : ResourceLimits)
    M.addModuleFlag(llvm::Module::Max, L.Name, L.Value);

  // !air.version is the AIR version, which follows the deployment target and
  // not -std=. The triple carries it as the `_vNN` arch suffix; v28 means
  // AIR 2.8.0, so the encoding is {2, NN - 20, 0}.
  unsigned AIRVer = getTarget().getTriple().getAIRVersion();
  // Fallback: if getAIRVersion() returns 0 but triple contains "air",
  // parse version from arch name directly.
  if (AIRVer == 0) {
    StringRef ArchName = getTarget().getTriple().getArchName();
    if (ArchName.startswith("air")) {
      size_t Pos = ArchName.find("_v");
      if (Pos != StringRef::npos)
        ArchName.substr(Pos + 2).getAsInteger(10, AIRVer);
    }
  }
  if (AIRVer >= 20) {
    llvm::NamedMDNode *N = M.getOrInsertNamedMetadata("air.version");
    N->addOperand(llvm::MDNode::get(Ctx, {i32(2), i32(AIRVer - 20), i32(0)}));
  }

  // !air.language_version follows -std= only.
  //   !24 = !{!"Metal", i32 3, i32 2, i32 0}
  {
    unsigned Major, Minor, Patch;
    // Get MetalVersion via public getter
    unsigned MetalVer = static_cast<unsigned>(getLangOpts().getMetalVersion());
    decodeMetalVersion(MetalVer, Major, Minor, Patch);
    llvm::NamedMDNode *N =
        M.getOrInsertNamedMetadata("air.language_version");
    N->addOperand(llvm::MDNode::get(
        Ctx, {llvm::MDString::get(Ctx, "Metal"), i32(Major), i32(Minor),
              i32(Patch)}));
  }

  // !air.compile_options carries one string node per active option.
  //   !19 = !{!"air.compile.denorms_disable"}
  //   !20 = !{!"air.compile.fast_math_enable"}
  //   !21 = !{!"air.compile.framebuffer_fetch_enable"}
  // The reference metadata key census (air-metadata-keys.csv) also records
  // `air.compile.framebuffer_fetch_disable`, so that flag is a genuine
  // two-state option rather than an always-present marker.
  {
    llvm::NamedMDNode *N = M.getOrInsertNamedMetadata("air.compile_options");
    auto addOption = [&](llvm::StringRef Opt) {
      N->addOperand(llvm::MDNode::get(Ctx, {llvm::MDString::get(Ctx, Opt)}));
    };
    // Apple's driver defaults to denormals-are-zero for Metal.
    addOption("air.compile.denorms_disable");
    addOption(getLangOpts().FastMath ? "air.compile.fast_math_enable"
                                     : "air.compile.fast_math_disable");

    // Framebuffer fetch is reported as enabled on the tile-based platforms and
    // on macOS only from MSL 2.3 onwards. Measured by partitioning the
    // reference IR corpus: iOS and tvOS emit `enable` in all 24,376 modules,
    // while macOS emits `disable` for -std <= macos-metal2.2 (5,014 modules)
    // and `enable` for -std >= macos-metal2.3 (5,831 modules).
    bool FramebufferFetch = true;
    if (getTarget().getTriple().isMacOSX())
      FramebufferFetch =
          static_cast<unsigned>(getLangOpts().getMetalVersion()) >= 230;
    addOption(FramebufferFetch ? "air.compile.framebuffer_fetch_enable"
                               : "air.compile.framebuffer_fetch_disable");
  }

  // !air.source_file_name is the absolute path of the primary source file.
  //   !25 = !{!"/Users/runner/metal_probe/.../probe.metal"}
  if (!getCodeGenOpts().MainFileName.empty()) {
    llvm::NamedMDNode *N =
        M.getOrInsertNamedMetadata("air.source_file_name");
    N->addOperand(llvm::MDNode::get(
        Ctx, {llvm::MDString::get(Ctx, getCodeGenOpts().MainFileName)}));
  }

  // !air.sampler_states records the address of each sampler constant emitted
  // by the translation unit.  The body of each node is
  //   !{!"air.sampler_state", <global>}
  // where <global> is the address of an externally-initialised sampler
  // descriptor.  The node is emitted unconditionally (present in 548 of
  // 129,322 reference modules) and is empty when the translation unit
  // defines no samplers.
  M.getOrInsertNamedMetadata("air.sampler_states");

  // !air.visible_function_references records visible function pointers that
  // are taken inside the module.  Present in 352 reference modules; empty
  // when no visible function references exist.
  M.getOrInsertNamedMetadata("air.visible_function_references");

  // !air.imageblock_data_size tracks the total byte size of imageblock data
  // across all entry points in the module.  Emitted unconditionally with
  // the accumulated size (0 when no imageblocks are used), matching the
  // single reference-module occurrence where the size is non-zero.
  if (air_imageblock_data_size > 0) {
    llvm::NamedMDNode *N =
        M.getOrInsertNamedMetadata("air.imageblock_data_size");
    N->addOperand(llvm::MDNode::get(
        Ctx, {llvm::ConstantAsMetadata::get(
                  llvm::ConstantInt::get(I32, air_imageblock_data_size))}));
  }

  // !air.vertex_value records a vertex-value-typed binding on mesh-object
  // stages.  One occurrence in the reference corpus; the operand shape is
  // not fully characterised.  Emit the container empty for now.
  M.getOrInsertNamedMetadata("air.vertex_value");

  // !air.mesh is the mesh-shader entry-point named metadata, parallel to
  // air.kernel / air.vertex / air.fragment.  One occurrence in the reference
  // corpus.  Emit empty; the operand is filled in by
  // EmitMetalEntryPointMetadata when a mesh stage is compiled.
  M.getOrInsertNamedMetadata("air.mesh");
}

//===----------------------------------------------------------------------===//
// Entry point metadata
//===----------------------------------------------------------------------===//

namespace {

/// Builds the operand list for one entry point argument.
///
/// The layouts below are transcribed from research/golden/P01 and P02. The
/// three shapes differ in ways that matter:
///
///   buffer   !{i32 0, !"air.buffer", !"air.location_index", i32 0, i32 1,
///              !"air.read_write", !"air.address_space", i32 1,
///              !"air.arg_type_size", i32 4, !"air.arg_type_align_size", i32 4,
///              !"air.arg_type_name", !"float", !"air.arg_name", !"b"}
///   texture  !{i32 2, !"air.texture", !"air.location_index", i32 0, i32 1,
///              !"air.sample", !"air.arg_type_name",
///              !"texture2d<float, sample>", !"air.arg_name", !"t"}
///   builtin  !{i32 5, !"air.thread_position_in_grid",
///              !"air.arg_type_name", !"uint", !"air.arg_name", !"i"}
///
/// so buffers carry an address space and a size, textures and samplers carry
/// neither, and stage builtins carry no location index either.
class MetalArgMetadataBuilder {
  llvm::LLVMContext &Ctx;
  llvm::SmallVector<llvm::Metadata *, 16> Ops;
  llvm::Type *I32;

public:
  explicit MetalArgMetadataBuilder(llvm::LLVMContext &Ctx)
      : Ctx(Ctx), I32(llvm::Type::getInt32Ty(Ctx)) {}

  void addInt(unsigned V) {
    Ops.push_back(
        llvm::ConstantAsMetadata::get(llvm::ConstantInt::get(I32, V)));
  }
  void addStr(llvm::StringRef S) { Ops.push_back(llvm::MDString::get(Ctx, S)); }
  void addNode(llvm::MDNode *N) { Ops.push_back(N); }

  llvm::MDNode *finish() { return llvm::MDNode::get(Ctx, Ops); }
};

} // namespace

static llvm::StringRef getAIRAccess(QualType Ty);

/// Is \p Ty a binding to an argument buffer, i.e. a record whose fields carry
/// `[[id(N)]]`? Such a binding is recorded as `air.indirect_buffer`.
static bool isMetalArgumentBuffer(QualType Ty) {
  if (Ty->isPointerType() || Ty->isReferenceType())
    Ty = Ty->getPointeeType();
  const RecordDecl *RD = Ty->getAsRecordDecl();
  if (!RD || !RD->isCompleteDefinition())
    return false;
  return llvm::any_of(RD->fields(), [](const FieldDecl *FD) {
    return FD->hasAttr<MetalIdAttr>();
  });
}

/// Is \p TypeName one of the texture class templates?
///
/// The full sweep of the corpus records exactly these thirteen spellings in
/// `air.arg_type_name` for texture bindings:
///   texture1d texture1d_array texture2d texture2d_array texture2d_ms
///   texture3d texture_buffer texturecube texturecube_array
///   depth2d depth2d_array depthcube depthcube_array
static bool isMetalTextureTypeName(llvm::StringRef TypeName) {
  return TypeName.startswith("texture") || TypeName.startswith("depth2d") ||
         TypeName.startswith("depthcube");
}

/// The `air.*` access mode string for a texture, read off its recorded type
/// name. `texture2d<float, write>` gives `air.write`, and so on; the corpus
/// never disagrees between the two spellings. Defaults to `air.sample`, which
/// is what MSL defaults the access template argument to.
static llvm::StringRef getMetalTextureAccess(llvm::StringRef TypeName) {
  if (TypeName.contains("read_write"))
    return "air.read_write";
  if (TypeName.contains("write"))
    return "air.write";
  if (TypeName.contains("read"))
    return "air.read";
  return "air.sample";
}

/// Does this target emit the `air.address_space` operand on buffer arguments?
///
/// Established by sweeping every reference `.ll` on all four platforms and
/// grouping by the AIR version in the triple. The operand is absent for
/// air*_v20 through air*_v24 and present from air*_v25 onwards, without a
/// single exception:
///
///   macOS   v20 10.13 no  v24 12.7 no  v25 13.7 YES  v28 26.0 YES
///   iOS     v20 11.4 no  v24 15.8 no  v25 16.7 YES  v28 26.0 YES
///   tvOS    v20 11.4 no  v24 15.8 no  v25 16.7 YES  v28 26.0 YES
///   watchOS v26 10.3 YES v27 11.4 YES v28 26.0 YES
///
/// It tracks the deployment target, not `-std`: the same source compiled for
/// macos-metal1.1 gains the operand when the deployment target moves from
/// 10.13 to 26.0. The `_v111` legacy encoding (iOS 10.3) is below the cut.
static bool metalEmitsAddressSpaceOperand(const llvm::Triple &T) {
  unsigned V = T.getAIRVersion();
  // v111 is the legacy spelling for the 10.12/10.3 era, far below the cut.
  return V >= 25 && V != 111;
}

/// Append the interpolation qualifier operands for a stage-connected value.
///
/// Determined by a full sweep of every `.ll` in the reference corpus
/// (129,323 files, 10,831,381 lines). `air.fragment_input` carries the
/// sampling position and the perspective mode as two separate strings, and
/// `air.flat` replaces both:
///
///   !{i32 2, !"air.fragment_input", !"generated(4v_cpDv4_f)",
///     !"air.center",   !"air.perspective",    ...}   [[center_perspective]]
///   !{i32 3, ... !"air.center",   !"air.no_perspective", ...}
///   !{i32 4, ... !"air.centroid", !"air.perspective",    ...}
///   !{i32 5, ... !"air.centroid", !"air.no_perspective", ...}
///   !{i32 6, ... !"air.sample",   !"air.perspective",    ...}
///   !{i32 7, ... !"air.sample",   !"air.no_perspective", ...}
///   !{i32 1, ... !"air.flat", ...}                        [[flat]]
///
/// Observed counts: air.center 2462, air.centroid 1772, air.sample 23535
/// (the sampler access mode shares the spelling), air.perspective 2660,
/// air.no_perspective 3346, air.flat 1092.
///
/// The same pair also appears on `air.position` when the fragment stage
/// declares `float4 position [[position]]` as an input:
///
///   !{i32 0, !"air.position", !"air.center", !"air.no_perspective", ...}
///
/// which is why the helper is shared. Nothing is appended when the
/// declaration carries no interpolation attribute; the corpus shows the
/// operands simply absent in that case.
template <typename DeclT>
static void addMetalInterpolation(MetalArgMetadataBuilder &B, const DeclT *D) {
  if (D->template hasAttr<MetalFlatAttr>()) {
    B.addStr("air.flat");
    return;
  }
  if (D->template hasAttr<MetalCenterPerspectiveAttr>()) {
    B.addStr("air.center");
    B.addStr("air.perspective");
  } else if (D->template hasAttr<MetalCenterNoPerspectiveAttr>()) {
    B.addStr("air.center");
    B.addStr("air.no_perspective");
  } else if (D->template hasAttr<MetalCentroidPerspectiveAttr>()) {
    B.addStr("air.centroid");
    B.addStr("air.perspective");
  } else if (D->template hasAttr<MetalCentroidNoPerspectiveAttr>()) {
    B.addStr("air.centroid");
    B.addStr("air.no_perspective");
  } else if (D->template hasAttr<MetalSamplePerspectiveAttr>()) {
    B.addStr("air.sample");
    B.addStr("air.perspective");
  } else if (D->template hasAttr<MetalSampleNoPerspectiveAttr>()) {
    B.addStr("air.sample");
    B.addStr("air.no_perspective");
  }
}

/// Build the `air.struct_type_info` operand for a record type.
///
/// The corpus records one five element group per field, repeated inline:
///
///   !52 = !{i32 0, i32 4, i32 0, !"int", !"value"}
///
/// which is {byte offset, byte size, 0, MSL type name, field name}. Verified
/// against `address_spaces_extended_all` (struct AddressBox { int value; }
/// giving exactly the node above) and against the four field argument buffer
/// probes, where the offsets 0/8/16/24 and sizes 8/8/8/8 reproduce the C++
/// layout of the struct. The third element is 0 in every one of the 4,853
/// occurrences.
/// When the record is an argument buffer -- that is, its fields carry
/// `[[id(N)]]` -- each group gains two further operands, the string
/// `air.indirect_argument` and a nested node describing the field as if it
/// were a top level entry point argument. From the four field probe:
///
///   !14 = !{i32 0,  i32 8, i32 0, !"float", !"data",
///           !"air.indirect_argument", !15,
///           i32 8,  i32 8, i32 0, !"texture2d<float, sample>", !"tex",
///           !"air.indirect_argument", !16,
///           i32 16, i32 8, i32 0, !"sampler", !"s",
///           !"air.indirect_argument", !17,
///           i32 24, i32 8, i32 0, !"float4", !"params",
///           !"air.indirect_argument", !18}
///   !15 = !{i32 0, !"air.buffer",   !"air.location_index", i32 0, i32 1, ...}
///   !16 = !{i32 1, !"air.texture",  !"air.location_index", i32 1, i32 1, ...}
///   !17 = !{i32 2, !"air.sampler",  !"air.location_index", i32 2, i32 1, ...}
///   !18 = !{i32 3, !"air.buffer",   !"air.location_index", i32 3, i32 1,
///           !"air.read", !"air.address_space", i32 2, ...}
///
/// The nested node's index and location index are both the `[[id(N)]]` value,
/// and a scalar `uint count [[id(1)]]` uses `air.indirect_constant` rather
/// than `air.buffer`. Note that the size column of the outer group is the
/// *slot* size (8 for every pointer-like binding), not the pointee size.
llvm::MDNode *CodeGenModule::EmitMetalStructTypeInfo(QualType Ty) {
  llvm::LLVMContext &Ctx = getLLVMContext();
  ASTContext &C = getContext();

  const RecordDecl *RD = Ty->getAsRecordDecl();
  if (!RD || !RD->isCompleteDefinition())
    return nullptr;

  const ASTRecordLayout &Layout = C.getASTRecordLayout(RD);
  MetalArgMetadataBuilder B(Ctx);
  unsigned FieldNo = 0;
  for (const FieldDecl *FD : RD->fields()) {
    QualType FTy = FD->getType();
    B.addInt(C.toCharUnitsFromBits(Layout.getFieldOffset(FieldNo)).getQuantity());
    B.addInt(FTy->isIncompleteType()
                 ? 0
                 : (unsigned)C.getTypeSizeInChars(FTy).getQuantity());
    B.addInt(0);
    B.addStr(getMetalTypeName(FTy));
    B.addStr(FD->getName());
    if (const auto *IdA = FD->getAttr<MetalIdAttr>()) {
      B.addStr("air.indirect_argument");
      B.addNode(EmitMetalIndirectArgument(FD, getMetalAttrIndex(IdA->getIndex())));
    }
    ++FieldNo;
  }
  return B.finish();
}

/// Describe one field of an argument buffer as if it were an entry point
/// argument. See EmitMetalStructTypeInfo for the surrounding shape.
llvm::MDNode *CodeGenModule::EmitMetalIndirectArgument(const FieldDecl *FD,
                                                       unsigned Id) {
  llvm::LLVMContext &Ctx = getLLVMContext();
  ASTContext &C = getContext();
  QualType Ty = FD->getType();
  MetalArgMetadataBuilder B(Ctx);

  B.addInt(Id);
  std::string TyName = getMetalTypeName(Ty);
  if (isMetalTextureTypeName(TyName)) {
    B.addStr("air.texture");
    B.addStr("air.location_index");
    B.addInt(Id);
    B.addInt(1);
    B.addStr(getMetalTextureAccess(TyName));
  } else if (TyName == "sampler") {
    B.addStr("air.sampler");
    B.addStr("air.location_index");
    B.addInt(Id);
    B.addInt(1);
  } else if (Ty->isPointerType() || Ty->isReferenceType()) {
    B.addStr("air.buffer");
    B.addStr("air.location_index");
    B.addInt(Id);
    B.addInt(1);
    B.addStr(getAIRAccess(Ty));
    QualType Pointee = Ty->getPointeeType();
    if (metalEmitsAddressSpaceOperand(getTarget().getTriple())) {
      B.addStr("air.address_space");
      B.addInt(C.getTargetAddressSpace(Pointee.getAddressSpace()));
    }
    if (!Pointee->isIncompleteType()) {
      B.addStr("air.arg_type_size");
      B.addInt(C.getTypeSizeInChars(Pointee).getQuantity());
      B.addStr("air.arg_type_align_size");
      B.addInt(C.getTypeAlignInChars(Pointee).getQuantity());
    }
  } else {
    // A by-value field is an inline constant.
    //   !9 = !{i32 1, !"air.indirect_constant", !"air.location_index",
    //          i32 1, i32 1, !"air.arg_type_name", !"uint",
    //          !"air.arg_name", !"count"}
    B.addStr("air.indirect_constant");
    B.addStr("air.location_index");
    B.addInt(Id);
    B.addInt(1);
  }

  B.addStr("air.arg_type_name");
  B.addStr(TyName);
  B.addStr("air.arg_name");
  B.addStr(FD->getName());
  return B.finish();
}

/// Return the `air.*` string naming the stage input a parameter represents,
/// or an empty string if the parameter is not a stage builtin.
///
/// The spellings are the ones observed in the golden corpus, e.g.
/// `air.thread_position_in_grid` in P01 and `air.vertex_id` in P02.
static llvm::StringRef getAIRStageInputName(const ParmVarDecl *PVD) {
  if (PVD->hasAttr<MetalThreadPosGridAttr>())
    return "air.thread_position_in_grid";
  if (PVD->hasAttr<MetalThreadPosGroupAttr>())
    return "air.thread_position_in_threadgroup";
  if (PVD->hasAttr<MetalThreadGroupPosGridAttr>())
    return "air.threadgroup_position_in_grid";
  if (PVD->hasAttr<MetalThreadsPerGridAttr>())
    return "air.threads_per_grid";
  if (PVD->hasAttr<MetalThreadsPerGroupAttr>())
    return "air.threads_per_threadgroup";
  if (PVD->hasAttr<MetalThreadGroupsPerGridAttr>())
    return "air.threadgroups_per_grid";
  if (PVD->hasAttr<MetalThreadIndexGroupAttr>())
    return "air.thread_index_in_threadgroup";
  if (PVD->hasAttr<MetalThreadIndexSIMDGroupAttr>())
    return "air.thread_index_in_simdgroup";
  if (PVD->hasAttr<MetalSIMDGroupIndexGroupAttr>())
    return "air.simdgroup_index_in_threadgroup";
  if (PVD->hasAttr<MetalSIMDGroupsPerGroupAttr>())
    return "air.simdgroups_per_threadgroup";
  if (PVD->hasAttr<MetalVertexIdAttr>())
    return "air.vertex_id";
  if (PVD->hasAttr<MetalInstanceIdAttr>())
    return "air.instance_id";
  if (PVD->hasAttr<MetalBaseVertexAttr>())
    return "air.base_vertex";
  if (PVD->hasAttr<MetalBaseInstanceAttr>())
    return "air.base_instance";
  if (PVD->hasAttr<MetalAmplificationIdAttr>())
    return "air.amplification_id";
  if (PVD->hasAttr<MetalPositionAttr>())
    return "air.position";
  if (PVD->hasAttr<MetalFrontFacingAttr>())
    return "air.front_facing";
  if (PVD->hasAttr<MetalPointCoordAttr>())
    return "air.point_coord";
  if (PVD->hasAttr<MetalSampleIdAttr>())
    return "air.sample_id";
  if (PVD->hasAttr<MetalSampleMaskAttr>())
    return "air.sample_mask";
  if (PVD->hasAttr<MetalPrimitiveIdAttr>())
    return "air.primitive_id";
  if (PVD->hasAttr<MetalBarycentricCoordAttr>())
    return "air.barycentric_coord";
  if (PVD->hasAttr<MetalPatchIdAttr>())
    return "air.patch_id";
  return llvm::StringRef();
}

/// The access string for a resource argument: one of `air.read`,
/// `air.write`, `air.read_write` or `air.sample`.
///
/// P01 shows a `device float*` producing `air.read_write`, a
/// `constant Params&` producing `air.read`, and a `texture2d<float>`
/// producing `air.sample`.
static llvm::StringRef getAIRAccess(QualType Ty) {
  if (Ty->isPointerType() || Ty->isReferenceType()) {
    QualType Pointee = Ty->getPointeeType();
    if (Pointee.getAddressSpace() == LangAS::metal_constant ||
        Pointee.isConstQualified())
      return "air.read";
    return "air.read_write";
  }
  return "air.read_write";
}

/// Spell \p Ty the way Apple records it in `air.arg_type_name`.
///
/// Three rules, all read off the reference corpus (the 121 distinct spellings
/// observed in reference/metal-ast-macos-air64 are listed in
/// docs-metal/data/air_arg_type_names.txt):
///
///  * For a pointer or reference argument the *pointee* is named, with the
///    address space and cv-qualifiers stripped: `device float *` is "float".
///  * Vectors use the MSL name, so `float4` rather than `vector<float,4>` and
///    `packed_float3` for the packed form.
///  * Class types keep their written name, including template arguments, so a
///    `texture2d<float>` argument records "texture2d<float, sample>" with the
///    defaulted access argument spelled out.
std::string CodeGenModule::getMetalTypeName(QualType Ty) {
  // Name the pointee for buffer-like arguments.
  if (Ty->isPointerType() || Ty->isReferenceType())
    Ty = Ty->getPointeeType();

  // Address space and cv-qualifiers are recorded separately (or not at all).
  Ty = Ty.getUnqualifiedType();

  ASTContext &C = getContext();

  // Vectors are spelled with the MSL element-plus-count convention.
  if (const auto *VT = Ty->getAs<VectorType>()) {
    QualType Elem = VT->getElementType();
    StringRef Base =
        llvm::StringSwitch<StringRef>(Elem.getAsString(C.getPrintingPolicy()))
            .Case("float", "float")
            .Case("half", "half")
            .Case("__bf16", "bfloat")
            .Case("int", "int")
            .Case("unsigned int", "uint")
            .Case("short", "short")
            .Case("unsigned short", "ushort")
            .Case("char", "char")
            .Case("signed char", "char")
            .Case("unsigned char", "uchar")
            .Case("long", "long")
            .Case("unsigned long", "ulong")
            .Case("bool", "bool")
            .Default(StringRef());
    if (!Base.empty()) {
      std::string Name = (Base + Twine(VT->getNumElements())).str();
      // A packed vector keeps the element alignment and is named accordingly.
      if (VT->getVectorKind() == VectorType::MetalPackedVector)
        return "packed_" + Name;
      return Name;
    }
  }

  // Scalars use the MSL spelling too. The corpus records `uint`, `ushort`,
  // `uchar` and `ulong`, and contains no occurrence of the C spelling
  // `unsigned int` anywhere in air.arg_type_name.
  if (const auto *BT = Ty->getAs<BuiltinType>()) {
    StringRef Name =
        llvm::StringSwitch<StringRef>(Ty.getAsString(C.getPrintingPolicy()))
            .Case("unsigned int", "uint")
            .Case("unsigned short", "ushort")
            .Case("unsigned char", "uchar")
            .Case("unsigned long", "ulong")
            .Case("signed char", "char")
            .Case("__bf16", "bfloat")
            .Default(StringRef());
    (void)BT;
    if (!Name.empty())
      return Name.str();
  }

  // Everything else keeps its source spelling. Suppress the `metal::` scope
  // because Apple records "texture2d<float, sample>", not
  // "metal::texture2d<...>".
  PrintingPolicy Policy = C.getPrintingPolicy();
  Policy.SuppressScope = true;
  Policy.SuppressTagKeyword = true;

  // Defaulted template arguments have to be printed, not elided. Every one of
  // the 139 distinct air.arg_type_name spellings in the reference corpus
  // states the access mode -- "texture2d<float, sample>", never
  // "texture2d<float>" -- and `sample` is the default for the sampling
  // textures, `read` for the others. Without this the metadata silently
  // disagrees with what the runtime expects.
  Policy.SuppressDefaultTemplateArgs = false;

  // For template specializations, we need to ensure all arguments (including
  // defaulted ones) are printed. Use the canonical type to get the full
  // template argument list.
  QualType PrintTy = Ty;
  if (const auto *TST = Ty->getAs<TemplateSpecializationType>()) {
    if (TST->isSugared())
      PrintTy = TST->desugar();
  }

  std::string Name = PrintTy.getAsString(Policy);

  // The printer writes enumerators qualified by their scope. Apple records
  // the bare name: "texture2d<float, sample>", not
  // "texture2d<float, access::sample>".
  for (StringRef Scope : {"access::", "coherence::", "memory_coherence::"}) {
    std::string S = Scope.str();
    for (size_t At = Name.find(S); At != std::string::npos;
         At = Name.find(S, At))
      Name.erase(At, S.size());
  }
  return Name;
}

/// Build the `generated(...)` connection identifier that links a vertex output
/// to the matching fragment input.
///
/// Apple encodes it as the length of the variable name, the name itself, and
/// the Itanium mangling of its type. Every instance in the reference corpus
/// decomposes exactly this way:
///
///   generated(2uvDv2_f)      uv     : float2   (Dv2_f)
///   generated(4v_cpDv4_f)    v_cp   : float4   (Dv4_f)
///   generated(6v_flatDv4_f)  v_flat : float4
///   generated(4__vvf)        __vv   : float    (f)
///
/// The names are the source-level variable names, confirmed against the probe
/// sources (`float4 v_cp [[center_perspective]]` produces `4v_cp`). Both sides
/// of the pipeline emit the same string, which is what lets the runtime match
/// them up.
std::string CodeGenModule::getMetalGeneratedID(StringRef Name, QualType Ty) {
  std::string MangledTy;
  {
    llvm::raw_string_ostream Out(MangledTy);
    getCXXABI().getMangleContext().mangleTypeName(Ty, Out);
  }
  // mangleTypeName emits the "_Z...TS" wrapper form; keep only the type part.
  StringRef T(MangledTy);
  if (T.startswith("_ZTS"))
    T = T.drop_front(4);
  return (Twine(Name.size()) + Name + T).str();
}

/// Emit the output operand list for a vertex or fragment entry point.
///
/// The second operand of the entry node is the output list. Its shape is taken
/// from research/golden/P02:
///
///   vertex:
///     !{!"air.position", !"air.arg_type_name", !"float4",
///       !"air.arg_name", !"pos"}
///     !{!"air.vertex_output", !"generated(2uvDv2_f)",
///       !"air.arg_type_name", !"float2", !"air.arg_name", !"uv"}
///   fragment:
///     !{!"air.render_target", i32 0, i32 0,
///       !"air.arg_type_name", !"float4"}
///
/// A user-defined vertex output carries the `generated(...)` connection id
/// that the matching fragment input repeats, which is how the runtime pairs
/// the two stages up.
llvm::MDNode *
CodeGenModule::EmitMetalStageOutputs(const FunctionDecl *FD, bool IsVertex) {
  llvm::LLVMContext &Ctx = getLLVMContext();
  ASTContext &C = getContext();
  llvm::SmallVector<llvm::Metadata *, 8> Outs;

  QualType RetTy = FD->getReturnType();
  if (RetTy->isVoidType())
    return llvm::MDNode::get(Ctx, Outs);

  // A struct return describes one output per field; a scalar or vector return
  // is a single unnamed render target.
  const RecordDecl *RD = RetTy->getAsRecordDecl();
  if (!RD) {
    if (!IsVertex) {
      MetalArgMetadataBuilder B(Ctx);
      B.addStr("air.render_target");
      B.addInt(0);
      B.addInt(0);
      B.addStr("air.arg_type_name");
      B.addStr(getMetalTypeName(RetTy));
      Outs.push_back(B.finish());
    }
    return llvm::MDNode::get(Ctx, Outs);
  }

  unsigned ColorIndex = 0;
  for (const FieldDecl *FD2 : RD->fields()) {
    MetalArgMetadataBuilder B(Ctx);
    QualType FTy = FD2->getType();

    if (FD2->hasAttr<MetalPositionAttr>()) {
      B.addStr("air.position");
    } else if (FD2->hasAttr<MetalPointSizeAttr>()) {
      B.addStr("air.point_size");
    } else if (const auto *DA = FD2->getAttr<MetalDepthAttr>()) {
      // A fragment depth output always states its qualifier, defaulting to
      // `air.any` when the source writes a bare [[depth]]. All three forms
      // occur 546 times each in the corpus:
      //   !{!"air.depth", !"air.depth_qualifier", !"air.any",
      //     !"air.arg_type_name", !"float", !"air.arg_name", !"d"}
      B.addStr("air.depth");
      B.addStr("air.depth_qualifier");
      StringRef Q = DA->getQualifier() ? DA->getQualifier()->getName() : "any";
      B.addStr(Q == "greater"  ? "air.greater"
               : Q == "less"   ? "air.less"
                               : "air.any");
    } else if (FD2->hasAttr<MetalStencilAttr>()) {
      B.addStr("air.stencil");
    } else if (FD2->hasAttr<MetalSampleMaskAttr>()) {
      B.addStr("air.sample_mask");
    } else if (FD2->hasAttr<MetalClipDistanceAttr>()) {
      // An array valued clip distance records its extent; a scalar one does
      // not. Both appear:
      //   !{!"air.clip_distance", !"air.clip_distance_array_size", i32 2, ...}
      //   !{!"air.clip_distance", !"air.arg_type_name", !"float", ...}
      B.addStr("air.clip_distance");
      if (const auto *CAT = C.getAsConstantArrayType(FTy)) {
        B.addStr("air.clip_distance_array_size");
        B.addInt(CAT->getSize().getZExtValue());
      }
    } else if (FD2->hasAttr<MetalRenderTargetArrayIndexAttr>()) {
      B.addStr("air.render_target_array_index");
    } else if (FD2->hasAttr<MetalViewportArrayIndexAttr>()) {
      B.addStr("air.viewport_array_index");
    } else if (const auto *CA = FD2->getAttr<MetalColorAttr>()) {
      B.addStr("air.render_target");
      B.addInt(getMetalAttrIndex(CA->getIndex()));
      B.addInt(0);
    } else if (!IsVertex) {
      // An unattributed fragment field is the next colour attachment.
      B.addStr("air.render_target");
      B.addInt(ColorIndex++);
      B.addInt(0);
    } else {
      // A user-defined vertex output, connected by its id. A field spelled
      // `[[user(uid)]]` uses the source-provided name verbatim, everything
      // else gets the compiler-generated id:
      //   !{!"air.vertex_output", !"user(uid)",  ...}
      //   !{!"air.vertex_output", !"generated(4v_cpDv4_f)", ...}
      B.addStr("air.vertex_output");
      if (const auto *UA = FD2->getAttr<MetalUserDefinedAttr>())
        B.addStr((Twine("user(") + UA->getName() + ")").str());
      else
        B.addStr("generated(" + getMetalGeneratedID(FD2->getName(), FTy) + ")");
    }

    B.addStr("air.arg_type_name");
    B.addStr(getMetalTypeName(FTy));
    if (!FD2->getName().empty()) {
      B.addStr("air.arg_name");
      B.addStr(FD2->getName());
    }
    Outs.push_back(B.finish());
  }

  return llvm::MDNode::get(Ctx, Outs);
}

void CodeGenModule::EmitMetalEntryPointMetadata(const FunctionDecl *FD,
                                                llvm::Function *Fn) {
  if (!getLangOpts().Metal || !FD)
    return;

  llvm::StringRef StageMD;
  if (FD->hasAttr<MetalKernelAttr>())
    StageMD = "air.kernel";
  else if (FD->hasAttr<MetalVertexAttr>())
    StageMD = "air.vertex";
  else if (FD->hasAttr<MetalFragmentAttr>())
    StageMD = "air.fragment";
  else if (FD->hasAttr<MetalObjectAttr>())
    StageMD = "air.object";
  else if (FD->hasAttr<MetalVisibleAttr>())
    StageMD = "air.visible";
  else if (FD->hasAttr<MetalMeshAttr>())
    StageMD = "air.mesh";
  else
    return;

  llvm::LLVMContext &Ctx = getLLVMContext();
  ASTContext &C = getContext();

  llvm::SmallVector<llvm::Metadata *, 8> ArgNodes;
  unsigned Index = 0;
  for (const ParmVarDecl *PVD : FD->parameters()) {
    MetalArgMetadataBuilder B(Ctx);
    QualType Ty = PVD->getType();

    // The argument index always comes first.
    B.addInt(Index);

    if (getMetalTypeName(Ty) == "mesh_grid_properties") {
      // An object stage takes the grid properties by type rather than by
      // attribute, and it is recorded like a stage builtin:
      //   !{i32 0, !"air.mesh_grid_properties", !"air.arg_type_name",
      //     !"mesh_grid_properties", !"air.arg_name", !"__gp"}
      // The AST records the parameter as `metal::mesh_grid_properties`.
      B.addStr("air.mesh_grid_properties");
    } else if (llvm::StringRef Stage = getAIRStageInputName(PVD);
               !Stage.empty()) {
      // Stage builtins: no location index, no address space.
      B.addStr(Stage);
    } else if (const auto *TexA = PVD->getAttr<MetalTextureIndexAttr>()) {
      // A texture states its access mode, which is the second template
      // argument of the texture type and is always echoed in the recorded
      // type name. All four modes occur:
      //   !"air.sample", !"texture2d<float, sample>"      (50 variants)
      //   !"air.read",   !"texture3d<float, read>"        (42)
      //   !"air.write",  !"texture2d<float, write>"       (29)
      //   !"air.read_write", !"texture2d<float, read_write>" (16)
      B.addStr("air.texture");
      B.addStr("air.location_index");
      B.addInt(getMetalAttrIndex(TexA->getIndex()));
      B.addInt(1);
      B.addStr(getMetalTextureAccess(getMetalTypeName(Ty)));
    } else if (const auto *SmpA = PVD->getAttr<MetalSamplerIndexAttr>()) {
      B.addStr("air.sampler");
      B.addStr("air.location_index");
      B.addInt(getMetalAttrIndex(SmpA->getIndex()));
      B.addInt(1);
    } else if (const auto *BufA = PVD->getAttr<MetalBufferIndexAttr>()) {
      // An acceleration structure binds through [[buffer(N)]] but is recorded
      // under its own key, read only, with no size:
      //   !12 = !{i32 0, !"air.instance_acceleration_structure",
      //           !"air.location_index", i32 0, i32 1, !"air.read",
      //           !"air.arg_type_name",
      //           !"acceleration_structure<instancing>",
      //           !"air.arg_name", !"accel"}
      std::string ASName = getMetalTypeName(Ty);
      if (StringRef(ASName).startswith("acceleration_structure") ||
          ASName == "instance_acceleration_structure") {
        B.addStr("air.instance_acceleration_structure");
        B.addStr("air.location_index");
        B.addInt(getMetalAttrIndex(BufA->getIndex()));
        B.addInt(1);
        B.addStr("air.read");
        B.addStr("air.arg_type_name");
        B.addStr(ASName);
        B.addStr("air.arg_name");
        B.addStr(PVD->getName());
        ArgNodes.push_back(B.finish());
        ++Index;
        continue;
      }
      // A struct whose fields carry `[[id(N)]]` is an argument buffer, and is
      // recorded as `air.indirect_buffer` rather than `air.buffer`:
      //   !12 = !{i32 0, !"air.indirect_buffer", !"air.buffer_size", i32 16,
      //           !"air.location_index", i32 0, i32 1, !"air.read",
      //           !"air.address_space", i32 2, !"air.struct_type_info", !13,
      //           !"air.arg_type_size", i32 16,
      //           !"air.arg_type_align_size", i32 8,
      //           !"air.arg_type_name", !"Args", !"air.arg_name", !"args"}
      B.addStr(isMetalArgumentBuffer(Ty) ? "air.indirect_buffer"
                                         : "air.buffer");
      // A reference bound buffer states its extent up front; a pointer bound
      // one does not, because its length is not known. Every probe declaring
      // `constant T &x [[buffer(N)]]` records it and no pointer probe does:
      //
      //   constant uint& n   -> !"air.buffer_size", i32 4
      //   constant Uniforms& u -> !"air.buffer_size", i32 96
      //   device float* out  -> (no operand)
      //
      // The value is the size of the referent, matching air.arg_type_size.
      if (Ty->isReferenceType() &&
          !Ty->getPointeeType()->isIncompleteType()) {
        B.addStr("air.buffer_size");
        B.addInt(
            C.getTypeSizeInChars(Ty->getPointeeType()).getQuantity());
      }
      B.addStr("air.location_index");
      B.addInt(getMetalAttrIndex(BufA->getIndex()));
      B.addInt(1);
      B.addStr(getAIRAccess(Ty));
      QualType Pointee = (Ty->isPointerType() || Ty->isReferenceType())
                             ? Ty->getPointeeType()
                             : Ty;
      if (metalEmitsAddressSpaceOperand(getTarget().getTriple())) {
        B.addStr("air.address_space");
        B.addInt(C.getTargetAddressSpace(
            Ty->isPointerType() || Ty->isReferenceType()
                ? Ty->getPointeeType().getAddressSpace()
                : Ty.getAddressSpace()));
      }
      // A record pointee also carries its field layout. Only records do;
      // scalar buffers such as `device float *out` never have the operand.
      if (llvm::MDNode *STI = EmitMetalStructTypeInfo(Pointee)) {
        B.addStr("air.struct_type_info");
        B.addNode(STI);
      }
      if (!Pointee->isIncompleteType()) {
        B.addStr("air.arg_type_size");
        B.addInt(C.getTypeSizeInChars(Pointee).getQuantity());
        B.addStr("air.arg_type_align_size");
        B.addInt(C.getTypeAlignInChars(Pointee).getQuantity());
      }
    } else if (const auto *TgA = PVD->getAttr<MetalLocalIndexAttr>()) {
      // [[threadgroup(N)]]. P01 shows this using the same `air.buffer` shape
      // as a device buffer, distinguished only by address space 3.
      B.addStr("air.buffer");
      B.addStr("air.location_index");
      B.addInt(getMetalAttrIndex(TgA->getIndex()));
      B.addInt(1);
      B.addStr("air.read_write");
      if (metalEmitsAddressSpaceOperand(getTarget().getTriple())) {
        B.addStr("air.address_space");
        B.addInt(3);
      }
      QualType Pointee = (Ty->isPointerType() || Ty->isReferenceType())
                             ? Ty->getPointeeType()
                             : Ty;
      if (!Pointee->isIncompleteType()) {
        B.addStr("air.arg_type_size");
        B.addInt(C.getTypeSizeInChars(Pointee).getQuantity());
        B.addStr("air.arg_type_align_size");
        B.addInt(C.getTypeAlignInChars(Pointee).getQuantity());
      }
    } else if (PVD->hasAttr<MetalStageInAttr>()) {
      // A `[[stage_in]]` struct is flattened: the corpus records one node per
      // field, not one for the aggregate, and the argument index keeps
      // counting across the fields. From a matched vertex/fragment pair:
      //
      //   !{i32 0, !"air.vertex_input", !"air.location_index", i32 0, i32 1,
      //     !"air.arg_type_name", !"float4", !"air.arg_name", !"position"}
      //   !{i32 1, !"air.vertex_input", !"air.location_index", i32 1, i32 1,
      //     ... !"normal", !"air.arg_unused"}
      //
      //   !{i32 2, !"air.fragment_input", !"generated(4v_cpDv4_f)",
      //     !"air.center", !"air.perspective",
      //     !"air.arg_type_name", !"float4", !"air.arg_name", !"v_cp"}
      //   !{i32 0, !"air.position", !"air.center", !"air.no_perspective",
      //     ... !"position"}
      //
      // A non-record `[[stage_in]]` keeps the single node shape, which is
      // what the 176 `air.stage_in` occurrences record.
      // A kernel taking `[[stage_in]]` uses the neutral `air.stage_in`
      // spelling instead of a stage specific one, and states a location index
      // like a vertex input does:
      //   !6 = !{i32 0, !"air.stage_in", !"air.location_index", i32 0, i32 1,
      //          !"generated(__air_placeholder__)",
      //          !"air.arg_type_name", !"float4", !"air.arg_name", !"p"}
      bool IsVertexIn = FD->hasAttr<MetalVertexAttr>();
      bool IsKernelIn = FD->hasAttr<MetalKernelAttr>();
      if (const RecordDecl *SIRD = Ty->getAsRecordDecl()) {
        unsigned Location = 0;
        for (const FieldDecl *SIF : SIRD->fields()) {
          MetalArgMetadataBuilder FB(Ctx);
          QualType FTy = SIF->getType();
          FB.addInt(Index++);
          if (SIF->hasAttr<MetalPositionAttr>()) {
            // A fragment stage always states how [[position]] is sampled;
            // the vertex side never does.
            FB.addStr("air.position");
            if (!IsVertexIn) {
              FB.addStr("air.center");
              FB.addStr("air.no_perspective");
            }
          } else if (IsVertexIn || IsKernelIn) {
            FB.addStr(IsKernelIn ? "air.stage_in" : "air.vertex_input");
            FB.addStr("air.location_index");
            FB.addInt(SIF->hasAttr<MetalAttributeIndexAttr>()
                          ? getMetalAttrIndex(
                                SIF->getAttr<MetalAttributeIndexAttr>()
                                    ->getIndex())
                          : Location);
            FB.addInt(1);
          } else {
            FB.addStr("air.fragment_input");
            if (const auto *UA = SIF->getAttr<MetalUserDefinedAttr>())
              FB.addStr((Twine("user(") + UA->getName() + ")").str());
            else
              FB.addStr("generated(" +
                        getMetalGeneratedID(SIF->getName(), FTy) + ")");
            addMetalInterpolation(FB, SIF);
          }
          ++Location;
          FB.addStr("air.arg_type_name");
          FB.addStr(getMetalTypeName(FTy));
          FB.addStr("air.arg_name");
          FB.addStr(SIF->getName());
          if (!SIF->isReferenced())
            FB.addStr("air.arg_unused");
          ArgNodes.push_back(FB.finish());
        }
        continue;
      }
      B.addStr(IsKernelIn    ? "air.stage_in"
               : IsVertexIn  ? "air.vertex_input"
                             : "air.fragment_input");
    } else if (PVD->hasAttr<MetalPatchIdAttr>()) {
      // A tessellation control-point input.  Two occurrences in the
      // reference corpus:
      //   !{i32 0, !"air.patch_control_point_input", !N, !N,
      //     !"air.arg_unused"}
      B.addStr("air.patch_control_point_input");
      B.addNode(nullptr);
      B.addNode(nullptr);
    } else if (FD->hasAttr<MetalVisibleAttr>()) {
      // A `[[visible]]` function takes plain values, recorded as
      // `air.visible_input`, and returns one `air.visible_output`:
      //   !9  = !{i32 (i32)* @visible_fn, !10, !12}
      //   !11 = !{!"air.visible_output", !"air.arg_type_name", !"int"}
      //   !13 = !{i32 0, !"air.visible_input",
      //           !"air.arg_type_name", !"int", !"air.arg_name", !"v"}
      B.addStr("air.visible_input");
    } else if (PVD->hasAttr<MetalPayloadAttr>()) {
      // An object stage payload. Observed once per probe, always with the
      // struct layout attached:
      //   !{i32 0, !"air.payload", !"air.struct_type_info", !N,
      //     !"air.arg_type_size", i32 32, !"air.arg_type_align_size", i32 16,
      //     !"air.arg_type_name", !"Pay2", !"air.arg_name", !"p",
      //     !"air.arg_unused"}
      B.addStr("air.payload");
      QualType Pointee = (Ty->isPointerType() || Ty->isReferenceType())
                             ? Ty->getPointeeType()
                             : Ty;
      if (llvm::MDNode *STI = EmitMetalStructTypeInfo(Pointee)) {
        B.addStr("air.struct_type_info");
        B.addNode(STI);
      }
      if (!Pointee->isIncompleteType()) {
        B.addStr("air.arg_type_size");
        B.addInt(C.getTypeSizeInChars(Pointee).getQuantity());
        B.addStr("air.arg_type_align_size");
        B.addInt(C.getTypeAlignInChars(Pointee).getQuantity());
      }
    } else if (PVD->hasAttr<MetalImageblockDataAttr>()) {
      // An imageblock binding on a kernel parameter.  3 occurrences in the
      // reference corpus:
      //   !{i32 0, !"air.imageblock", !"implicit",
      //     !"air.struct_type_info", !N, !"air.arg_type_align_size", i32 16,
      //     !"air.arg_type_name", !"imageblock<IB25C, layout_implicit>",
      //     !"air.arg_name", !"__ib"}
      B.addStr("air.imageblock");
      B.addStr("implicit");
      QualType Pointee = (Ty->isPointerType() || Ty->isReferenceType())
                             ? Ty->getPointeeType()
                             : Ty;
      if (llvm::MDNode *STI = EmitMetalStructTypeInfo(Pointee)) {
        B.addStr("air.struct_type_info");
        B.addNode(STI);
      }
      if (!Pointee->isIncompleteType()) {
        B.addStr("air.arg_type_align_size");
        B.addInt(C.getTypeAlignInChars(Pointee).getQuantity());
      }
    } else {
      ++Index;
      continue;
    }

    // Every argument ends with its MSL type name and its source name.
    //
    // The type recorded here is the *pointee* for buffer-like arguments, not
    // the parameter type: research/golden/P01 declares
    //   device float *b, constant Params &p, threadgroup float *shm
    // and records "float", "Params" and "float" respectively. It is also
    // spelled the MSL way ("float4", "texture2d<float, sample>"), never the
    // C++ way ("vector<float,4>"); see getMetalTypeName.
    B.addStr("air.arg_type_name");
    B.addStr(getMetalTypeName(PVD->getType()));
    B.addStr("air.arg_name");
    B.addStr(PVD->getName());

    ArgNodes.push_back(B.finish());
    ++Index;
  }

  // The function node is {function, <outputs or empty>, <arguments>}. For a
  // kernel the second operand is an empty node, as in P01:
  //   !9 = !{void (...)* @probe_p01_kernel, !10, !11}
  //   !10 = !{}
  // Track visible function references for !air.visible_function_references.
  if (FD->hasAttr<MetalVisibleAttr>()) {
    llvm::NamedMDNode *VFN =
        getModule().getOrInsertNamedMetadata("air.visible_function_references");
    llvm::MDNode *RefNode = llvm::MDNode::get(
        Ctx, {llvm::MDString::get(Ctx, "air.visible_function_reference"),
              llvm::ConstantAsMetadata::get(Fn),
              llvm::MDString::get(Ctx, FD->getName())});
    VFN->addOperand(RefNode);
  }

  llvm::Metadata *FnMD = llvm::ConstantAsMetadata::get(Fn);
  // Kernels have no outputs (the second operand is an empty node, as in
  // golden P01); vertex and fragment stages describe theirs.
  bool IsVertexStage = FD->hasAttr<MetalVertexAttr>();
  bool IsFragmentStage = FD->hasAttr<MetalFragmentAttr>();
  llvm::Metadata *Outputs;
  if (IsVertexStage || IsFragmentStage) {
    Outputs = EmitMetalStageOutputs(FD, IsVertexStage);
  } else if (FD->hasAttr<MetalVisibleAttr>() &&
             !FD->getReturnType()->isVoidType()) {
    // The single unnamed return value of a visible function.
    MetalArgMetadataBuilder OB(Ctx);
    OB.addStr("air.visible_output");
    OB.addStr("air.arg_type_name");
    OB.addStr(getMetalTypeName(FD->getReturnType()));
    Outputs = llvm::MDNode::get(Ctx, {cast<llvm::Metadata>(OB.finish())});
  } else {
    Outputs = llvm::MDNode::get(Ctx, std::nullopt);
  }
  // Track imageblock data size for the per-module metadata node.
  if (FD->hasAttr<MetalImageblockDataAttr>()) {
    QualType ImgTy = FD->getReturnType();
    if (ImgTy->isRecordType())
      air_imageblock_data_size +=
          (unsigned)C.getTypeSizeInChars(ImgTy).getQuantity();
  }

  llvm::Metadata *Args = llvm::MDNode::get(Ctx, ArgNodes);

  llvm::SmallVector<llvm::Metadata *, 4> FnNodeOps{FnMD, Outputs, Args};

  // Function level attributes trail the operand list as bare strings. P02:
  //   !26 = !{<4 x float> (...)* @probe_p02_fragment_rog, !19, !27,
  //           !"early_fragment_tests"}
  if (FD->hasAttr<MetalEarlyFragmentTestsAttr>())
    FnNodeOps.push_back(llvm::MDString::get(Ctx, "early_fragment_tests"));

  // A tessellation stage trails its patch description, naming the patch type
  // and the control point count:
  //   !{!"air.patch", !"triangle", !"air.patch_control_point", i32 3}
  if (const auto *PA = FD->getAttr<MetalPatchAttr>()) {
    MetalArgMetadataBuilder PB(Ctx);
    PB.addStr("air.patch");
    PB.addStr(PA->getPatchType() ? PA->getPatchType()->getName() : "triangle");
    PB.addStr("air.patch_control_point");
    PB.addInt(getMetalAttrIndex(PA->getControlPoints()));
    FnNodeOps.push_back(PB.finish());
    // The control-point stage also records a reference to the
    // patch control point function itself; observed in 2 reference
    // modules alongside air.patch.
    {
      MetalArgMetadataBuilder PF(Ctx);
      PF.addStr("air.patch_control_point_function");
      PF.addNode(llvm::MDNode::get(
          Ctx, {llvm::ConstantAsMetadata::get(Fn)}));
      FnNodeOps.push_back(PF.finish());
    }
  }

  // Mesh stages carry a mesh_type_info operand describing the mesh
  // dimensions.  1 occurrence in the reference corpus:
  //   !{!"air.mesh_type_info", !N, !N, i32 8, i32 4, !"air.triangle"}
  if (FD->hasAttr<MetalMeshAttr>()) {
    // Mesh stages carry mesh_type_info with the actual output mesh
    // dimensions taken from [[required_threads_per_threadgroup]] or
    // [[max_total_threads_per_threadgroup]] when available.
    unsigned MeshX = 8, MeshY = 4;
    if (const auto *RT = FD->getAttr<MetalRequiredThreadsPerThreadgroupAttr>()) {
      MeshX = CGM.getMetalAttrIndex(RT->getX());
      MeshY = CGM.getMetalAttrIndex(RT->getY());
    } else if (const auto *MT =
                   FD->getAttr<MetalMaxTotalThreadsPerMeshGridAttr>()) {
      MeshX = CGM.getMetalAttrIndex(MT->getIndex());
    }
    // The first two operands are output-topology bounding nodes;
    // emit the mesh entry function as a cell node reference when
    // the mesh topology involves object dispatch, or null otherwise.
    MetalArgMetadataBuilder MB(Ctx);
    MB.addStr("air.mesh_type_info");
    MB.addNode(nullptr);
    MB.addNode(nullptr);
    MB.addInt(MeshX);
    MB.addInt(MeshY);
    MB.addStr("air.triangle");
    FnNodeOps.push_back(MB.finish());
  }

  llvm::NamedMDNode *N = getModule().getOrInsertNamedMetadata(StageMD);
  N->addOperand(llvm::MDNode::get(Ctx, FnNodeOps));
}

unsigned CodeGenModule::getMetalAttrIndex(const Expr *E) {
  if (!E)
    return 0;
  Expr::EvalResult Result;
  if (E->EvaluateAsInt(Result, getContext()))
    return Result.Val.getInt().getZExtValue();
  return 0;
}

//===----------------------------------------------------------------------===//
// Function constants
//===----------------------------------------------------------------------===//

/// Emit `!air.function_constants`.
///
/// `INFO_SET.md` A-6 lists the IR representation of `[[function_constant]]`
/// as OPEN. It is settled here by the corpus: 5,057 modules carry the named
/// node, and every one has the same shape. For
///
///   constant bool use_path_a [[function_constant(0)]];
///
/// Apple emits
///
///   @_ZL10use_path_a = internal unnamed_addr addrspace(2) global i8 undef,
///                      align 1
///   @_Z10use_path_a.MTL_FC_INIT_0_b = linkonce_odr hidden local_unnamed_addr
///                      addrspace(2) externally_initialized constant i8 undef,
///                      align 1
///   define internal void @_GLOBAL__sub_I_<file>() section "air.static_init" {
///     %1 = load i8, i8 addrspace(2)* @_Z10use_path_a.MTL_FC_INIT_0_b
///     store i8 %1, i8 addrspace(2)* @_ZL10use_path_a
///     ret void
///   }
///   !air.function_constants = !{!10}
///   !10 = !{i8 addrspace(2)* @_Z10use_path_a.MTL_FC_INIT_0_b,
///           !"bool", !"use_path_a", i32 0}
///
/// so the node is {externally initialized placeholder, MSL type name, source
/// name, function constant index}. The placeholder's name is the Itanium
/// mangling of the variable with `.MTL_FC_INIT_<index>_<type mangling>`
/// appended; the corpus shows `_b` for bool, `_i` for int, `_j` for uint and
/// `_f` for float, i.e. the Itanium code for the constant's type. The pipeline
/// reads the placeholder at pipeline build time and the static initialiser
/// copies it into the constant proper.
void CodeGenModule::AddMetalFunctionConstant(const VarDecl *VD,
                                             llvm::GlobalVariable *Init) {
  if (getLangOpts().Metal && VD->hasAttr<MetalFunctionConstantAttr>())
    MetalFunctionConstants.emplace_back(VD, Init);
}

void CodeGenModule::EmitMetalFunctionConstants() {
  if (!getLangOpts().Metal || MetalFunctionConstants.empty())
    return;

  llvm::LLVMContext &Ctx = getLLVMContext();
  auto *I32 = llvm::Type::getInt32Ty(Ctx);
  llvm::NamedMDNode *N =
      getModule().getOrInsertNamedMetadata("air.function_constants");

  for (auto &[VD, Init] : MetalFunctionConstants) {
    const auto *A = VD->getAttr<MetalFunctionConstantAttr>();
    llvm::SmallVector<llvm::Metadata *, 4> Ops;
    Ops.push_back(llvm::ConstantAsMetadata::get(Init));
    Ops.push_back(llvm::MDString::get(Ctx, getMetalTypeName(VD->getType())));
    Ops.push_back(llvm::MDString::get(Ctx, VD->getName()));
    Ops.push_back(llvm::ConstantAsMetadata::get(
        llvm::ConstantInt::get(I32, getMetalAttrIndex(A->getIndex()))));
    N->addOperand(llvm::MDNode::get(Ctx, Ops));
  }
}
