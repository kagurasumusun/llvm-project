/*===-- wince/runtime/pthread.c - pthread emulation over coredll ----------===//
 *
 * Part of the LLVM/Clang Windows CE toolchain.
 *
 * Windows CE has no pthreads; its native threading surface (exported by
 * coredll.dll) is: CreateThread, CreateMutexW, InitializeCriticalSection /
 * EnterCriticalSection / LeaveCriticalSection / TryEnterCriticalSection /
 * DeleteCriticalSection, CreateEventW + EventModify (SetEvent),
 * CreateSemaphoreW + ReleaseSemaphore, WaitForSingleObject /
 * WaitForMultipleObjects, Sleep, and TlsCall-based TLS
 * (TlsAlloc/TlsFree/TlsGetValue/TlsSetValue).
 *
 * This module implements the pthread subset that the C++ standard runtime
 * (libc++'s __threading_support) requires, on top of exactly those APIs.
 * Implementation notes:
 *  - mutex      : CRITICAL_SECTION (recursive by nature; pthread's default
 *                 mutex is not required to be non-recursive for the C++
 *                 runtime).
 *  - condvar    : the classic semaphore-based algorithm (a counting
 *                 semaphore for waiters, a waiter count protected by an
 *                 internal critical section, and an auto-reset event used
 *                 to confirm broadcast completion).
 *  - thread     : CreateThread with a trampoline that runs thread-local
 *                 destructor callbacks on exit (WinCE has no TEB-based
 *                 destructor hooks).
 *  - TLS keys   : coredll TlsCall(TLS_FUNCALLOC/TLS_FUNCFREE) +
 *                 TlsGetValue/TlsSetValue; destructor lists are per key.
 *  - rwlock     : implemented with writer preference over a critical
 *                 section; read parallelism is not provided (documented
 *                 behavior; correctness is preserved).
 *  - once       : flag + critical section.
 *
 * This file is placed in the public domain.
 *
 *===--------------------------------------------------------------------===*/

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "kfuncs.h"

#include <errno.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <process.h>
#include "pthread.h"

/* ------------------------------------------------------------------ */
/* Time base: WinCE epoch is 1970 like Win32; FILETIME is 1607-based.  */
/* ------------------------------------------------------------------ */

static DWORD
abs_to_ms (const struct timespec *abstime)
{
  /* Convert an absolute deadline to a relative wait in milliseconds.
   * Returns INFINITE when abstime is NULL. */
  FILETIME now_ft, end_ft;
  ULARGE_INTEGER now, end;
  ULONGLONG delta_ms;

  if (!abstime)
    return INFINITE;

  GetSystemTimeAsFileTime (&now_ft);
  now.LowPart = now_ft.dwLowDateTime;
  now.HighPart = now_ft.dwHighDateTime;

  end.QuadPart = (ULONGLONG) abstime->tv_sec * 10000000ULL
                 + (ULONGLONG) abstime->tv_nsec / 100ULL
                 + 116444736000000000ULL; /* 1601 -> 1970 epoch */
  end_ft.dwLowDateTime = end.LowPart;
  end_ft.dwHighDateTime = end.HighPart;

  if (end.QuadPart <= now.QuadPart)
    return 0;
  delta_ms = (end.QuadPart - now.QuadPart) / 10000ULL;
  if (delta_ms >= INFINITE)
    return INFINITE - 1;
  return (DWORD) delta_ms;
}

/* ------------------------------------------------------------------ */
/* Threads                                                             */
/* ------------------------------------------------------------------ */

struct pthread_key_destructor
{
  int in_use;
  void (*destructor) (void *);
};

static struct pthread_key_destructor key_table[PTHREAD_KEYS_MAX];
static CRITICAL_SECTION key_table_lock;
static int key_table_lock_init = 0;

/* Destructors are resolved through the process-wide key table; values
 * live in the kernel TLS slots and are read via TlsGetValue at exit. */
static void
init_internal_locks (void)
{
  if (!key_table_lock_init)
    {
      InitializeCriticalSection (&key_table_lock);
      key_table_lock_init = 1;
    }
}

