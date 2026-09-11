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
      // "arm" is kept as the spelling of ARMNT for compatibility with MinGW
      // drivers, which pass -machine:arm for 32-bit Windows on ARM, so "armnt"
      // is added as the unambiguous spelling that lib.exe itself accepts.  The
      // plain ARM machine type needs no flag of its own: it is not part of
      // lib.exe's set, and it is taken from the input files like any other
      // machine.
      .Case("arm", COFF::IMAGE_FILE_MACHINE_ARMNT)
      .Case("armnt", COFF::IMAGE_FILE_MACHINE_ARMNT)
      .Case("arm64", COFF::IMAGE_FILE_MACHINE_ARM64)
      .Case("arm64ec", COFF::IMAGE_FILE_MACHINE_ARM64EC)
      .Case("arm64x", COFF::IMAGE_FILE_MACHINE_ARM64X)
      .Case("mips", COFF::IMAGE_FILE_MACHINE_R4000)
      .Default(COFF::IMAGE_FILE_MACHINE_UNKNOWN);
}

StringRef llvm::machineToStr(COFF::MachineTypes MT) {
  switch (MT) {
  // The two 32-bit Windows ARM machine types are named apart here although one
  // flag name covers both, since saying "arm" for both would make a message
  // about an ARM file and an ARMNT library contradict itself.  "armnt" is the
  // spelling lib.exe takes for the first of them, which is also what the "arm"
  // MinGW drivers pass stands for, while plain "arm" is the machine type a
  // Windows CE image carries and no /machine: flag selects.
  case COFF::IMAGE_FILE_MACHINE_ARMNT:
    return "armnt";
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
