/*
 * libgmon - user-mode sampling profiler for Windows CE.
 *
 * WinCE has no POSIX timers and no signal delivery, so the classic
 * profil()/setitimer() sampling cannot exist.  What COREDLL does export
 * (all verified against coredll.def) is enough for a sampling thread:
 *
 *   CreateThread / SetThreadPriority / DuplicateHandle
 *   SuspendThread / ResumeThread / GetThreadContext
 *   QueryPerformanceCounter / Sleep
 *   GetModuleFileNameW / CreateFileW / WriteFile / CloseHandle
 *
 * Output is a BSD/gprof "gmon.out" containing a GMON_TAG_TIME_HIST record
 * over the module image window, so host `gprof` reads the flat profile.
 *
 * `-pg` support: clang instruments every function entry with a call to
 * `mcount` (TargetInfo::MCountName == "mcount" for this target).  The
 * plain-BSD mcount contract needs the caller's frame layout, which clang
 * does not guarantee here, so mcount is a no-op; the call-graph arc table
 * of gmon.out is left empty and the flat profile carries the data.
 *
 * Everything is user-mode and additive: no WinCE platform behavior is
 * changed, and programs not linked with -pg never see any of this.
 */

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdlib.h>
#include <string.h>

/* ------------------------------------------------------------------ */
/* gmon.out (BSD 4.4 / GNU gprof) format                              */
/* ------------------------------------------------------------------ */

#define GMON_MAGIC       "gmon"
#define GMON_VERSION     1
#define GMON_TAG_TIME_HIST 0
#define GMON_TAG_CGM       1

#pragma pack(push, 1)
struct gmon_hdr {
  char   cookie[4];      /* "gmon"                       */
  UINT32 version;        /* 1                            */
  char   spare[12];      /* zero                         */
};

struct gmon_hist_hdr {
  UINT32 low_pc;         /* histogram window start       */
  UINT32 high_pc;        /* histogram window end         */
  UINT32 hist_size;      /* number of u16 buckets        */
  UINT32 prof_rate;      /* samples per second           */
};
#pragma pack(pop)

/* ------------------------------------------------------------------ */
/* Configuration                                                       */
/* ------------------------------------------------------------------ */

#ifndef GMON_WINDOW_BYTES   /* profiled span of the module image   */
#define GMON_WINDOW_BYTES (2u * 1024u * 1024u)
#endif
#ifndef GMON_BUCKET_BYTES   /* bytes per histogram bucket          */
#define GMON_BUCKET_BYTES 16u
#endif
#ifndef GMON_SAMPLE_MS      /* sampling interval                   */
#define GMON_SAMPLE_MS 10
#endif

#define GMON_HIST_SIZE (GMON_WINDOW_BYTES / GMON_BUCKET_BYTES)

/* ------------------------------------------------------------------ */
/* State                                                               */
/* ------------------------------------------------------------------ */

static volatile LONG g_running;      /* sampler keeps looping while 1 */
static HANDLE g_hMain;               /* real handle to the main thread */
static HANDLE g_hSampler;            /* sampler thread handle          */
static HANDLE g_hDone;               /* signalled by the sampler exit  */
static UINT16 *g_counts;             /* GMON_HIST_SIZE buckets         */
static UINT32 g_samples;             /* total samples taken            */
static UINT32 g_rate = 100;          /* nominal samples/second         */

extern IMAGE_DOS_HEADER __ImageBase; /* linker-provided                */

/* ------------------------------------------------------------------ */
/* mcount: clang -pg calls this at every function entry.  See the file  */
/* comment: the sampling thread carries the data, so this is a no-op    */
/* (it must exist so -pg links).                                        */
/* ------------------------------------------------------------------ */

void mcount(void) { }

/* ------------------------------------------------------------------ */
/* Sampler                                                              */
/* ------------------------------------------------------------------ */

