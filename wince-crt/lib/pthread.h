/*===-- wince/runtime/pthread.h - pthread interface over coredll ----------===//
 *
 * Part of the LLVM/Clang Windows CE toolchain.
 *
 * Provides the pthread subset required by the C++ standard runtime
 * (libc++ __threading_support, libunwind) on top of the Windows CE
 * coredll.dll threading primitives.  See pthread.c for the exact
 * implementation mapping and documented behavioral limitations
 * (rwlocks are exclusive-only; pthread_join does not transport the
 * thread's return value; PTHREAD_KEYS_MAX = 64 kernel TLS slots).
 *
 * This file is placed in the public domain.
 *
 *===--------------------------------------------------------------------===*/

#ifndef _WCE_PTHREAD_H
#define _WCE_PTHREAD_H

#include <process.h>
#include <time.h>
#include <errno.h>

#ifndef _TIMESPEC_DEFINED
struct timespec
{
  time_t tv_sec;
  long tv_nsec;
};
#define _TIMESPEC_DEFINED 1
#endif

#ifdef __cplusplus
extern "C" {
#endif

#define PTHREAD_KEYS_MAX 64
#define PTHREAD_DESTRUCTOR_ITERATIONS 4
#define PTHREAD_ONCE_INIT { 0, { 0 } }
#define PTHREAD_MUTEX_INITIALIZER { 0 }
#define PTHREAD_COND_INITIALIZER { 0 }

typedef HANDLE pthread_t;
typedef unsigned long pthread_key_t;

typedef struct
{
  int init;
  CRITICAL_SECTION cs;
} pthread_mutex_t;

typedef struct
{
  unsigned ignore;
} pthread_mutexattr_t;

typedef struct
{
  int init;
  HANDLE waiters_semaphore;
  HANDLE waiters_done;
  CRITICAL_SECTION waiters_lock;
  int waiters_count;
  int was_broadcast;
} pthread_cond_t;

typedef struct
{
  unsigned ignore;
} pthread_condattr_t;

typedef struct
{
  int done;
  pthread_mutex_t lock;
} pthread_once_t;

typedef struct
{
  int init;
  CRITICAL_SECTION cs;
} pthread_rwlock_t;

typedef struct
{
  unsigned ignore;
} pthread_rwlockattr_t;

typedef struct
{
  unsigned ignore;
} pthread_attr_t;

/* Mutex kinds.  The WinCE implementation is based on CRITICAL_SECTION,
 * which is recursive by nature; all kinds therefore share the same
 * (correct) mutual exclusion and recursion semantics. */
#define PTHREAD_MUTEX_DEFAULT 0
#define PTHREAD_MUTEX_RECURSIVE 1
#define PTHREAD_MUTEX_ERRORCHECK 2

typedef struct
{
  unsigned kind;
} pthread_mutexattr_t;

int pthread_mutexattr_init (pthread_mutexattr_t *);
int pthread_mutexattr_settype (pthread_mutexattr_t *, int);
int pthread_mutexattr_gettype (const pthread_mutexattr_t *, int *);
int pthread_mutexattr_destroy (pthread_mutexattr_t *);

/* Error values (matching the common errno conventions); only defined
 * when the C library headers do not provide them. */
#ifndef ETIMEDOUT
#define ETIMEDOUT 138
#endif
#ifndef EDEADLK
#define EDEADLK 36
#endif
#ifndef EBUSY
#define EBUSY 16
#endif

int pthread_create (pthread_t *, const pthread_attr_t *,
                    void *(*)(void *), void *);
int pthread_join (pthread_t, void **);
int pthread_detach (pthread_t);
pthread_t pthread_self (void);
int pthread_equal (pthread_t, pthread_t);
void pthread_exit (void *);

int pthread_mutex_init (pthread_mutex_t *, const pthread_mutexattr_t *);
int pthread_mutex_destroy (pthread_mutex_t *);
int pthread_mutex_lock (pthread_mutex_t *);
int pthread_mutex_trylock (pthread_mutex_t *);
int pthread_mutex_unlock (pthread_mutex_t *);

int pthread_cond_init (pthread_cond_t *, const pthread_condattr_t *);
int pthread_cond_destroy (pthread_cond_t *);
int pthread_cond_wait (pthread_cond_t *, pthread_mutex_t *);
int pthread_cond_timedwait (pthread_cond_t *, pthread_mutex_t *,
                            const struct timespec *);
int pthread_cond_signal (pthread_cond_t *);
int pthread_cond_broadcast (pthread_cond_t *);

int pthread_key_create (pthread_key_t *, void (*)(void *));
int pthread_key_delete (pthread_key_t);
void *pthread_getspecific (pthread_key_t);
int pthread_setspecific (pthread_key_t, const void *);

int pthread_once (pthread_once_t *, void (*)(void));

int pthread_rwlock_init (pthread_rwlock_t *, const pthread_rwlockattr_t *);
int pthread_rwlock_destroy (pthread_rwlock_t *);
int pthread_rwlock_rdlock (pthread_rwlock_t *);
int pthread_rwlock_tryrdlock (pthread_rwlock_t *);
int pthread_rwlock_wrlock (pthread_rwlock_t *);
int pthread_rwlock_trywrlock (pthread_rwlock_t *);
int pthread_rwlock_unlock (pthread_rwlock_t *);

int sched_yield (void);

#ifdef __cplusplus
}
#endif

#endif /* _WCE_PTHREAD_H */
