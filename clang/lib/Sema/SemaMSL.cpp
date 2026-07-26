//===--- SemaMSL.cpp - MSL two-axis compile-mode helpers ------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file implements only the parts of the SemaMSL facade that need
// access to the full ``Sema`` class definition (i.e. the ones that would
// require ``Sema.h`` transitively from a header).  Everything else is
// inline-defined in include/clang/Sema/SemaMSL.h so most call sites cost
// zero function-call overhead.
//
//===----------------------------------------------------------------------===//

#include "clang/Sema/SemaMSL.h"
#include "clang/Sema/Sema.h"

namespace clang {
namespace SemaMSL {

bool inInternalsMode(const Sema &S) {
  return inInternalsMode(S.getLangOpts());
}

} // namespace SemaMSL
} // namespace clang
