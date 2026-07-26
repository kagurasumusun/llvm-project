//===--- SemaMSL.h - MSL two-axis compilation-mode helpers ------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This header defines the tiny facade the rest of Sema uses to consult
// Metal Shading Language (MSL) compile-mode axes without embedding
// MSL-specific branches inline in the upstream Sema*.cpp files.
//
// Two axes exist per Apple metalfe design (verified in
// kagurasumusun/metal-info reference-apple/clang/32023.883/include/metal/*
// via ``#pragma METAL internals : enable/disable`` markers):
//
//   Axis 1 (user code, .metal shaders)
//     Strict MSL spec rules apply.  Half-precision in a parameter list
//     is *forbidden*, private-member access via friend tricks rejected,
//     ``__metal_*`` builtins not name-resolvable in ordinary lookup, etc.
//
//   Axis 2 (stdlib parse, Apple's <metal_*> headers)
//     Apple's stdlib deliberately violates axis-1 rules.  metalfe
//     brackets the header window with ``#pragma METAL internals :
//     enable / disable``; every axis-1 gate is relaxed inside.
//
// The stdlib header parse enables MetalInternals; user code parses with
// it cleared.  Every Metal-specific gate in Sema* consults
// ``SemaMSL::inInternalsMode(S)`` before applying the relaxed rule.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_CLANG_SEMA_SEMAMSL_H
#define LLVM_CLANG_SEMA_SEMAMSL_H

#include "clang/Basic/LangOptions.h"

namespace clang {
class Sema;

namespace SemaMSL {

/// Return true if we are compiling in Metal mode at all
/// (``-x metal`` or an ``-std=metal*`` was selected).  Every axis-2 gate
/// implies this.
inline bool isMetalMode(const LangOptions &LO) { return LO.Metal; }

/// Return true if the parser has entered a ``#pragma METAL internals :
/// enable`` region.  Only Apple's own metal_stdlib toggles this bit; user
/// code never does (the pragma is undocumented and semantically an
/// implementation detail).
inline bool inInternalsMode(const LangOptions &LO) {
  return LO.Metal && LO.MetalInternals;
}

/// Convenience overload used from Sema code that has a Sema& in scope
/// (the vast majority of call sites).  Defined inline to avoid a call
/// through the vtable.
bool inInternalsMode(const Sema &S);

//===----------------------------------------------------------------------===//
// Per-diagnostic gates.  Every one is a one-liner around
// ``inInternalsMode`` today; they exist as named predicates so future
// axis-2 additions (e.g. per-header enables via ``__attribute__((...))``,
// or a per-feature bit inside MetalInternals) do not require a
// touch-and-recompile of every Sema*.cpp file.
//===----------------------------------------------------------------------===//

/// Whether ``half`` / ``__fp16`` may appear as a function parameter or
/// return type without requiring the ``cl_khr_fp16`` extension.  Metal
/// user code allows this natively (MSL 2.0+ s2.1), so we return true
/// whenever ``isMetalMode`` is true -- the axis-2 aspect enters only if
/// a strict axis-1 subset is later requested via a flag.
inline bool allowHalfArgsAndReturns(const LangOptions &LO) {
  return isMetalMode(LO);
}

/// Whether ``0.5h`` / ``65504.0h`` half literals are accepted without
/// ``cl_khr_fp16``.  Same rationale as above.
inline bool allowHalfLiteral(const LangOptions &LO) {
  return isMetalMode(LO);
}

/// Whether the Sema pass should attempt to transfer a member-function's
/// method address-space qualifier (``thread`` / ``device`` / ``constant``
/// / ``threadgroup``) into the resulting FunctionProtoType.
///
/// Enabled for all Metal member functions -- axis 2 doesn't relax it
/// because Apple's stdlib member functions rely on the transfer for
/// overload resolution to work.
inline bool transferMethodAddressSpace(const LangOptions &LO) {
  return isMetalMode(LO);
}

} // namespace SemaMSL
} // namespace clang

#endif // LLVM_CLANG_SEMA_SEMAMSL_H
