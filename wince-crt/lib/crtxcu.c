/*===-- wince/runtime/crtxcu.c - CRT$X* table dispatch --------------------===//
 *
 * Part of the LLVM/Clang Windows CE toolchain.
 *
 * Clang targeting a COFF environment emits static constructors into the
 * ".CRT$XCU" section and C __attribute__((destructor)) functions into
 * ".CRT$XTU"; the linker sorts contributions between the .CRT$XCA and
 * .CRT$XCZ (resp. .CRT$XTA/.CRT$XTZ) brackets.  This module provides:
 *
 *   __xc_a/__xc_z - constructor table brackets (walked by __main()).
 *   __xt_a/__xt_z - destructor table brackets (registered with atexit in
 *                   forward order at startup, so they run LIFO at exit).
 *   __main()      - runs the constructor table once (called from crt0.c /
 *                   dllcrt0.c; replaces GCC's libgcc __main).
 *
 * This file is placed in the public domain following the mingw-runtime
 * origin of the surrounding CRT.
 *
 *===--------------------------------------------------------------------===*/

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdlib.h>

typedef void (*_PVFV) (void);

/* Linker-ordered tables.  LLD sorts .CRT$X* contributions alphabetically,
 * producing the same layout convention as the MSVC CRT. */
#if defined(__clang__)
#pragma section(".CRT$XCA", read, long)
#pragma section(".CRT$XCZ", read, long)
#pragma section(".CRT$XTA", read, long)
#pragma section(".CRT$XTZ", read, long)
__declspec(allocate(".CRT$XCA")) __attribute__((used)) const _PVFV __xc_a[1] = { 0 };
__declspec(allocate(".CRT$XCZ")) __attribute__((used)) const _PVFV __xc_z[1] = { 0 };
__declspec(allocate(".CRT$XTA")) __attribute__((used)) const _PVFV __xt_a[1] = { 0 };
__declspec(allocate(".CRT$XTZ")) __attribute__((used)) const _PVFV __xt_z[1] = { 0 };
#else
#error "This runtime requires a COFF compiler (clang) for section placement"
#endif

static int __wce_ctors_done = 0;

void
__main (void)
{
  const _PVFV *p;

  if (__wce_ctors_done)
    return;
  __wce_ctors_done = 1;

  /* Register C-level destructors (.CRT$XTU) with atexit in forward order;
   * the LIFO atexit dispatch then runs them in reverse. */
  for (p = __xt_a; p < __xt_z; p++)
    {
      if (*p)
        atexit (*p);
    }

  /* Run constructors in table order. */
  for (p = __xc_a; p < __xc_z; p++)
    {
      if (*p)
        (**p) ();
    }
}
