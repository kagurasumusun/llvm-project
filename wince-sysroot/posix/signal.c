#include <stdarg.h>   /* __gnuc_va_list for mingwrt stdio.h */
/*
 * POSIX signal() family for Windows CE.
 *
 * CE has no kernel signal delivery and no POSIX timers, and the CeGCC
 * coredll.def does not export the vectored-exception API, so:
 *
 *   - signal()/raise() implement the registry + cooperative delivery:
 *     handlers run on the thread that calls raise().  mingwrt's abort()
 *     raises SIGABRT, which reaches user handlers through here.
 *   - SIGALRM/alarm(): a dedicated timer thread delivers the signal
 *     (it runs on the timer thread, like pthreads4w's timer paths).
 *   - Faults (SIGSEGV/SIGFPE/SIGILL from real hardware faults) cannot
 *     be delivered: COREDLL does not export the vectored-exception API
 *     in the CeGCC def set.  raise(SIG*) with an explicit raise() works.
 */

#include <stdarg.h>   /* __gnuc_va_list for mingwrt stdio.h */

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdlib.h>
#include <errno.h>
#include <signal.h>

/* mingwrt's signal.h defines the SIG* values, sig_atomic_t and
   __p_sig_fn_t.  */

/* mingwrt's CE signal.h defines the six MS signals but not SIGALRM;
   POSIX value 14 is unused by them.  */
#ifndef SIGALRM
#define SIGALRM 14
#endif

#define NSIG_MAX 32

static __p_sig_fn_t handlers[NSIG_MAX];

static void (__cdecl *g_alarm_fn)(int);
static volatile LONG g_alarm_pending_sec;

static void (__cdecl *g_segv_fn)(int);
static void (__cdecl *g_fpe_fn)(int);
static void (__cdecl *g_ill_fn)(int);

__p_sig_fn_t
signal (int sig, __p_sig_fn_t handler)
{
  __p_sig_fn_t old;

  if (sig <= 0 || sig >= NSIG_MAX)
    {
      errno = EINVAL;
      return SIG_ERR;
    }

  old = handlers[sig];

  switch (sig)
    {
    case SIGABRT:
    case SIGINT:
    case SIGTERM:
    case SIGALRM:
      handlers[sig] = handler;
      break;
    case SIGSEGV:
      /* fault-delivery unavailable (no VEH export); a raise(SIGSEGV)
         still reaches the handler.  */
      g_segv_fn = handler;
      handlers[sig] = handler;
      break;
    case SIGFPE:
      g_fpe_fn = handler;
      handlers[sig] = handler;
      break;
    case SIGILL:
      g_ill_fn = handler;
      handlers[sig] = handler;
      break;
    default:
      errno = EINVAL;
      return SIG_ERR;
    }
  return old;
}

int
raise (int sig)
{
  __p_sig_fn_t h;

  if (sig <= 0 || sig >= NSIG_MAX)
    {
      errno = EINVAL;
      return -1;
    }
  h = handlers[sig];
  if (h == SIG_IGN || h == NULL)
    {
      if (sig == SIGABRT)
        ExitProcess (3); /* no handler: MSVC-style abort exit */
      return 0;
    }
  h (sig);
  return 0;
}

/* ------------------------------------------------------------------ */
/* alarm / SIGALRM (timer thread delivery)                              */
/* ------------------------------------------------------------------ */

static HANDLE g_alarm_thread;

static DWORD WINAPI
alarm_thread (LPVOID arg)
{
  DWORD secs = (DWORD) (UINT_PTR) arg;
  void (__cdecl *fn)(int);

  Sleep (secs * 1000);
  fn = g_alarm_fn;
  if (fn != NULL)
    fn (SIGALRM);
  return 0;
}

static void
alarm_thread_cleanup (HANDLE h)
{
  if (h != NULL)
    {
      TerminateThread (h, 0);
      CloseHandle (h);
    }
  g_alarm_thread = NULL;
}

unsigned int
alarm (unsigned int seconds)
{
  HANDLE h;

  alarm_thread_cleanup (g_alarm_thread);

  if (seconds == 0)
    return 0;

  g_alarm_fn = handlers[SIGALRM];
  h = CreateThread (NULL, 32 * 1024, alarm_thread,
                    (LPVOID) (UINT_PTR) seconds, 0, NULL);
  if (h == NULL)
    return 0;
  SetThreadPriority (h, THREAD_PRIORITY_BELOW_NORMAL);
  g_alarm_thread = h;
  return seconds; /* remaining time of the previous alarm is not kept */
}
