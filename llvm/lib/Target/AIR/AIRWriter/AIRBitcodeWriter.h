//===- AIRBitcodeWriter.h - Write AIR bitcode with typed pointers ---------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// Internal header for the AIR (Metal IR) typed-pointer bitcode writer.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_TARGET_AIR_AIRBITCODEWRITER_INTERNAL_H
#define LLVM_TARGET_AIR_AIRBITCODEWRITER_INTERNAL_H

#include "llvm/Target/AIR/AIRBitcodeWriter.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/IR/ModuleSummaryIndex.h"
#include "llvm/MC/StringTableBuilder.h"
#include "llvm/Support/Allocator.h"
#include "llvm/Support/MemoryBufferRef.h"
#include <memory>
#include <vector>

namespace llvm {

class BitstreamWriter;
class Module;
class raw_ostream;

namespace air {

class BitcodeWriter {
  SmallVectorImpl<char> &Buffer;
  std::unique_ptr<BitstreamWriter> Stream;

  StringTableBuilder StrtabBuilder{StringTableBuilder::RAW};
  BumpPtrAllocator Alloc;

  std::vector<Module *> Mods;

public:
  BitcodeWriter(SmallVectorImpl<char> &Buffer);
  ~BitcodeWriter();

  void writeModule(const Module &M);
};

/// Internal entry point — writes AIR bitcode to the given stream.
void WriteAIRToFile(const Module &M, raw_ostream &Out);

} // end namespace air
} // end namespace llvm

#endif // LLVM_TARGET_AIR_AIRBITCODEWRITER_INTERNAL_H
