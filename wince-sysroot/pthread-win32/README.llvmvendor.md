# pthread-win32 / pthreads4w (vendored)

Vendored in-tree copy of the pthreads4w (pthreads-win32) POSIX threads library,
providing `pthread.h` / `sched.h` / `semaphore.h` and the static
`libpthread.a` for the Windows CE target (the Win32 thread API needs
SRWLOCK/INIT_ONCE, which WinCE does not have).

* Upstream: https://github.com/GerHobbelt/pthread-win32 (branch `master`),
  the actively maintained combined successor fork of pthreads-win32 /
  pthreads4w (upstream pthreads4w has been dormant for years); it retains
  the historical Windows CE support hooks (`NEED_SEM`, `NEED_CREATETHREAD`,
  `ptw32_getprocessors() == 1`, ...).
* Vendored at upstream commit `06e7608` plus three local WinCE 6.0 build
  fixes (compiler/build-compat only; no API or semantic changes off-WinCE):
  1. thread entry/exit take the `_beginthreadex`/`_endthreadex` paths,
     which `implement.h` maps to `CreateThread`/`ExitThread` under
     `NEED_CREATETHREAD` — the only thread primitives COREDLL provides
     (the guards keyed on `__MINGW32__`/`__MSVCRT__` only, routing
     CeGCC-style builds to the CRTDLL `_beginthread()` that does not
     exist on WinCE): `create.c`, `pthread_exit.c`,
     `ptw32_threadDestroy.c`, `ptw32_threadStart.c`, `ptw32_throw.c`,
     `implement.h`;
  2. the GNU interlocked block in `implement.h` is restricted to x86 —
     it emits x86 inline assembly for any `__GNUC__` target; ARM must use
     the COREDLL `Interlocked*` exports;
  3. `_ptw32.h` accepts clang at the `#if ! defined __declspec` guard
     (clang implements `__declspec` as a keyword, not a macro — same
     root cause as the mingwrt `_mingw.h` probe).
* Built as-is by `utils/wince/build-wince-sysroot.sh`
  (`-DPTW32_STATIC_LIB -D__CLEANUP_C -D__PTHREAD_JUMBO_BUILD__` — the
  setjmp/longjmp C cleanup variant, since structured-exception unwinding
  does not exist on WinCE).
* Local pruning vs upstream: none beyond VCS metadata.
* License: Apache-2.0 (pthreads4w v3) / LGPL for the v2 lineage; see the
  upstream license headers in each file and `manual/` for details.
