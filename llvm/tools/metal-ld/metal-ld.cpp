//===- metal-ld.cpp - Link .air files into .metallib ----------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This tool takes multiple .air files as input and produces a .metallib file
// by creating a Mach-O universal binary container with MTLB sections.
//
//===----------------------------------------------------------------------===//

#include "llvm/ADT/SmallVector.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/MD5.h"
#include "llvm/Support/MemoryBuffer.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/ToolOutputFile.h"
#include "llvm/Support/WithColor.h"
#include "llvm/Support/Endian.h"

#include <cstring>

using namespace llvm;

static cl::list<std::string> InputFilenames(cl::Positional, cl::OneOrMore,
                                            cl::desc("<input .air files>"));

static cl::opt<std::string> OutputFilename("o", cl::desc("Output filename"),
                                           cl::value_desc("filename"),
                                           cl::Required);

static cl::opt<std::string> LibraryName("name", cl::desc("Library name"),
                                        cl::init("metal_library"));

// Mach-O universal binary constants
static const uint32_t FAT_MAGIC = 0xCAFEBABE;

// Section types
static const char SECTION_NAME[] = "NAME";
static const char SECTION_TYPE[] = "TYPE";
static const char SECTION_HASH[] = "HASH";
static const char SECTION_XSOFFT[] = "XSOFFT";
static const char SECTION_VERS[] = "VERS";
static const char SECTION_MDSZ[] = "MDSZ";
static const char SECTION_ENDT[] = "ENDT";

static void writeSection(raw_ostream &OS, StringRef Name, StringRef Data) {
  // Section format:
  // - 4 bytes: section name (e.g., "NAME", "TYPE")
  // - 4 bytes: padding
  // - 8 bytes: data size (little-endian)
  // - Variable: data
  
  OS.write(Name.data(), Name.size());
  
  // Padding to 4 bytes
  size_t Padding = 4 - Name.size();
  for (size_t i = 0; i < Padding; ++i)
    OS.write('\0');
  
  // Data size (little-endian)
  uint64_t DataSize = Data.size();
  OS.write(reinterpret_cast<const char *>(&DataSize), 8);
  
  // Data
  OS.write(Data.data(), Data.size());
  
  // Align to 8 bytes
  size_t TotalSize = 4 + Padding + 8 + Data.size();
  size_t AlignPadding = (8 - (TotalSize % 8)) % 8;
  for (size_t i = 0; i < AlignPadding; ++i)
    OS.write('\0');
}

static void writeFatHeader(raw_ostream &OS, unsigned NumArchs,
                         const SmallVectorImpl<uint64_t> &ArchSizes) {
  // Fat header format (big-endian):
  // - 4 bytes: magic (0xCAFEBABE)
  // - 4 bytes: number of architectures
  // - For each architecture (20 bytes each):
  //   - 4 bytes: CPU type
  //   - 4 bytes: CPU subtype
  //   - 4 bytes: offset
  //   - 4 bytes: size
  //   - 4 bytes: alignment
  
  // Magic (big-endian)
  uint32_t Magic = llvm::support::endian::byte_swap<uint32_t, llvm::support::big>(FAT_MAGIC);
  OS.write(reinterpret_cast<const char *>(&Magic), 4);
  
  // Number of architectures (big-endian)
  uint32_t NumArchsBE = llvm::support::endian::byte_swap<uint32_t, llvm::support::big>(NumArchs);
  OS.write(reinterpret_cast<const char *>(&NumArchsBE), 4);
  
  // Architecture descriptors
  uint32_t Offset = 8 + (NumArchs * 20); // Start after header + descriptors
  for (unsigned i = 0; i < NumArchs; ++i) {
    // CPU type (example: 0x01000007 for x86_64)
    uint32_t CPUType = llvm::support::endian::byte_swap<uint32_t, llvm::support::big>(0x01000007);
    OS.write(reinterpret_cast<const char *>(&CPUType), 4);
    
    // CPU subtype
    uint32_t CPUSubtype = llvm::support::endian::byte_swap<uint32_t, llvm::support::big>(0x00000003);
    OS.write(reinterpret_cast<const char *>(&CPUSubtype), 4);
    
    // Offset (big-endian)
    uint32_t OffsetBE = llvm::support::endian::byte_swap<uint32_t, llvm::support::big>(Offset);
    OS.write(reinterpret_cast<const char *>(&OffsetBE), 4);
    
    // Size (big-endian)
    uint32_t SizeBE = llvm::support::endian::byte_swap<uint32_t, llvm::support::big>(ArchSizes[i]);
    OS.write(reinterpret_cast<const char *>(&SizeBE), 4);
    
    // Alignment (2^4 = 16 bytes)
    uint32_t Align = llvm::support::endian::byte_swap<uint32_t, llvm::support::big>(4);
    OS.write(reinterpret_cast<const char *>(&Align), 4);
    
    // Align offset to 16 bytes
    Offset += ArchSizes[i];
    Offset = (Offset + 15) & ~15;
  }
}

