//===-- llvm-metallib.cpp - Package bitcode into a .metallib --------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// llvm-metallib links one or more AIR bitcode modules and wraps the result
// in a .metallib container (see llvm/Object/Metallib.h) — the role Apple's
// `metallib` tool plays in its own toolchain, in the single-slice ("case A",
// no --list/--all-link) form every measured plain-metal invocation produces:
//
//   metallib file.air -o file.metallib
//
// The public symbol directory is populated from the linked module's
// !air.kernel / !air.vertex / !air.fragment metadata, exactly as in the
// measured containers (k.metallib lists its kernel; P02's probe.metallib
// lists its vertex and fragment entry points).
//
// Divergences from Apple's tool (documented in Metallib.h): no per-function
// re-serialization / reflection restructuring, and the container UUID
// defaults to all-zeros for reproducible builds instead of a random value
// (--uuid overrides).
//
//===----------------------------------------------------------------------===//

#include "llvm/AsmParser/Parser.h"
#include "llvm/Bitcode/BitcodeReader.h"
#include "llvm/Bitcode/BitcodeWriter.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Metadata.h"
#include "llvm/IR/Module.h"
#include "llvm/IRReader/IRReader.h"
#include "llvm/Linker/Linker.h"
#include "llvm/Object/Metallib.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Error.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/InitLLVM.h"
#include "llvm/Support/MemoryBuffer.h"
#include "llvm/Support/SourceMgr.h"
#include "llvm/Support/ToolOutputFile.h"
#include "llvm/Support/WithColor.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/TargetParser/Triple.h"
#include <memory>
#include <system_error>

using namespace llvm;

static cl::OptionCategory MetallibCategory("llvm-metallib Options");

static cl::list<std::string> InputFilenames(cl::Positional, cl::OneOrMore,
                                            cl::desc("<input bitcode files>"),
                                            cl::cat(MetallibCategory));

static cl::opt<std::string> OutputFilename("o", cl::desc("Output filename"),
                                           cl::Required,
                                           cl::value_desc("filename"),
                                           cl::cat(MetallibCategory));

static cl::opt<std::string>
    TripleName("triple",
               cl::desc("AIR triple recorded in the container (defaults to "
                        "the linked module's target triple)"),
               cl::value_desc("triple"), cl::cat(MetallibCategory));

static cl::opt<std::string> UuidOpt(
    "uuid",
    cl::desc("Container UUID as 32 hex characters (default: all zeros, "
             "reproducible)"),
    cl::value_desc("hex"), cl::cat(MetallibCategory));

static cl::opt<std::string> ReflectionFile(
    "reflection",
    cl::desc("Raw reflection blob appended verbatim after the bitcode, as in "
             "the load-verified generator's -r capture mode"),
    cl::value_desc("filename"), cl::cat(MetallibCategory));

static void exitWithError(Twine Message) {
  WithColor::error(errs(), "llvm-metallib") << Message << "\n";
  exit(1);
}

static void exitWithError(Error E, Twine Message) {
  handleAllErrors(std::move(E), [&](const ErrorInfoBase &EI) {
    WithColor::error(errs(), "llvm-metallib") << Message << ": "
                                              << EI.message() << "\n";
  });
  exit(1);
}

/// Parse one input file as IR (bitcode wrapper, raw bitcode or textual IR).
static std::unique_ptr<Module> loadModule(StringRef Filename,
                                          LLVMContext &Ctx) {
  ErrorOr<std::unique_ptr<MemoryBuffer>> BufferOrErr =
      MemoryBuffer::getFileOrSTDIN(Filename);
  if (std::error_code EC = BufferOrErr.getError())
    exitWithError("cannot open " + Filename + ": " + EC.message());
  std::unique_ptr<MemoryBuffer> Buffer = std::move(BufferOrErr.get());

  SMDiagnostic Err;
  std::unique_ptr<Module> M;
  if (isBitcode(reinterpret_cast<const unsigned char *>(
                    Buffer->getBufferStart()),
                reinterpret_cast<const unsigned char *>(
                    Buffer->getBufferEnd()))) {
    Expected<std::unique_ptr<Module>> MOrErr =
        parseBitcodeFile(Buffer->getMemBufferRef(), Ctx);
    if (!MOrErr)
      exitWithError("cannot load " + Filename + ": " +
                    toString(MOrErr.takeError()));
    M = std::move(*MOrErr);
  } else {
    M = parseAssembly(Buffer->getMemBufferRef(), Err, Ctx);
  }
  if (!M)
    exitWithError("cannot load " + Filename + ": " + Err.getMessage());
  return M;
}

