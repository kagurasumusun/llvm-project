/*
 * carg/cargf for Windows CE: the CE complex object set filters out the
 * carg sources, but clog/cpow reference them.  Additive.
 */
#include <math.h>
#include <complex.h>

double carg (double _Complex z)  { return atan2 (cimag (z), creal (z)); }
float  cargf(float _Complex z)   { return atan2f (cimagf (z), crealf (z)); }
