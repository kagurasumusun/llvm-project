/*===-- wince/runtime/dllcrt0.c - Windows CE DLL startup ------------------===//
 *
 * Part of the LLVM/Clang Windows CE toolchain. Derived from the public
 * domain mingw-runtime dllcrt1.c shipped with CeGCC, adapted for an
 * LLVM/Clang/LLD based toolchain (the GCC __gccmain/__do_global_dtors
 * machinery is replaced by the .CRT$XCU table walk and by libc++abi's
 * __cxa_atexit registration performed inside __main).
 *
 * Windows CE invokes a DLL's entry point (DllMainCRTStartup) with the
 * standard (hDll, dwReason, lpReserved) arguments.
 *
 * This file is placed in the public domain following its mingw-runtime
 * origin.
 *
 *===--------------------------------------------------------------------===*/

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdlib.h>
#include <stdio.h>

/* atexit.c */
extern BOOL __atexit_init (void);
extern void __dll_exit (void);

extern void __main (void);

extern BOOL WINAPI DllMain (HANDLE, DWORD, LPVOID);

/* Whether DllMainCRTStartup completed PROCESS_ATTACH initialization; the
 * mingw-runtime atexit table accessor is static, so the attach state is
 * tracked here rather than reaching into atexit.c. */
static BOOL wce_dll_attached = FALSE;

BOOL WINAPI
DllMainCRTStartup (HANDLE hDll, DWORD dwReason, LPVOID lpReserved)
{
  BOOL bRet;

  if (dwReason == DLL_PROCESS_ATTACH)
    {
      /* Initialize the private atexit table for this DLL. */
      if (!__atexit_init ())
        return FALSE;

      /* Run .CRT$XCU constructors; also registers C++ finalization with
       * the atexit table (via libc++abi) so C++ static destructors run
       * during DLL_PROCESS_DETACH / process exit. */
      __main ();
      wce_dll_attached = TRUE;
    }

  /* Call the user-supplied DllMain; dllmain.c in wce.lib provides a
   * default returning TRUE when the user does not supply one. */
  bRet = DllMain (hDll, dwReason, lpReserved);

  if ((dwReason == DLL_PROCESS_ATTACH) && !bRet)
    {
      /* User DllMain rejected the attach: run our terminators now so the
       * process does not tear a half-initialized DLL down later. */
      __dll_exit ();
    }

  if (dwReason == DLL_PROCESS_DETACH)
    {
      /* C++ static destructors of this DLL (registered via
       * __cxa_atexit(NULL,...)) run inside __dll_exit's atexit sweep. */
      __dll_exit ();
      /* If we never completed attach initialization, report failure the
       * way the CeGCC CRT did. */
      if (!wce_dll_attached)
        bRet = FALSE;
    }

  return bRet;
}
