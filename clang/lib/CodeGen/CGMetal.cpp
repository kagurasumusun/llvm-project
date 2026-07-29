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

#include "CodeGenFunction.h"
#include "CodeGenModule.h"
#include "clang/AST/Attr.h"
#include "clang/AST/Decl.h"
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
#define METAL_BUILTIN(ID, TYPE, ATTRS, AIRNAME)                                \
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
#define METAL_BUILTIN(ID, TYPE, ATTRS, AIRNAME) case Builtin::BI##ID:
#include "clang/Basic/BuiltinsMetal.def"
    return true;
  default:
    return false;
  }
}

std::optional<RValue>
CodeGenFunction::EmitMetalBuiltinExpr(unsigned BuiltinID, const CallExpr *E) {
  if (!isMetalBuiltin(BuiltinID))
    return std::nullopt;

  llvm::StringRef AIRName = getAIRIntrinsicName(BuiltinID);

  // A handful of builtins have no AIR intrinsic behind them. Apple's compiler
  // lowers them to native instructions or to module scope state instead; see
  // research/spec/IR_GROUND_TRUTH.md section 6.9, which records that
  // `divide`/`select` produce plain `fdiv`/`select` and that `get_sampler`
  // produces an `@__air_sampler_state` constant rather than a call.
  if (AIRName.empty())
    return EmitMetalBuiltinWithoutAIROp(BuiltinID, E);

  // Evaluate the arguments and call the intrinsic by name. The intrinsics are
  // declared lazily with the exact signature of the call site, matching how
  // Apple's output carries one `declare` per instantiated intrinsic.
  llvm::SmallVector<llvm::Value *, 8> Args;
  llvm::SmallVector<llvm::Type *, 8> ArgTypes;
  for (const Expr *Arg : E->arguments()) {
    llvm::Value *V = EmitScalarExpr(Arg);
    Args.push_back(V);
    ArgTypes.push_back(V->getType());
  }

  llvm::Type *RetTy = ConvertType(E->getType());
  llvm::FunctionType *FTy =
      llvm::FunctionType::get(RetTy, ArgTypes, /*isVarArg=*/false);
  llvm::FunctionCallee Callee = CGM.CreateRuntimeFunction(FTy, AIRName);

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
  // Only the cases the reference set positively establishes are implemented
  // here. Anything else is left to the generic path rather than guessed at.
  switch (BuiltinID) {
  case Builtin::BI__metal_divide: {
    // research/spec/IR_GROUND_TRUTH.md section 6.9: "divide/select -> air op
    // does not exist (native fdiv/select)".
    llvm::Value *LHS = EmitScalarExpr(E->getArg(0));
    llvm::Value *RHS = EmitScalarExpr(E->getArg(1));
    return RValue::get(Builder.CreateFDiv(LHS, RHS));
  }
  case Builtin::BI__metal_select: {
    // Same source: `select` is emitted natively. MSL's argument order is
    // (false_value, true_value, condition), following OpenCL.
    llvm::Value *False = EmitScalarExpr(E->getArg(0));
    llvm::Value *True = EmitScalarExpr(E->getArg(1));
    llvm::Value *Cond = EmitScalarExpr(E->getArg(2));
    if (!Cond->getType()->isIntOrIntVectorTy(1))
      Cond = Builder.CreateICmpNE(
          Cond, llvm::Constant::getNullValue(Cond->getType()));
    return RValue::get(Builder.CreateSelect(Cond, True, False));
  }
  default:
    // The remaining seven builtins in this class (get_sampler,
    // get_control_point, struct_has_render_target, the tensor accessors and
    // get_num_patch_control_points) need module scope state or synthesised
    // helpers whose exact form the reference set marks as not yet measured.
    // Fall through to the generic handling rather than invent one.
    return std::nullopt;
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
  if (AIRVer >= 20) {
    llvm::NamedMDNode *N = M.getOrInsertNamedMetadata("air.version");
    N->addOperand(llvm::MDNode::get(Ctx, {i32(2), i32(AIRVer - 20), i32(0)}));
  }

  // !air.language_version follows -std= only.
  //   !24 = !{!"Metal", i32 3, i32 2, i32 0}
  {
    unsigned Major, Minor, Patch;
    decodeMetalVersion(getLangOpts().MetalVersion, Major, Minor, Patch);
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
      FramebufferFetch = getLangOpts().MetalVersion >= 230;
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
      if (VT->getVectorKind() == VectorType::GenericVector &&
          C.getTypeAlignInChars(Ty) == C.getTypeAlignInChars(Elem))
        return "packed_" + Name;
      return Name;
    }
  }

  // Everything else keeps its source spelling. Suppress the `metal::` scope
  // because Apple records "texture2d<float, sample>", not
  // "metal::texture2d<...>".
  PrintingPolicy Policy = C.getPrintingPolicy();
  Policy.SuppressScope = true;
  Policy.SuppressTagKeyword = true;
  return Ty.getAsString(Policy);
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

    if (llvm::StringRef Stage = getAIRStageInputName(PVD); !Stage.empty()) {
      // Stage builtins: no location index, no address space.
      B.addStr(Stage);
    } else if (const auto *TexA = PVD->getAttr<MetalTextureIndexAttr>()) {
      B.addStr("air.texture");
      B.addStr("air.location_index");
      B.addInt(getMetalAttrIndex(TexA->getIndex()));
      B.addInt(1);
      B.addStr("air.sample");
    } else if (const auto *SmpA = PVD->getAttr<MetalSamplerIndexAttr>()) {
      B.addStr("air.sampler");
      B.addStr("air.location_index");
      B.addInt(getMetalAttrIndex(SmpA->getIndex()));
      B.addInt(1);
    } else if (const auto *BufA = PVD->getAttr<MetalBufferIndexAttr>()) {
      B.addStr("air.buffer");
      B.addStr("air.location_index");
      B.addInt(getMetalAttrIndex(BufA->getIndex()));
      B.addInt(1);
      B.addStr(getAIRAccess(Ty));
      B.addStr("air.address_space");
      B.addInt(C.getTargetAddressSpace(Ty->isPointerType() ||
                                               Ty->isReferenceType()
                                           ? Ty->getPointeeType().getAddressSpace()
                                           : Ty.getAddressSpace()));
      QualType Pointee = (Ty->isPointerType() || Ty->isReferenceType())
                             ? Ty->getPointeeType()
                             : Ty;
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
      B.addStr("air.address_space");
      B.addInt(3);
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
      B.addStr(FD->hasAttr<MetalVertexAttr>() ? "air.vertex_input"
                                              : "air.fragment_input");
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
  llvm::Metadata *FnMD = llvm::ConstantAsMetadata::get(Fn);
  llvm::Metadata *Outputs = llvm::MDNode::get(Ctx, std::nullopt);
  llvm::Metadata *Args = llvm::MDNode::get(Ctx, ArgNodes);

  llvm::SmallVector<llvm::Metadata *, 4> FnNodeOps{FnMD, Outputs, Args};

  // Function level attributes trail the operand list as bare strings. P02:
  //   !26 = !{<4 x float> (...)* @probe_p02_fragment_rog, !19, !27,
  //           !"early_fragment_tests"}
  if (FD->hasAttr<MetalEarlyFragmentTestsAttr>())
    FnNodeOps.push_back(llvm::MDString::get(Ctx, "early_fragment_tests"));

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
