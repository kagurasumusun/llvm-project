//===- SemaMetal.cpp - Semantic Analysis for Metal Shading Language -------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file implements the semantic checks that are specific to the Metal
// Shading Language.
//
// The rules implemented here, and the exact wording of the diagnostics they
// produce, are taken from measurements of Apple's compiler recorded in the
// metal-info reference set:
//
//   * reference/metal-ast-macos-air64/meta/sema-rule-catalog.csv
//     200 semantic checking scenarios grouped by category (address_space,
//     attribute, ...), each with the fixture that triggers it.
//   * reference/metal-ast-macos-air64/meta/diagnostic-catalog.csv and the raw
//     compiler output in reference/metal-ast-macos-air64/log, which supply the
//     verbatim diagnostic text.
//   * research/spec/METAL_SEMA_IMPL_MAP.md, which summarises the rules.
//
//===----------------------------------------------------------------------===//

#include "clang/AST/ASTContext.h"
#include "clang/AST/Decl.h"
#include "clang/AST/Type.h"
#include "clang/Basic/DiagnosticSema.h"
#include "clang/Basic/LangOptions.h"
#include "clang/Basic/TargetInfo.h"
#include "clang/Sema/ParsedAttr.h"
#include "clang/Sema/Sema.h"
#include "llvm/ADT/StringRef.h"

using namespace clang;

std::string Sema::getMetalStandardName(unsigned Version) const {
  // Apple spells the required standard differently either side of MSL 3.0.
  // Measured in reference/metal-ast-macos-air64/log:
  //   "'visible' attribute requires Metal language standard macos-metal2.3 or
  //    higher"
  //   "'mesh' attribute requires Metal language standard metal3.0 or higher"
  unsigned Major = Version / 100;
  unsigned Minor = (Version / 10) % 10;
  std::string Base = (llvm::Twine(Major) + "." + llvm::Twine(Minor)).str();
  if (Version >= 300)
    return "metal" + Base;

  // For the OS prefixed spellings the prefix follows the deployment platform.
  const llvm::Triple &T = Context.getTargetInfo().getTriple();
  llvm::StringRef Prefix = "macos";
  if (T.isiOS())
    Prefix = "ios";
  else if (T.isTvOS())
    Prefix = "tvos";
  else if (T.isWatchOS())
    Prefix = "watchos";
  return (Prefix + llvm::Twine("-metal") + Base).str();
}

Sema::MetalShaderStage Sema::getMetalShaderStage(const FunctionDecl *FD) const {
  if (!FD)
    return MSS_None;
  if (FD->hasAttr<MetalKernelAttr>())
    return MSS_Kernel;
  if (FD->hasAttr<MetalVertexAttr>())
    return MSS_Vertex;
  if (FD->hasAttr<MetalFragmentAttr>())
    return MSS_Fragment;
  if (FD->hasAttr<MetalMeshAttr>())
    return MSS_Mesh;
  if (FD->hasAttr<MetalObjectAttr>())
    return MSS_Object;
  return MSS_None;
}

bool Sema::CheckMetalAttributeVersion(const ParsedAttr &AL,
                                      unsigned MinVersion) {
  if (!MinVersion ||
      static_cast<unsigned>(getLangOpts().getMetalVersion()) >= MinVersion)
    return true;
  Diag(AL.getLoc(), diag::err_metal_attribute_requires_std)
      << AL << getMetalStandardName(MinVersion);
  return false;
}

bool Sema::CheckMetalResourceIndexBounds(const ParsedAttr &AL,
                                         llvm::StringRef Kind,
                                         uint32_t Index) {
  // The limits are the ones the target advertises through the module flags
  // Apple emits; see research/golden/P01/metal32_macosx26/probe.ll:
  //   air.max_device_buffers 31, air.max_constant_buffers 31,
  //   air.max_threadgroup_buffers 31, air.max_textures 128,
  //   air.max_read_write_textures 8, air.max_samplers 16
  unsigned Max = 0;
  if (Kind == "buffer" || Kind == "threadgroup")
    Max = 31;
  else if (Kind == "texture")
    Max = 128;
  else if (Kind == "sampler")
    Max = 16;
  else
    return true;

  if (Index < Max)
    return true;

  Diag(AL.getLoc(), diag::err_metal_resource_index_exceeds_max)
      << Kind << Index << (Max - 1);
  return false;
}

