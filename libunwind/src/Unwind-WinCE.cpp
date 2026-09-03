//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
//  Windows CE: Option B (MS-style) C++ exceptions - raise path.
//
// On Windows CE a C++ throw is routed through the kernel exception
// dispatcher (RaiseException -> coredll RtlDispatchException) so that the
// per-frame PDATA_EH handlers are invoked by the OS, exactly like the MS CRT
// does for __try/__except.  The kernel walks the compressed .pdata table,
// reads each C++ frame's PDATA_EH pair {__wince_cxx_frame_handler, FuncInfoB}
// and calls the handler with the fault CONTEXT.  See <wince_cxx_eh.h> for the
// shared ABI and the option-b design notes for the verified kernel contract
// (CORE/DLL/exdsptch.c, ARM/unwind.c, rtlsup.s).
//
//===----------------------------------------------------------------------===//

#if defined(__WINCE__)

#include <excpt.h>
#include <string.h>
#include <stdint.h>
#include <winbase.h>
#include <winnt.h>

#include <wince_cxx_eh.h>

#include "config.h"
#include "libunwind_ext.h"

// Per-image slot delivering the fault-time exception object to the CE landing
// pad.  The kernel overwrites R0 on resume (CONTEXT_TO_RETVAL =
// ExceptionCode, nkarm.h), so the object cannot be returned in a register;
// the handler stores it here and the generated landing-pad wrapper loads it
// into R0 before the Itanium landing pad body runs.  Zero-initialised (.bss).
uintptr_t WINCE_CXX_CURRENT_OBJ_NAME = 0;

/// Windows CE replacement for the self-unwinding _Unwind_RaiseException:
/// raise a kernel exception so coredll walks the frames and invokes the
/// per-frame PDATA_EH handlers (our C++ frame handler for C++ frames).
///
/// Returns only when no frame handled the exception (every handler continued
/// the search): that is an unhandled throw, and libc++abi calls
/// std::terminate() on the returned _URC_END_OF_STACK.
extern "C" _Unwind_Reason_Code
wince_unwind_raise_exception(_Unwind_Exception *exception_object) {
  _LIBUNWIND_TRACE_API("wince_unwind_raise_exception(ex_obj=%p)",
                       static_cast<void *>(exception_object));

  // Mark that this is not a forced unwind (EHABI 7.2 compatibility; also the
  // rethrow marker consulted by _Unwind_Resume_or_Rethrow).
  exception_object->unwinder_cache.reserved1 = 0;

  ULONG_PTR info[2] = {
      WINCE_CXX_EH_MAGIC,  // consistency marker (validated by the handler)
      (ULONG_PTR)exception_object, // __cxa_exception* (begins _Unwind_Exception)
  };
  RaiseException(WINCE_CXX_EH_NUMBER, EXCEPTION_NONCONTINUABLE, 2,
                 (const DWORD *)info);

  // Every frame continued the search: unhandled exception.
  return _URC_END_OF_STACK;
}

/// Build an EHABI cursor at the frame represented by \p ctx (a CE CONTEXT*).
///
/// The C++ frame handler (__wince_cxx_frame_handler) is invoked by the kernel
/// for a specific frame and must run the Itanium personality on that frame to
/// decide whether it catches the exception (and, in unwind mode, to run that
/// frame's cleanup).  It calls this with a CONTEXT holding that frame's
/// registers (Pc = the frame's return address, Sp/Lr = the frame's values) to
/// obtain a cursor whose IP points at the frame; the frame's EHT (and hence
/// its LSDA) is then located by the standard IP lookup, so no explicit EHT or
/// LSDA pointer is needed.
///
/// Only the core integer registers are transferred (EHABI #4.7: __unw_getcontext
/// saves the core integer set; VFP is demand-saved and not needed for the
/// personality).  The ARM unw_context_t layout is r0-r12, sp, lr, pc (see the
/// __unw_getcontext assembly and Registers_arm::GPRs).
extern "C" int __unw_init_local_at_frame(unw_cursor_t *cursor,
                                         const void *ctx) {
  const CONTEXT *c = static_cast<const CONTEXT *>(ctx);
  unw_context_t uc;
  memset(&uc, 0, sizeof(uc));
  uint32_t *g = reinterpret_cast<uint32_t *>(uc.data);
  g[0]  = c->R0;  g[1]  = c->R1;  g[2]  = c->R2;  g[3]  = c->R3;
  g[4]  = c->R4;  g[5]  = c->R5;  g[6]  = c->R6;  g[7]  = c->R7;
  g[8]  = c->R8;  g[9]  = c->R9;  g[10] = c->R10; g[11] = c->R11;
  g[12] = c->R12; g[13] = c->Sp;  g[14] = c->Lr;  g[15] = c->Pc;
  return __unw_init_local(cursor, &uc);
}

#endif // defined(__WINCE__)
