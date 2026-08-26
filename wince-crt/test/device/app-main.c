/* Device test 2: C, main() entry, printf/malloc/argv.
 * Build: clang --target=arm-pc-wince -mconsole app-main.c -o app-main.exe
 * Pass:  running "app-main.exe alpha beta" shows
 *        argc=3 argv[1]=alpha sum=...; exit code 0.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int
main (int argc, char **argv)
{
  char *p = (char *) malloc (64);
  if (!p)
    return 2;
  strcpy (p, "ok");
  printf ("argc=%d\n", argc);
  for (int i = 0; i < argc; ++i)
    printf ("argv[%d]=%s\n", i, argv[i]);
  printf ("malloc=%s\n", p);
  free (p);
  return (argc >= 1 && strcmp (p ? "ok" : "x", "ok") == 0) ? 0 : 1;
}