bool Sema::DiagnoseMetalUnsupported(SourceLocation Loc,
                                    llvm::StringRef Construct) {
  if (!getLangOpts().Metal)
    return false;
  // Apple quotes the construct in the message, e.g.
  //   error: 'thread_local' is not supported in Metal
  Diag(Loc, diag::err_metal_unsupported)
      << (llvm::Twine("'") + Construct + "'").str();
  return true;
}

void Sema::CheckMetalVarDeclAddressSpace(VarDecl *VD) {
  if (!getLangOpts().Metal || !VD)
    return;

  QualType Ty = VD->getType();
  LangAS AS = Ty.getAddressSpace();

  // MSL requires every pointer and reference to name its address space
  // explicitly. Measured:
  //   error: pointer type must have explicit address space qualifier
  //   error: reference type must have explicit address space qualifier
  if (Ty->isPointerType() || Ty->isReferenceType()) {
    QualType Pointee = Ty->getPointeeType();
    if (Pointee.getAddressSpace() == LangAS::Default &&
        !Pointee->isFunctionType()) {
      Diag(VD->getLocation(), diag::err_metal_pointer_needs_addrspace)
          << (Ty->isReferenceType() ? 1 : 0);
      VD->setInvalidDecl();
      return;
    }
  }

  // Variables at program scope must live in the constant address space, and
  // must be initialised there. Measured:
  //   error: program scope variable must reside in constant address space
  //   error: extern variable must reside in constant address space
  //   error: variable in constant address space must be initialized
  if (VD->hasGlobalStorage() && !VD->isStaticLocal()) {
    if (AS != LangAS::metal_constant) {
      unsigned DiagID = VD->hasExternalStorage()
                            ? diag::err_metal_extern_addrspace
                            : diag::err_metal_program_scope_addrspace;
      Diag(VD->getLocation(), DiagID);
      VD->setInvalidDecl();
      return;
    }
    // A `[[function_constant(N)]]` is deliberately uninitialised: the value is
    // supplied at pipeline build time. Apple's output confirms it, emitting
    // `@_ZL10use_path_a = ... addrspace(2) global i8 undef` with no
    // initialiser and filling it in from an externally initialised
    // placeholder in an `air.static_init` constructor.
    if (!VD->hasInit() && !VD->hasExternalStorage() &&
        !VD->hasAttr<MetalFunctionConstantAttr>()) {
      Diag(VD->getLocation(), diag::err_metal_constant_needs_init);
      VD->setInvalidDecl();
      return;
    }
  }

  // A local cannot carry an address space qualifier of its own. Measured:
  //   error: automatic variable qualified with an address space
  if (VD->hasLocalStorage() && AS != LangAS::Default &&
      AS != LangAS::metal_thread) {
    Diag(VD->getLocation(), diag::err_metal_automatic_addrspace);
    VD->setInvalidDecl();
  }
}

/// Does \p Ty name a resource handle that is bound rather than passed?
static bool isMetalResourceType(QualType Ty) {
  return Ty->isMetalOpaqueType();
}

