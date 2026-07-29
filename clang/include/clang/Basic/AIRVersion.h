//===--- AIRVersion.h - AIR version from deployment target ------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// Maps an Apple deployment target to the AIR version that Apple's Metal
// toolchain encodes in the `_vNN` suffix of the target triple.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_CLANG_BASIC_AIRVERSION_H
#define LLVM_CLANG_BASIC_AIRVERSION_H

namespace clang {
namespace targets {

/// Map a deployment target macOS version to its AIR version.
///
/// Measured by sweeping Apple's driver over every supported deployment target
/// and reading back the triple it selected; see the `-###` logs in
/// reference/metal-ast-macos-air64/log. The AIR version depends only on the
/// deployment target, never on `-std=`.
///
///   10.11 -> 18    10.15 -> 22    13.x -> 25
///   10.12 -> 111   11.x  -> 23    14.x -> 26
///   10.13 -> 20    12.x  -> 24    15.x -> 27
///   10.14 -> 21                   26.x -> 28
unsigned getAIRVersionForMacOSVersion(unsigned Major, unsigned Minor);

} // namespace targets
} // namespace clang

#endif // LLVM_CLANG_BASIC_AIRVERSION_H
