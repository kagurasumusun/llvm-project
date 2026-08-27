/*
 * wcsdup for Windows CE: coredll6.def declares the export as "_wcsdup"
 * (desktop-underscore leftover), which never matches; provide the plain
 * wcsdup locally instead.  Additive.
 */
#include <wchar.h>
#include <stdlib.h>
#include <string.h>

wchar_t *
wcsdup (const wchar_t *s)
{
  size_t n = (wcslen (s) + 1) * sizeof (wchar_t);
  wchar_t *p = malloc (n);
  if (p != NULL)
    memcpy (p, s, n);
  return p;
}
