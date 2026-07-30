//===--- AIR.h - Declare AIR target feature support -------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file declares AIR TargetInfo objects. AIR (Apple Intermediate
// Representation) is the target of Apple's Metal Shading Language compiler.
//
// Everything in this file is derived from measurements of Apple's toolchain
// collected in the metal-info reference set:
//
//   * DataLayout        research/spec/IR_GROUND_TRUTH.md section 2.3 (identical
//                       across all 701 runtime modules and all golden probes)
//                       and reference/metal-ast-macos-air32 for the 32-bit form
//   * Address spaces    research/spec/IR_GROUND_TRUTH.md section 2.4 plus the
//                       golden corpus in research/golden
//   * Resource limits   the module flags emitted in research/golden/P01
//   * Predefined macros reference/metal-ast-macos-air64/meta/
//                       metal-predefined-macros.txt
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_CLANG_LIB_BASIC_TARGETS_AIR_H
#define LLVM_CLANG_LIB_BASIC_TARGETS_AIR_H

#include "clang/Basic/AIRVersion.h"
#include "clang/Basic/TargetInfo.h"
#include "clang/Basic/TargetOptions.h"
#include "llvm/Support/Compiler.h"
#include "llvm/TargetParser/Triple.h"

namespace clang {
namespace targets {

/// Address space mapping for AIR.
///
/// The target values are the `addrspace(N)` numbers observed in Apple
/// generated AIR:
///
/// Established by scanning every `.ll` in the reference set (129,266 files);
/// the counts below are the total occurrences of each space.
///
///   0  thread (the default)        unqualified `ptr`
///   1  device                      1,393,063; also carries texture handles
///   2  constant                      199,446; also carries sampler handles
///   3  threadgroup                    24,284
///   4  threadgroup_imageblock             44  the imageblock handle lives here:
///                                            %"struct.metal::_imageblock_base"
///                                            = type { %struct._imageblock_t
///                                                     addrspace(4)* }
///   6  object_data                       112  measured from the object shader
///                                            probe `object_payload.metal`,
///                                            whose `object_data Pay2 &p
///                                            [[payload]]` parameter becomes
///                                            `%struct.Pay2 addrspace(6)*`
///
/// Three further spaces appear in generated code but are not reachable from a
/// source-level address space keyword, so they are not mapped here:
///
///   5   80 occurrences, an i8 pointer returned alongside the intersection
///        result by `air.intersect_direct_access.*`
///   7   20 occurrences, only ever `%struct._mesh_t addrspace(7)*`, i.e. the
///        mesh handle type rather than an address space a user can name
///   9  177 occurrences, only ever `%struct._intersection_result_t
///        addrspace(9)*`
///
/// `ray_data` is therefore an OPEN question: the keyword exists and mangles as
/// U10MTLraydata, but no probe in the corpus produces a function that takes a
/// `ray_data` parameter, so its numeric value has never been observed. It is
/// mapped to 9 here because that is the space the intersection result (the
/// only ray payload the corpus shows) lives in, but this is the one entry in
/// this table that is inferred rather than measured.
static const unsigned AIRAddrSpaceMap[] = {
    0, // Default (thread)
    1, // opencl_global
    3, // opencl_local
    2, // opencl_constant
    0, // opencl_private
    0, // opencl_generic
    1, // opencl_global_device
    1, // opencl_global_host
    1, // cuda_device
    2, // cuda_constant
    3, // cuda_shared
    1, // sycl_global
    1, // sycl_global_device
    1, // sycl_global_host
    3, // sycl_local
    0, // sycl_private
    0, // ptr32_sptr
    0, // ptr32_uptr
    0, // ptr64
    3, // hlsl_groupshared
    1, // metal_device
    2, // metal_constant
    3, // metal_threadgroup
    0, // metal_thread
    4, // metal_threadgroup_imageblock
    6, // metal_object_data
    9, // metal_ray_data
    1, // metal_device_coherent
};

class LLVM_LIBRARY_VISIBILITY AIRTargetInfo : public TargetInfo {
public:
  AIRTargetInfo(const llvm::Triple &Triple, const TargetOptions &)
      : TargetInfo(Triple) {
    // Metal has no thread local storage and no variable length arrays.
    TLSSupported = false;
    VLASupported = false;
    NoAsmVariants = true;

    // `half` is a first class type with native arithmetic in MSL.
    HasLegalHalfType = true;
    HasFloat16 = true;
    HalfArgsAndReturns = true;

    // MSL 2.2 introduced bfloat; the reference toolchain predefines the
    // __BFLT16_* limit macros unconditionally.
    HasBFloat16 = true;

    AddrSpaceMap = &AIRAddrSpaceMap;
    UseAddrSpaceMapMangling = false;

    PlatformMinVersion = Triple.getOSVersion();
    PlatformName = llvm::Triple::getOSTypeName(Triple.getOS());

    // Measured from reference/metal-ast-macos-air64/meta/
    // metal-predefined-macros.txt:
    //   __BIGGEST_ALIGNMENT__ 8, __SIZEOF_LONG__ 8, __LONG_WIDTH__ 64,
    //   __WCHAR_WIDTH__ 32, __WCHAR_TYPE__ int, __SIZE_TYPE__ unsigned long,
    //   __PTRDIFF_TYPE__ long, __INTPTR_TYPE__ long, __INT64_TYPE__ long.
    LongWidth = LongAlign = 64;
    SuitableAlign = 64;
    DefaultAlignForAttributeAligned = 64;
    WCharType = SignedInt;
    WIntType = SignedInt;
    IntPtrType = SignedLong;
    PtrDiffType = SignedLong;
    SizeType = UnsignedLong;
    Int64Type = SignedLong;
    IntMaxType = SignedLong;

    if (Triple.getArch() == llvm::Triple::air32) {
      PointerWidth = PointerAlign = 32;
      resetDataLayout(getAIRDataLayout(/*Is64Bit=*/false));
    } else {
      PointerWidth = PointerAlign = 64;
      resetDataLayout(getAIRDataLayout(/*Is64Bit=*/true));
    }

    // Metal is a C++ dialect; use the Itanium ABI, which is what the observed
    // mangled names (`_Z11read_devicePU9MTLdevice10AddressBox`) follow.
    TheCXXABI.set(TargetCXXABI::GenericItanium);
  }

