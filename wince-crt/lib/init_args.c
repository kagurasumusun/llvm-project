/*===-- wince/runtime/init_args.c - argv/envp/fmode glue ------------------===//
 *
 * Part of the LLVM/Clang Windows CE toolchain.
 *
 * Windows CE has no process environment block: argv must be parsed from
 * GetCommandLineW(), and "environ" is an empty table.  The accessors
 * __p___argc()/__p___argv()/__p__environ()/__p___fmode() mirror the mingw
 * CRT interface that the mingw-runtime headers (stdlib.h) expect.
 *
 * The argument splitting follows the CommandLineToArgvW rules:
 *   - arguments are separated by runs of spaces/tabs,
 *   - double quotes toggle "in quotes" mode,
 *   - 2n backslashes followed by a quote -> n backslashes and quote
 *     toggling, 2n+1 backslashes followed by a quote -> n backslashes
 *     followed by a literal quote,
 *   - backslashes not followed by a quote are literal.
 *
 * This file is placed in the public domain following the mingw-runtime
 * origin of the surrounding CRT.
 *
 *===--------------------------------------------------------------------===*/

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdlib.h>
#include <string.h>

int __argc = 0;
char **__argv = NULL;
static char *__argv_buffer = NULL; /* owns the argument text storage */
char **environ = NULL; /* WinCE exposes no environment block. */
char **_environ = NULL;
int _fmode_dll = 0;

/* Kept for mingw-runtime source compatibility; globbing is disabled. */
int _CRT_glob = 0;

int *__cdecl __p___argc (void) { return &__argc; }
char ***__cdecl __p___argv (void) { return &__argv; }
char ***__cdecl __p__environ (void) { return &_environ; }
int *__cdecl __p___fmode (void) { return &_fmode_dll; }

/* Split SRC into arguments, writing the (NUL-terminated) copies to DST and
 * the argument pointers to ARGV.  Either destination may be NULL for a
 * counting pass.  Returns the argument count. */
static int
wce_split_cmdline (const char *src, char *dst, char **argv)
{
  int argc = 0;

  for (;;)
    {
      const char *argstart;
      char *out = dst;
      int in_quotes = 0;

      while (*src == ' ' || *src == '\t')
        src++;
      if (!*src)
        break;

      if (argv)
        argv[argc] = dst;
      argstart = src;
      (void) argstart;

      while (*src && (in_quotes || (*src != ' ' && *src != '\t')))
        {
          int nbs = 0;
          while (*src == '\\')
            {
              nbs++;
              src++;
            }
          if (*src == '"')
            {
              int n = nbs / 2;
              if (nbs & 1)
                {
                  /* Odd number of backslashes: the quote is literal. */
                  while (n--)
                    if (dst)
                      *dst++ = '\\';
                  if (dst)
                    *dst++ = '"';
                  src++;
                }
              else
                {
                  while (n--)
                    if (dst)
                      *dst++ = '\\';
                  in_quotes = !in_quotes;
                  src++;
                }
            }
          else
            {
              while (nbs--)
                if (dst)
                  *dst++ = '\\';
              if (dst)
                *dst++ = *src;
              src++;
            }
        }
      if (dst)
        *dst++ = '\0';
      argc++;
    }

  if (argv)
    argv[argc] = NULL;
  return argc;
}

void
__mingw32_init_mainargs (void)
{
  LPWSTR wcmd = GetCommandLineW ();
  int wlen = lstrlenW (wcmd);
  int bytes = WideCharToMultiByte (CP_UTF8, 0, wcmd, wlen + 1, NULL, 0,
                                   NULL, NULL);
  char *wbuf, *abuf;
  int argc;
  char **argv;

  if (bytes <= 0)
    {
      /* Very long or invalid command line: fall back to a plain byte copy
       * of the ANSI view so that argv[0] is at least usable. */
      char *ansi = (char *) malloc (wlen + 1);
      if (!ansi)
        return;
      WideCharToMultiByte (CP_ACP, 0, wcmd, wlen + 1, ansi, wlen + 1,
                           NULL, NULL);
      ansi[wlen] = '\0';
      argc = wce_split_cmdline (ansi, NULL, NULL);
      argv = (char **) malloc ((argc + 1) * sizeof (char *));
      if (!argv)
        {
          free (ansi);
          return;
        }
      abuf = (char *) malloc (wlen + 1);
      if (!abuf)
        {
          free (argv);
          free (ansi);
          return;
        }
      wce_split_cmdline (ansi, abuf, argv);
      free (ansi);
      __argv = argv;
      __argv_buffer = abuf;
      __argc = argc;
      environ = _environ = NULL;
      return;
    }

  wbuf = (char *) malloc (bytes);
  if (!wbuf)
    return;
  WideCharToMultiByte (CP_UTF8, 0, wcmd, wlen + 1, wbuf, bytes, NULL, NULL);
  wbuf[bytes - 1] = '\0';

  argc = wce_split_cmdline (wbuf, NULL, NULL);
  argv = (char **) malloc ((argc + 1) * sizeof (char *));
  if (!argv)
    {
      free (wbuf);
      return;
    }
  abuf = (char *) malloc (bytes + 1);
  if (!abuf)
    {
      free (argv);
      free (wbuf);
      return;
    }
  wce_split_cmdline (wbuf, abuf, argv);
  free (wbuf);

  __argv = argv;
  __argv_buffer = abuf;
  __argc = argc;
  environ = _environ = NULL;
}

void
__mingw32_free_mainargs (void)
{
  if (__argv)
    {
      free (__argv_buffer);
      __argv_buffer = NULL;
      free (__argv);
      __argv = NULL;
      __argc = 0;
    }
}