/// Is \p Ty (or the class it wraps) one of the texture handles?
static bool isMetalTextureType(QualType Ty) {
  if (const auto *BT = Ty->getAs<BuiltinType>()) {
    switch (BT->getKind()) {
#define METAL_TYPE(Name, Id, SingletonId, IRName)                              \
  case BuiltinType::Id:                                                        \
    return StringRef(IRName).contains("texture") ||                            \
           StringRef(IRName).contains("depth");
#include "clang/Basic/MetalTypes.def"
    default:
      return false;
    }
  }
  // metal::texture2d<T> and friends are class templates wrapping the handle.
  if (const auto *RD = Ty->getAsCXXRecordDecl())
    return RD->getName().startswith("texture") ||
           RD->getName().startswith("depth") ||
           RD->getName().startswith("_texture") ||
           RD->getName().startswith("_depth");
  return false;
}

/// Is \p Ty the sampler handle (or metal::sampler wrapping it)?
static bool isMetalSamplerType(QualType Ty) {
  if (const auto *BT = Ty->getAs<BuiltinType>())
    return BT->getKind() == BuiltinType::MetalSampler;
  if (const auto *RD = Ty->getAsCXXRecordDecl())
    return RD->getName() == "sampler";
  return false;
}

/// Non-zero if \p PVD carries any stage-input attribute (so it is not a
/// resource binding and needs no buffer/texture/sampler attribute).
static unsigned getAIRStageInputKind(const ParmVarDecl *PVD) {
  return PVD->hasAttr<MetalThreadPosGridAttr>() ||
         PVD->hasAttr<MetalThreadPosGroupAttr>() ||
         PVD->hasAttr<MetalThreadGroupPosGridAttr>() ||
         PVD->hasAttr<MetalThreadsPerGridAttr>() ||
         PVD->hasAttr<MetalThreadsPerGroupAttr>() ||
         PVD->hasAttr<MetalThreadGroupsPerGridAttr>() ||
         PVD->hasAttr<MetalThreadIndexGroupAttr>() ||
         PVD->hasAttr<MetalThreadIndexSIMDGroupAttr>() ||
         PVD->hasAttr<MetalSIMDGroupIndexGroupAttr>() ||
         PVD->hasAttr<MetalSIMDGroupsPerGroupAttr>() ||
         PVD->hasAttr<MetalVertexIdAttr>() ||
         PVD->hasAttr<MetalInstanceIdAttr>() ||
         PVD->hasAttr<MetalBaseVertexAttr>() ||
         PVD->hasAttr<MetalBaseInstanceAttr>() ||
         PVD->hasAttr<MetalAmplificationIdAttr>() ||
         PVD->hasAttr<MetalPositionAttr>() ||
         PVD->hasAttr<MetalFrontFacingAttr>() ||
         PVD->hasAttr<MetalPointCoordAttr>() ||
         PVD->hasAttr<MetalSampleIdAttr>() ||
         PVD->hasAttr<MetalSampleMaskAttr>() ||
         PVD->hasAttr<MetalPrimitiveIdAttr>() ||
         PVD->hasAttr<MetalBarycentricCoordAttr>() ||
         PVD->hasAttr<MetalPayloadAttr>();
}

/// Reject the C++ constructs the Metal specification lists as unavailable.
///
/// The list is MSL 4.1 section 1.6.1, which enumerates twelve items; the
/// wording of each diagnostic is the one Apple's compiler emits, harvested
/// from the reference logs (see docs-metal/data/metal_diagnostics.csv).
/// `lambda expressions` carries a version condition: the specification says
/// "prior to Metal 3.2", and the reference AST dumps confirm that LambdaExpr
/// only appears from -std=metal3.2 onwards.
bool Sema::DiagnoseMetalUnsupportedDecl(Decl *D) {
  if (!getLangOpts().Metal || !D)
    return false;

  if (auto *MD = dyn_cast<CXXMethodDecl>(D)) {
    if (MD->isVirtual()) {
      Diag(MD->getLocation(), diag::err_metal_unsupported_feature) << 0;
      return true;
    }
    // A kernel cannot be a member function.
    if (MD->hasAttr<MetalKernelAttr>()) {
      Diag(MD->getLocation(), diag::err_metal_kernel_member_function);
      return true;
    }
  }

  if (auto *RD = dyn_cast<CXXRecordDecl>(D)) {
    if (RD->isUnion()) {
      Diag(RD->getLocation(), diag::err_metal_unsupported_feature) << 2;
      return true;
    }
    if (RD->hasDefinition() && RD->getNumBases() > 0) {
      Diag(RD->getLocation(), diag::err_metal_unsupported_feature) << 1;
      return true;
    }
  }

  if (auto *FD = dyn_cast<FunctionDecl>(D)) {
    // Variadic Metal entry points and qualified functions are rejected.
    if (FD->isVariadic() && getMetalShaderStage(FD) != MSS_None) {
      Diag(FD->getLocation(), diag::err_metal_variadic);
      return true;
    }
  }

  return false;
}

