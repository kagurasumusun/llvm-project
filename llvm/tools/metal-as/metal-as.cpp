//===- metal-as.cpp - Assemble LLVM bitcode into .air files ---------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This tool takes LLVM bitcode (.bc) as input and produces .air files
// by adding the MTLB header that Apple's Metal compiler uses.
//
//===----------------------------------------------------------------------===//

#include "llvm/ADT/StringRef.h"
#include "llvm/Bitcode/BitcodeReader.h"
#include "llvm/Bitcode/BitcodeWriter.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/MemoryBuffer.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/ToolOutputFile.h"
#include "llvm/Support/WithColor.h"

#include <cstring>

using namespace llvm;

static cl::opt<std::string> InputFilename(cl::Positional, cl::desc("<input bitcode>"),
                                          cl::Required);

static cl::opt<std::string> OutputFilename("o", cl::desc("Output filename"),
                                           cl::value_desc("filename"),
                                           cl::Required);

static cl::opt<std::string> TargetTriple("triple", cl::desc("Target triple"),
                                         cl::init("air64-apple-macosx10.15.0"));

static void createMTLBHeader(raw_ostream &OS, size_t BitcodeSize,
                             StringRef Triple) {
  // MTLB header structure (284 bytes)
  // Based on reverse engineering of Apple's .air files
  
  char Header[284] = {0};
  
  // Magic number
  std::memcpy(Header, "MTLB", 4);
  
  // Version/flags (little-endian)
  uint32_t Version = 0x00028001;
  std::memcpy(Header + 4, &Version, 4);
  
  // Various fields (based on reverse engineering)
  uint64_t Field1 = 4;
  std::memcpy(Header + 8, &Field1, 8);
  
  uint64_t TotalSize = sizeof(Header) + BitcodeSize + Triple.size() + 1;
  std::memcpy(Header + 16, &TotalSize, 8);
  
  uint64_t Field2 = 0x82;
  std::memcpy(Header + 24, &Field2, 8);
  
  uint64_t Field3 = 0xf8;
  std::memcpy(Header + 32, &Field3, 8);
  
  uint64_t Field4 = 8;
  std::memcpy(Header + 40, &Field4, 8);
  
  uint64_t Field5 = 0x100;
  std::memcpy(Header + 48, &Field5, 8);
  
  OS.write(Header, sizeof(Header));
}

int main(int argc, char **argv) {
  cl::ParseCommandLineOptions(argc, argv, "Metal assembler\n");
  
  // Read input bitcode
  ErrorOr<std::unique_ptr<MemoryBuffer>> MBOrErr =
      MemoryBuffer::getFileOrSTDIN(InputFilename);
  if (!MBOrErr) {
    WithColor::error() << "Could not open input file: " << MBOrErr.getError().message() << "\n";
    return 1;
  }
  
  std::unique_ptr<MemoryBuffer> MB = std::move(MBOrErr.get());
  
  // Verify it's LLVM bitcode
  if (MB->getBufferSize() < 4 ||
      std::memcmp(MB->getBufferStart(), "BC\xc0\xde", 4) != 0) {
    WithColor::warning() << "Input does not appear to be LLVM bitcode\n";
  }
  
  // Create output file
  std::error_code EC;
  std::unique_ptr<ToolOutputFile> Out =
      std::make_unique<ToolOutputFile>(OutputFilename, EC, sys::fs::OF_None);
  if (EC) {
    WithColor::error() << EC.message() << '\n';
    return 1;
  }
  
  // Write MTLB header
  createMTLBHeader(Out->os(), MB->getBufferSize(), TargetTriple);
  
  // Write bitcode
  Out->os().write(MB->getBufferStart(), MB->getBufferSize());
  
  // Write triple (null-terminated)
  Out->os().write(TargetTriple.data(), TargetTriple.size());
  Out->os().write('\0');
  
  Out->keep();
  
  outs() << "Created " << OutputFilename << " ("
         << (284 + MB->getBufferSize() + TargetTriple.size() + 1)
         << " bytes)\n";
  
  return 0;
}
