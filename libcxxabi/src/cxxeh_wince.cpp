//===- cxxeh_wince.cpp - Windows CE Option B C++ frame handler ------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// Windows CE "Option B" C++ exception frame handler.
//
// Windows CE has no MSVC C++ exception tables and its kernel does not
// understand the Itanium LSDA. Option B drives the unwind through the CE
// kernel's own exception machinery (exactly like the MS CRT does for
// __try/__except): each C++ frame's PDATA_EH pair points at
// __wince_cxx_frame_handler, which runs the Itanium *type matching*
// (__gxx_personality_v0 + the already-emitted EHT/LSDA) on a libunwind cursor
// built from the CONTEXT the kernel hands it.
//
// The CE kernel (exdsptch.c) calls this handler in two modes:
//
//   * DISPATCH (search) mode - IS_DISPATCHING(flags). The kernel walks the
//     frames virtually and asks each C++ frame whether it catches. The handler
//     must decide LOCALLY (only this frame; no cleanup, no stepping up the
//     stack - the kernel does the stepping by calling the next frame's handler).
//     On a match it sets pDC->ControlPc to the landing pad and returns
//     ExceptionExecuteHandler; the kernel then performs the real unwind
//     (RtlUnwind) to this frame and resumes at the landing pad.
//
//   * UNWIND (cleanup) mode - IS_UNWINDING(flags). The kernel unwinds from the
//     fault point to the catch frame, invoking the handler for each frame so
//     its locals' destructors run. For the catch frame itself the
//     EXCEPTION_TARGET_UNWIND flag is set.
//
// R0 clobber: the kernel resumes with R0 = the exception code
// (CONTEXT_TO_RETVAL), so the caught _Unwind_Exception* cannot be delivered in
// R0. The handler stores it in __wince_cxx_current_obj and the
// compiler-generated landing-pad wrapper loads it into R0 (R1 = typeinfo is
// delivered normally).
//
// This translation unit compiles to nothing for non-Windows-CE targets.
//
//===----------------------------------------------------------------------===//

#if defined(__arm__) && defined(__WINCE__)

// Order the includes the way the CE SDK umbrella expects (stdarg.h ->
// windef.h -> winbase.h, all provided by <windows.h>; individual CE headers do
// not self-include their basic types - see libunwind's Unwind-WinCE.cpp).
#include <windows.h>
#include <stdint.h>

#include "wince_cxx_eh.h"

// The Itanium reason codes (_Unwind_Reason_Code / _URC_*) come from unwind.h;
// unwind_arm_ehabi.h uses them but does not include unwind.h, so order matters.
#include <unwind.h>
#include <unwind_arm_ehabi.h>
// The libunwind cursor type. _Unwind_GetIP / _Unwind_GetGR are the inline
// accessors in unwind_arm_ehabi.h (operating on a libunwind cursor cast to
// _Unwind_Context* via _Unwind_VRS_Get/Set, exported from libunwind).
#include <libunwind.h>

extern "C" {
// libunwind SPI (C linkage), defined in libunwind's Unwind-WinCE.cpp and
// exported for the handler (Unit 1): build an EHABI cursor from a CE CONTEXT
// (GPRs) and resolve the frame's EHT/LSDA by the IP.
int __unw_init_local_at_frame(unw_cursor_t *cursor, const void *ctx);

// The Itanium personality (same library). EHABI 3-arg convention. On a match in
// the search state it saves the results to the exception's barrier cache
// (landing pad = bitpattern[3], typeinfo = bitpattern[4], sp = the CFA) and
// returns _URC_HANDLER_FOUND; in the cleanup state it matches the CFA against
// barrier_cache.sp and installs R0/R1/IP on the context.
_Unwind_Reason_Code __gxx_personality_v0(_Unwind_State state,
                                         _Unwind_Exception *exception_object,
                                         struct _Unwind_Context *context);
}

namespace {

// Validate the per-function handler table the kernel hands us as
// pDC->FunctionEntry->HandlerData (the PDATA_EH pHandlerData).
bool validFuncInfo(const PDISPATCHER_CONTEXT pDC) {
  const FuncInfoB *fi =
      reinterpret_cast<const FuncInfoB *>(pDC->FunctionEntry->HandlerData);
  return fi != nullptr && fi->magic == WINCE_FUNCINFOB_MAGIC &&
         fi->version == WINCE_FUNCINFOB_VERSION;
}

// Build an EHABI cursor for the frame. \p pc is a code address inside the
// frame (used to resolve its EHT/LSDA); \p cfa (the establisher frame) is the
// frame's home stack pointer, which the personality uses for both the search
// (barrier_cache.sp) and the cleanup. Only these two CONTEXT fields are read.
bool initFrameCursor(unw_cursor_t *cursor, uintptr_t pc, uintptr_t cfa) {
  CONTEXT frameCtx;
  __builtin_memset(&frameCtx, 0, sizeof(frameCtx));
  frameCtx.Pc = (DWORD)pc;
  frameCtx.Sp = (DWORD)cfa;
  frameCtx.Lr = 0;
  return __unw_init_local_at_frame(cursor, &frameCtx) == 0;
}

} // namespace

