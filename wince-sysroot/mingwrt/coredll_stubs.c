#include <errno.h>

#ifdef __COREDLL__

int* _errno()
{
  static int e = 0;
  return &e;
}

char* setlocale(int category, const char *locale)
{
  return 0;
}

long _get_osfhandle(int fd)
{
  return (long)fd;
}

#endif
