/* Device test 5: pthread shim (mutex/condvar/once/TLS) over coredll.
 * Build: clang --target=arm-pc-wince pthread-test.c -o pthread-test.exe
 * Pass:  prints "pthread ok: N" (N == 4) and exits 0.
 */
#include <pthread.h>
#include <stdio.h>

static pthread_mutex_t mtx = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
static pthread_once_t once = PTHREAD_ONCE_INIT;
static pthread_key_t key;
static int counter = 0;
static int once_ran = 0;

static void
once_fn (void)
{
  once_ran = 1;
}

static void *
worker (void *arg)
{
  pthread_once (&once, once_fn);
  pthread_setspecific (key, (const void *) (size_t) 42);
  pthread_mutex_lock (&mtx);
  counter++;
  pthread_cond_signal (&cond);
  pthread_mutex_unlock (&mtx);
  return (void *) (size_t) pthread_getspecific (key);
}

int
main (void)
{
  pthread_t th[4];
  void *vals[4];
  int i, ok = 1;

  pthread_key_create (&key, NULL);
  for (i = 0; i < 4; ++i)
    ok &= pthread_create (&th[i], NULL, worker, NULL) == 0;
  pthread_mutex_lock (&mtx);
  while (counter < 4)
    pthread_cond_wait (&cond, &mtx);
  pthread_mutex_unlock (&mtx);
  for (i = 0; i < 4; ++i)
    {
      pthread_join (th[i], &vals[i]);
      ok &= (size_t) vals[i] == 42;
    }
  ok &= once_ran == 1;
  ok &= counter == 4;
  pthread_key_delete (key);
  printf ("pthread ok: %d %d\n", ok ? 1 : 0, counter);
  return ok ? 0 : 1;
}