int main(int argc, char **argv) {
  cl::ParseCommandLineOptions(argc, argv, "Metal linker\n");
  
  // Read all .air files
  SmallVector<std::unique_ptr<MemoryBuffer>, 4> AIRBuffers;
  for (const auto &Filename : InputFilenames) {
    ErrorOr<std::unique_ptr<MemoryBuffer>> MBOrErr =
        MemoryBuffer::getFile(Filename);
    if (!MBOrErr) {
      WithColor::error() << "Could not open input file '" << Filename 
                         << "': " << MBOrErr.getError().message() << "\n";
      return 1;
    }
    AIRBuffers.push_back(std::move(MBOrErr.get()));
  }
  
  // Create sections for each .air file
  SmallVector<std::string, 4> SectionsData;
  for (const auto &MB : AIRBuffers) {
    std::string Sections;
    raw_string_ostream SOS(Sections);
    
    // Add sections
    writeSection(SOS, SECTION_NAME, LibraryName);
    writeSection(SOS, SECTION_TYPE, "air");
    
    // Compute MD5 hash
    MD5 Hash;
    Hash.update(MB->getBuffer());
    MD5::MD5Result Result;
    Hash.final(Result);
    writeSection(SOS, SECTION_HASH, StringRef(reinterpret_cast<const char*>(Result.data()), 16));
    
    // XSOFFT (offset to bitcode) - placeholder
    uint64_t Offset = 0;
    writeSection(SOS, SECTION_XSOFFT, StringRef(reinterpret_cast<char*>(&Offset), 8));
    
    // VERS (version)
    writeSection(SOS, SECTION_VERS, "32023.883");
    
    // MDSZ (metadata size)
    uint64_t Size = MB->getBufferSize();
    writeSection(SOS, SECTION_MDSZ, StringRef(reinterpret_cast<char*>(&Size), 8));
    
    // Write .air data
    SOS.write(MB->getBufferStart(), MB->getBufferSize());
    
    // ENDT (end)
    writeSection(SOS, SECTION_ENDT, "");
    
    SOS.flush();
    SectionsData.push_back(std::move(Sections));
  }
  
  // Create output file
  std::error_code EC;
  ToolOutputFile Out(OutputFilename, EC, sys::fs::OF_None);
  if (EC) {
    WithColor::error() << "Could not open output file: " << EC.message() << "\n";
    return 1;
  }
  
  // Write fat header
  SmallVector<uint64_t, 4> ArchSizes;
  for (const auto &S : SectionsData)
    ArchSizes.push_back(S.size());
  
  writeFatHeader(Out.os(), SectionsData.size(), ArchSizes);
  
  // Write each section with alignment
  uint64_t Offset = 8 + (SectionsData.size() * 20);
  for (const auto &S : SectionsData) {
    // Align to 16 bytes
    size_t Padding = (16 - (Offset % 16)) % 16;
    for (size_t i = 0; i < Padding; ++i)
      Out.os().write('\0');
    
    Out.os().write(S.data(), S.size());
    Offset += Padding + S.size();
  }
  
  Out.keep();
  
  outs() << "Created " << OutputFilename << " with " << InputFilenames.size() 
         << " .air file(s)\n";
  
  return 0;
}
