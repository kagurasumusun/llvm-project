/* Console-style application using the C runtime. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main (int argc, char **argv)
{
  char buf[64];
  strcpy (buf, "hi");
  strcat (buf, "!");
  printf ("%s argc=%d\n", buf, argc);
  {
    void *p = malloc (32);
    if (p)
      {
        memset (p, 0, 32);
        free (p);
      }
  }
  return 0;
}
