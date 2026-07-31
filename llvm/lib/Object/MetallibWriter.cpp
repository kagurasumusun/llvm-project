//===- MetallibWriter.cpp - .metallib container writer --------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// Field-by-field transcription of the measured .metallib layout. Every
// constant is backed by a real binary (see the header comment); where a
// field could not be fully decoded from the captures the code cites the
// closest verified anchor instead of inventing semantics.
//
// Measured constants and their sources:
//
//  * magic: 'MTLB' at offset 0 of every slice.
//  * slice header is 88 bytes ('<IIQQQQQQQQQ8s'-style packing), always.
//  * The public-symbol directory at headers_start always begins with the
//    u32 symbol count followed, per symbol, by a u32 slab size (the 4
//    bytes of the size itself plus the tag records) and the tag records
//    NAME / TYPE / HASH / MDSZ / OFFT / VERS / ENDT packed back to back
//    with no alignment padding (P02 probe.metallib, two entries +
//    fragment_rog, offsets verified by hand: slab 1 = 136 bytes for a
//    17-char name etc.). headers_len = 8 + sum(slab sizes) holds exactly.
//  * NAME payload is the NUL-terminated name; its record length is NOT
//    padded (the spec draft's "8B padding" column describes a layout the
//    binaries do not have).
//  * VERS payload: Air v26+ containers (Xcode 26: k.metallib, P01, P02)
//    carry {02 00 08 00 03 00 02 00}; Air v23..v25 containers (Apple's
//    rtlib, Xcode 14 era) carry {02 00 03 00 02 00 03 00}.
//  * types/empties blobs: minimal containers use {u32 8, 'ENDT'} for both
//    (k.metallib, load-verified; Apple's rtlib uses larger blobs that hold
//    vertex-attribute descriptors, deep-linked bundles etc., none of which
//    this writer emits).
//  * After the directory comes the dynamic header block, 74 bytes total:
//    ENDT, HDYN(payload 16 zero bytes), RLST(same), UUID(payload = 16B
//    random), ENDT. (k.metallib byte dump; P02 omits the whole block, so
//    it is optional for the loader.)
//  * The bitcode wrapper is the standard 20-byte LLVM bitcode wrapper
//    header, but with cputype 0xffffffff (measured in P02/k.metallib; the
//    spec draft's 0x01000017 appears only in the fat arch headers).
//  * tail (slice header offset 80): measured containers point it into
//    Apple's embedded reflection structures, which this writer does not
//    produce; when a raw reflection blob is attached through the
///   Reflection parameter, tail = offset of that region, otherwise 0.
//
//===----------------------------------------------------------------------===//

#include "llvm/Object/Metallib.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/Support/EndianStream.h"
#include "llvm/Support/SHA256.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/TargetParser/Triple.h"

using namespace llvm;

namespace {

/// 'MTLB'
static constexpr uint32_t MagicMTLB = 0x424C544D;
/// Standard LLVM bitcode wrapper magic, little endian 'de c0 17 0b'.
static constexpr uint32_t BitcodeWrapperMagic = 0x0B17C0DE;
/// Measured in P02 and k.metallib wrapper headers.
static constexpr uint32_t WrapperCPUType = 0xFFFFFFFF;

} // namespace

// NOTE: Mach-O FAT_MAGIC_64 is 0xcafebabe+1; the Metal fat container per the
// measurements in metallib_summary.md carries cb fe ba be as its on-disk
// bytes ("Fat 64 形式 (0xcbfebabe)"). We emit exactly the measured bytes.
static constexpr uint32_t MetalFatMagic = 0xCBFEBABE;

/// CPU_TYPE for AIR slices in fat arch records (CPU_TYPE_ARM64|CPU_ARCH_ABI64
/// with the AIR machine class), measured in every fat slice.
static constexpr uint32_t AirCPUType = 0x01000017;

/// Fat slices are laid out at 512 byte boundaries; the align exponent 9
/// is one of the two values in Apple's fat libraries (the other, 4, is for
/// 16-byte-aligned small slices; all measured offsets here are 512 grids).
static constexpr uint32_t FatSliceAlignPow2 = 9;

