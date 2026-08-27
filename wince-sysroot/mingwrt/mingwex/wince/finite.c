/*
 * finite for Windows CE: true when x is neither infinity nor NaN.
 */
#include <math.h>

int
finite (double x)
{
  union { double d; unsigned long long u; } ux;
  ux.d = x;
  return (ux.u & 0x7ff0000000000000ULL) != 0x7ff0000000000000ULL;
}
