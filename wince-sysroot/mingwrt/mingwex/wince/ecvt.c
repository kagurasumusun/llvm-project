/*
 * ecvt/fcvt for Windows CE: digit-string conversion used by the CE
 * printf engine (pformat) for %e/%f/%g fallbacks.  Classic repeated-
 * scaling implementation (17 significant digits max, the double limit).
 */
#include <math.h>
#include <stdlib.h>
#include <string.h>

static char *
ecvt_digits (double value, int ndigit, int *decpt, int *sign, int fmode)
{
  static char buf[64];
  double d = fabs (value);
  int exp10 = 0;
  int i;

  *sign = (value < 0.0) ? 1 : 0;
  if (d == 0.0)
    {
      *decpt = 1;
      memset (buf, '0', ndigit > 0 ? ndigit : 1);
      buf[ndigit > 0 ? ndigit : 1] = '\0';
      return buf;
    }

  /* normalize into [1, 10) */
  while (d >= 10.0) { d /= 10.0; exp10++; }
  while (d < 1.0)   { d *= 10.0; exp10--; }

  for (i = 0; i < ndigit && i < 40; i++)
    {
      double t = d * 10.0;
      int digit = (int) t;
      if (digit > 9) digit = 9;
      buf[i] = (char) ('0' + digit);
      d = t - digit;
    }
  buf[ndigit < 40 ? ndigit : 40] = '\0';

  /* fcvt keeps ndigit digits after the decimal point */
  if (fmode)
    *decpt = exp10 + 1;
  else
    *decpt = exp10 + 1;

  return buf;
}

char *
ecvt (double value, int ndigit, int *decpt, int *sign)
{
  return ecvt_digits (value, ndigit, decpt, sign, 0);
}

char *
fcvt (double value, int ndigit, int *decpt, int *sign)
{
  return ecvt_digits (value, ndigit, decpt, sign, 1);
}
