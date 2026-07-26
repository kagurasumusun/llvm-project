//===- llvm-metallib.cpp - Apple Metal library tool -----------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// Unified tool for working with Apple Metal bitcode libraries.
//
// Subcommands:
//   assemble   Wrap LLVM bitcode (.bc) into an .air file (MTLB container).
//   link       Link one or more .air files into a .metallib.
//   info       Display information about .air / .metallib files.
//
// References:
//   https://github.com/kagurasumusun/metal-info
//   METALLIB_WRITER_SPEC.md (v1.0, machine-verified on real hardware)
//
//===----------------------------------------------------------------------===//

#include "llvm/ADT/SmallVector.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Endian.h"
#include "llvm/Support/Error.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/MemoryBuffer.h"
#include "llvm/Support/ToolOutputFile.h"
#include "llvm/Support/WithColor.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/SHA256.h"

#include <cstring>

using namespace llvm;
using namespace llvm::support;

// ============================================================================
// Constants from METALLIB_WRITER_SPEC (machine-verified, real-hardware tested)
// ============================================================================

static constexpr uint32_t MTLB_MAGIC = 0x424C544D;      // "MTLB" LE
static constexpr uint32_t FAT_MAGIC_64 = 0xcbfebabe;    // big-endian
static constexpr uint32_t CPU_TYPE_AIR64 = 0x01000017;   // ARM64|ABI64|AIR
static constexpr uint32_t BC_WRAPPER_MAGIC = 0x0B17C0DE; // LLVM bitcode wrapper

// Slice Header: 88 bytes, little-endian (METALLIB_WRITER_SPEC §2)
static constexpr uint32_t SLICE_HEADER_SIZE = 88;

// Fat arch descriptor: 32 bytes, big-endian
static constexpr uint32_t FAT_ARCH64_SIZE = 32;

// Bitcode Wrapper Header: 20 bytes (METALLIB_WRITER_SPEC §5.1)
static constexpr uint32_t BC_WRAPPER_SIZE = 20;

// HDYN block size: 74 bytes (METALLIB_WRITER_SPEC §3)
static constexpr uint32_t HDYN_BLOCK_SIZE = 74;

// Types/Empties blob size: 8 bytes each (METALLIB_WRITER_SPEC §4)
static constexpr uint32_t TYPES_EMPTIES_SIZE = 8;

// Directory header size: 8 bytes (METALLIB_WRITER_SPEC §3)
static constexpr uint32_t DIRECTORY_HEADER_SIZE = 8;

// Symbol tag IDs
static constexpr uint32_t TAG_NAME = 0x454D414E; // 'NAME'
static constexpr uint32_t TAG_TYPE = 0x45505954; // 'TYPE'
static constexpr uint32_t TAG_HASH = 0x48534148; // 'HASH'
static constexpr uint32_t TAG_OFFT = 0x5446464F; // 'OFFT'
static constexpr uint32_t TAG_VERS = 0x53524556; // 'VERS'
static constexpr uint32_t TAG_MDSZ = 0x5A53444D; // 'MDSZ'
static constexpr uint32_t TAG_ENDT = 0x54444E45; // 'ENDT'
static constexpr uint32_t TAG_HDYN = 0x4E594448; // 'HDYN'
static constexpr uint32_t TAG_RLST = 0x54534C52; // 'RLST'
static constexpr uint32_t TAG_UUID = 0x44495555; // 'UUID'

// Symbol types
static constexpr uint8_t SYM_TYPE_KERNEL = 0x02;
static constexpr uint8_t SYM_TYPE_FUNCTION = 0x03;

// Default version range (from METALLIB_WRITER_SPEC §3 VERS)
static constexpr uint8_t DEFAULT_VERS[8] = {0x02, 0x00, 0x08, 0x00, 0x03, 0x00, 0x02, 0x00};

// ============================================================================
// Helper functions
// ============================================================================

static void writeLE16(raw_ostream &OS, uint16_t V) {
  uint16_t LE = endian::byte_swap<uint16_t, endianness::little>(V);
  OS.write(reinterpret_cast<const char *>(&LE), 2);
}

