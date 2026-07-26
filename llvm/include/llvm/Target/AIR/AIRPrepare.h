//===- llvm/Target/AIR/AIRPrepare.h - AIR pass declarations -----*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_TARGET_AIR_AIRPREPARE_H
#define LLVM_TARGET_AIR_AIRPREPARE_H

namespace llvm {

class ModulePass;

/// Create a pass that prepares a module for AIR bitcode emission.
/// Strips unsupported attributes and inserts no-op bitcasts for typed
/// pointer reconstruction.
ModulePass *createAIRPrepareModulePass();

} // end namespace llvm

#endif // LLVM_TARGET_AIR_AIRPREPARE_H
