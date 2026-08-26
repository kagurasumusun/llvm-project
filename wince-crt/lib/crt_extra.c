/*===-- wince-crt/lib/crt_extra.c - header-contract glue ------------------===//
 *
 * Part of the LLVM/Clang Windows CE toolchain.
 *
 * The mingw-runtime headers (compiled for the __COREDLL__ environment)
 * model MB_CUR_MAX as the function __mb_cur_max(); COREDLL.dll does not
 * export it and the CeGCC runtime does not provide it.  The Windows CE
 * CRT only supports the "C" locale, which is single-byte: report 1.
 *
 * This file is placed in the public domain.
 *
 *===--------------------------------------------------------------------===*/

#ifdef __COREDLL__

int __mb_cur_max (void)
{
  return 1;
}

#endif
