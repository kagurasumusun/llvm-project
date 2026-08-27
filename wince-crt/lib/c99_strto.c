/*===-- wince/runtime/c99_strto.c - C99 strto* for WinCE ----------------===//
 *
 * Part of the LLVM/Clang Windows CE toolchain.
 *
 * coredll exports strtod/strtol but none of the C99 relatives that the
 * mingwrt headers declare (stdlib.h: strtof, strtold, strtoll,
 * strtoull).  This file provides them in the CRT supplement (mingwex):
 *
 *   strtof   - strtod narrowed to float (the double result is fully
 *              rounded before the narrowing cast)
 *   strtold  - ARM WinCE uses the AAPCS mapping where long double has
 *              the same representation as double, so this is strtod
 *   strtoll  - the CeGCC strtoimax (intmax_t == long long on this
 *              target) reached through its public C99 name
 *   strtoull - likewise over strtoumax
 *
 * This file is placed in the public domain.
 *
 *===--------------------------------------------------------------------===*/

#include <stdlib.h>
#include <stdint.h>

float
strtof (const char * __restrict__ nptr, char ** __restrict__ endptr)
{
  return (float) strtod (nptr, endptr);
}

long double
strtold (const char * __restrict__ nptr, char ** __restrict__ endptr)
{
  /* long double == double on ARM WinCE (AAPCS soft-float). */
  return strtod (nptr, endptr);
}

long long
strtoll (const char * __restrict__ nptr, char ** __restrict__ endptr, int base)
{
  return (long long) strtoimax (nptr, endptr, base);
}

unsigned long long
strtoull (const char * __restrict__ nptr, char ** __restrict__ endptr, int base)
{
  return (unsigned long long) strtoumax (nptr, endptr, base);
}