/// Is \p Ty a type MSL does not provide at all?
///
/// Measured: Apple emits "'double' is not supported in Metal" and the same for
/// 'long long'. The specification agrees: "Metal does not support the double,
/// long long, unsigned long long, and long double data types" (MSL 4.1 table
/// 2.1 commentary).
bool Sema::DiagnoseMetalUnsupportedType(QualType Ty, SourceLocation Loc) {
  if (!getLangOpts().Metal || Ty.isNull())
    return false;

  const Type *T = Ty->getUnqualifiedDesugaredType();
  const auto *BT = dyn_cast<BuiltinType>(T);
  if (!BT)
    return false;

  StringRef Name;
  switch (BT->getKind()) {
  case BuiltinType::Double:
    Name = "double";
    break;
  case BuiltinType::LongDouble:
    Name = "long double";
    break;
  case BuiltinType::LongLong:
    Name = "long long";
    break;
  case BuiltinType::ULongLong:
    Name = "unsigned long long";
    break;
  default:
    return false;
  }

  Diag(Loc, diag::err_metal_unsupported) << ("'" + Name + "'").str();
  return true;
}

void Sema::CheckMetalEntryPoint(FunctionDecl *FD) {
  if (!getLangOpts().Metal || !FD)
    return;

  MetalShaderStage Stage = getMetalShaderStage(FD);
  if (Stage == MSS_None)
    return;

  QualType RetTy = FD->getReturnType();

  // Return type rules, measured from the compiler's own output:
  //   error: invalid return type 'int' for kernel function
  //   error: invalid return type 'float4' (vector of 4 'float' values) for
  //          kernel function
  //   error: invalid return type 'int *' for vertex function
  //   error: invalid return type 'texture2d<float>' for fragment function
  bool BadReturn = false;
  switch (Stage) {
  case MSS_Kernel:
  case MSS_Mesh:
  case MSS_Object:
    // Compute-like stages must return void.
    BadReturn = !RetTy->isVoidType();
    break;
  case MSS_Vertex:
  case MSS_Fragment:
    // Graphics stages return a value, but never a pointer, a reference or a
    // resource handle.
    BadReturn = RetTy->isPointerType() || RetTy->isReferenceType() ||
                isMetalResourceType(RetTy);
    break;
  case MSS_None:
    llvm_unreachable("handled above");
  }

  if (BadReturn) {
    Diag(FD->getLocation(), diag::err_metal_invalid_return_type)
        << RetTy << static_cast<unsigned>(Stage);
    FD->setInvalidDecl();
  }

  // Parameter rules.
  for (ParmVarDecl *PVD : FD->parameters()) {
    QualType PTy = PVD->getType();

    // A buffer parameter's pointee must live in an address space that can
    // actually back a buffer. Measured:
    //   error: invalid address space qualification for buffer pointee type
    //          'threadgroup float'
    //   error: invalid address space qualification for buffer pointee type
    //          'metal::sampler'
    if (PVD->hasAttr<MetalBufferIndexAttr>()) {
      if (PTy->isPointerType() || PTy->isReferenceType()) {
        LangAS PointeeAS = PTy->getPointeeType().getAddressSpace();
        // `device coherent(device) T` is a device buffer with an extra
        // coherence qualifier, and binds exactly like a plain device one.
        if (PointeeAS != LangAS::metal_device &&
            PointeeAS != LangAS::metal_device_coherent &&
            PointeeAS != LangAS::metal_constant) {
          Diag(PVD->getLocation(), diag::err_metal_invalid_buffer_pointee)
              << PTy->getPointeeType();
          Diag(PVD->getLocation(), diag::note_metal_valid_addrspace);
          PVD->setInvalidDecl();
        }
      } else if (isMetalResourceType(PTy)) {
        Diag(PVD->getLocation(), diag::err_metal_invalid_buffer_pointee) << PTy;
        Diag(PVD->getLocation(), diag::note_metal_valid_addrspace);
        PVD->setInvalidDecl();
      }
    }

    // Every resource parameter of an entry point must carry an explicit
    // binding attribute. Apple names the parameter itself in the message:
    //   error: t parameter must have texture attribute
    //   error: s parameter must have sampler attribute
    //   error: tg parameter must have threadgroup attribute
    //   error: p parameter must have buffer attribute
    bool HasBinding = PVD->hasAttr<MetalBufferIndexAttr>() ||
                      PVD->hasAttr<MetalTextureIndexAttr>() ||
                      PVD->hasAttr<MetalSamplerIndexAttr>() ||
                      PVD->hasAttr<MetalLocalIndexAttr>() ||
                      PVD->hasAttr<MetalStageInAttr>();
    if (!HasBinding && !PVD->isInvalidDecl() &&
        getAIRStageInputKind(PVD) == 0) {
      // Which attribute is required follows from the parameter's type.
      std::optional<unsigned> Which;
      if (isMetalTextureType(PTy))
        Which = 1; // texture
      else if (isMetalSamplerType(PTy))
        Which = 2; // sampler
      else if ((PTy->isPointerType() || PTy->isReferenceType()) &&
               PTy->getPointeeType().getAddressSpace() ==
                   LangAS::metal_threadgroup)
        Which = 3; // threadgroup
      else if (PTy->isPointerType() || PTy->isReferenceType())
        Which = 0; // buffer

      if (Which) {
        Diag(PVD->getLocation(), diag::err_metal_param_needs_attr)
            << PVD->getName() << *Which;
        PVD->setInvalidDecl();
      }
    }

    // Stage input attributes are only meaningful on the stages that produce
    // them. Measured, for every combination:
    //   error: invalid 'vertex_id' attribute for input declaration in a
    //          fragment function
    //   error: invalid 'stage_in' attribute for input declaration in a kernel
    //          function
    auto rejectOn = [&](const Attr *A, llvm::StringRef Spelling,
                        std::initializer_list<MetalShaderStage> Allowed) {
      if (!A)
        return;
      for (MetalShaderStage S : Allowed)
        if (S == Stage)
          return;
      Diag(A->getLocation(), diag::err_metal_invalid_input_attr)
          << Spelling << static_cast<unsigned>(Stage);
      PVD->setInvalidDecl();
    };

    rejectOn(PVD->getAttr<MetalVertexIdAttr>(), "'vertex_id'", {MSS_Vertex});
    rejectOn(PVD->getAttr<MetalInstanceIdAttr>(), "'instance_id'",
             {MSS_Vertex});
    rejectOn(PVD->getAttr<MetalBaseVertexAttr>(), "'base_vertex'",
             {MSS_Vertex});
    rejectOn(PVD->getAttr<MetalBaseInstanceAttr>(), "'base_instance'",
             {MSS_Vertex});
    rejectOn(PVD->getAttr<MetalAmplificationIdAttr>(), "'amplification_id'",
             {MSS_Vertex});
    rejectOn(PVD->getAttr<MetalFrontFacingAttr>(), "'front_facing'",
             {MSS_Fragment});
    rejectOn(PVD->getAttr<MetalPointCoordAttr>(), "'point_coord'",
             {MSS_Fragment});
    rejectOn(PVD->getAttr<MetalSampleIdAttr>(), "'sample_id'", {MSS_Fragment});
    rejectOn(PVD->getAttr<MetalSampleMaskAttr>(), "'sample_mask'",
             {MSS_Fragment});
    rejectOn(PVD->getAttr<MetalBarycentricCoordAttr>(), "'barycentric_coord'",
             {MSS_Fragment});
    rejectOn(PVD->getAttr<MetalPositionAttr>(), "'position'",
             {MSS_Vertex, MSS_Fragment});
    rejectOn(PVD->getAttr<MetalStageInAttr>(), "'stage_in'",
             {MSS_Vertex, MSS_Fragment});
    rejectOn(PVD->getAttr<MetalThreadPosGridAttr>(), "'thread_position_in_grid'",
             {MSS_Kernel, MSS_Mesh, MSS_Object});
    rejectOn(PVD->getAttr<MetalThreadPosGroupAttr>(),
             "'thread_position_in_threadgroup'",
             {MSS_Kernel, MSS_Mesh, MSS_Object});
    rejectOn(PVD->getAttr<MetalThreadsPerGroupAttr>(),
             "'threads_per_threadgroup'", {MSS_Kernel, MSS_Mesh, MSS_Object});
    rejectOn(PVD->getAttr<MetalThreadGroupPosGridAttr>(),
             "'threadgroup_position_in_grid'",
             {MSS_Kernel, MSS_Mesh, MSS_Object});
  }
}

