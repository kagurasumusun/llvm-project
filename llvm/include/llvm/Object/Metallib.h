//===- Metallib.h - Metal library (.metallib) container writer --*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
///
/// \file
/// Writer for the .metallib container format that wraps Apple Metal AIR
/// bitcode into the archive the Metal runtime loads with
/// -[MTLDevice makeLibraryWithData:].
///
/// The container layout implemented here is byte-for-byte transcribed from
/// measured, real fixtures:
///
///  * The single-slice form and the 88-byte MTLB slice header come from a
///    real `metal` (metalfe-32023.883, Xcode 26) compile of a one-kernel
///    shader ("k.metallib") and from the golden captures
///    golden/P01| P02/metal32_macosx26/probe.metallib; the P-series files
///    were additionally load-tested through MTLDevice.makeLibrary on real
///    hardware by the metal-info project's write_metallib.py.
///  * The one-wrapped-module / many-directory-entries layout follows
///    Apple's own shipping runtime libraries
///    (lib/tracepoint_rt_{ios,osx,tvos,watchos,...}.metallib, parsed into
///    data/metallib_structure.csv), which the runtime loader demonstrably
///    accepts.
///  * The fat (multi-slice) form is the standard Mach-O FAT_MAGIC_64
///    envelope observed with cputype 0x01000017 (CPU_TYPE_AIR) slices.
///
/// Notes on the places where Apple and this writer intentionally differ:
///
///  * Apple's own `metallib` tool post-processes the module ("glueCode"):
///    it materialises one re-serialised bitcode module per public function,
///    emits read-only-global clones (e.g. probe_p02_fragment_rog) and folds
///    the reflection data into the last blob. Those transformations live in
///    the closed-source AIR backend and are not reproduced; this writer
///    embeds the module as produced by the frontend, which is the form the
///    load-verified minimal generator used.
///  * The per-symbol HASH payload matches neither SHA-256 of the wrapped
///    region nor plain SHA-256 of the bitcode in the P02 fixture (Apple's
///    disassembly documents a digest over the per-function module, which
///    does not exist here), so it is filled with SHA-256 of the embedded
///    bitcode. It functions as a cache key, not a signature.
///  * The `tail` field of the slice header points into Apple's reflection
///    structures when a reflection region is produced; it is 0 unless an
///    external reflection blob is appended, in which case it is the offset
///    of that region.
///
//===----------------------------------------------------------------------===//

#ifndef LLVM_OBJECT_METALLIB_H
#define LLVM_OBJECT_METALLIB_H

#include "llvm/ADT/ArrayRef.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/Support/Error.h"
#include <array>
#include <cstdint>
#include <string>

namespace llvm {

class Triple;
class raw_ostream;

namespace metallib {

/// Symbol type codes recorded in the public symbol directory's TYPE tag,
/// merged from all measured containers: P02's probe.metallib carries TYPE
/// 0x00 for the vertex entry and 0x01 for the fragment entry, k.metallib
/// carries 0x02 for the kernel entry, and Apple's runtime libraries carry
/// 0x03 for every (non-entry) function they publish.
enum SymbolType : uint8_t {
  VertexEntry = 0x00,
  FragmentEntry = 0x01,
  KernelEntry = 0x02,
  Function = 0x03,
};

/// One public-symbol directory record: NAME plus the stage code above.
struct PublicSymbol {
  std::string Name;
  uint8_t Type = Function;
};

/// Per-slice configuration whose fields depend on the compilation target.
struct SliceConfig {
  /// AIR triple the module was compiled for, e.g.
  /// `air64_v28-apple-macosx26.0.0`. Drives the version field (offset 8 of
  /// the slice header), the ABI-flags word at offset 4 and the fat slice's
  /// cpusubtype, all per the measurements documented in the .cpp file.
  std::string TripleName;

  /// 128-bit random build UUID written into the HDYN block. Apple's
  /// disassembly notes the field is "cogen u128 directly from ld (not u64
  /// pair)"; callers that need reproducible output pass a fixed value.
  std::array<uint8_t, 16> UUID = {};
};

/// Serialize one complete (thin) .metallib to \p OS.
///
/// \param Bitcode     The (unwrapped) LLVM bitcode of the module to embed.
/// \param Symbols     The module's public symbols. Measured containers
///                    list the entry points taken from its !air.kernel /
///                    !air.vertex / !air.fragment metadata.
/// \param Reflection  Optional raw reflection bytes, appended verbatim
///                    after the wrapped bitcode; the slice header's tail
///                    field then holds the region's offset (mirroring how
///                    the load-verified generator attached real reflection
///                    captures). Empty: tail is emitted as 0.
Error writeSlice(raw_ostream &OS, ArrayRef<uint8_t> Bitcode,
                 ArrayRef<PublicSymbol> Symbols, const SliceConfig &Cfg,
                 StringRef Reflection = StringRef());

/// Serialize one slice image into memory. Same contract as writeSlice;
/// used by the fat writer which needs the sizes up front.
Error buildSlice(SmallVectorImpl<char> &Out, ArrayRef<uint8_t> Bitcode,
                 ArrayRef<PublicSymbol> Symbols, const SliceConfig &Cfg,
                 StringRef Reflection = StringRef());

/// Serialize a FAT_MAGIC_64 multi-slice .metallib from pre-rendered slice
/// images (buildSlice output). Slices are laid out at 512-byte alignment
/// (the align value 8+1=9 measured in Apple's fat libraries) with the
/// FAT_ARCH_64 headers computed from each slice's configuration.
Error writeFat(raw_ostream &OS,
               ArrayRef<std::pair<ArrayRef<char>, const SliceConfig *>>
                   Slices);

/// Compute the 64-bit version field at offset 8 of the slice header from
/// an AIR triple. Exposed for tests.
uint64_t computeVersionField(const Triple &T);

/// Compute the 32-bit ABI-flags word at offset 4 of the slice header
/// ("unknown4" in the reverse-engineering notes) from an AIR triple.
uint32_t computeABIFlags(const Triple &T);

/// Compute the cpusubtype for a fat slice from an AIR triple.
uint32_t computeCPUSubType(const Triple &T);

} // end namespace metallib
} // end namespace llvm

#endif // LLVM_OBJECT_METALLIB_H