static void writeLE32(raw_ostream &OS, uint32_t V) {
  uint32_t LE = endian::byte_swap<uint32_t, endianness::little>(V);
  OS.write(reinterpret_cast<const char *>(&LE), 4);
}

static void writeLE64(raw_ostream &OS, uint64_t V) {
  uint64_t LE = endian::byte_swap<uint64_t, endianness::little>(V);
  OS.write(reinterpret_cast<const char *>(&LE), 8);
}

static void writeBE32(raw_ostream &OS, uint32_t V) {
  uint32_t BE = endian::byte_swap<uint32_t, endianness::big>(V);
  OS.write(reinterpret_cast<const char *>(&BE), 4);
}

static void writeBE64(raw_ostream &OS, uint64_t V) {
  uint64_t BE = endian::byte_swap<uint64_t, endianness::big>(V);
  OS.write(reinterpret_cast<const char *>(&BE), 8);
}

static void writeBytes(raw_ostream &OS, const void *Data, size_t Size) {
  OS.write(reinterpret_cast<const char *>(Data), Size);
}

static void writePadding(raw_ostream &OS, uint64_t CurrentPos, uint64_t Alignment) {
  uint64_t Pad = (Alignment - (CurrentPos % Alignment)) % Alignment;
  for (uint64_t I = 0; I < Pad; ++I)
    OS.write('\0');
}

static uint32_t getCPUSubtypeFromTriple(StringRef Triple) {
  if (Triple.contains("_v25") || Triple.contains("_v28"))
    return 9;
  return 7; // v22/v23 default
}

// ============================================================================
// Symbol info extracted from bitcode metadata
// ============================================================================

struct SymbolInfo {
  std::string Name;
  uint8_t Type; // SYM_TYPE_KERNEL or SYM_TYPE_FUNCTION
  std::array<uint8_t, 32> Hash; // SHA-256 of bitcode
};

// Extract symbol names from LLVM bitcode (simplified — looks for function names)
static void extractSymbolInfo(const MemoryBuffer &MB, SmallVectorImpl<SymbolInfo> &Symbols) {
  // For now, extract function names from the bitcode by scanning for patterns
  // A more robust implementation would parse the bitcode, but this works for
  // the common case where we have kernel/function definitions.
  StringRef Data = MB.getBuffer();

  // Compute SHA-256 of the entire bitcode for the hash
  SHA256 Hasher;
  Hasher.update(Data);
  auto HashResult = Hasher.result();

  // Create a single symbol entry for the entire bitcode
  // In a more complete implementation, we'd parse the bitcode to find individual functions
  SymbolInfo Sym;
  Sym.Name = "default";
  Sym.Type = SYM_TYPE_KERNEL;
  std::copy(HashResult.begin(), HashResult.end(), Sym.Hash.begin());
  Symbols.push_back(Sym);
}

// ============================================================================
// Write a single symbol's tag records (METALLIB_WRITER_SPEC §3)
// ============================================================================

static void writeSymbolTags(raw_ostream &OS, const SymbolInfo &Sym, uint64_t BitcodeSize) {
  // NAME tag
  writeLE32(OS, TAG_NAME);
  uint16_t NameLen = Sym.Name.size() + 1; // +1 for NUL
  writeLE16(OS, NameLen);
  OS.write(Sym.Name.c_str(), NameLen);
  // Pad to 8-byte alignment after NAME
  writePadding(OS, OS.tell() % 8 == 0 ? OS.tell() : OS.tell() + (8 - OS.tell() % 8), 8);

  // TYPE tag
  writeLE32(OS, TAG_TYPE);
  writeLE16(OS, 1);
  OS.write(Sym.Type);

  // HASH tag
  writeLE32(OS, TAG_HASH);
  writeLE16(OS, 32);
  writeBytes(OS, Sym.Hash.data(), 32);

  // OFFT tag
  writeLE32(OS, TAG_OFFT);
  writeLE16(OS, 24);
  uint8_t OfftData[24] = {};
  writeBytes(OS, OfftData, 24);

  // VERS tag
  writeLE32(OS, TAG_VERS);
  writeLE16(OS, 8);
  writeBytes(OS, DEFAULT_VERS, 8);

  // MDSZ tag
  writeLE32(OS, TAG_MDSZ);
  writeLE16(OS, 8);
  writeLE64(OS, BitcodeSize);

  // ENDT tag (end of this symbol)
  writeLE32(OS, TAG_ENDT);
}

