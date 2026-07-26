//===- llvm/Target/AIR/AIRBitcodeWriter.h - AIR bitcode writer -*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// Public interface for the AIR (Metal IR) typed-pointer bitcode writer.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_TARGET_AIR_AIRBITCODEWRITER_H
#define LLVM_TARGET_AIR_AIRBITCODEWRITER_H

namespace llvm {

class Module;
class raw_ostream;

namespace air {

/// Write the module in AIR bitcode format (with typed pointers) to \p Out.
/// This produces bitcode compatible with Apple's Metal runtime, using
/// TYPE_CODE_POINTER instead of TYPE_CODE_OPAQUE_POINTER.
void WriteAIRBitcodeToFile(const Module &M, raw_ostream &Out);

} // end namespace air
} // end namespace llvm

#endif // LLVM_TARGET_AIR_AIRBITCODEWRITER_H
