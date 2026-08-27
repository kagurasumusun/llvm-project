/*
 * hypot for Windows CE: sqrt(x*x + y*y) with the naive scaling guard.
 * The CE runtime has no platform hypot; consumers here are the complex
 * functions (catan/cpow), where this precision is sufficient.
 */
#include <math.h>

double
hypot (double x, double y)
{
  double ax = fabs (x), ay = fabs (y);
  double hi, lo;
  if (ax > ay)
    {
      hi = ax; lo = ay;
    }
  else
    {
      hi = ay; lo = ax;
    }
  if (lo == 0.0)
    return hi;
  return sqrt (hi * hi + lo * lo);
}