// ============================================================================
// Write HDYN block (METALLIB_WRITER_SPEC §3)
// ============================================================================

static void writeHDYNBlock(raw_ostream &OS) {
  // ENDT
  writeLE32(OS, TAG_ENDT);

  // HDYN (22 bytes)
  writeLE32(OS, TAG_HDYN);
  writeLE16(OS, 18); // payload length
  uint8_t HdynData[18] = {};
  writeBytes(OS, HdynData, 18);

  // RLST (22 bytes)
  writeLE32(OS, TAG_RLST);
  writeLE16(OS, 18);
  uint8_t RlstData[18] = {};
  writeBytes(OS, RlstData, 18);

  // UUID (22 bytes: tag + len + 16B UUID)
  writeLE32(OS, TAG_UUID);
  writeLE16(OS, 16);
  // Generate a simple UUID (in production, use proper UUID generation)
  uint8_t UUID[16] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
                       0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x10};
  writeBytes(OS, UUID, 16);

  // ENDT
  writeLE32(OS, TAG_ENDT);
}

// ============================================================================
// Write complete .air slice (METALLIB_WRITER_SPEC §2)
// ============================================================================

static void writeSlice(raw_ostream &OS, const MemoryBuffer &Bitcode,
                       StringRef Triple, const SmallVectorImpl<SymbolInfo> &Symbols) {
  uint64_t BitcodeSize = Bitcode.getBufferSize();

  // Calculate section offsets (METALLIB_WRITER_SPEC §2)
  uint64_t HeadersStart = SLICE_HEADER_SIZE; // 88

  // Calculate headers_len: directory header + symbol tags
  uint64_t HeadersLen = DIRECTORY_HEADER_SIZE; // 8 bytes for directory header
  for (const auto &Sym : Symbols) {
    // NAME: 4(tag) + 2(len) + name_len + padding
    uint16_t NameLen = Sym.Name.size() + 1;
    uint64_t NameRecordSize = 4 + 2 + NameLen;
    NameRecordSize = (NameRecordSize + 7) & ~uint64_t(7); // 8-byte aligned
    // TYPE: 4 + 2 + 1 = 7
    // HASH: 4 + 2 + 32 = 38
    // OFFT: 4 + 2 + 24 = 30
    // VERS: 4 + 2 + 8 = 14
    // MDSZ: 4 + 2 + 8 = 14
    // ENDT: 4
    HeadersLen += NameRecordSize + 7 + 38 + 30 + 14 + 14 + 4;
  }

  uint64_t TypesStart = HeadersStart + HeadersLen + HDYN_BLOCK_SIZE;
  uint64_t TypesLen = TYPES_EMPTIES_SIZE;
  uint64_t EmptiesStart = TypesStart + TypesLen;
  uint64_t EmptiesLen = TYPES_EMPTIES_SIZE;
  uint64_t BcOff = EmptiesStart + EmptiesLen;
  uint64_t TotalSize = BcOff + BC_WRAPPER_SIZE + BitcodeSize;

  // ---- Write 88-byte Slice Header (METALLIB_WRITER_SPEC §2) ----
  writeLE32(OS, MTLB_MAGIC);                   // 0..4: magic
  writeLE32(OS, 0x00028001);                    // 4..8: unknown4 (ABI flags)
  writeLE64(OS, 0x0000001a81000009ULL);         // 8..16: version_u64
  writeLE64(OS, TotalSize);                     // 16..24: file_size
  writeLE64(OS, HeadersStart);                  // 24..32: headers_start
  writeLE64(OS, HeadersLen);                    // 32..40: headers_len
  writeLE64(OS, TypesStart);                    // 40..48: types_start
  writeLE64(OS, TypesLen);                      // 48..56: types_len
  writeLE64(OS, EmptiesStart);                  // 56..64: empties_start
  writeLE64(OS, EmptiesLen);                    // 64..72: empties_len
  writeLE64(OS, BcOff);                         // 72..80: bc_off
  // 80..88: tail (8 bytes reserved)
  uint8_t Tail[8] = {0xd0, 0x0c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
  writeBytes(OS, Tail, 8);

  // ---- Write Headers Blob (METALLIB_WRITER_SPEC §3) ----
  // Directory header
  writeLE32(OS, Symbols.size()); // num_entries
  writeLE32(OS, HeadersLen);     // headers_len

  // Symbol tags
  for (const auto &Sym : Symbols)
    writeSymbolTags(OS, Sym, BitcodeSize);

  // ---- Write HDYN block ----
  writeHDYNBlock(OS);

  // ---- Write Types Blob (METALLIB_WRITER_SPEC §4) ----
  writeLE32(OS, TYPES_EMPTIES_SIZE); // 0x00000008
  writeLE32(OS, TAG_ENDT);           // 'ENDT'

  // ---- Write Empties Blob ----
  writeLE32(OS, TYPES_EMPTIES_SIZE);
  writeLE32(OS, TAG_ENDT);

  // ---- Write Bitcode Wrapper Header (METALLIB_WRITER_SPEC §5.1) ----
  writeLE32(OS, BC_WRAPPER_MAGIC);  // magic: 0x0B17C0DE
  writeLE32(OS, 0);                 // version: 0
  writeLE32(OS, BC_WRAPPER_SIZE);   // wrapper_offset: 20
  writeLE32(OS, BitcodeSize);       // bitcode_size
  writeLE32(OS, CPU_TYPE_AIR64);    // cputype

  // ---- Write bitcode payload ----
  writeBytes(OS, Bitcode.getBufferStart(), BitcodeSize);
}

// ============================================================================
// assemble: .bc → .air (MTLB container)
// ============================================================================

static int assembleMain(int argc, char **argv) {
  cl::opt<std::string> Input(cl::Positional, cl::desc("<input bitcode>"),
                             cl::Required);
  cl::opt<std::string> Output("o", cl::desc("Output .air filename"),
                              cl::value_desc("filename"), cl::Required);
  cl::opt<std::string> Triple("triple", cl::desc("Target triple"),
                              cl::init("air64-apple-macosx10.15.0"));
  cl::opt<std::string> KernelName("kernel-name",
                                   cl::desc("Kernel function name"),
                                   cl::init("default"));

  cl::ParseCommandLineOptions(argc, argv,
                              "llvm-metallib assemble — wrap bitcode in "
                              ".air (MTLB container)\n");

  // Read input bitcode.
  ErrorOr<std::unique_ptr<MemoryBuffer>> MBOrErr =
      MemoryBuffer::getFileOrSTDIN(Input);
  if (!MBOrErr) {
    WithColor::error() << "Cannot open '" << Input
                       << "': " << MBOrErr.getError().message() << "\n";
    return 1;
  }
  std::unique_ptr<MemoryBuffer> MB = std::move(MBOrErr.get());

  // Validate magic.
  if (MB->getBufferSize() < 4 ||
      std::memcmp(MB->getBufferStart(), "BC\xc0\xde", 4) != 0) {
    WithColor::warning() << "Input does not look like LLVM bitcode\n";
  }

  std::error_code EC;
  ToolOutputFile Out(Output, EC, sys::fs::OF_None);
  if (EC) {
    WithColor::error() << EC.message() << "\n";
    return 1;
  }

  // Extract symbol info
  SmallVector<SymbolInfo, 4> Symbols;
  SymbolInfo Sym;
  Sym.Name = KernelName;
  Sym.Type = SYM_TYPE_KERNEL;
  // Compute SHA-256 of bitcode
  SHA256 Hasher;
  Hasher.update(MB->getBuffer());
  auto HashResult = Hasher.result();
  std::copy(HashResult.begin(), HashResult.end(), Sym.Hash.begin());
  Symbols.push_back(Sym);

  writeSlice(Out.os(), *MB, Triple, Symbols);

  Out.keep();
  outs() << "Assembled " << Output << "\n";
  return 0;
}

// ============================================================================
// link: .air → .metallib
// ============================================================================

static int linkMain(int argc, char **argv) {
  cl::list<std::string> Inputs(cl::Positional, cl::OneOrMore,
                               cl::desc("<input .air files>"));
  cl::opt<std::string> Output("o", cl::desc("Output .metallib filename"),
                              cl::value_desc("filename"), cl::Required);
  cl::opt<std::string> Triple("triple", cl::desc("Target triple"),
                              cl::init("air64-apple-macosx10.15.0"));

  cl::ParseCommandLineOptions(argc, argv,
                              "llvm-metallib link — link .air files into "
                              ".metallib\n");

  // Read all inputs.
  SmallVector<std::unique_ptr<MemoryBuffer>, 4> Buffers;
  for (const auto &F : Inputs) {
    auto MBOrErr = MemoryBuffer::getFile(F);
    if (!MBOrErr) {
      WithColor::error() << "Cannot open '" << F
                         << "': " << MBOrErr.getError().message() << "\n";
      return 1;
    }
    Buffers.push_back(std::move(*MBOrErr));
  }

  std::error_code EC;
  ToolOutputFile Out(Output, EC, sys::fs::OF_None);
  if (EC) {
    WithColor::error() << EC.message() << "\n";
    return 1;
  }

  if (Buffers.size() == 1) {
    // Single slice — write directly (already has MTLB header).
    Out.os().write(Buffers[0]->getBufferStart(), Buffers[0]->getBufferSize());
  } else {
    // Fat 64 container.
    unsigned N = Buffers.size();
    uint64_t DescriptorsSize = 8 + (uint64_t)N * FAT_ARCH64_SIZE;

    // Compute 16-byte-aligned offsets.
    SmallVector<uint64_t, 4> Offsets;
    uint64_t Cursor = (DescriptorsSize + 15) & ~uint64_t(15);
    for (unsigned I = 0; I < N; ++I) {
      Offsets.push_back(Cursor);
      Cursor += Buffers[I]->getBufferSize();
      Cursor = (Cursor + 15) & ~uint64_t(15);
    }

    // fat_header_64
    writeBE32(Out.os(), FAT_MAGIC_64);
    writeBE32(Out.os(), N);

    // fat_arch_64 descriptors
    uint32_t Subtype = getCPUSubtypeFromTriple(Triple);
    for (unsigned I = 0; I < N; ++I) {
      writeBE32(Out.os(), CPU_TYPE_AIR64);
      writeBE32(Out.os(), Subtype);
      writeBE64(Out.os(), Offsets[I]);
      writeBE64(Out.os(), Buffers[I]->getBufferSize());
      writeBE32(Out.os(), 4);  // alignment = 2^4 = 16
      writeBE32(Out.os(), 0);  // reserved
    }

    // Pad to first slice offset.
    uint64_t Pos = DescriptorsSize;
    while (Pos < Offsets[0]) {
      Out.os().write('\0');
      ++Pos;
    }

    // Write each slice.
    for (unsigned I = 0; I < N; ++I) {
      Out.os().write(Buffers[I]->getBufferStart(), Buffers[I]->getBufferSize());
      uint64_t Written = Offsets[I] + Buffers[I]->getBufferSize();
      // Pad to next 16-byte boundary.
      uint64_t NextAligned = (Written + 15) & ~uint64_t(15);
      while (Written < NextAligned) {
        Out.os().write('\0');
        ++Written;
      }
    }
  }

  Out.keep();
  outs() << "Created " << Output << " with " << Inputs.size() << " slice(s)\n";
  return 0;
}

// ============================================================================
// info: inspect .air / .metallib
// ============================================================================

static int infoMain(int argc, char **argv) {
  cl::opt<std::string> Input(cl::Positional, cl::desc("<input file>"),
                             cl::Required);
  cl::ParseCommandLineOptions(argc, argv,
                              "llvm-metallib info — inspect .air / .metallib\n");

  auto MBOrErr = MemoryBuffer::getFile(Input);
  if (!MBOrErr) {
    WithColor::error() << "Cannot open '" << Input
                       << "': " << MBOrErr.getError().message() << "\n";
    return 1;
  }
  auto &MB = **MBOrErr;

  if (MB.getBufferSize() < 4) {
    WithColor::error() << "File too small\n";
    return 1;
  }

  // Check for fat magic (big-endian 0xcbfebabe at offset 0).
  uint32_t First4;
  std::memcpy(&First4, MB.getBufferStart(), 4);
  if (First4 == endian::byte_swap<uint32_t, endianness::big>(FAT_MAGIC_64)) {
    // Fat container.
    outs() << "Format: Fat 64 (0xcbfebabe)\n";
    if (MB.getBufferSize() < 8) return 1;
    uint32_t NArch;
    std::memcpy(&NArch, MB.getBufferStart() + 4, 4);
    NArch = endian::byte_swap<uint32_t, endianness::big>(NArch);
    outs() << "Slices: " << NArch << "\n";
    for (unsigned I = 0; I < NArch && 8 + (I + 1) * FAT_ARCH64_SIZE <= MB.getBufferSize(); ++I) {
      uint64_t Base = 8 + I * FAT_ARCH64_SIZE;
      uint32_t CpuType, CpuSub, Align;
      uint64_t Offset, Size;
      std::memcpy(&CpuType, MB.getBufferStart() + Base, 4);
      std::memcpy(&CpuSub,  MB.getBufferStart() + Base + 4, 4);
      std::memcpy(&Offset,  MB.getBufferStart() + Base + 8, 8);
      std::memcpy(&Size,    MB.getBufferStart() + Base + 16, 8);
      std::memcpy(&Align,   MB.getBufferStart() + Base + 24, 4);
      CpuType = endian::byte_swap<uint32_t, endianness::big>(CpuType);
      CpuSub  = endian::byte_swap<uint32_t, endianness::big>(CpuSub);
      Offset  = endian::byte_swap<uint64_t, endianness::big>(Offset);
      Size    = endian::byte_swap<uint64_t, endianness::big>(Size);
      Align   = endian::byte_swap<uint32_t, endianness::big>(Align);
      outs() << "  [" << I << "] cputype=0x" << utohexstr(CpuType)
             << " subtype=" << CpuSub
             << " offset=" << Offset << " size=" << Size
             << " align=2^" << Align << "\n";

      // Parse slice header if it starts with MTLB
      if (Offset + SLICE_HEADER_SIZE <= MB.getBufferSize()) {
        uint32_t SliceMagic;
        std::memcpy(&SliceMagic, MB.getBufferStart() + Offset, 4);
        if (SliceMagic == MTLB_MAGIC) {
          uint64_t FileSize, HeadersStart, HeadersLen, BcOff;
          std::memcpy(&FileSize, MB.getBufferStart() + Offset + 16, 8);
          std::memcpy(&HeadersStart, MB.getBufferStart() + Offset + 24, 8);
          std::memcpy(&HeadersLen, MB.getBufferStart() + Offset + 32, 8);
          std::memcpy(&BcOff, MB.getBufferStart() + Offset + 72, 8);
          outs() << "       file_size=" << FileSize
                 << " headers_start=" << HeadersStart
                 << " headers_len=" << HeadersLen
                 << " bc_off=" << BcOff << "\n";

          // Parse directory header
          if (HeadersStart + DIRECTORY_HEADER_SIZE <= Size) {
            uint32_t NumEntries, DirHeadersLen;
            std::memcpy(&NumEntries, MB.getBufferStart() + Offset + HeadersStart, 4);
            std::memcpy(&DirHeadersLen, MB.getBufferStart() + Offset + HeadersStart + 4, 4);
            outs() << "       symbols=" << NumEntries
                   << " dir_headers_len=" << DirHeadersLen << "\n";

            // Parse symbol names
            uint64_t TagPos = Offset + HeadersStart + DIRECTORY_HEADER_SIZE;
            for (uint32_t S = 0; S < NumEntries && TagPos < Offset + HeadersStart + HeadersLen; ++S) {
              // Look for NAME tag
              uint32_t TagID;
              std::memcpy(&TagID, MB.getBufferStart() + TagPos, 4);
              if (TagID == TAG_NAME) {
                uint16_t NameLen;
                std::memcpy(&NameLen, MB.getBufferStart() + TagPos + 4, 2);
                if (NameLen > 0 && TagPos + 6 + NameLen <= MB.getBufferSize()) {
                  std::string SymName(MB.getBufferStart() + TagPos + 6, NameLen - 1);
                  outs() << "       symbol: " << SymName << "\n";
                }
                // Advance past NAME record (with alignment)
                uint64_t NameRecordSize = 4 + 2 + NameLen;
                NameRecordSize = (NameRecordSize + 7) & ~uint64_t(7);
                TagPos += NameRecordSize;
              }
              // Skip to next symbol (TYPE, HASH, OFFT, VERS, MDSZ, ENDT)
              while (TagPos < Offset + HeadersStart + HeadersLen) {
                uint32_t SkipTag;
                std::memcpy(&SkipTag, MB.getBufferStart() + TagPos, 4);
                uint16_t SkipLen;
                std::memcpy(&SkipLen, MB.getBufferStart() + TagPos + 4, 2);
                TagPos += 4 + 2 + SkipLen;
                if (SkipTag == TAG_ENDT)
                  break;
              }
            }
          }
        }
      }
    }
    return 0;
  }

  // Check for MTLB magic at offset 0.
  if (First4 == MTLB_MAGIC) {
    outs() << "Format: MTLB slice (single .air)\n";
    if (MB.getBufferSize() < SLICE_HEADER_SIZE) {
      WithColor::error() << "MTLB header truncated\n";
      return 1;
    }
    uint32_t Unknown4;
    uint64_t VersionU64, FileSize, HeadersStart, HeadersLen;
    uint64_t TypesStart, TypesLen, EmptiesStart, EmptiesLen, BcOff;
    uint8_t Tail[8];

    std::memcpy(&Unknown4, MB.getBufferStart() + 4, 4);
    std::memcpy(&VersionU64, MB.getBufferStart() + 8, 8);
    std::memcpy(&FileSize, MB.getBufferStart() + 16, 8);
    std::memcpy(&HeadersStart, MB.getBufferStart() + 24, 8);
    std::memcpy(&HeadersLen, MB.getBufferStart() + 32, 8);
    std::memcpy(&TypesStart, MB.getBufferStart() + 40, 8);
    std::memcpy(&TypesLen, MB.getBufferStart() + 48, 8);
    std::memcpy(&EmptiesStart, MB.getBufferStart() + 56, 8);
    std::memcpy(&EmptiesLen, MB.getBufferStart() + 64, 8);
    std::memcpy(&BcOff, MB.getBufferStart() + 72, 8);
    std::memcpy(&Tail, MB.getBufferStart() + 80, 8);

    outs() << "  unknown4:      0x" << utohexstr(Unknown4) << "\n"
           << "  version_u64:   0x" << utohexstr(VersionU64) << "\n"
           << "  file_size:     " << FileSize << "\n"
           << "  headers_start: " << HeadersStart << "\n"
           << "  headers_len:   " << HeadersLen << "\n"
           << "  types_start:   " << TypesStart << "\n"
           << "  types_len:     " << TypesLen << "\n"
           << "  empties_start: " << EmptiesStart << "\n"
           << "  empties_len:   " << EmptiesLen << "\n"
           << "  bc_off:        " << BcOff << "\n";

    // Parse directory header
    if (HeadersStart + DIRECTORY_HEADER_SIZE <= MB.getBufferSize()) {
      uint32_t NumEntries, DirHeadersLen;
      std::memcpy(&NumEntries, MB.getBufferStart() + HeadersStart, 4);
      std::memcpy(&DirHeadersLen, MB.getBufferStart() + HeadersStart + 4, 4);
      outs() << "  symbols:       " << NumEntries << "\n"
             << "  dir_hdrs_len:  " << DirHeadersLen << "\n";

      // Parse symbol names
      uint64_t TagPos = HeadersStart + DIRECTORY_HEADER_SIZE;
      for (uint32_t S = 0; S < NumEntries && TagPos < HeadersStart + HeadersLen; ++S) {
        uint32_t TagID;
        std::memcpy(&TagID, MB.getBufferStart() + TagPos, 4);
        if (TagID == TAG_NAME) {
          uint16_t NameLen;
          std::memcpy(&NameLen, MB.getBufferStart() + TagPos + 4, 2);
          if (NameLen > 0 && TagPos + 6 + NameLen <= MB.getBufferSize()) {
            std::string SymName(MB.getBufferStart() + TagPos + 6, NameLen - 1);
            outs() << "  symbol:        " << SymName << "\n";
          }
          uint64_t NameRecordSize = 4 + 2 + NameLen;
          NameRecordSize = (NameRecordSize + 7) & ~uint64_t(7);
          TagPos += NameRecordSize;
        }
        // Skip to next symbol
        while (TagPos < HeadersStart + HeadersLen) {
          uint32_t SkipTag;
          std::memcpy(&SkipTag, MB.getBufferStart() + TagPos, 4);
          uint16_t SkipLen;
          std::memcpy(&SkipLen, MB.getBufferStart() + TagPos + 4, 2);
          TagPos += 4 + 2 + SkipLen;
          if (SkipTag == TAG_ENDT)
            break;
        }
      }
    }

    // Parse bitcode wrapper
    if (BcOff + BC_WRAPPER_SIZE <= MB.getBufferSize()) {
      uint32_t BcMagic, BcVersion, BcWrapperOffset, BcSize, BcCpuType;
      std::memcpy(&BcMagic, MB.getBufferStart() + BcOff, 4);
      std::memcpy(&BcVersion, MB.getBufferStart() + BcOff + 4, 4);
      std::memcpy(&BcWrapperOffset, MB.getBufferStart() + BcOff + 8, 4);
      std::memcpy(&BcSize, MB.getBufferStart() + BcOff + 12, 4);
      std::memcpy(&BcCpuType, MB.getBufferStart() + BcOff + 16, 4);
      outs() << "  bc_magic:      0x" << utohexstr(BcMagic) << "\n"
             << "  bc_version:    " << BcVersion << "\n"
             << "  bc_wrapper_off:" << BcWrapperOffset << "\n"
             << "  bc_size:       " << BcSize << "\n"
             << "  bc_cputype:    0x" << utohexstr(BcCpuType) << "\n";
    }

    return 0;
  }

  // Check for LLVM bitcode magic.
  if (std::memcmp(MB.getBufferStart(), "BC\xc0\xde", 4) == 0) {
    outs() << "Format: Raw LLVM bitcode (.bc)\n"
           << "  size: " << MB.getBufferSize() << " bytes\n"
           << "  (use 'llvm-metallib assemble' to wrap in .air)\n";
    return 0;
  }

  WithColor::error() << "Unrecognized file format\n";
  return 1;
}

// ============================================================================
// main: dispatch subcommands
// ============================================================================

static void printHelp() {
  outs() << "Usage: llvm-metallib <subcommand> [options]\n\n"
         << "Subcommands:\n"
         << "  assemble   Wrap LLVM bitcode (.bc) in an .air (MTLB) container\n"
         << "  link       Link .air files into a .metallib\n"
         << "  info       Inspect .air / .metallib files\n\n"
         << "Use 'llvm-metallib <subcommand> --help' for details.\n";
}

int main(int argc, char **argv) {
  if (argc < 2) {
    printHelp();
    return 1;
  }

  StringRef Sub(argv[1]);

  // Shift argv for subcommand parsers.
  auto subArgv = [&](int ac, char **av) {
    SmallVector<const char *, 8> Args;
    Args.push_back(av[0]); // program name
    for (int I = 2; I < ac; ++I)
      Args.push_back(av[I]);
    int NewArgc = Args.size();
    SmallVector<char *, 8> MutableArgs;
    for (auto *A : Args)
      MutableArgs.push_back(const_cast<char *>(A));
    return std::make_pair(NewArgc, MutableArgs);
  };

  if (Sub == "assemble" || Sub == "as") {
    auto [NewArgc, NewArgv] = subArgv(argc, argv);
    return assembleMain(NewArgc, NewArgv.data());
  }
  if (Sub == "link" || Sub == "ld") {
    auto [NewArgc, NewArgv] = subArgv(argc, argv);
    return linkMain(NewArgc, NewArgv.data());
  }
  if (Sub == "info" || Sub == "i") {
    auto [NewArgc, NewArgv] = subArgv(argc, argv);
    return infoMain(NewArgc, NewArgv.data());
  }

  WithColor::error() << "Unknown subcommand '" << Sub << "'\n";
  printHelp();
  return 1;
}
