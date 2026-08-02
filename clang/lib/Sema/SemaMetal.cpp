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

// Builtins.h has to come before anything that pulls in Builtins.def with only
// a subset of the callback macros defined (Expr.h does that for
// ATOMIC_BUILTIN); otherwise the METAL_BUILTIN entries never reach the
// Builtin::ID enum and getMetalBuiltinArity fails to compile.
#include "clang/Basic/Builtins.h"
#include "clang/AST/ASTContext.h"
#include "clang/AST/Decl.h"
#include "clang/AST/Expr.h"
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

  // For the OS prefixed spellings only two prefix families exist before
  // MSL 3.0: `macos` and `ios`. Every other Apple platform compiles pre-3.0
  // Metal with the `ios-metal*` standards — measured cc1 invocations:
  //   reference/metal-ast-tvos-air64/meta/metal-cc1-invocations.txt:
  //     tvOS targets use -std=ios-metal2.0 .. ios-metal2.4
  //   reference/metal-ast-watchos-air64/meta/...: watchOS uses the same set.
  // There is no `tvos-metal*` / `watchos-metal*` / `xros-metal*` spelling in
  // any specification or measurement, so diagnostics must never fabricate it.
  const llvm::Triple &T = Context.getTargetInfo().getTriple();
  llvm::StringRef Prefix = T.isMacOSX() ? "macos" : "ios";
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

