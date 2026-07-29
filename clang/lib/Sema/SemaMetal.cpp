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
  if (!MinVersion || getLangOpts().MetalVersion >= MinVersion)
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
    if (!VD->hasInit() && !VD->hasExternalStorage()) {
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
        if (PointeeAS != LangAS::metal_device &&
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
