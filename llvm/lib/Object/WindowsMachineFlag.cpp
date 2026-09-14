//===- WindowsMachineFlag.cpp ---------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// Functions for implementing the /machine: flag.
//
//===----------------------------------------------------------------------===//

#include "llvm/Object/WindowsMachineFlag.h"

#include "llvm/ADT/StringRef.h"
#include "llvm/ADT/StringSwitch.h"
#include "llvm/BinaryFormat/COFF.h"
#include "llvm/Support/ErrorHandling.h"

using namespace llvm;

// Returns /machine's value.
COFF::MachineTypes llvm::getMachineType(StringRef S) {
  // Flags must be a superset of Microsoft lib.exe /machine flags.
  return StringSwitch<COFF::MachineTypes>(S.lower())
      .Cases({"x64", "amd64"}, COFF::IMAGE_FILE_MACHINE_AMD64)
      .Cases({"x86", "i386"}, COFF::IMAGE_FILE_MACHINE_I386)
      .Case("arm", COFF::IMAGE_FILE_MACHINE_ARMNT)
      .Case("arm64", COFF::IMAGE_FILE_MACHINE_ARM64)
      .Case("arm64ec", COFF::IMAGE_FILE_MACHINE_ARM64EC)
      .Case("arm64x", COFF::IMAGE_FILE_MACHINE_ARM64X)
      .Case("mips", COFF::IMAGE_FILE_MACHINE_R4000)
      .Default(COFF::IMAGE_FILE_MACHINE_UNKNOWN);
}

StringRef llvm::machineToStr(COFF::MachineTypes MT) {
  switch (MT) {
  // Upstream spells the ARMNT machine "arm" and that spelling is kept: it is
  // what MinGW drivers pass and what existing messages say.  The plain ARM
  // machine type a Windows CE image carries shares the spelling rather than
  // taking a name of its own, because no /machine: flag selects it and a
  // second name for 32-bit ARM would be invented.  Where the two must be told
  // apart the target triple says which: it is the only spelling that names an
  // OS, and -m takes one.
  case COFF::IMAGE_FILE_MACHINE_ARMNT:
  case COFF::IMAGE_FILE_MACHINE_ARM:
    return "arm";
  case COFF::IMAGE_FILE_MACHINE_ARM64:
    return "arm64";
  case COFF::IMAGE_FILE_MACHINE_ARM64EC:
    return "arm64ec";
  case COFF::IMAGE_FILE_MACHINE_ARM64X:
    return "arm64x";
  case COFF::IMAGE_FILE_MACHINE_AMD64:
    return "x64";
  case COFF::IMAGE_FILE_MACHINE_I386:
    return "x86";
  default:
    llvm_unreachable("unknown machine type");
  }
}
