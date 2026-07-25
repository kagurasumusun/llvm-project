//===--- AIR.h - Declare Apple AIR target feature support -------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file declares TargetInfo support for Apple AIR (Metal IR) bitcode.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_CLANG_LIB_BASIC_TARGETS_AIR_H
#define LLVM_CLANG_LIB_BASIC_TARGETS_AIR_H

#include "clang/Basic/MacroBuilder.h"
#include "clang/Basic/TargetInfo.h"
#include "clang/Basic/TargetOptions.h"
#include "llvm/Support/Compiler.h"
#include "llvm/TargetParser/Triple.h"

namespace clang {
namespace targets {

// AIR uses the same numeric address spaces observed in Apple Metal-generated
// LLVM IR for the OpenCL-style address spaces that Clang already models:
// global/device = 1, constant = 2, local/threadgroup = 3, private/thread = 0.
static const unsigned AIRAddrSpaceMap[] = {
    0,  // Default
    1,  // opencl_global
    3,  // opencl_local
    2,  // opencl_constant
    0,  // opencl_private
    4,  // opencl_generic
    1,  // opencl_global_device
    1,  // opencl_global_host
    1,  // cuda_device
    2,  // cuda_constant
    3,  // cuda_shared
    1,  // sycl_global
    1,  // sycl_global_device
    1,  // sycl_global_host
    3,  // sycl_local
    0,  // sycl_private
    0,  // ptr32_sptr
    0,  // ptr32_uptr
    0,  // ptr64
    3,  // hlsl_groupshared
    2,  // hlsl_constant
    0,  // hlsl_private
    1,  // hlsl_device
    0,  // hlsl_input
    2,  // hlsl_push_constant
    20, // wasm_funcref
    // AIR-specific Metal address spaces (verified against Apple golden IR in
    // metal-info / research/spec/TYPE_LAYOUT_MAP.md and
    // METAL_TARGETINFO_IMPL_MAP.md):
    //   ray_data              -> as9 (`%struct._intersection_result_t addrspace(9)*`)
    //   object_data           -> as9 (Apple groups them together with ray_data)
    //   threadgroup_imageblock-> as4 (`%struct._imageblock_t addrspace(4)*`)
    9, // metal_ray_data
    9, // metal_object_data
    4, // metal_threadgroup_imageblock
};

class LLVM_LIBRARY_VISIBILITY AIRTargetInfo : public TargetInfo {
  bool Is64Bit;

public:
  AIRTargetInfo(const llvm::Triple &Triple, const TargetOptions &)
      : TargetInfo(Triple), Is64Bit(Triple.getArch() == llvm::Triple::air64) {
    assert((Triple.getArch() == llvm::Triple::air32 ||
            Triple.getArch() == llvm::Triple::air64) &&
           "Invalid architecture for AIR.");

    TLSSupported = false;
    VLASupported = false;
    NoAsmVariants = true;
    AddrSpaceMap = &AIRAddrSpaceMap;
    UseAddrSpaceMapMangling = true;
    HasFastHalfType = true;
    HasFloat16 = true;
    PlatformMinVersion = Triple.getOSVersion();
    PlatformName = llvm::Triple::getOSTypeName(Triple.getOS());
    TheCXXABI.set(TargetCXXABI::GenericItanium);

    LongWidth = LongAlign = Is64Bit ? 64 : 32;
    PointerWidth = PointerAlign = Is64Bit ? 64 : 32;
    SizeType = Is64Bit ? TargetInfo::UnsignedLong : TargetInfo::UnsignedInt;
    PtrDiffType = IntPtrType =
        Is64Bit ? TargetInfo::SignedLong : TargetInfo::SignedInt;

    resetDataLayout(Is64Bit
                        ? "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-"
                          "i32:32:32-i64:64:64-i128:128:128-"
                          "f32:32:32-f64:64:64-"
                          "v16:16:16-v24:32:32-v32:32:32-v48:64:64-"
                          "v64:64:64-v96:128:128-v128:128:128-"
                          "v192:256:256-v256:256:256-v512:512:512-"
                          "v1024:1024:1024-n8:16:32"
                        : "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-"
                          "i32:32:32-i64:64:64-i128:128:128-"
                          "f32:32:32-f64:64:64-"
                          "v16:16:16-v24:32:32-v32:32:32-v48:64:64-"
                          "v64:64:64-v96:128:128-v128:128:128-"
                          "v192:256:256-v256:256:256-v512:512:512-"
                          "v1024:1024:1024-n8:16:32");
  }

  bool useFP16ConversionIntrinsics() const override { return false; }

  void getTargetDefines(const LangOptions &, MacroBuilder &Builder) const override {
    Builder.defineMacro("__AIR__");
    Builder.defineMacro("__air__");
    Builder.defineMacro(Is64Bit ? "__AIR64__" : "__AIR32__");
  }

  bool hasFeature(StringRef Feature) const override {
    return Feature == "air" || Feature == (Is64Bit ? "air64" : "air32") ||
           Feature == "metal";
  }

  llvm::SmallVector<Builtin::InfosShard> getTargetBuiltins() const override {
    return {};
  }

  std::string_view getClobbers() const override { return ""; }

  ArrayRef<const char *> getGCCRegNames() const override { return {}; }

  bool validateAsmConstraint(const char *&, TargetInfo::ConstraintInfo &) const override {
    return true;
  }

  ArrayRef<TargetInfo::GCCRegAlias> getGCCRegAliases() const override {
    return {};
  }

  BuiltinVaListKind getBuiltinVaListKind() const override {
    return TargetInfo::VoidPtrBuiltinVaList;
  }
};

} // namespace targets
} // namespace clang

#endif // LLVM_CLANG_LIB_BASIC_TARGETS_AIR_H
