//===- AIR.h - AIR (Metal IR) pass definitions ------------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This header declares the passes and utilities for the AIR (Metal IR) target.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_TARGET_AIR_AIR_H
#define LLVM_TARGET_AIR_AIR_H

namespace llvm {

class ModulePass;
class Module;
class raw_ostream;

/// Create a pass that prepares a module for AIR bitcode emission.
/// Strips unsupported attributes and inserts no-op bitcasts for type
/// reconstruction.
ModulePass *createAIRPrepareModulePass();

} // end namespace llvm

#endif // LLVM_TARGET_AIR_AIR_H
