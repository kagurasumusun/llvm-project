/*
 * intrin.h - Windows CE compiler-intrinsic entry point.
 *
 * Sources written for the MSVC CE compiler include <intrin.h> for
 * Interlocked* barriers, cache flush, and similar.  Under this toolchain
 * the compiler provides the ARM intrinsics via <arm_acle.h> and the ARM
 * <armintr.h> shipped in the clang resource directory; this header maps
 * the MSVC spellings onto them.  (Do not include the resource-dir
 * intrin.h directly for x86: on i386-pc-wince the generic clang header
 * dispatches by architecture the same way.)
 */
#ifndef __WINCE_INTRIN_H
#define __WINCE_INTRIN_H

#define WIN32_LEAN_AND_MEAN
#include <windows.h>    /* Interlocked* exports, CacheSync declaration */

#if defined(__arm__) || defined(_ARM_)
/* NB: do NOT include <arm_acle.h> on the ARMv5TE baseline - the clang
   resource header binds __dmb/__dsb/__isb to LLVM intrinsics that have
   no ARMv5 lowering.  On ARMv6+ it is fine and provides the native
   instructions.  */
#if defined(__ARM_ARCH) && __ARM_ARCH >= 6
#include <arm_acle.h>
#endif
/*
 * The resource-dir armintr.h only defines the barrier enum for real MSVC
 * (_MSC_VER path); define the barrier constants here so CE-era sources
 * using _ARMINTR_BARRIER_TYPE / __intrinsic_barrier compile.
 */
#ifndef _ARMINTR_BARRIER_TYPE
typedef enum
{
  _ARM_BARRIER_SY    = 0xF,
  _ARM_BARRIER_ST    = 0xE,
  _ARM_BARRIER_ISH   = 0xB,
  _ARM_BARRIER_ISHST = 0xA,
  _ARM_BARRIER_NSH   = 0x7,
  _ARM_BARRIER_NSHST = 0x6,
  _ARM_BARRIER_OSH   = 0x3,
  _ARM_BARRIER_OSHST = 0x2
} _ARMINTR_BARRIER_TYPE;
#endif

/* MSVC CE intrinsic spellings -> ARM instructions / ACLE.
 * __dmb needs ARMv6+; on the ARMv5TE baseline route through the kernel
 * (CacheSync is a full barrier on CE).  */
#if defined(__ARM_ARCH) && __ARM_ARCH >= 6
#define __intrinsic_barrier(_t) __dmb((_ARMINTR_BARRIER_TYPE)(_t))
#else
/* Source-level barrier: a compiler barrier plus the kernel cache-sync
   call (which is a full system barrier on CE).  */
static __inline void __wince_barrier(void)
{
  extern void CacheSync (int flags, LPVOID base, int length);
  __asm__ volatile ("" ::: "memory");
  CacheSync (0, NULL, 0);
}
#define __dmb(_t)      __wince_barrier()
#define __dsb(_t)      __wince_barrier()
#define __isb(_t)      __wince_barrier()
#define __intrinsic_barrier(_t) __wince_barrier()
#endif

static __inline void _ARM_cache_flush(unsigned int base, unsigned int size)
{
  /* CACHE_SYNC flushes both instruction and data caches (COREDLL
     CacheSync export).  Kept out-of-line: the kernel call needs the
     exact range.  */
  extern void CacheSync (int flags, LPVOID base, int length);
  CacheSync (0 /* CACHE_SYNC_ALL */, (LPVOID)(UINT_PTR)base, (int) size);
}

/* eMbedded Visual C++ also exposed these spellings:  */
#define _cache_flush(b,s) _ARM_cache_flush((b),(s))

#endif /* __arm__ */

#if defined(_M_IX86) || defined(__i386__)
/* CEPC (x86): pull the clang resource x86 intrinsics explicitly.  */
#include <x86intrin.h>
#endif

/* MSVC-style no-ops that CE-era sources reference from <intrin.h>.  */
#ifndef _CountLeadingZeros
static __inline unsigned int _CountLeadingZeros(unsigned long x)
{
  unsigned int n = 0;
  if (x == 0) return 32;
  while (!(x & 0x80000000UL)) { n++; x <<= 1; }
  return n;
}
#endif

#endif /* __WINCE_INTRIN_H */