/// Collect the public entry points of the linked module from its !air.*
/// named metadata. Measured containers list, in order, every entry in
/// !air.kernel followed by !air.vertex then !air.fragment (P02 lists its
/// two symbols as vertex, fragment; k.metallib its one kernel).
static std::vector<metallib::PublicSymbol> collectPublicSymbols(Module &M) {
  struct Stage {
    StringRef NamedMD;
    uint8_t Type;
  };
  static const Stage Stages[] = {
      {"air.kernel", metallib::KernelEntry},
      {"air.vertex", metallib::VertexEntry},
      {"air.fragment", metallib::FragmentEntry},
      {"air.object", metallib::Function},
      {"air.mesh", metallib::Function},
  };

  std::vector<metallib::PublicSymbol> Symbols;
  for (const Stage &S : Stages) {
    NamedMDNode *NMD = M.getNamedMetadata(S.NamedMD);
    if (!NMD)
      continue;
    for (const MDNode *Entry : NMD->operands()) {
      // Each !air.<stage> operand starts with the entry function as a
      // ConstantAsMetadata (the measured module layout), followed by the
      // stage- and argument-metadata nodes.
      if (Entry->getNumOperands() < 1)
        continue;
      auto *CAM = dyn_cast_or_null<ConstantAsMetadata>(Entry->getOperand(0));
      Function *F = CAM ? dyn_cast<Function>(CAM->getValue()) : nullptr;
      if (!F)
        continue;
      Symbols.push_back({F->getName().str(), S.Type});
    }
  }
  return Symbols;
}

int main(int argc, char **argv) {
  InitLLVM X(argc, argv);
  cl::HideUnrelatedOptions({&MetallibCategory, &getColorCategory()});
  cl::ParseCommandLineOptions(argc, argv, "llvm-metallib - Metal library writer\n");

  LLVMContext Ctx;
  std::unique_ptr<Module> Composite =
      std::make_unique<Module>("llvm.metallib", Ctx);

  // Merge every input module; this is the same module-merge Apple's
  // metallib performs on its .air inputs before wrapping.
  for (StringRef F : InputFilenames) {
    std::unique_ptr<Module> M = loadModule(F, Ctx);
    if (Linker::linkModules(*Composite, std::move(M), Linker::Flags::None))
      exitWithError("linking input module '" + F + "' failed");
  }

  // Re-serialize the linked module; the embedded bytes are exactly the
  // module the container's directory describes.
  SmallVector<char, 0> Bitcode;
  {
    raw_svector_ostream OS(Bitcode);
    WriteBitcodeToFile(*Composite, OS);
  }

  std::string TripleStr = TripleName.getValue();
  if (TripleStr.empty())
    TripleStr = Composite->getTargetTriple();
  Triple T(TripleStr);
  if (!T.isAIR())
    exitWithError("target triple '" + TripleStr +
                  "' is not an AIR triple");
  if (T.getAIRVersion() == 0)
    WithColor::warning(errs(), "llvm-metallib")
        << "triple '" << TripleStr
        << "' carries no AIR version (expected airXX_vNN-apple-*); "
           "legacy header fields will be written\n";

  metallib::SliceConfig Cfg;
  Cfg.TripleName = TripleStr;
  if (!UuidOpt.empty()) {
    if (UuidOpt.size() != 32)
      exitWithError("--uuid expects exactly 32 hex characters");
    for (unsigned I = 0; I < 16; ++I) {
      unsigned Byte;
      if (StringRef(UuidOpt).substr(2 * I, 2).getAsInteger(16, Byte))
        exitWithError("--uuid is not valid hex: '" + UuidOpt + "'");
      Cfg.UUID[I] = static_cast<uint8_t>(Byte);
    }
  }

  std::string Reflection;
  if (!ReflectionFile.empty()) {
    ErrorOr<std::unique_ptr<MemoryBuffer>> BufferOrErr =
        MemoryBuffer::getFile(ReflectionFile);
    if (std::error_code EC = BufferOrErr.getError())
      exitWithError("cannot open reflection blob '" + ReflectionFile +
                    "': " + EC.message());
    Reflection = (*BufferOrErr)->getBuffer().str();
  }

  std::vector<metallib::PublicSymbol> Symbols = collectPublicSymbols(*Composite);

  std::error_code EC;
  auto Out = std::make_unique<ToolOutputFile>(OutputFilename, EC,
                                              sys::fs::OF_None);
  if (EC)
    exitWithError("cannot open output '" + OutputFilename + "': " +
                  EC.message());

  ArrayRef<uint8_t> BitcodeBytes(
      reinterpret_cast<const uint8_t *>(Bitcode.data()), Bitcode.size());
  if (Error E = metallib::writeSlice(Out->os(), BitcodeBytes, Symbols, Cfg,
                                     Reflection))
    exitWithError(std::move(E), "writing container failed");
  Out->keep();
  return 0;
}
