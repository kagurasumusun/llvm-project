/*
 * copysign for Windows CE (ARM: soft-float, no FP bit-twiddling asm).
 * Additive to the CE mingwex object set; no ABI impact.
 */
#include <math.h>

double
copysign (double x, double y)
{
  union { double d; unsigned long long u; } ux, uy;
  ux.d = x;
  uy.d = y;
  ux.u = (ux.u & 0x7fffffffffffffffULL) | (uy.u & 0x8000000000000000ULL);
  return ux.d;
}