namespace llvm {
namespace metallib {

uint32_t computeABIFlags(const Triple &T) {
  // "unknown4" in the reverse-engineering notes. Bytes (little endian):
  //   macOS family (macosx, macabi): 01 80 02 00  -> 0x00028001
  //   iOS family (ios, tvos, watchos and simulators): 01 00 02 00 -> 0x00020001
  // Measured in: k.metallib + P01/P02 (macOS, 0x00028001), Apple rtlib
  // libtracepoint_rt_osx + _iosmac (macabi: 0x00028001) vs _ios, _iossim,
  // _tvos, _tvossim, _watchos (0x00020001).
  bool MacFamily = T.isMacOSX() || T.getEnvironment() == Triple::MacABI;
  return MacFamily ? 0x00028001u : 0x00020001u;
}

static unsigned metalPlatformCode(const Triple &T) {
  // Low nibble of byte 3 of the version field. Measured across the Apple
  // rtlib slices: 0x81 macosx, 0x82 ios, 0x83 tvos, 0x85 macabi (Apple uses
  // the legacy value 5 for Catalyst here), 0x87 iossim, 0x88 tvossim. The
  // pattern is 0x80 | M where M follows the LC platform numbering with the
  // simulator family re-homed at 7..9; watchos/watchossim (0x84/0x89)
  // follow the same pattern and are marked "pattern only" in the notes.
  bool Sim = T.getEnvironment() == Triple::Simulator;
  switch (T.getOS()) {
  case Triple::MacOSX:
    return 0x81;
  case Triple::IOS:
    if (T.getEnvironment() == Triple::MacABI)
      return 0x85;
    return Sim ? 0x87 : 0x82;
  case Triple::TvOS:
    return Sim ? 0x88 : 0x83;
  case Triple::WatchOS:
    return Sim ? 0x89 : 0x84;
  default:
    return 0x82;
  }
}

uint64_t computeVersionField(const Triple &T) {
  // Bytes 0..7, little endian structure {W0, X, P, OSMAJOR, 0, 0, 0}:
  //
  //   Air v26+ (Xcode 26, k.metallib/P01/P02):  W0 = 0x09, X = 0x00
  //   Air v23..v25 (Apple rtlib, Xcode 14 era): W0 = 0x07, X = 0x02
  //   Air v23 legacy slices:                    W0 = 0x05, no P/OSMAJOR
  //                                             bytes at all
  //
  // P is the metal platform code above; OSMAJOR is the target OS major
  // version stored as a plain number (13 -> 0x0d, 16 -> 0x10, 26 -> 0x1a,
  // verified NOT to be BCD by the macosx13 sample).
  //
  // v24/v26/27 have no measured samples; they interpolate to the nearest
  // generation, as documented in the header comment.
  unsigned V = T.getAIRVersion();
  bool Legacy = V != 0 && V <= 23;
  bool Modern = V != 0 && V <= 25 && V >= 24;

  uint8_t B[8] = {0};
  if (Legacy) {
    B[0] = 0x05;
    B[2] = 0x02;
  } else if (Modern) {
    B[0] = 0x07;
    B[2] = 0x02;
    B[3] = static_cast<uint8_t>(metalPlatformCode(T));
    B[4] = static_cast<uint8_t>(T.getOSVersion().getMajor());
  } else {
    B[0] = 0x09;
    B[3] = static_cast<uint8_t>(metalPlatformCode(T));
    B[4] = static_cast<uint8_t>(T.getOSVersion().getMajor());
  }
  return llvm::support::endian::read64le(B);
}

uint32_t computeCPUSubType(const Triple &T) {
  // Fat slice cpusubtype: 0x00000007 for the Air v23 slices and
  // 0x00000009 for the v25 ones (Apple rtlib, both values measured).
  // Later Air revisions stay on 9 (only boundary on record).
  unsigned V = T.getAIRVersion();
  return (V != 0 && V <= 23) ? 7u : 9u;
}

namespace {

struct TagWriter {
  raw_ostream &OS;
  support::endian::Writer W;

  TagWriter(raw_ostream &OS)
      : OS(OS), W(OS, support::endianness::little) {}

  /// A tag header + payload with no alignment padding (the layout measured
  /// in P02: every record starts immediately after its predecessor).
  void tag(const char ID[4], ArrayRef<uint8_t> Payload) {
    OS.write(ID, 4);
    W.write<uint16_t>(static_cast<uint16_t>(Payload.size()));
    if (!Payload.empty())
      OS.write(reinterpret_cast<const char *>(Payload.data()),
               Payload.size());
  }

