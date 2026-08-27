#include <stdarg.h>   /* __gnuc_va_list for mingwrt stdio.h */
/*
 * POSIX process family for Windows CE, over COREDLL's process API
 * (CreateProcess / WaitForSingleObject / GetExitCodeProcess /
 * TerminateProcess - all verified coredll exports).
 *
 * Semantics notes (documented platform approximations):
 *   - exec* replaces the running image as far as the *parent* can
 *     observe: the replacement process is created, this process waits
 *     for it and exits with its exit code.  The child therefore runs
 *     while "we" still occupy our pid, which is what a waiting parent
 *     needs.  On exec failure -1 is returned (POSIX: only failure
 *     returns).
 *   - system() passes the command line directly to CreateProcess: CE
 *     has no shell, so metacharacters are not interpreted.
 *   - waitpid only sees children of this runtime that were created via
 *     exec* / system / popen (CE has no inherited handle listing).
 */

#include <stdarg.h>   /* __gnuc_va_list for mingwrt stdio.h */

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <errno.h>
#include <process.h>

#include <sys/wait.h>

/* ------------------------------------------------------------------ */
/* child table                                                          */
/* ------------------------------------------------------------------ */

#define MAX_CHILDREN 32

struct child {
  pid_t  pid;
  HANDLE handle;
  int    used;
};
static struct child children[MAX_CHILDREN];

static struct child *
child_add (HANDLE h, DWORD pid)
{
  int i;
  for (i = 0; i < MAX_CHILDREN; i++)
    if (!children[i].used)
      {
        children[i].pid = (pid_t) pid;
        children[i].handle = h;
        children[i].used = 1;
        return &children[i];
      }
  return NULL;
}

static struct child *
child_find (pid_t pid)
{
  int i;
  for (i = 0; i < MAX_CHILDREN; i++)
    if (children[i].used && (pid == -1 || children[i].pid == pid))
      return &children[i];
  return NULL;
}

static void
child_remove (struct child *c)
{
  if (c->handle != NULL)
    CloseHandle (c->handle);
  c->used = 0;
  c->handle = NULL;
}

/* ------------------------------------------------------------------ */
/* exec: spawn the replacement, wait, exit with its code                */
/* ------------------------------------------------------------------ */

static int
exec_common (const char *path, char *const argv[])
{
  STARTUPINFOW si;
  PROCESS_INFORMATION pi;
  wchar_t wpath[MAX_PATH];
  char cmdline[2048];
  wchar_t wcmdline[2048];
  size_t len = 0;
  int i;
  DWORD code = 127; /* POSIX: 127 = command not found */

  if (path == NULL || argv == NULL || argv[0] == NULL)
    {
      errno = EFAULT;
      return -1;
    }

  cmdline[0] = '\0';
  /* argv[0] first (quoted), then the remaining arguments */
  if (strlen (path) + 3 >= sizeof (cmdline))
    {
      errno = E2BIG;
      return -1;
    }
  strcat (cmdline, "\"");
  strcat (cmdline, path);
  strcat (cmdline, "\"");
  for (i = 1; argv[i] != NULL; i++)
    {
      len = strlen (cmdline);
      if (len + strlen (argv[i]) + 4 >= sizeof (cmdline))
        {
          errno = E2BIG;
          return -1;
        }
      strcat (cmdline, " \"");
      strcat (cmdline, argv[i]);
      strcat (cmdline, "\"");
    }

  MultiByteToWideChar (CP_UTF8, 0, path, -1, wpath, MAX_PATH);
  MultiByteToWideChar (CP_UTF8, 0, cmdline, -1, wcmdline, 2048);

  memset (&si, 0, sizeof (si));
  si.cb = sizeof (si);
  if (!CreateProcessW (wpath, wcmdline, NULL, NULL, FALSE, 0,
                       NULL, NULL, &si, &pi))
    {
      errno = ENOENT;
      return -1;
    }

  /* POSIX exec never returns on success: block on the replacement and
     exit with its exit code, so a waiting parent observes the right
     status.  */
  WaitForSingleObject (pi.hProcess, INFINITE);
  GetExitCodeProcess (pi.hProcess, &code);
  CloseHandle (pi.hThread);
  CloseHandle (pi.hProcess);
  ExitProcess (code);
  return -1; /* not reached */
}

