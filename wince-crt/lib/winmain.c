/*===-- wince/runtime/winmain.c - WinMain<->main adapters -----------------===//
 *
 * Part of the LLVM/Clang Windows CE toolchain. Derived from the public
 * domain mingw-runtime sources (main.c, winmain_ce.c, dllmain.c) shipped
 * with CeGCC.
 *
 * Linker-driven selection mirrors the mingw convention:
 *   - program defines main(): this module's WinMain() is pulled in and
 *     builds argc/argv from the wide command line, then calls main().
 *   - program defines WinMain(): the user's WinMain is used and this
 *     module's copy is not linked at all (archive semantics).
 *   - DLL without user DllMain(): dllmain() below is pulled in.
 *
 * This file is placed in the public domain following its mingw-runtime
 * origin.
 *
 *===--------------------------------------------------------------------===*/

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdlib.h>

#define ISSPACE(a) ((a) == ' ' || (a) == '\t')

extern int __mingw32_init_mainargs (void);
extern int __mingw32_free_mainargs (void);
extern char **environ;

/*
 * Adapter for programs that only define main(): invoked by
 * WinMainCRTStartup with the WinCE loader's arguments.  Builds
 * __argc/__argv (UTF-8) from the wide command line and calls main().
 */
int PASCAL
WinMain (HINSTANCE hInst, HINSTANCE hPrevInst, LPWSTR szCmdLine, int nShow)
{
  extern int main (int, char **, char **);
  int nRet;

  (void) hInst;
  (void) hPrevInst;
  (void) szCmdLine;
  (void) nShow;

  __mingw32_init_mainargs ();
  nRet = main (__argc, __argv, environ);
  __mingw32_free_mainargs ();
  return nRet;
}

/*
 * Default DllMain for DLLs that do not provide one (dllmain.c).
 */
BOOL WINAPI
DllMain (HANDLE hDll, DWORD dwReason, LPVOID lpReserved)
{
  (void) hDll;
  (void) dwReason;
  (void) lpReserved;
  return TRUE;
}