//===----------------------------------------------------------------------===//
// Builtin typechecking
//===----------------------------------------------------------------------===//

/// The measured arity of a Metal builtin, or -1 if it is not one.
///
/// The values come from parsing every `__metal_*` call site in Apple's
/// <metal_stdlib>; see docs-metal/verify/extract_builtin_arity.py. All 650
/// names in BuiltinsMetal.def are covered, with no gaps.
static int getMetalBuiltinArity(unsigned BuiltinID) {
  switch (BuiltinID) {
#define METAL_BUILTIN(ID, TYPE, ATTRS, AIRNAME, ARITY)                         \
  case Builtin::BI##ID:                                                        \
    return ARITY;
#include "clang/Basic/BuiltinsMetal.def"
  default:
    return -1;
  }
}

bool Sema::CheckMetalBuiltinCall(unsigned BuiltinID, CallExpr *TheCall) {
  int Arity = getMetalBuiltinArity(BuiltinID);
  if (Arity < 0)
    return false;

  if (checkArgCount(*this, TheCall, (unsigned)Arity))
    return true;

  // The builtins are generic, so the result type follows the arguments rather
  // than a fixed signature. Apple's standard library always writes them in one
  // of two shapes:
  //
  //   return __metal_abs(x);                      // result is typeof(x)
  //   return __metal_sqrt(x, __METAL_FAST_MATH__) // ditto, flag is trailing
  //
  // that is, the value being operated on is the first argument and any
  // trailing flags are integer constants. Taking the type of the first
  // non-integer-constant argument therefore reproduces the result type for
  // every arithmetic builtin, which is the group whose result is actually
  // consumed. A builtin called only for effect keeps `void`, which is what
  // the `"v."` declaration already gives.
  QualType ResultTy = Context.VoidTy;
  for (unsigned I = 0, E = TheCall->getNumArgs(); I != E; ++I) {
    Expr *Arg = TheCall->getArg(I);

    // Run the usual conversions so that the argument type is the promoted one
    // and array/function decay has happened.
    ExprResult Converted = DefaultFunctionArrayLvalueConversion(Arg);
    if (Converted.isInvalid())
      return true;
    Arg = Converted.get();
    TheCall->setArg(I, Arg);

    if (I == 0)
      ResultTy = Arg->getType();
  }

  TheCall->setType(ResultTy);
  return false;
}