int
execv (const char *path, char *const argv[])
{
  return exec_common (path, argv);
}

int
execvp (const char *file, char *const argv[])
{
  /* CE's CreateProcess already searches the executable's directory and
     \Windows; a PATH environment does not exist on CE.  */
  return exec_common (file, argv);
}

int
execl (const char *path, const char *arg0, ...)
{
  /* Re-entry through execv with the variadic list: the CE ABI is
     cdecl everywhere, so the varargs are already on the stack in the
     right shape; walking them portably requires va_list - do that.  */
  char *argv[64];
  int i = 0;
  va_list ap;
  const char *a;

  va_start (ap, arg0);
  argv[i++] = (char *) arg0;
  while (i < 63 && (a = va_arg (ap, const char *)) != NULL)
    argv[i++] = (char *) a;
  argv[i] = NULL;
  va_end (ap);

  return exec_common (path, argv);
}

int
execlp (const char *file, const char *arg0, ...)
{
  char *argv[64];
  int i = 1;
  va_list ap;
  const char *a;

  argv[0] = (char *) arg0;
  va_start (ap, arg0);
  while (i < 63 && (a = va_arg (ap, const char *)) != NULL)
    argv[i++] = (char *) a;
  argv[i] = NULL;
  va_end (ap);

  return execvp (file, argv);
}

/* ------------------------------------------------------------------ */
/* waitpid                                                              */
/* ------------------------------------------------------------------ */

pid_t
waitpid (pid_t pid, int *status, int options)
{
  struct child *c;
  DWORD code = 0;

  if (status != NULL)
    *status = 0;

  for (;;)
    {
      c = child_find (pid);
      if (c == NULL)
        {
          errno = ECHILD;
          return -1;
        }

      if (options & WNOHANG)
        {
          DWORD w = WaitForSingleObject (c->handle, 0);
          if (w != WAIT_OBJECT_0)
            return 0; /* still running */
        }
      else
        {
          DWORD w = WaitForSingleObject (c->handle, INFINITE);
          if (w != WAIT_OBJECT_0)
            {
              errno = EINTR;
              return -1;
            }
        }

      if (!GetExitCodeProcess (c->handle, &code))
        code = (DWORD) -1;
      if (status != NULL)
        *status = (int) ((code & 0xFF) << 8);
      pid = c->pid;
      child_remove (c);
      return (pid_t) pid;
    }
}

/* ------------------------------------------------------------------ */
/* system                                                               */
/* ------------------------------------------------------------------ */

int
system (const char *command)
{
  STARTUPINFOW si;
  PROCESS_INFORMATION pi;
  wchar_t wcmd[2048];
  DWORD code = 127;
  struct child *c;

  if (command == NULL)
    return 1; /* a shell is available (we always have CreateProcess) */

  MultiByteToWideChar (CP_UTF8, 0, command, -1, wcmd, 2048);
  memset (&si, 0, sizeof (si));
  si.cb = sizeof (si);
  if (!CreateProcessW (NULL, wcmd, NULL, NULL, FALSE, 0,
                       NULL, NULL, &si, &pi))
    return -1;

  /* Track it so waitpid() can also see children started via system(). */
  c = child_add (pi.hProcess, (DWORD) pi.dwProcessId);
  if (c == NULL)
    CloseHandle (pi.hProcess); /* untracked: close our copy */

  WaitForSingleObject (pi.hProcess, INFINITE);
  GetExitCodeProcess (pi.hProcess, &code);
  if (c != NULL && c->handle == pi.hProcess)
    child_remove (c);
  CloseHandle (pi.hThread);
  CloseHandle (pi.hProcess);

  /* POSIX encoding so WEXITSTATUS() works on system()'s result */
  return (int) ((code & 0xFF) << 8);
}