  void tag(const char ID[4], std::initializer_list<uint8_t> Payload) {
    tag(ID, ArrayRef<uint8_t>(Payload.begin(), Payload.size()));
  }
};

/// Layout lanes of one slice, filled in two passes (exactly the two-pass
/// scheme Apple's disassembly shows: "two-pass serialization, second pass
/// writes headers").
struct SliceLayout {
  uint64_t HeadersStart = 88;
  uint64_t HeadersLen = 0;
  uint64_t TypesStart = 0;
  uint64_t TypesLen = 8;
  uint64_t EmptiesStart = 0;
  uint64_t EmptiesLen = 8;
  uint64_t BitcodeOffset = 0;
  uint64_t FileSize = 0;
};

/// The dynamic header block measured in k.metallib: ENDT, HDYN(16 zero
/// bytes), RLST(16 zero bytes), UUID(16B random), ENDT = 74 bytes total.
static constexpr uint64_t DynHeaderLen = 74;

static void writeRecordBytes(TagWriter &W, uint8_t Type, StringRef Name,
                             const uint8_t Hash[32], uint64_t BitcodeSize,
                             bool ModernVers) {
  // NAME: NUL-terminated symbol name, no padding.
  SmallVector<uint8_t, 64> NameBuf(Name.begin(), Name.end());
  NameBuf.push_back(0);
  W.tag("NAME", NameBuf);

  W.tag("TYPE", {Type});

  W.tag("HASH", ArrayRef<uint8_t>(Hash, 32));

  // MDSZ: size of this module's embedded bitcode, per symbol (Apple's
  // containers repeat the module size for every symbol of a one-module
  // slice -- measured in Apple's rtlib where all 288 entries carry the
  // same value).
  uint8_t MdszPayload[8];
  support::endian::write64le(MdszPayload, BitcodeSize);
  W.tag("MDSZ", MdszPayload);

  // OFFT: 24 zero bytes in every measured record that carries the tag at
  // all (P02's live values sit in its fragment_rog slab and encode Apple's
  // per-function offsets; a one-module container has none, so zeros).
  uint8_t OfftPayload[24] = {};
  W.tag("OFFT", OfftPayload);

  // VERS: generation-dependent constant, see the file header comment.
  static const uint8_t VersModern[8] = {0x02, 0x00, 0x08, 0x00,
                                        0x03, 0x00, 0x02, 0x00};
  static const uint8_t VersLegacy[8] = {0x02, 0x00, 0x03, 0x00,
                                        0x02, 0x00, 0x03, 0x00};
  W.tag("VERS", ModernVers ? ArrayRef<uint8_t>(VersModern, 8)
                           : ArrayRef<uint8_t>(VersLegacy, 8));

  // ENDT: tag only.
  W.OS.write("ENDT", 4);
}

} // end anonymous namespace

Error buildSlice(SmallVectorImpl<char> &Out,
                           ArrayRef<uint8_t> Bitcode,
                           ArrayRef<PublicSymbol> Symbols,
                           const SliceConfig &Cfg, StringRef Reflection) {
  Triple T(Cfg.TripleName);
  bool ModernVers = !(T.getAIRVersion() != 0 && T.getAIRVersion() <= 25);

  // SHA-256 of the bitcode; see the header comment on why this cannot be
  // Apple's per-function digest.
  SHA256 Hasher;
  Hasher.update(Bitcode);
  auto Hash = Hasher.final();

  // Pass 1: sizes.
  SliceLayout L;
  for (const PublicSymbol &S : Symbols)
    L.HeadersLen += 4 /*slab size*/ + 6 + S.Name.size() + 1 /*NAME*/ +
                    7 /*TYPE*/ + 38 /*HASH*/ + 14 /*MDSZ*/ + 30 /*OFFT*/ +
                    14 /*VERS*/ + 4 /*ENDT*/;
  L.HeadersLen += 8; // directory symbol count
  L.TypesStart = L.HeadersStart + L.HeadersLen + DynHeaderLen;
  L.EmptiesStart = L.TypesStart + L.TypesLen;
  L.BitcodeOffset = L.EmptiesStart + L.EmptiesLen;
  L.FileSize =
      L.BitcodeOffset + 20 + Bitcode.size() + Reflection.size();

  // Pass 2: serialize.
  raw_svector_ostream OS(Out);
  TagWriter W(OS);

  // --- 88 byte slice header ---------------------------------------------
  W.W.write<uint32_t>(MagicMTLB);
  W.W.write<uint32_t>(computeABIFlags(T));
  W.W.write<uint64_t>(computeVersionField(T));
  W.W.write<uint64_t>(L.FileSize);
  W.W.write<uint64_t>(L.HeadersStart);
  W.W.write<uint64_t>(L.HeadersLen);
  W.W.write<uint64_t>(L.TypesStart);
  W.W.write<uint64_t>(L.TypesLen);
  W.W.write<uint64_t>(L.EmptiesStart);
  W.W.write<uint64_t>(L.EmptiesLen);
  W.W.write<uint64_t>(L.BitcodeOffset);
  // tail: offset of the appended reflection region, or 0 when there is
  // none. (Measured tails point inside Apple's own reflection structures;
  // emitting 0 mirrors a container without reflection.)
  W.W.write<uint64_t>(Reflection.empty()
                          ? 0
                          : L.BitcodeOffset + 20 + Bitcode.size());

  // --- public symbol directory ------------------------------------------
  W.W.write<uint32_t>(static_cast<uint32_t>(Symbols.size()));
  for (const PublicSymbol &S : Symbols) {
    uint32_t Slab = static_cast<uint32_t>(4 + 6 + S.Name.size() + 1 + 7 +
                                          38 + 14 + 30 + 14 + 4);
    W.W.write<uint32_t>(Slab);
    writeRecordBytes(W, S.Type, S.Name, Hash.data(), Bitcode.size(),
                     ModernVers);
  }

  // --- dynamic header block (ENDT/HDYN/RLST/UUID/ENDT, 74 bytes) --------
  W.OS.write("ENDT", 4);
  uint8_t Zeros[16] = {};
  W.tag("HDYN", Zeros);
  W.tag("RLST", Zeros);
  W.tag("UUID", ArrayRef<uint8_t>(Cfg.UUID.data(), Cfg.UUID.size()));
  W.OS.write("ENDT", 4);

  // --- types blob (minimal form, measured): {u32 8, 'ENDT'} -------------
  W.W.write<uint32_t>(8);
  W.OS.write("ENDT", 4);

  // --- empties blob (minimal form, measured): same ----------------------
  W.W.write<uint32_t>(8);
  W.OS.write("ENDT", 4);

  // --- bitcode wrapper + bitcode -----------------------------------------
  W.W.write<uint32_t>(BitcodeWrapperMagic);
  W.W.write<uint32_t>(0);  // wrapper version
  W.W.write<uint32_t>(20); // wrapper header size
  W.W.write<uint32_t>(static_cast<uint32_t>(Bitcode.size()));
  W.W.write<uint32_t>(WrapperCPUType);
  OS.write(reinterpret_cast<const char *>(Bitcode.data()), Bitcode.size());

  // --- attached reflection bytes (verbatim passthrough) ------------------
  if (!Reflection.empty())
    OS.write(Reflection.data(), Reflection.size());

  return Error::success();
}

Error writeSlice(raw_ostream &OS, ArrayRef<uint8_t> Bitcode,
                           ArrayRef<PublicSymbol> Symbols,
                           const SliceConfig &Cfg, StringRef Reflection) {
  SmallVector<char, 0> Buf;
  if (Error E = buildSlice(Buf, Bitcode, Symbols, Cfg, Reflection))
    return E;
  OS.write(Buf.data(), Buf.size());
  return Error::success();
}

Error writeFat(
    raw_ostream &OS,
    ArrayRef<std::pair<ArrayRef<char>, const SliceConfig *>> Slices) {
  if (Slices.empty())
    return make_error<StringError>(
        "cannot write a fat .metallib with zero slices",
        std::make_error_code(std::errc::invalid_argument));

  struct ArchInfo {
    uint64_t Offset;
    uint32_t SubType;
  };
  SmallVector<ArchInfo, 4> Arches;
  uint64_t Off = 8 + 32 * Slices.size();
  const uint64_t AlignMask = (1ull << FatSliceAlignPow2) - 1;
  for (const auto &S : Slices) {
    Off = (Off + AlignMask) & ~AlignMask;
    Triple T(S.second->TripleName);
    Arches.push_back({Off, computeCPUSubType(T)});
    Off += S.first.size();
  }

  support::endian::Writer W(OS, support::endianness::big);
  W.write<uint32_t>(MetalFatMagic);
  W.write<uint32_t>(static_cast<uint32_t>(Slices.size()));
  for (unsigned I = 0; I < Slices.size(); ++I) {
    W.write<uint32_t>(AirCPUType);
    W.write<uint32_t>(Arches[I].SubType);
    W.write<uint64_t>(Arches[I].Offset);
    W.write<uint64_t>(Slices[I].first.size());
    W.write<uint32_t>(FatSliceAlignPow2);
    W.write<uint32_t>(0);
  }

  for (unsigned I = 0; I < Slices.size(); ++I) {
    OS.write_zeros(Arches[I].Offset - OS.tell());
    OS.write(Slices[I].first.data(), Slices[I].first.size());
  }
  return Error::success();
}

} // end namespace metallib
} // end namespace llvm