void Sema::ActOnMetalFPMathMode(SourceLocation Loc, MetalFPMathMode Mode) {
  if (!getLangOpts().Metal)
    return;

  // Map the Metal spelling onto the float_control pair: safe and precise
  // request the precise evaluation mode (all fast-math relaxations off,
  // contraction within a statement); fast requests the fast-math mode. This
  // is exactly the override pair FPOptionsOverride::setFPPreciseEnabled
  // computes, and it matches how Apple's <metal_math> uses math_mode(safe)
  // 90 times around bit-twiddling helpers that must not be reassociated.
  //
  // The scope-of-effect is the enclosing compound statement. Nothing has to
  // be undone here: ParseCompoundStatementBody wraps every body in
  // FPFeaturesStateRAII, so CurFPFeatures and FpPragmaStack.CurrentValue are
  // restored when the closing '}' is consumed. A pragma at file scope is
  // outside any compound statement and lasts to the end of the TU.
  FPOptionsOverride NewFPFeatures = CurFPFeatureOverrides();
  NewFPFeatures.setFPPreciseEnabled(Mode != MetalFPMath_Fast);
  FpPragmaStack.Act(Loc, PSK_Set, StringRef(), NewFPFeatures);
  CurFPFeatures = NewFPFeatures.applyOverrides(getLangOpts());
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

/// The inclusive binding-slot count the target grants for a resource kind.
///
/// Measured per target triple and language version by probing which index
/// values Apple rejects with
///   error: '<kind>' attribute parameter is out of bounds: must be between 0
///          and <Limit-1>
/// (reference/metal-ast-*/log, fixtures attr_*_overflow and
/// sig_vertex_too_many_inputs). Buffers (device, constant and threadgroup)
/// and samplers are uniform everywhere: 31 buffers, 16 samplers. Vertex
/// [[attribute(n)]] slots are 31. Only the texture count varies; the
/// complete measured matrix is:
///
///   [[texture(32)]]       ios-metal1.0  1.1  1.2   2.0+ (per std)
///   ios-air32               REJECT     ok    ok     ok
///   ios-air64 + sim-air64   REJECT   REJ   REJ     ok
///   tvos-air32 / air64      n/a      REJ   REJ     ok
///   watchos-air32 / air64    ok       ok    ok     ok
///   macos-air32 / air64      ok       ok    ok     ok
///   xros-air64               ok (only version measured)
///
/// i.e. the legacy 31-texture limit applies to iOS/tvOS (device and
/// simulator) before Metal 2.0, with the odd measured exception that 32-bit
/// iOS raises it already at ios-metal1.1. macOS, watchOS and xrOS accept
/// high indices at every measured version. Where the raised limit applies
/// the compiler advertises 128 slots through the air.max_textures module
/// flag (research/golden/P01/metal32_macosx26/probe.ll), so 128 is also the
/// Sema limit.
static unsigned getMetalResourceLimit(const ASTContext &Ctx,
                                      const LangOptions &Opts,
                                      llvm::StringRef Kind) {
  if (Kind == "sampler")
    return 16;
  if (Kind != "texture")
    return 31; // buffer, threadgroup and [[attribute(n)]] all measured 31.

  const llvm::Triple &T = Ctx.getTargetInfo().getTriple();
  if (T.isMacOSX() || T.isWatchOS())
    return 128;
  // Remaining platforms are the iOS family, including tvOS (Triple::isiOS
  // covers both).
  if (!T.isiOS())
    return 128;
  if (static_cast<unsigned>(Opts.getMetalVersion()) >= 200)
    return 128;
  // 32-bit iOS raises the limit one standard earlier, measured above.
  if (Ctx.getTargetInfo().getPointerWidth(LangAS::Default) == 32 &&
      static_cast<unsigned>(Opts.getMetalVersion()) >= 110)
    return 128;
  return 31;
}

/// Wrap an attribute spelling in the single quotes Apple prints around
/// attribute names in diagnostics. std::string diagnostic arguments are
/// inserted verbatim (the diagnostics engine adds quotes only for
/// QualType/Attr*/ParsedAttr arguments), so the quotes must be part of
/// the argument itself.
static std::string metalQuoteSpelling(llvm::StringRef Name) {
  return ("'" + Name + "'").str();
}

bool Sema::CheckMetalResourceIndexBounds(const ParsedAttr &AL,
                                         llvm::StringRef Kind,
                                         int64_t Index) {
  unsigned Limit = getMetalResourceLimit(Context, getLangOpts(), Kind);
  if (Index >= 0 && static_cast<uint64_t>(Index) < Limit)
    return true;

  Diag(AL.getLoc(), diag::err_metal_attr_out_of_bounds)
      << metalQuoteSpelling(Kind) << 0 << (Limit - 1);
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

/// Is \p Ty a pointer or member pointer to a function type?
static bool isMetalFunctionPointerType(QualType Ty) {
  const Type *T = Ty->getUnqualifiedDesugaredType();
  if (const auto *PT = dyn_cast<PointerType>(T))
    return PT->getPointeeType()->isFunctionType();
  if (const auto *MPT = dyn_cast<MemberPointerType>(T))
    return MPT->getPointeeType()->isFunctionType();
  return false;
}

/// Is \p Ty one of the Metal matrix types (float4x4 and friends)? The
/// standard library declares them as the class template
///   template <typename T, int Cols, int Rows = Cols> struct metal::matrix;
/// (<metal_matrix>), and float4x4 is a typedef of matrix<float, 4, 4>. The
/// alias is already desugared away when the member type is examined, so
/// match the record by name, the same way textures and samplers are
/// recognized. Clang's -fenable-matrix extension type is honored too.
static bool isMetalMatrixType(QualType Ty) {
  const Type *T = Ty->getUnqualifiedDesugaredType();
  if (T->isMatrixType())
    return true;
  if (const auto *RD = T->getAsCXXRecordDecl())
    return RD->getName() == "matrix" || RD->getName() == "_matrix";
  return false;
}

/// Is \p Ty one of the packed vector types?
static bool isMetalPackedVectorType(QualType Ty) {
  const auto *VT =
      dyn_cast<VectorType>(Ty->getUnqualifiedDesugaredType());
  return VT && VT->getVectorKind() == VectorType::MetalPackedVector;
}

/// Is \p Ty exactly float4, the only legal [[position]] type
/// (MSL 3.2 table 5.4)?
static bool isMetalFloat4Type(QualType Ty) {
  const Type *T = Ty->getUnqualifiedDesugaredType();
  const auto *VT = dyn_cast<VectorType>(T);
  if (!VT || VT->getNumElements() != 4 ||
      VT->getVectorKind() == VectorType::MetalPackedVector)
    return false;
  const auto *BT = dyn_cast<BuiltinType>(VT->getElementType());
  return BT && BT->getKind() == BuiltinType::Float;
}

void Sema::CheckMetalVarDeclAddressSpace(VarDecl *VD) {
  if (!getLangOpts().Metal || !VD)
    return;

  // Measured storage class restrictions:
  //   error: variables in function scope cannot be declared static
  //     (cf_static_local fixture, every measured standard)
  //   error: Metal does not support the 'register' storage class specifier
  //     (type_register fixture)
  if (VD->isStaticLocal()) {
    Diag(VD->getLocation(), diag::err_metal_static_local);
    VD->setInvalidDecl();
    return;
  }
  if (VD->getStorageClass() == SC_Register) {
    Diag(VD->getLocation(), diag::err_metal_storage_class) << "register";
    VD->setInvalidDecl();
    return;
  }

  // Pointers to functions (including pointers to member *functions*) are
  // illegal before Metal 2.1. Measured boundary: era_function_pointer_* and
  // cf_pointer_to_member_function error with
  //   error: pointers to functions are not allowed
  // at macos-metal1.1/1.2/2.0 and compile cleanly from macos-metal2.1 on.
  // Pointers to member *data* are fine at every standard (cxx_022 passes at
  // macos-metal1.1), so only pointee function types are rejected.
  if (static_cast<unsigned>(getLangOpts().getMetalVersion()) < 210 &&
      isMetalFunctionPointerType(VD->getType()))
    Diag(VD->getLocation(), diag::err_metal_function_pointer);

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

    // Measured (cf_main_function): a plain function must not be called main:
    //   error: non-qualified function cannot be called 'main'
    if (FD->getDeclName().isIdentifier() && FD->getName() == "main" &&
        getMetalShaderStage(FD) == MSS_None) {
      Diag(FD->getLocation(), diag::err_metal_main_function);
      return true;
    }
  }

  // goto is not permitted in Metal (MSL 4.1 section 1.6.1).
  // Label statements are caught by their parent function; rejecting here
  // would fire on every label, not just goto targets.  The actual goto
  // diagnostic is emitted in SemaStmt.cpp when the parser sees 'goto'.

  return false;
}

/// Is \p Ty a type MSL does not provide at all?
///
/// Measured: Apple emits "'double' is not supported in Metal" and the same for
/// 'long long'. The specification agrees: "Metal does not support the double,
/// long long, unsigned long long, and long double data types" (MSL 4.1 table
/// 2.1 commentary).
bool Sema::DiagnoseMetalUnsupportedExpr(Expr *E) {
  if (!getLangOpts().Metal || !E)
    return false;

  // MSL 4.1 section 1.6.1 lists dynamic_cast and typeid as unavailable.
  if (isa<CXXDynamicCastExpr>(E) || isa<CXXTypeidExpr>(E)) {
    Diag(E->getBeginLoc(), diag::err_metal_unsupported)
        << (llvm::Twine("'") + E->getStmtClassName() + "'").str();
    return true;
  }

  // RTTI (typeid on types, dynamic_cast) is broadly unavailable.
  // CXXDynamicCast and CXXTypeidExpr cover the expression forms;
  // RTTI descriptor generation is suppressed via -fno-rtti in the
  // driver, which is the Metal default.

  return false;
}

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

/// The built-in stage attributes that are unique per entry point signature.
/// Measured, for a [[position]] present both inside the stage_in struct and
/// on a direct parameter:
///   error: declaration with attribute 'position' already specified
///   note: previous declaration with attribute 'position' here
static llvm::SmallVector<std::pair<const Attr *, StringRef>, 4>
getMetalBuiltinStageAttrs(const Decl *D) {
  llvm::SmallVector<std::pair<const Attr *, StringRef>, 4> Out;
  auto add = [&](const Attr *A, StringRef N) {
    if (A)
      Out.push_back({A, N});
  };
  add(D->getAttr<MetalPositionAttr>(), "position");
  add(D->getAttr<MetalPointSizeAttr>(), "point_size");
  add(D->getAttr<MetalClipDistanceAttr>(), "clip_distance");
  add(D->getAttr<MetalFrontFacingAttr>(), "front_facing");
  add(D->getAttr<MetalPointCoordAttr>(), "point_coord");
  add(D->getAttr<MetalSampleIdAttr>(), "sample_id");
  add(D->getAttr<MetalSampleMaskAttr>(), "sample_mask");
  add(D->getAttr<MetalVertexIdAttr>(), "vertex_id");
  add(D->getAttr<MetalInstanceIdAttr>(), "instance_id");
  add(D->getAttr<MetalBaseVertexAttr>(), "base_vertex");
  add(D->getAttr<MetalBaseInstanceAttr>(), "base_instance");
  add(D->getAttr<MetalAmplificationIdAttr>(), "amplification_id");
  add(D->getAttr<MetalPrimitiveIdAttr>(), "primitive_id");
  add(D->getAttr<MetalBarycentricCoordAttr>(), "barycentric_coord");
  add(D->getAttr<MetalRenderTargetArrayIndexAttr>(),
      "render_target_array_index");
  add(D->getAttr<MetalViewportArrayIndexAttr>(), "viewport_array_index");
  return Out;
}

/// Is \p Ty a scalar that can never be an entry-point input on its own?
static bool isMetalEntryInvalidScalar(QualType Ty) {
  const Type *T = Ty->getUnqualifiedDesugaredType();
  return T->isArithmeticType() || T->isEnumeralType();
}

/// Validate the struct behind a [[stage_in]] parameter. Apple cascades from
/// the individual member errors to the whole stage_in declaration:
///   error: type 'MIn' is not valid for attribute 'stage_in'
///   error: type 'metal::float4x4' (aka 'matrix<float; 4; 4>') is not valid
///          for attribute 'attribute'
/// measured for struct members that are matrices, packed vectors, or whose
/// own [[attribute(n)]] index is out of bounds.
void Sema::CheckMetalStageInStruct(ParmVarDecl *PVD, CXXRecordDecl *RD) {
  if (!RD || !RD->getDefinition())
    return;

  unsigned AttrLimit =
      getMetalResourceLimit(Context, getLangOpts(), "attribute");
  bool Bad = false;

  for (FieldDecl *Field : RD->getDefinition()->fields()) {
    QualType FTy = Field->getType();
    const auto *AA = Field->getAttr<MetalAttributeIndexAttr>();

    // A member whose [[attribute(n)]] index is out of bounds invalidates
    // the whole stage_in struct; the out-of-bounds error itself was already
    // reported when the field declaration was parsed.
    if (AA && cast<ConstantExpr>(AA->getIndex())
                      ->getResultAsAPSInt()
                      .getExtValue() >= (int64_t)AttrLimit)
      Bad = true;

    if (!isMetalMatrixType(FTy) && !isMetalPackedVectorType(FTy))
      continue;
    Bad = true;
    // The member itself is diagnosed against the attribute it carries,
    // measured as a separate line after the stage_in error.
    if (AA)
      Diag(AA->getLocation(), diag::err_metal_invalid_attr_type)
          << FTy << "'attribute'";
  }

  if (Bad) {
    Diag(PVD->getLocation(), diag::err_metal_invalid_attr_type)
        << Context.getTypeDeclType(RD) << "'stage_in'";
    PVD->setInvalidDecl();
  }
}

void Sema::CheckMetalParamDecl(ParmVarDecl *PVD) {
  if (!getLangOpts().Metal || !PVD)
    return;

  // 'register' is banned anywhere (type_register fixture measures the
  // variable form; the parameter form carries the same specifier).
  if (PVD->getStorageClass() == SC_Register) {
    Diag(PVD->getLocation(), diag::err_metal_storage_class) << "register";
    PVD->setInvalidDecl();
    return;
  }

  // Pre-Metal 2.1 function pointer parameters, same rule as variables
  // (measured on the declaration: "pointers to functions are not allowed").
  if (static_cast<unsigned>(getLangOpts().getMetalVersion()) < 210 &&
      isMetalFunctionPointerType(PVD->getType()))
    Diag(PVD->getLocation(), diag::err_metal_function_pointer);

  QualType T = PVD->getType();
  bool IsPtrOrRef = T->isPointerType() || T->isReferenceType();

  // Measured (address_spaces_ray_data_all): a parameter whose *value* type
  // itself carries an address space qualifier is rejected with
  //   error: parameter may not be qualified with an address space
  // The qualifier on a pointer's pointee is fine; that is the buffer form.
  if (!IsPtrOrRef && T.getAddressSpace() != LangAS::Default &&
      T.getAddressSpace() != LangAS::metal_thread) {
    Diag(PVD->getLocation(), diag::err_metal_param_addrspace);
    PVD->setInvalidDecl();
  }

  // Measured at metal2.3 through metal3.1: a stage input attribute on a
  // reference parameter type is rejected with
  //   error: type 'ray_data uint &' (aka '...') is not valid for attribute
  //          'primitive_id'
  //   error: type 'ray_data float2 &' is not valid for attribute
  //          'barycentric_coord'
  // regardless of which address space the pointee lives in. Stage inputs
  // are passed by value.
  {
    auto rejectRef = [&](const Attr *A, const char *Name) {
      if (!A || !IsPtrOrRef)
        return;
      Diag(A->getLocation(), diag::err_metal_invalid_attr_type)
          << PVD->getType() << metalQuoteSpelling(Name);
      PVD->setInvalidDecl();
    };
    rejectRef(PVD->getAttr<MetalStageInAttr>(), "stage_in");
    rejectRef(PVD->getAttr<MetalThreadPosGridAttr>(),
              "thread_position_in_grid");
    rejectRef(PVD->getAttr<MetalThreadPosGroupAttr>(),
              "thread_position_in_threadgroup");
    rejectRef(PVD->getAttr<MetalThreadGroupPosGridAttr>(),
              "threadgroup_position_in_grid");
    rejectRef(PVD->getAttr<MetalThreadsPerGridAttr>(), "threads_per_grid");
    rejectRef(PVD->getAttr<MetalThreadsPerGroupAttr>(),
              "threads_per_threadgroup");
    rejectRef(PVD->getAttr<MetalThreadGroupsPerGridAttr>(),
              "threadgroups_per_grid");
    rejectRef(PVD->getAttr<MetalThreadIndexGroupAttr>(),
              "thread_index_in_threadgroup");
    rejectRef(PVD->getAttr<MetalThreadIndexSIMDGroupAttr>(),
              "thread_index_in_simdgroup");
    rejectRef(PVD->getAttr<MetalSIMDGroupIndexGroupAttr>(),
              "simdgroup_index_in_threadgroup");
    rejectRef(PVD->getAttr<MetalSIMDGroupsPerGroupAttr>(),
              "simdgroups_per_threadgroup");
    rejectRef(PVD->getAttr<MetalVertexIdAttr>(), "vertex_id");
    rejectRef(PVD->getAttr<MetalInstanceIdAttr>(), "instance_id");
    rejectRef(PVD->getAttr<MetalBaseVertexAttr>(), "base_vertex");
    rejectRef(PVD->getAttr<MetalBaseInstanceAttr>(), "base_instance");
    rejectRef(PVD->getAttr<MetalAmplificationIdAttr>(), "amplification_id");
    rejectRef(PVD->getAttr<MetalPositionAttr>(), "position");
    rejectRef(PVD->getAttr<MetalFrontFacingAttr>(), "front_facing");
    rejectRef(PVD->getAttr<MetalPointCoordAttr>(), "point_coord");
    rejectRef(PVD->getAttr<MetalSampleIdAttr>(), "sample_id");
    rejectRef(PVD->getAttr<MetalSampleMaskAttr>(), "sample_mask");
    rejectRef(PVD->getAttr<MetalPrimitiveIdAttr>(), "primitive_id");
    rejectRef(PVD->getAttr<MetalBarycentricCoordAttr>(), "barycentric_coord");
    rejectRef(PVD->getAttr<MetalPayloadAttr>(), "payload");
  }
}

void Sema::CheckMetalEntryPoint(FunctionDecl *FD) {
  if (!getLangOpts().Metal || !FD)
    return;

  MetalShaderStage Stage = getMetalShaderStage(FD);
  if (Stage == MSS_None)
    return;

  // Built-in stage attributes may appear once per entry point signature.
  // The stage_in struct members are declarations of their own, so they
  // participate in the same uniqueness rule; the measured conflict (the
  // struct's [[position]] vs the direct parameter's) is reported with
  // err_metal_attr_already_specified + note_metal_attr_previous_specified.
  llvm::SmallDenseMap<StringRef, SourceLocation, 16> SeenBuiltins;
  auto noteDuplicates = [&](const Decl *D) {
    for (const auto &Slot : getMetalBuiltinStageAttrs(D)) {
      auto R = SeenBuiltins.try_emplace(Slot.second, Slot.first->getLocation());
      if (!R.second) {
        Diag(Slot.first->getLocation(),
             diag::err_metal_attr_already_specified)
            << metalQuoteSpelling(Slot.second);
        Diag(R.first->second, diag::note_metal_attr_previous_specified)
            << metalQuoteSpelling(Slot.second);
      }
    }
  };
  for (ParmVarDecl *PVD : FD->parameters()) {
    if (!PVD->hasAttr<MetalStageInAttr>())
      continue;
    // Use RecordDecl rather than CXXRecordDecl: a stage_in record can be
    // introduced through a typedef or a non-C++ record spelling, but its
    // fields still share the entry point's builtin-attribute namespace.
    if (auto *RD = PVD->getType()->getUnqualifiedDesugaredType()
                       ->getAsRecordDecl())
      if (const RecordDecl *Def = RD->getDefinition())
        for (const FieldDecl *Field : Def->fields())
          noteDuplicates(Field);
  }
  for (ParmVarDecl *PVD : FD->parameters())
    noteDuplicates(PVD);

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

  // Struct returns: every member that carries a *stage output* attribute
  // must carry one that belongs to the returning stage. Measured, with the
  // cascade note pointing at the offending member:
  //   error: invalid return type 'VertexOut' for vertex function
  //   note: invalid 'color' attribute for output declaration
  // The per-stage output attribute sets come from MSL 3.2 tables 5.4 (vertex)
  // and 5.7 (fragment): color, depth and sample_mask are fragment outputs;
  // position and point_size are vertex outputs.
  SmallVector<std::pair<const Attr *, const char *>, 2> BadOutputAttrs;
  if (!BadReturn) {
    if (const auto *RT = RetTy->getUnqualifiedDesugaredType();
        RT->isRecordType()) {
      auto *RD = cast<CXXRecordDecl>(RT->getAsRecordDecl());
      for (FieldDecl *Field : RD->getDefinition()->fields()) {
        auto memberBad = [&](const Attr *A, const char *Name, bool VertexOK) {
          if (!A)
            return;
          bool Bad = (Stage == MSS_Vertex) ? !VertexOK : VertexOK;
          if (Bad)
            BadOutputAttrs.push_back({A, Name});
        };
        memberBad(Field->getAttr<MetalColorAttr>(), "color", false);
        memberBad(Field->getAttr<MetalDepthAttr>(), "depth", false);
        memberBad(Field->getAttr<MetalSampleMaskAttr>(), "sample_mask", false);
        memberBad(Field->getAttr<MetalPositionAttr>(), "position", true);
        memberBad(Field->getAttr<MetalPointSizeAttr>(), "point_size", true);
        memberBad(Field->getAttr<MetalClipDistanceAttr>(), "clip_distance",
                  true);
      }
      if (!BadOutputAttrs.empty())
        BadReturn = true;
    }
  }

  if (BadReturn) {
    Diag(FD->getLocation(), diag::err_metal_invalid_return_type)
        << RetTy << static_cast<unsigned>(Stage);
    FD->setInvalidDecl();
    for (const auto &Bad : BadOutputAttrs)
      Diag(Bad.first->getLocation(), diag::note_metal_invalid_output_attr)
          << metalQuoteSpelling(Bad.second);
  }

  // Duplicate resource binding locations are claimed per index:
  //   error: cannot reserve 'buffer' resource location at index 0
  // measured on two parameters sharing [[buffer(0)]] or [[texture(0)]].
  llvm::SmallDenseMap<unsigned, SmallVector<unsigned, 4>, 4> Reserved[3];

  // Parameter rules.
  for (ParmVarDecl *PVD : FD->parameters()) {
    QualType PTy = PVD->getType();

    // [[buffer]] validation. Measured shapes:
    //   scalar:               type 'uint' (aka 'unsigned int') is not valid
    //                         for attribute 'buffer'
    //   pointer into          type 'device ulong *' (aka 'device unsigned
    //   unsupported scalar:   long *') is not valid for attribute 'buffer'
    //                         + note: type 'ulong' (aka 'unsigned long')
    //                           cannot be used in buffer pointee type
    //   wrong pointee space:  invalid address space qualification for buffer
    //                         pointee type 'threadgroup float'
    if (PVD->hasAttr<MetalBufferIndexAttr>()) {
      if (PTy->isPointerType() || PTy->isReferenceType()) {
        QualType Pointee = PTy->getPointeeType();
        LangAS PointeeAS = Pointee.getAddressSpace();
        // `device coherent(device) T` is a device buffer with an extra
        // coherence qualifier, and binds exactly like a plain device one.
        if (PointeeAS != LangAS::metal_device &&
            PointeeAS != LangAS::metal_device_coherent &&
            PointeeAS != LangAS::metal_constant) {
          Diag(PVD->getLocation(), diag::err_metal_invalid_buffer_pointee)
              << PTy->getPointeeType();
          Diag(PVD->getLocation(), diag::note_metal_valid_addrspace);
          PVD->setInvalidDecl();
        } else {
          const Type *PT = Pointee->getUnqualifiedDesugaredType();
          const auto *BT = dyn_cast<BuiltinType>(PT);
          if (BT && (BT->getKind() == BuiltinType::Long ||
                     BT->getKind() == BuiltinType::ULong) &&
              static_cast<unsigned>(getLangOpts().getMetalVersion()) < 400) {
            Diag(PVD->getLocation(), diag::err_metal_invalid_attr_type)
                << PTy << "'buffer'";
            Diag(PVD->getLocation(), diag::note_metal_buffer_pointee_unsupported)
                << Pointee;
            PVD->setInvalidDecl();
          }
        }
      } else if (!isMetalTextureType(PTy) && !isMetalSamplerType(PTy)) {
        Diag(PVD->getLocation(), diag::err_metal_invalid_attr_type)
            << PTy << "'buffer'";
        PVD->setInvalidDecl();
      } else {
        Diag(PVD->getLocation(), diag::err_metal_invalid_buffer_pointee)
            << PTy;
        Diag(PVD->getLocation(), diag::note_metal_valid_addrspace);
        PVD->setInvalidDecl();
      }
    }

    // Resource attributes only make sense on matching handle types:
    // [[texture]] on a pointer or [[sampler]] on a non-sampler is the same
    // class of error as [[buffer]] on a scalar, and Apple reports it with
    // the uniform "type ... is not valid for attribute ..." message.
    if (PVD->hasAttr<MetalTextureIndexAttr>() && !isMetalTextureType(PTy) &&
        !PTy->isPointerType() && !PTy->isReferenceType()) {
      Diag(PVD->getLocation(), diag::err_metal_invalid_attr_type)
          << PTy << "'texture'";
      PVD->setInvalidDecl();
    }
    if (PVD->hasAttr<MetalSamplerIndexAttr>() && !isMetalSamplerType(PTy)) {
      Diag(PVD->getLocation(), diag::err_metal_invalid_attr_type)
          << PTy << "'sampler'";
      PVD->setInvalidDecl();
    }
    if (PVD->hasAttr<MetalLocalIndexAttr>() &&
        !(PTy->isPointerType() || PTy->isReferenceType())) {
      Diag(PVD->getLocation(), diag::err_metal_invalid_attr_type)
          << PTy << "'threadgroup'";
      PVD->setInvalidDecl();
    }

    // Claim the binding location of every resource attribute.
    auto claim = [&](const Attr *A, llvm::StringRef Kind, unsigned KindIdx) {
      if (!A)
        return;
      const Expr *IndexE = nullptr;
      if (const auto *BA = dyn_cast<MetalBufferIndexAttr>(A))
        IndexE = BA->getIndex();
      else if (const auto *TA = dyn_cast<MetalTextureIndexAttr>(A))
        IndexE = TA->getIndex();
      else if (const auto *SA = dyn_cast<MetalSamplerIndexAttr>(A))
        IndexE = SA->getIndex();
      // The attribute stores the argument as it was parsed (an
      // IntegerLiteral for the common [[buffer(0)]]), not a folded
      // ConstantExpr, so evaluate it generically here.
      Expr::EvalResult Res;
      if (!IndexE || !IndexE->EvaluateAsInt(Res, getASTContext()))
        return;
      // Out-of-bounds indices (including wrapped negatives) were already
      // rejected by the attribute handler; skip them here.
      int64_t Index = Res.Val.getInt().getExtValue();
      if (Index < 0)
        return;
      SmallVector<unsigned, 4> &Seen = Reserved[KindIdx][(unsigned)Index];
      if (!Seen.empty())
        Diag(A->getLocation(), diag::err_metal_resource_index_reserved)
            << metalQuoteSpelling(Kind) << Index;
      Seen.push_back((unsigned)Index);
    };
    claim(PVD->getAttr<MetalBufferIndexAttr>(), "buffer", 0);
    claim(PVD->getAttr<MetalTextureIndexAttr>(), "texture", 1);
    claim(PVD->getAttr<MetalSamplerIndexAttr>(), "sampler", 2);

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
      // Function pointers are legal entry-point parameters beginning in
      // Metal 2.1.  They are not buffer resources and therefore must not
      // fall through to the generic pointer -> [[buffer]] rule.
      else if (isMetalFunctionPointerType(PTy))
        Which = std::nullopt;
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

      // A by-value scalar with no binding and no stage input attribute is
      // not a legal entry point input at all. Measured for kernels with
      //   error: invalid type 'int' for input declaration in a kernel
      //          function
      // (misc_unused_parameter fixture); the message carries the stage
      // select, so the same structural rule is applied to every stage.
      if (!PVD->isInvalidDecl() && isMetalEntryInvalidScalar(PTy) &&
          PTy.getAddressSpace() == LangAS::Default) {
        Diag(PVD->getLocation(), diag::err_metal_invalid_input_type)
            << PTy << static_cast<unsigned>(Stage);
        PVD->setInvalidDecl();
      }
    }

    // [[position]] accepts float4 only (MSL 3.2 table 5.4), measured:
    //   error: type 'int' is not valid for attribute 'position'
    if (PVD->hasAttr<MetalPositionAttr>() && !isMetalFloat4Type(PTy)) {
      Diag(PVD->getLocation(), diag::err_metal_invalid_attr_type)
          << PTy << "'position'";
      PVD->setInvalidDecl();
    }

    // [[stage_in]]: the parameter type must be a struct, and its members
    // must not be matrices or packed vectors; member errors cascade to the
    // stage_in type error measured above.
    if (PVD->hasAttr<MetalStageInAttr>() && !PVD->isInvalidDecl()) {
      auto *RD = PTy->getUnqualifiedDesugaredType()->getAsCXXRecordDecl();
      CheckMetalStageInStruct(PVD, RD);
    }

    // Stage input attributes are only meaningful on the stages that produce
    // them. Measured, for every combination:
    //   error: invalid 'vertex_id' attribute for input declaration in a
    //          fragment function
    //   error: invalid 'stage_in' attribute for input declaration in a kernel
    //          function
    //   error: invalid 'depth' attribute for input declaration in a vertex
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
    // depth is an output-only attribute; measured invalid on input in every
    // stage (attr_depth_in_vertex fixture).
    rejectOn(PVD->getAttr<MetalDepthAttr>(), "'depth'", {});
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

/// Diagnose a call whose argument count is not \p Desired.
///
/// SemaChecking.cpp has a `checkArgCount` for this, but it is static to that
/// file, so the same two diagnostics are produced here. Returns true on error.
static bool checkMetalArgCount(Sema &S, CallExpr *Call, unsigned Desired) {
  unsigned ArgCount = Call->getNumArgs();
  if (ArgCount == Desired)
    return false;

  if (ArgCount < Desired)
    return S.Diag(Call->getEndLoc(), diag::err_typecheck_call_too_few_args)
           << /*function call*/ 0 << Desired << ArgCount
           << Call->getSourceRange();

  SourceRange Excess(Call->getArg(Desired)->getBeginLoc(),
                     Call->getArg(ArgCount - 1)->getEndLoc());
  return S.Diag(Excess.getBegin(), diag::err_typecheck_call_too_many_args)
         << /*function call*/ 0 << Desired << ArgCount << Excess;
}

/// Is \p BuiltinID a Metal builtin that returns void (has no meaningful
/// return value)?
///
/// The builtins are all declared with the placeholder `"v."` type string, so
/// the return type cannot be derived from the signature. This helper inspects
/// the AIR intrinsic name recorded in BuiltinsMetal.def to decide whether the
/// operation produces a value or is called only for its side effects.
///
/// Void-returning categories:
///   * unimplemented builtins with an empty AIR name (except divide/select)
///   * barriers, fences, flushes, memcpy/move/set, discard, abort
///   * atomic store intrinsics (air.atomic.*.store.*)
///
/// All other builtins produce a value whose type is the first argument's type
/// (following the convention that Apple's <metal_stdlib> always passes the
/// value operand first).
static bool isMetalBuiltinVoidReturning(unsigned BuiltinID) {
  // extract the AIR intrinsic name from the table.
  llvm::StringRef AIRName;
  switch (BuiltinID) {
#define METAL_BUILTIN(ID, TYPE, ATTRS, AIRNAME, ARITY)                         \
  case Builtin::BI##ID:                                                        \
    AIRName = AIRNAME;                                                         \
    break;
#include "clang/Basic/BuiltinsMetal.def"
  default:
    return false;
  }

  // Unimplemented: empty AIR name means no AIR intrinsic; these are either
  // native instructions (divide/select) or not-yet-implemented helpers.
  // divide/select are value-returning; everything else with an empty name
  // keeps void.
  if (AIRName.empty()) {
    switch (BuiltinID) {
    case Builtin::BI__metal_divide:
    case Builtin::BI__metal_select:
      return false;
    default:
      return true;
    }
  }

  // Explicitly void: barriers.
  if (AIRName.contains("barrier"))
    return true;

  // Explicitly void: fences and flushes.
  if (AIRName.startswith("air.fence") || AIRName.contains("fence_texture") ||
      AIRName.contains("flush"))
    return true;

  // Explicitly void: atomics that store or fence (air.atomic.*.store.* /
  // air.atomic.fence).  Air names for void atomics contain ".store" or
  // ".fence"; value-returning atomics leave those segments out.
  if (AIRName.startswith("air.atomic.") &&
      (AIRName.contains(".store") || AIRName.contains(".fence")))
    return true;

  // Explicitly void: discard, abort, memcpy, memmove, memset.
  if (AIRName.startswith("air.discard_fragment") ||
      AIRName.startswith("air.abort_intersection_query") ||
      AIRName.startswith("air.memcpy") ||
      AIRName.startswith("air.memmove") ||
      AIRName.startswith("air.memset"))
    return true;

  // Explicitly void: clear/set barrier commands.
  if (AIRName.startswith("air.clear_barrier_") ||
      AIRName.startswith("air.set_barrier_"))
    return true;

  // get_descriptor_size_tensor returns void (the result goes through an
  // out-parameter).
  if (AIRName.startswith("air.get_descriptor_size_"))
    return true;

  // Everything else produces a value.
  return false;
}

bool Sema::CheckMetalBuiltinCall(unsigned BuiltinID, CallExpr *TheCall) {
  int Arity = getMetalBuiltinArity(BuiltinID);
  if (Arity < 0)
    return false;

  if (checkMetalArgCount(*this, TheCall, (unsigned)Arity))
    return true;

  // Run the usual conversions on every argument so that the argument type is
  // the promoted one and array/function decay has happened.
  for (unsigned I = 0, E = TheCall->getNumArgs(); I != E; ++I) {
    ExprResult Converted =
        DefaultFunctionArrayLvalueConversion(TheCall->getArg(I));
    if (Converted.isInvalid())
      return true;
    TheCall->setArg(I, Converted.get());
  }

  // Determine the result type.  All builtins are declared with `"v."`
  // (void return, no formal parameter list) because a single builtin serves
  // every scalar width and vector length.  The result type follows the first
  // value argument (Apple's standard library always writes the value operand
  // first).
  //
  // Void-returning builtins (barriers, fences, stores, discards, ...) keep
  // void; they are called for side effects only.
  QualType ResultTy = Context.VoidTy;
  if (!isMetalBuiltinVoidReturning(BuiltinID) && TheCall->getNumArgs() > 0) {
    ResultTy = TheCall->getArg(0)->getType();
  }

  TheCall->setType(ResultTy);
  return false;
}