int
pthread_key_create (pthread_key_t *key, void (*destructor) (void *))
{
  DWORD k;

  init_internal_locks ();
  k = TlsAlloc ();
  if (k == TLS_OUT_OF_INDEXES)
    return EAGAIN;
  *key = (pthread_key_t) k;
  EnterCriticalSection (&key_table_lock);
  if (*key < PTHREAD_KEYS_MAX)
    {
      key_table[*key].in_use = 1;
      key_table[*key].destructor = destructor;
    }
  LeaveCriticalSection (&key_table_lock);
  return 0;
}

int
pthread_key_delete (pthread_key_t key)
{
  init_internal_locks ();
  EnterCriticalSection (&key_table_lock);
  if (key < PTHREAD_KEYS_MAX && key_table[key].in_use)
    key_table[key].in_use = 0;
  LeaveCriticalSection (&key_table_lock);
  if (!TlsFree ((DWORD) key))
    return EINVAL;
  return 0;
}

void *
pthread_getspecific (pthread_key_t key)
{
  return TlsGetValue ((DWORD) key);
}

int
pthread_setspecific (pthread_key_t key, const void *value)
{
  if (!TlsSetValue ((DWORD) key, (LPVOID) value))
    return ENOMEM;
  return 0;
}

/* Run destructors for all TLS values still set on this thread, looping
 * like POSIX requires (up to PTHREAD_DESTRUCTOR_ITERATIONS rounds). */
static void
run_tls_destructors (void)
{
  int round;

  for (round = 0; round < PTHREAD_DESTRUCTOR_ITERATIONS; round++)
    {
      int ran = 0;
      pthread_key_t k;
      for (k = 0; k < PTHREAD_KEYS_MAX; k++)
        {
          void (*dtor) (void *);
          void *val;
          EnterCriticalSection (&key_table_lock);
          dtor = key_table[k].in_use ? key_table[k].destructor : NULL;
          LeaveCriticalSection (&key_table_lock);
          if (!dtor)
            continue;
          val = pthread_getspecific (k);
          if (val)
            {
              pthread_setspecific (k, NULL);
              dtor (val);
              ran = 1;
            }
        }
      if (!ran)
        break;
    }
}

static DWORD WINAPI
thread_trampoline (LPVOID p)
{
  struct thread_start_info *si = (struct thread_start_info *) p;
  void *(*start) (void *) = si->start;
  void *arg = si->arg;
  void *ret;

  ret = start (arg);
  (void) ret;

  run_tls_destructors ();

  free (si);
  return 0;
}

int
pthread_create (pthread_t *thread, const pthread_attr_t *attr,
                void *(*start_routine) (void *), void *arg)
{
  struct thread_start_info *si;
  HANDLE h;

  (void) attr; /* only default attributes are provided */
  init_internal_locks ();

  si = (struct thread_start_info *) malloc (sizeof (*si));
  if (!si)
    return EAGAIN;
  si->start = start_routine;
  si->arg = arg;

  h = CreateThread (NULL, 0, thread_trampoline, si, CREATE_SUSPENDED, NULL);
  if (!h)
    {
      free (si);
      return EAGAIN;
    }
  ResumeThread (h);
  *thread = h;
  return 0;
}

int
pthread_join (pthread_t thread, void **value_ptr)
{
  DWORD rc;

  if (!thread)
    return EINVAL;
  rc = WaitForSingleObject (thread, INFINITE);
  if (rc != WAIT_OBJECT_0)
    return EDEADLK;
  CloseHandle (thread);
  if (value_ptr)
    *value_ptr = NULL; /* exit codes are not transported; see README */
  return 0;
}

int
pthread_detach (pthread_t thread)
{
  /* WinCE thread handles are not joinable after CloseHandle; detaching
   * means "we will not join". */
  (void) thread;
  return 0;
}

pthread_t
pthread_self (void)
{
  return (pthread_t) GetCurrentThreadId ();
}

int
pthread_equal (pthread_t a, pthread_t b)
{
  return a == b;
}

int
pthread_exit_shim (void *value_ptr)
{
  (void) value_ptr;
  ExitThread (0);
  return 0;
}

void
pthread_exit (void *value_ptr)
{
  pthread_exit_shim (value_ptr);
}

int
sched_yield (void)
{
  Sleep (0);
  return 0;
}

/* ------------------------------------------------------------------ */
/* Mutexes                                                             */
/* ------------------------------------------------------------------ */