  /// The AIR DataLayout string.
  ///
  /// Verified byte for byte against every module in the metal-info corpus:
  /// the 64-bit form appears in all 701 runtime modules
  /// (research/spec/IR_GROUND_TRUTH.md section 2.3) and all golden probes; the
  /// 32-bit form differs only in the pointer size and was read from
  /// reference/metal-ast-macos-air32/ir.
  static const char *getAIRDataLayout(bool Is64Bit) {
    if (Is64Bit)
      return "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-"
             "f32:32:32-f64:64:64-v16:16:16-v24:32:32-v32:32:32-v48:64:64-"
             "v64:64:64-v96:128:128-v128:128:128-v192:256:256-v256:256:256-"
             "v512:512:512-v1024:1024:1024-n8:16:32";
    return "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-"
           "f32:32:32-f64:64:64-v16:16:16-v24:32:32-v32:32:32-v48:64:64-"
           "v64:64:64-v96:128:128-v128:128:128-v192:256:256-v256:256:256-"
           "v512:512:512-v1024:1024:1024-n8:16:32";
  }

  void getTargetDefines(const LangOptions &Opts,
                        MacroBuilder &Builder) const override;

  bool hasFeature(StringRef Feature) const override {
    return Feature == "air" || Feature == "metal";
  }

  ArrayRef<Builtin::Info> getTargetBuiltins() const override {
    return std::nullopt;
  }

  const char *getClobbers() const override { return ""; }

  ArrayRef<const char *> getGCCRegNames() const override {
    return std::nullopt;
  }

  ArrayRef<TargetInfo::GCCRegAlias> getGCCRegAliases() const override {
    return std::nullopt;
  }

  bool validateAsmConstraint(const char *&Name,
                             TargetInfo::ConstraintInfo &Info) const override {
    return false;
  }

  BuiltinVaListKind getBuiltinVaListKind() const override {
    return TargetInfo::CharPtrBuiltinVaList;
  }

  bool allowsLargerPreferedTypeAlignment() const override { return false; }

  /// Resource limits, emitted as `!llvm.module.flags` entries.
  ///
  /// Measured from research/golden/P01/metal32_macosx26/probe.ll:
  ///   air.max_device_buffers 31, air.max_constant_buffers 31,
  ///   air.max_threadgroup_buffers 31, air.max_textures 128,
  ///   air.max_read_write_textures 8, air.max_samplers 16
  static constexpr unsigned MaxDeviceBuffers = 31;
  static constexpr unsigned MaxConstantBuffers = 31;
  static constexpr unsigned MaxThreadgroupBuffers = 31;
  static constexpr unsigned MaxTextures = 128;
  static constexpr unsigned MaxReadWriteTextures = 8;
  static constexpr unsigned MaxSamplers = 16;
};

} // namespace targets
} // namespace clang

#endif // LLVM_CLANG_LIB_BASIC_TARGETS_AIR_H
