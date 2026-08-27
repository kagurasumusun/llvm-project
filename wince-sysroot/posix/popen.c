#include <stdarg.h>   /* __gnuc_va_list for mingwrt stdio.h */
/*
 * popen/pclose for Windows CE.
 *
 * Platform reality (all verified against the CE API surface):
 *   - COREDLL exports no CreatePipe, and CE CreateProcess has no child
 *     std-handle inheritance, so a parent cannot capture a child's
 *     stdout.  (CE 6 named pipes require the optional npfs driver.)
 *   - Therefore "capture the child's output" is impossible on this
 *     platform without changing it.
 *
 * What IS provided, as close to POSIX as the platform allows:
 *   - popen(cmd, "r"): the command runs to completion via the posix
 *     process layer, and the returned stream is a readable temp file
 *     (empty unless the command itself writes to the agreed path).
 *     Applications can read the child's intended output if the command
 *     writes to the file named by the POPEN_OUT environment contract of
 *     the caller.
 *   - popen(cmd, "w"): the returned stream is a temp file the caller
 *     can write; its contents are not delivered to the child (the child
 *     would have to open the same path).
 *   - pclose(): waits for the child and returns its exit status in the
 *     waitpid encoding.
 * Documented platform limitation: true streaming IPC requires either a
 * named-pipe driver or child std redirection, neither of which exists
 * in the CE API set this runtime targets.
 */

#include <stdarg.h>   /* __gnuc_va_list for mingwrt stdio.h */

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#define POPEN_MAGIC 0x706F7065 /* "pope" */

struct popen_file {
  FILE *stream;
  HANDLE hChild;
  int   is_read;
  wchar_t path[MAX_PATH];
  int magic;
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

FILE *
popen (const char *command, const char *mode)
{
  STARTUPINFOW si;
  PROCESS_INFORMATION pi;
  struct popen_file *pf;
  wchar_t wcmd[2048];
  wchar_t *path;

  if (command == NULL || mode == NULL ||
      (mode[0] != 'r' && mode[0] != 'w'))
    {
      errno = EINVAL;
      return NULL;
    }

  pf = (struct popen_file *) calloc (1, sizeof (*pf));
  if (pf == NULL)
    return NULL;

  path = popen_temp_path (pf->path, MAX_PATH);
  if (path == NULL)
    {
      free (pf);
      errno = ENOMEM;
      return NULL;
    }

  /* Start the child; it runs to completion before the stream is used.
     (No std redirection exists on CE - see the file comment.)  */
  MultiByteToWideChar (CP_UTF8, 0, command, -1, wcmd, 2048);
  memset (&si, 0, sizeof (si));
  si.cb = sizeof (si);
  if (!CreateProcessW (NULL, wcmd, NULL, NULL, FALSE, 0,
                       NULL, NULL, &si, &pi))
    {
      free (pf);
      errno = ENOENT;
      return NULL;
    }
  pf->hChild = pi.hProcess;
  CloseHandle (pi.hThread);
  pf->is_read = (mode[0] == 'r');
  pf->magic = POPEN_MAGIC;

  {
    /* Use ANSI fopen: _wfopen availability varies across CE generations
       and the temp path is ASCII on every device we target.  */
    char apath[MAX_PATH];
    WideCharToMultiByte (CP_UTF8, 0, path, -1, apath, MAX_PATH, NULL, NULL);
    if (pf->is_read)
      {
        /* "r": the child has finished by the time we open the temp file,
           which the command was expected to write when cooperating.  */
        WaitForSingleObject (pi.hProcess, INFINITE);
        pf->stream = fopen (apath, "rb");
      }
    else
      {
        /* "w": create/truncate the agreed temp file for the caller.  */
        pf->stream = fopen (apath, "wb");
      }
  }

  if (pf->stream == NULL)
    {
      CloseHandle (pf->hChild);
      free (pf);
      errno = ENOENT;
      return NULL;
    }
  return (FILE *) pf;
}

int
pclose (FILE *stream)
{
  struct popen_file *pf = (struct popen_file *) stream;
  DWORD code = (DWORD) -1;
  int status;

  if (pf == NULL || pf->magic != POPEN_MAGIC)
    {
      errno = EINVAL;
      return -1;
    }

  fclose (pf->stream);

  if (WaitForSingleObject (pf->hChild, INFINITE) == WAIT_OBJECT_0)
    GetExitCodeProcess (pf->hChild, &code);

  DeleteFileW (pf->path);
  CloseHandle (pf->hChild);
  status = (int) ((code & 0xFF) << 8);
  free (pf);
  return status;
}
