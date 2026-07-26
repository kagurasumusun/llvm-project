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
class Declarator;

namespace SemaMSL {

//===----------------------------------------------------------------------===//
// Language-mode gates (all inline).
//===----------------------------------------------------------------------===//

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

/// Convenience overload used from Sema code that has a Sema& in scope.
/// Defined out-of-line in SemaMSL.cpp to avoid forcing Sema.h transitively
/// on every consumer.
bool inInternalsMode(const Sema &S);

//===----------------------------------------------------------------------===//
// Per-diagnostic gates (all inline: zero call overhead at every site).
//===----------------------------------------------------------------------===//

/// Whether ``half`` / ``__fp16`` may appear as a function parameter or
/// return type without requiring the ``cl_khr_fp16`` extension.  Metal
/// user code allows this natively (MSL 2.0+ s2.1).
inline bool allowHalfArgsAndReturns(const LangOptions &LO) {
  return isMetalMode(LO);
}

/// Whether ``0.5h`` / ``65504.0h`` half literals are accepted without
/// ``cl_khr_fp16``.  Same rationale as above.
inline bool allowHalfLiteral(const LangOptions &LO) {
  return isMetalMode(LO);
}

/// Whether Sema should transfer a member-function's method address-space
/// qualifier into the resulting FunctionProtoType.  Metal always yes;
/// under OpenCL C++ same behaviour applies via the upstream code path.
inline bool transferMethodAddressSpace(const LangOptions &LO) {
  return isMetalMode(LO);
}

//===----------------------------------------------------------------------===//
// Encapsulated Sema-time helpers used by SemaType / SemaExpr / SemaDecl.
// These live out-of-line in SemaMSL.cpp so the upstream files consuming
// them retain a minimal delta from LLVM main.
//===----------------------------------------------------------------------===//

/// Should Sema::GetFullTypeForDeclarator's DeclaratorChunk::Function
/// path perform the Metal / OpenCL C++ method-qualifier address-space
/// transfer for this declarator?
///
/// Returns false for:
///   * non-Metal, non-OpenCLCPlusPlus modes,
///   * declarators that are ``friend`` (avoid AS mismatch with the
///     out-of-class definition -- see companion note in the .cpp),
///   * declarators whose ``DeclaratorContext`` is not ``Member`` /
///     ``File`` / ``CXXCatch`` (i.e. inner function types of parameters,
///     typedefs, etc., which must stay AS-free so template argument
///     deduction can succeed against AS-free reference specimens).
///
/// The upstream Sema call site keeps its ~40 lines of transfer logic;
/// this predicate simply owns the policy (WHEN to run it), letting the
/// call site drop from a 40-line inline conditional to a single
/// ``if (SemaMSL::shouldTransferMethodAddressSpace(state.getSema(), D))``
/// -- much less merge-conflict surface against upstream refactors of
/// clang/lib/Sema/SemaType.cpp.
bool shouldTransferMethodAddressSpace(const Sema &S, const Declarator &D);

} // namespace SemaMSL
} // namespace clang

#endif // LLVM_CLANG_SEMA_SEMAMSL_H