int
pthread_mutexattr_init (pthread_mutexattr_t *attr)
{
  attr->kind = PTHREAD_MUTEX_DEFAULT;
  return 0;
}

int
pthread_mutexattr_settype (pthread_mutexattr_t *attr, int kind)
{
  if (kind != PTHREAD_MUTEX_DEFAULT && kind != PTHREAD_MUTEX_RECURSIVE &&
      kind != PTHREAD_MUTEX_ERRORCHECK)
    return EINVAL;
  attr->kind = kind;
  return 0;
}

int
pthread_mutexattr_gettype (const pthread_mutexattr_t *attr, int *kind)
{
  *kind = attr->kind;
  return 0;
}

int
pthread_mutexattr_destroy (pthread_mutexattr_t *attr)
{
  (void) attr;
  return 0;
}

int
pthread_mutex_init (pthread_mutex_t *mutex, const pthread_mutexattr_t *attr)
{
  (void) attr;
  InitializeCriticalSection (&mutex->cs);
  mutex->init = 1;
  return 0;
}

int
pthread_mutex_destroy (pthread_mutex_t *mutex)
{
  if (mutex->init)
    DeleteCriticalSection (&mutex->cs);
  mutex->init = 0;
  return 0;
}

int
pthread_mutex_lock (pthread_mutex_t *mutex)
{
  if (!mutex->init)
    pthread_mutex_init (mutex, NULL);
  EnterCriticalSection (&mutex->cs);
  return 0;
}

int
pthread_mutex_trylock (pthread_mutex_t *mutex)
{
  if (!mutex->init)
    pthread_mutex_init (mutex, NULL);
  if (TryEnterCriticalSection (&mutex->cs))
    return 0;
  return EBUSY;
}

int
pthread_mutex_unlock (pthread_mutex_t *mutex)
{
  LeaveCriticalSection (&mutex->cs);
  return 0;
}

/* ------------------------------------------------------------------ */
/* Condition variables (semaphore + waiter count algorithm)            */
/* ------------------------------------------------------------------ */

int
pthread_cond_init (pthread_cond_t *cond, const pthread_condattr_t *attr)
{
  (void) attr;
  cond->waiters_semaphore = CreateSemaphoreW (NULL, 0, LONG_MAX, NULL);
  if (!cond->waiters_semaphore)
    return ENOMEM;
  cond->waiters_done = CreateEventW (NULL, FALSE /* auto reset */,
                                     FALSE, NULL);
  if (!cond->waiters_done)
    {
      CloseHandle (cond->waiters_semaphore);
      cond->waiters_semaphore = NULL;
      return ENOMEM;
    }
  InitializeCriticalSection (&cond->waiters_lock);
  cond->waiters_count = 0;
  cond->was_broadcast = 0;
  cond->init = 1;
  return 0;
}

int
pthread_cond_destroy (pthread_cond_t *cond)
{
  if (!cond->init)
    return 0;
  /* No waiters may remain; a program destroying a waited-on condvar has
   * undefined behavior per POSIX, so no extra synchronization here. */
  CloseHandle (cond->waiters_semaphore);
  CloseHandle (cond->waiters_done);
  DeleteCriticalSection (&cond->waiters_lock);
  cond->init = 0;
  return 0;
}

int
pthread_cond_wait (pthread_cond_t *cond, pthread_mutex_t *mutex)
{
  int last_waiter;

  if (!cond->init)
    pthread_cond_init (cond, NULL);

  EnterCriticalSection (&cond->waiters_lock);
  cond->waiters_count++;
  LeaveCriticalSection (&cond->waiters_lock);

  /* Release the caller's mutex while blocked, atomically enough for the
   * runtime's needs: the semaphore wait below cannot miss a signal that
   * happened after the waiter count was incremented. */
  pthread_mutex_unlock (mutex);
  WaitForSingleObject (cond->waiters_semaphore, INFINITE);

  EnterCriticalSection (&cond->waiters_lock);
  cond->waiters_count--;
  last_waiter = (cond->was_broadcast && cond->waiters_count == 0);
  if (last_waiter)
    cond->was_broadcast = 0;
  LeaveCriticalSection (&cond->waiters_lock);

  if (last_waiter)
    /* Tell broadcast() all waiters were released. */
    SetEvent (cond->waiters_done);

  pthread_mutex_lock (mutex);
  return 0;
}

