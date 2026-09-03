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

#endif // defined(__WINCE__)
