/*===-- wince/runtime/crt0.c - Windows CE EXE startup ---------------------===//
 *
 * Part of the LLVM/Clang Windows CE toolchain. Derived from the public
 * domain mingw-runtime sources shipped with CeGCC (crt3.c, crt1.c,
 * winmain_ce.c), adapted for an LLVM/Clang/LLD based toolchain:
 *  - GCC's __main()/__do_global_ctors constructor dispatch is replaced by
 *    the MSVC-style .CRT$XCU constructor table (walked by crtxcu.c).
 *  - The GCC-specific SJLJ machinery is not needed: the LLVM toolchain uses
 *    ARM EHABI unwinding through libunwind/libc++abi.
 *
 * Entry contract (Windows CE loader):
 *   The PE entry point is invoked with the WinMain argument set
 *   (hInstance, hPrevInstance, lpCmdLineW, nShowCmd).
 *
 *   WinMainCRTStartup: the classic WinCE entry; dispatches to WinMain
 *     (or to main() through the winmain.c adapter when the program only
 *     defines main).
 *   mainCRTStartup: for console-style programs that define main(); builds
 *     argc/argv from GetCommandLineW and dispatches to main().
 *
 * This file is placed in the public domain following its mingw-runtime
 * origin ("This file has no copyright assigned and is placed in the Public
 * Domain").
 *
 *===--------------------------------------------------------------------===*/

#define __IN_MINGW_RUNTIME
#include <stdlib.h>
#include <stdio.h>
#include <process.h>
#include <float.h>
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

/* Constructor table walker (crtxcu.c): runs .CRT$XCU constructors and
 * registers .CRT$XTU destructors. Replaces GCC's libgcc __main. */
extern void __main (void);

/* Private atexit table (atexit.c); coredll provides no atexit. */
extern BOOL __atexit_init (void);
extern void __dll_exit (void);

/* argv builder shared with dllcrt (init_args.c). */
extern void __mingw32_init_mainargs (void);
extern void __mingw32_free_mainargs (void);
extern char **environ; /* defined in init_args.c; WinCE exposes no env. */
void __cdecl _cexit (void); /* atexit.c: private atexit sweep + stdio flush */

/* WinCE has no filesystem globbing environment; init_args.c keeps the
 * mingw-compat copy of _CRT_glob.  The default translation mode is binary
 * (coredll's _fmode equivalent is not exported, matching the CeGCC CRT). */

void _fpreset (void)
{
  /* ARM soft-float targets have no x87-style control word to reset.  If a
   * VFP is present, clear the FPSCR exception flags so that the process
   * starts with a clean floating point environment. */
#if defined(__ARM_FP) && (__ARM_FP & 2)
  __asm__ volatile ("vmrs r0, fpsr\n\t"
                    "bic r0, r0, #0x9f\n\t"
                    "vmsr fpsr, r0" ::: "r0");
#endif
}

/* The C++ standard library (libc++abi) finalizes via __cxa_finalize; the
 * registration order below guarantees C++ destructors run before the C
 * library flushes stdio.  The weak reference keeps pure-C executables
 * linkable (the C++ runtime simply is not there). */
__attribute__((weak)) extern void __cxa_finalize (void *);
static void
__wce_run_cxx_finalizers (void)
{
  if (__cxa_finalize)
    __cxa_finalize (0);
}

static int
__wce_CRTStartup_common (void)
{
  int nRet;

  _fpreset ();

  /* Initialize the private atexit table (coredll has no atexit). */
  if (!__atexit_init ())
    return 255;

  /* C++ runtime: register __wce_run_cxx_finalizers first so it runs last
   * (LIFO), after user atexit handlers, and run .CRT$XCU constructors. */
  atexit (__wce_run_cxx_finalizers);
  __main ();

  return 0;
}

/*
 * Windows CE invokes the entry point with WinMain's arguments.
 */
void
WinMainCRTStartup (HINSTANCE hInst, HINSTANCE hPrevInst,
                   LPWSTR lpCmdLine, int nCmdShow)
{
  int nRet;
  extern int PASCAL WinMain (HINSTANCE, HINSTANCE, LPWSTR, int);

  nRet = __wce_CRTStartup_common ();
  if (nRet)
    ExitProcess (nRet);

  nRet = WinMain (hInst, hPrevInst, lpCmdLine, nCmdShow);

  /* C++ destructors, then user atexit handlers (LIFO). */
  _cexit ();
  __dll_exit ();
  ExitProcess (nRet);
}

/*
 * Console-style startup for programs defining main().
 * WinCE passes WinMain arguments even here; they are ignored.
 */
void
mainCRTStartup (void)
{
  int nRet;
  extern int main (int, char **, char **);

  nRet = __wce_CRTStartup_common ();
  if (nRet)
    ExitProcess (nRet);

  /* Build __argc/__argv from GetCommandLineW (UTF-8 argv). */
  __mingw32_init_mainargs ();

  nRet = main (__argc, __argv, environ);

  __mingw32_free_mainargs ();

  _cexit ();
  __dll_exit ();
  ExitProcess (nRet);
}