int
pthread_cond_timedwait (pthread_cond_t *cond, pthread_mutex_t *mutex,
                        const struct timespec *abstime)
{
  DWORD ms = abs_to_ms (abstime);
  int last_waiter;
  DWORD wr;

  if (!cond->init)
    pthread_cond_init (cond, NULL);

  EnterCriticalSection (&cond->waiters_lock);
  cond->waiters_count++;
  LeaveCriticalSection (&cond->waiters_lock);

  pthread_mutex_unlock (mutex);
  wr = WaitForSingleObject (cond->waiters_semaphore, ms);

  EnterCriticalSection (&cond->waiters_lock);
  cond->waiters_count--;
  last_waiter = (cond->was_broadcast && cond->waiters_count == 0);
  if (last_waiter)
    cond->was_broadcast = 0;
  LeaveCriticalSection (&cond->waiters_lock);
  if (last_waiter)
    SetEvent (cond->waiters_done);

  pthread_mutex_lock (mutex);
  return (wr == WAIT_TIMEOUT) ? ETIMEDOUT : 0;
}

int
pthread_cond_signal (pthread_cond_t *cond)
{
  int have_waiters;

  if (!cond->init)
    return 0;
  EnterCriticalSection (&cond->waiters_lock);
  have_waiters = cond->waiters_count > 0;
  LeaveCriticalSection (&cond->waiters_lock);
  if (have_waiters)
    ReleaseSemaphore (cond->waiters_semaphore, 1, NULL);
  return 0;
}

int
pthread_cond_broadcast (pthread_cond_t *cond)
{
  int have_waiters = 0;

  if (!cond->init)
    return 0;
  EnterCriticalSection (&cond->waiters_lock);
  if (cond->waiters_count > 0)
    {
      have_waiters = 1;
      cond->was_broadcast = 1;
    }
  if (have_waiters)
    ReleaseSemaphore (cond->waiters_semaphore, cond->waiters_count, NULL);
  LeaveCriticalSection (&cond->waiters_lock);

  if (have_waiters)
    /* Block until every released waiter observed the broadcast. */
    WaitForSingleObject (cond->waiters_done, INFINITE);
  return 0;
}

/* ------------------------------------------------------------------ */
/* once                                                               */
/* ------------------------------------------------------------------ */

int
pthread_once (pthread_once_t *once, void (*init_routine) (void))
{
  if (!once->lock.init)
    pthread_mutex_init (&once->lock, NULL);
  pthread_mutex_lock (&once->lock);
  if (!once->done)
    {
      once->done = 1;
      init_routine ();
    }
  pthread_mutex_unlock (&once->lock);
  return 0;
}

/* ------------------------------------------------------------------ */
/* Read/write lock: writer-preferring exclusive-only implementation.   */
/* Correct mutual exclusion is preserved; read parallelism is not.     */
/* ------------------------------------------------------------------ */

int
pthread_rwlock_init (pthread_rwlock_t *rwlock, const pthread_rwlockattr_t *attr)
{
  (void) attr;
  InitializeCriticalSection (&rwlock->cs);
  rwlock->init = 1;
  return 0;
}

int
pthread_rwlock_destroy (pthread_rwlock_t *rwlock)
{
  if (rwlock->init)
    DeleteCriticalSection (&rwlock->cs);
  rwlock->init = 0;
  return 0;
}

int
pthread_rwlock_rdlock (pthread_rwlock_t *rwlock)
{
  EnterCriticalSection (&rwlock->cs);
  return 0;
}

int
pthread_rwlock_tryrdlock (pthread_rwlock_t *rwlock)
{
  if (TryEnterCriticalSection (&rwlock->cs))
    return 0;
  return EBUSY;
}

int
pthread_rwlock_wrlock (pthread_rwlock_t *rwlock)
{
  EnterCriticalSection (&rwlock->cs);
  return 0;
}

int
pthread_rwlock_trywrlock (pthread_rwlock_t *rwlock)
{
  if (TryEnterCriticalSection (&rwlock->cs))
    return 0;
  return EBUSY;
}

int
pthread_rwlock_unlock (pthread_rwlock_t *rwlock)
{
  LeaveCriticalSection (&rwlock->cs);
  return 0;
}