// Prototype (satisfies -Wmissing-prototypes); the symbol is referenced by the
// compiler-generated PDATA_EH pair.
extern "C" EXCEPTION_DISPOSITION __wince_cxx_frame_handler(
    PEXCEPTION_RECORD pExr, void *EstablisherFrame, PCONTEXT pCtx,
    PDISPATCHER_CONTEXT pDC);

extern "C" EXCEPTION_DISPOSITION __wince_cxx_frame_handler(
    PEXCEPTION_RECORD pExr, void *EstablisherFrame, PCONTEXT pCtx,
    PDISPATCHER_CONTEXT pDC) {
  // 1. We only service C++ throws. pExr->ExceptionInformation[1] is the
  //    _Unwind_Exception (the __cxa_exception's unwindHeader control block);
  //    any other code (hardware fault, a foreign handler's exception) is not
  //    ours.
  if (pExr->ExceptionCode != WINCE_CXX_EH_NUMBER ||
      pExr->NumberParameters < 2 ||
      pExr->ExceptionInformation[0] != WINCE_CXX_EH_MAGIC)
    return ExceptionContinueSearch;
  _Unwind_Exception *ex =
      reinterpret_cast<_Unwind_Exception *>(pExr->ExceptionInformation[1]);
  if (ex == nullptr)
    return ExceptionContinueSearch;

  // 2. Validate the per-function table (guards against a Dispatcher Context
  //    that is not fully initialised, which can happen in unwind mode).
  if (!validFuncInfo(pDC))
    return ExceptionContinueSearch;

  const uintptr_t cfa = reinterpret_cast<uintptr_t>(EstablisherFrame);
  const bool dispatching = IS_DISPATCHING(pExr->ExceptionFlags);

  if (dispatching) {
    // ===== DISPATCH (search) mode: local catch decision, no cleanup =====
    // pDC->ControlPc is the return address of the call from this frame to the
    // callee that raised - a code address inside this frame, so the EHT lookup
    // resolves this frame's EHT/LSDA.
    unw_cursor_t cursor;
    if (!initFrameCursor(&cursor, (uintptr_t)pDC->ControlPc, cfa))
      return ExceptionContinueSearch;
    struct _Unwind_Context *ctx = reinterpret_cast<struct _Unwind_Context *>(&cursor);

    // Ask the personality (EHABI search state) whether THIS frame catches. It
    // matches the type against this frame's LSDA only; it does not step up the
    // stack (the kernel does that by calling the next frame's handler). On a
    // match it records the results in the exception's barrier cache (the
    // landing pad is barrier_cache.bitpattern[3]; the EHABI search phase does
    // NOT put it in the cursor IP) and records barrier_cache.sp = CFA.
    _Unwind_Reason_Code r =
        __gxx_personality_v0(_US_VIRTUAL_UNWIND_FRAME, ex, ctx);
    if (r == _URC_HANDLER_FOUND) {
      // This frame catches. Park the caught object for the landing-pad wrapper
      // (R0 is clobbered on resume) and tell the kernel to unwind to this frame
      // and resume at the landing pad (recovered from the barrier cache).
      WINCE_CXX_CURRENT_OBJ_NAME = reinterpret_cast<uintptr_t>(ex);
      const uintptr_t landingPad = ex->barrier_cache.bitpattern[3];
      if (landingPad == 0)
        return ExceptionContinueSearch;
      pDC->ControlPc = (DWORD)landingPad;
      return ExceptionExecuteHandler;
    }
    return ExceptionContinueSearch;
  }

  // ===== UNWIND (cleanup) mode: run this frame's destructors =====
  // pCtx->Eip was set to the unwind target (the catch frame's landing pad) by
  // RtlUnwind before the walk, and pCtx->Esp is the fault SP - neither is this
  // frame's own PC. The only reliable in-frame code address the kernel provides
  // in this mode is pDC->FunctionEntry (the frame's function entry); if it is
  // not a valid function entry the EHT lookup fails and we skip this frame's
  // cleanup (safe: no destructor runs, but nothing is corrupted).
  unw_cursor_t cursor;
  if (!initFrameCursor(&cursor, (uintptr_t)pDC->FunctionEntry, cfa))
    return ExceptionContinueSearch;
  struct _Unwind_Context *ctx = reinterpret_cast<struct _Unwind_Context *>(&cursor);

  _Unwind_Reason_Code r =
      __gxx_personality_v0(_US_UNWIND_FRAME_STARTING, ex, ctx);
  (void)r;

  if (IS_TARGET_UNWIND(pExr->ExceptionFlags)) {
    // This is the catch frame. The personality recognised it (its CFA matches
    // barrier_cache.sp from the search) and installed the landing-pad register
    // state on the cursor (R0 = caught object, R1 = typeinfo, IP = landing
    // pad). The kernel resumes from pCtx, so copy the EH return registers back.
    const uintptr_t ehR0 =
        _Unwind_GetGR(ctx, __builtin_eh_return_data_regno(0));
    const uintptr_t ehR1 =
        _Unwind_GetGR(ctx, __builtin_eh_return_data_regno(1));
    pCtx->R0 = (DWORD)ehR0;
    pCtx->R1 = (DWORD)ehR1;
    // pCtx->Pc already holds the landing pad (RtlUnwind set it to TargetIp);
    // leave it so the kernel resumes at the landing pad.
  }
  return ExceptionContinueSearch;
}

#endif // defined(__arm__) && defined(__WINCE__)
