/*
 * long double wrappers for Windows CE (ARM: long double == double, same
 * representation and ABI; the *l names only need to exist as symbols).
 * The CE complex functions (clogl, cpowl, csinhl, ...) call the *l names;
 * the desktop mingwex provided them, the CE object set did not.  These
 * wrappers complete the set - additive, no ABI impact.
 */
#include <math.h>
/* cimagl/creall live in <complex.h>; declare directly to keep this TU
   independent of the complex-header include graph.  */
long double creall(long double _Complex);
long double cimagl(long double _Complex);

long double expl  (long double x) { return (long double) exp ((double) x); }
long double logl  (long double x) { return (long double) log ((double) x); }
long double log10l(long double x) { return (long double) log10 ((double) x); }
long double powl  (long double x, long double y)
  { return (long double) pow ((double) x, (double) y); }
long double sqrtl (long double x) { return (long double) sqrt ((double) x); }
long double sinl  (long double x) { return (long double) sin ((double) x); }
long double cosl  (long double x) { return (long double) cos ((double) x); }
long double tanl  (long double x) { return (long double) tan ((double) x); }
long double asinl (long double x) { return (long double) asin ((double) x); }
long double acosl (long double x) { return (long double) acos ((double) x); }
long double atanl (long double x) { return (long double) atan ((double) x); }
long double atan2l(long double y, long double x)
  { return (long double) atan2 ((double) y, (double) x); }
long double sinhl (long double x) { return (long double) sinh ((double) x); }
long double coshl (long double x) { return (long double) cosh ((double) x); }
long double tanhl (long double x) { return (long double) tanh ((double) x); }
long double fabsl (long double x) { return (long double) fabs ((double) x); }
long double copysignl(long double x, long double y)
  { return (long double) copysign ((double) x, (double) y); }
long double hypotl(long double x, long double y)
  { return (long double) hypot ((double) x, (double) y); }
long double cargl(long double _Complex z)
  { return (long double) atan2 ((double) cimagl (z), (double) creall (z)); }
