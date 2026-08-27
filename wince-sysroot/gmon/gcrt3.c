/*
 * gcrt3.c - Windows CE startup for -pg (mon-style profiling).
 *
 * Derived from mingwrt's crt3.c (public domain, mingw-runtime package);
 * identical startup flow, plus the profiler hooks: the sampling profiler
 * starts after global constructors have run and stops after all atexit
 * handlers (which include global destructors) have completed, so the
 * histogram covers main()/WinMain() and teardown alike.
 *
 * Compiled to gcrt3.o; linked instead of crt3.o when the driver sees -pg
 * (CeGCC STARTFILE_SPEC: %{pg:gcrt3%O%s}), together with libgmon.a
 * (LIB_SPEC: %{pg:-lgmon}).
 */

#define __IN_MINGW_RUNTIME
#include <stdlib.h>
#include <stdio.h>
#include <process.h>
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

/* COREDLL export (declared here rather than via <float.h> so the startup
   object does not depend on which builtin float.h the compiler ships).  */
extern void __cdecl _fpreset(void);

extern void __gccmain(void);
extern void _pei386_runtime_relocator(void);

/* No atexit on coredll; mingwrt supplies the private table.  */
BOOL __atexit_init(void);

void __cdecl _cexit(void);

/* libgmon: user-mode sampling profiler (gmon.out, BSD format).  */
extern void __gmon_start(void);
extern void __gmon_stop(void);

void
WinMainCRTStartup (HINSTANCE hInst, HINSTANCE hPrevInst,
                   LPWSTR lpCmdLine, int nCmdShow)
{
  int nRet;

  _fpreset ();

  /* Adjust references to dllimported data that have non-zero offsets.  */
  _pei386_runtime_relocator ();

  __atexit_init ();

  /* Global class constructors; also registers the global destructor
     pass with atexit.  */
  __gccmain ();

  /* Start the sampling profiler; constructors above are not covered on
     purpose (they run before the profile window opens).  */
  __gmon_start ();

  nRet = WinMain (hInst, hPrevInst, lpCmdLine, nCmdShow);

  /* Flush output, run atexit handlers (global destructors included) -
     still inside the profile window.  */
  _cexit ();

  /* Stop the sampler and write gmon.out next to the executable.  */
  __gmon_stop ();

  ExitProcess (nRet);
}
