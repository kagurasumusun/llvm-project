/*
 * popen/pclose for Windows CE.
 *
 * Platform reality (verified against the CE API surface):
 *   - COREDLL exports no CreatePipe, and CE CreateProcess has no child
 *     std-handle inheritance, so a parent cannot capture a child's
 *     stdout.  True streaming IPC would require either the optional
 *     npfs named-pipe driver or an OS change.
 *
 * What is provided - as close to POSIX as the platform allows, and
 * deterministic:
 *   - popen(cmd, "r"): the command runs to completion FIRST (it cannot
 *     write our stream), then the returned stream is the agreed temp
 *     file, so a cooperating child that writes the file (or a caller
 *     that inspects it) works; reading yields whatever the command left.
 *   - popen(cmd, "w"): the stream is the temp file; the command is
 *     DEFERRED to pclose, so the caller writes the file completely
 *     before the child (which may read that path) ever runs.
 *   - pclose(): runs the deferred child ("w" mode), waits, deletes the
 *     temp file and returns the exit status in waitpid encoding.
 */

#define WIN32_LEAN_AND_MEAN
#include <stdarg.h>   /* __gnuc_va_list for mingwrt stdio.h */
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#define POPEN_MAGIC 0x706F7065 /* "pope" */

struct popen_file {
  FILE   *stream;
  HANDLE  hChild;      /* NULL while the child is deferred ("w" mode) */
  int     is_read;
  char   *command;     /* deferred command line ("w" mode)            */
  wchar_t path[MAX_PATH];
  int     magic;
};

static wchar_t *
popen_temp_path (wchar_t *buf, DWORD size)
{
  wchar_t *slash;
  if (!GetModuleFileNameW (NULL, buf, size))
    return NULL;
  slash = wcsrchr (buf, L'\\');
  if (slash != NULL)
    *(slash + 1) = L'\0';
  else
    buf[0] = L'\0';
  if (wcslen (buf) + 12 >= size)
    return NULL;
  wcscat (buf, L"popen.tmp");
  return buf;
}

static char *
popen_ansi_path (const wchar_t *wpath)
{
  static char apath[MAX_PATH];
  WideCharToMultiByte (CP_ACP, 0, wpath, -1, apath, MAX_PATH, NULL, NULL);
  return apath;
}

static BOOL
popen_spawn (const wchar_t *wcmd, HANDLE *out_child)
{
  STARTUPINFOW si;
  PROCESS_INFORMATION pi;

  memset (&si, 0, sizeof (si));
  si.cb = sizeof (si);
  if (!CreateProcessW (NULL, (LPWSTR) wcmd, NULL, NULL, FALSE, 0,
                       NULL, NULL, &si, &pi))
    return FALSE;
  *out_child = pi.hProcess;
  CloseHandle (pi.hThread);
  return TRUE;
}

FILE *
popen (const char *command, const char *mode)
{
  struct popen_file *pf;
  wchar_t wcmd[2048];
  const char *apath;

  if (command == NULL || mode == NULL ||
      (mode[0] != 'r' && mode[0] != 'w'))
    {
      errno = EINVAL;
      return NULL;
    }

  pf = (struct popen_file *) calloc (1, sizeof (*pf));
  if (pf == NULL)
    {
      errno = ENOMEM;
      return NULL;
    }
  if (popen_temp_path (pf->path, MAX_PATH) == NULL)
    {
      free (pf);
      errno = ENOMEM;
      return NULL;
    }
  {
    size_t n = strlen (command) + 1;
    pf->command = (char *) malloc (n);
    if (pf->command == NULL)
      {
        free (pf);
        errno = ENOMEM;
        return NULL;
      }
    memcpy (pf->command, command, n);
  }
  pf->magic = POPEN_MAGIC;
  pf->is_read = (mode[0] == 'r');
  apath = popen_ansi_path (pf->path);
  MultiByteToWideChar (CP_ACP, 0, command, -1, wcmd, 2048);

  if (pf->is_read)
    {
      /* "r": child first (it cannot write our stream), then hand the
         caller the agreed temp file it was expected to fill.  */
      if (!popen_spawn (wcmd, &pf->hChild))
        {
          int e = errno;
          free (pf->command);
          free (pf);
          errno = e;
          return NULL;
        }
      WaitForSingleObject (pf->hChild, INFINITE);
      pf->stream = fopen (apath, "rb");
    }
  else
    {
      /* "w": caller writes; the child is spawned by pclose().  */
      pf->stream = fopen (apath, "wb");
    }

  if (pf->stream == NULL)
    {
      int e = errno;
      if (pf->hChild != NULL)
        CloseHandle (pf->hChild);
      free (pf->command);
      free (pf);
      errno = e;
      return NULL;
    }
  return (FILE *) pf;
}

int
pclose (FILE *stream)
{
  struct popen_file *pf = (struct popen_file *) stream;
  wchar_t wcmd[2048];
  DWORD code = (DWORD) -1;
  int status;

  if (pf == NULL || pf->magic != POPEN_MAGIC)
    {
      errno = EINVAL;
      return -1;
    }

  fclose (pf->stream);

  if (pf->hChild == NULL && pf->command != NULL)
    {
      /* deferred "w" child: run it now that the temp file is closed */
      MultiByteToWideChar (CP_ACP, 0, pf->command, -1, wcmd, 2048);
      if (popen_spawn (wcmd, &pf->hChild))
        WaitForSingleObject (pf->hChild, INFINITE);
    }

  if (pf->hChild != NULL)
    {
      if (WaitForSingleObject (pf->hChild, INFINITE) == WAIT_OBJECT_0)
        GetExitCodeProcess (pf->hChild, &code);
      CloseHandle (pf->hChild);
    }

  DeleteFileW (pf->path);
  status = (int) ((code & 0xFF) << 8);
  free (pf->command);
  free (pf);
  return status;
}
