//===--- SemaMSL.cpp - MSL two-axis compile-mode helpers ------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// Out-of-line SemaMSL facade entries.  Every predicate in SemaMSL.h that
// does NOT need the full Sema/AST class definitions lives inline in the
// header; the ones here specifically encapsulate MSL-specific Sema logic
// so the upstream Sema*.cpp files stay minimal-delta from LLVM main.
//
//===----------------------------------------------------------------------===//

#include "clang/Sema/SemaMSL.h"
#include "clang/AST/Type.h"
#include "clang/Sema/DeclSpec.h"
#include "clang/Sema/Sema.h"

namespace clang {
namespace SemaMSL {

bool inInternalsMode(const Sema &S) {
  return inInternalsMode(S.getLangOpts());
}

//===----------------------------------------------------------------------===//
// shouldTransferMethodAddressSpace
//
// Predicate consumed by clang/lib/Sema/SemaType.cpp's DeclaratorChunk::
// Function branch to decide whether the Metal method-qualifier
// address-space transfer path should fire.  The heavy lifting
// (walking method qualifiers, resolving OpenCL vs Metal LangAS,
// applying the language default) stays inline in SemaType.cpp so we
// don't diverge the transfer semantics from the upstream OpenCL C++
// implementation; the *policy* (when to fire vs skip) is what lives
// here.
//
// Two exclusion axes captured here:
//
//   (a) FRIEND declarations.
//       ``friend METAL_FUNC void f(...);`` in a class body has non-member
//       semantics: the out-of-class definition
//       ``constexpr METAL_FUNC void f(...) { ... }`` does NOT run through
//       this branch and therefore gets no AS on its FunctionProtoType.  If
//       we attached __private to the friend prototype we would produce a
//       mismatch and 10+ "conflicting types" diagnostics against Apple's
//       <__bits/metal_texture_common>::_build_sampler_state helper.
//
//   (b) INNER function-typed declarators.
//       Function-pointer / function-typedef parameters
//       (``bool (*)(_acceleration_structure<Tags...>)``) live inside a
//       parameter declarator, not the member declaration itself.  Their
//       DeclaratorContext is ``Prototype`` / ``TypeName`` / ``LambdaExpr``,
//       NOT ``Member`` / ``File`` / ``CXXCatch``.  Tagging their inner
//       FunctionProtoType with __private makes Apple's <metal_raytracing>
//       ``__is_null_acceleration_structure`` template deduction fail
//       against the AS-free reference (46 diagnostics in CI run 30182354278).
//===----------------------------------------------------------------------===//

bool shouldTransferMethodAddressSpace(const Sema &S, const Declarator &D) {
  const LangOptions &LO = S.getLangOpts();
  if (!LO.OpenCLCPlusPlus && !LO.Metal)
    return false;
  if (D.getDeclSpec().isFriendSpecified())
    return false;
  DeclaratorContext Ctx = D.getContext();
  if (Ctx != DeclaratorContext::Member && Ctx != DeclaratorContext::File &&
      Ctx != DeclaratorContext::CXXCatch)
    return false;
  return true;
}

} // namespace SemaMSL
} // namespace clang