static DWORD WINAPI
gmon_sampler (LPVOID arg)
{
  __attribute__((aligned(16))) CONTEXT ctx; /* CONTEXT needs 16B (x86 FPU) */

  (void) arg;

  memset (&ctx, 0, sizeof (ctx));
  ctx.ContextFlags = CONTEXT_CONTROL;

  while (InterlockedCompareExchange (&g_running, 1, 1) == 1)
    {
      if (g_hMain != NULL &&
          SuspendThread (g_hMain) != (DWORD)-1)
        {
          ctx.ContextFlags = CONTEXT_CONTROL;
          if (GetThreadContext (g_hMain, &ctx))
            {
              UINT_PTR pc = (UINT_PTR)ctx.Pc;
              UINT_PTR base = (UINT_PTR)&__ImageBase;
              UINT_PTR off = pc - base;
              if (off < GMON_WINDOW_BYTES)
                {
                  UINT32 bucket = (UINT32)(off / GMON_BUCKET_BYTES);
                  if (bucket < GMON_HIST_SIZE && g_counts != NULL)
                    g_counts[bucket]++;
                  g_samples++;
                }
            }
          ResumeThread (g_hMain);
        }

      Sleep (GMON_SAMPLE_MS);
    }

  SetEvent (g_hDone);
  return 0;
}

/* ------------------------------------------------------------------ */
/* gmon.out writer                                                      */
/* ------------------------------------------------------------------ */

static void
gmon_write (void)
{
  wchar_t path[MAX_PATH];
  wchar_t *slash;
  DWORD n, written;
  HANDLE hf;

  struct gmon_hdr hdr;
  struct gmon_hist_hdr hh;

  if (!GetModuleFileNameW (NULL, path, MAX_PATH))
    return;
  slash = wcsrchr (path, L'\\');
  if (slash != NULL)
    *slash = L'\0';
  if (wcslen (path) + 9 >= MAX_PATH)
    return;
  wcscat (path, L"\\gmon.out");

  hf = CreateFileW (path, GENERIC_WRITE, 0, NULL,
                    CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
  if (hf == INVALID_HANDLE_VALUE)
    return;

  memset (&hdr, 0, sizeof (hdr));
  memcpy (hdr.cookie, GMON_MAGIC, 4);
  hdr.version = GMON_VERSION;
  WriteFile (hf, &hdr, sizeof (hdr), &written, NULL);

  memset (&hh, 0, sizeof (hh));
  hh.low_pc    = (UINT32)(UINT_PTR)&__ImageBase;
  hh.high_pc   = hh.low_pc + GMON_WINDOW_BYTES;
  hh.hist_size = GMON_HIST_SIZE;
  hh.prof_rate = g_rate;

  {
    unsigned char tag = GMON_TAG_TIME_HIST;
    WriteFile (hf, &tag, 1, &written, NULL);
  }
  WriteFile (hf, &hh, sizeof (hh), &written, NULL);
  if (g_counts != NULL)
    WriteFile (hf, g_counts, GMON_HIST_SIZE * sizeof (UINT16),
               &written, NULL);

  CloseHandle (hf);
}

/* ------------------------------------------------------------------ */
/* Lifecycle (called from gcrt3.o)                                      */
/* ------------------------------------------------------------------ */

void
__gmon_start (void)
{
  g_counts = (UINT16 *) HeapAlloc (GetProcessHeap (), HEAP_ZERO_MEMORY,
                                   GMON_HIST_SIZE * sizeof (UINT16));
  if (g_counts == NULL)
    return;

  g_hDone = CreateEventW (NULL, TRUE, FALSE, NULL);
  if (g_hDone == NULL)
    return;

  /* The sampler must suspend the thread that called __gmon_start, so
     turn the pseudo-handle into a real handle first.  */
  if (!DuplicateHandle (GetCurrentProcess (), GetCurrentThread (),
                        GetCurrentProcess (), &g_hMain,
                        0, FALSE, DUPLICATE_SAME_ACCESS))
    {
      g_hMain = NULL;
      return;
    }

  g_running = 1;
  g_hSampler = CreateThread (NULL, 64 * 1024, gmon_sampler, NULL,
                             CREATE_SUSPENDED, NULL);
  if (g_hSampler == NULL)
    {
      CloseHandle (g_hMain);
      g_hMain = NULL;
      return;
    }
  SetThreadPriority (g_hSampler, THREAD_PRIORITY_BELOW_NORMAL);
  ResumeThread (g_hSampler);
}

void
__gmon_stop (void)
{
  if (g_hSampler == NULL)
    return;

  InterlockedExchange (&g_running, 0);
  if (g_hMain != NULL)
    ResumeThread (g_hMain);       /* in case it died suspended */
  WaitForSingleObject (g_hDone, 2000);

  gmon_write ();

  if (g_counts != NULL)
    {
      HeapFree (GetProcessHeap (), 0, g_counts);
      g_counts = NULL;
    }
  CloseHandle (g_hSampler);
  CloseHandle (g_hMain);
  CloseHandle (g_hDone);
  g_hSampler = g_hMain = g_hDone = NULL;
}
