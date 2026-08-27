# wince-crt ↔ mingwrt (CeGCC) correspondence

Purpose: establish, file by file, what `wince-crt` takes **as-is** from the
`third-party/mingwrt` submodule, what it **adapts** (and why the mingwrt
original cannot be used unchanged), and what has **no mingwrt counterpart**
(LLVM/WinCE-specific).  This is the audit that decides the minimal shape
of the port layer: *reuse everything replaceable, keep only the
non-replaceable delta.*

Evidence base:

* `third-party/mingwrt` @ `7c35691` (CeGCC mingw-runtime, mingw32ce-capable);
* the mingw32ce build manifest in `mingwrt/Makefile.in` (what CeGCC itself
  compiled for WinCE):

      CRT0S(ce)    = CRT_noglob.o crtmt.o crtst.o crt3.o dllcrt3.o
      MINGW_OBJS(ce)= CRTglob.o CRTfmode.o CRTinit.o dllmain.o gccmain.o
                      coredll_stubs.o crtst.o mthr_stub.o pseudo-reloc.o
                      pseudo-reloc-list.o cpu_features.o
                      winmain_ce.o abort.o atexit.o assert.o
      MOLD_OBJS    = isascii iscsym iscsymf toascii strcasecmp strncasecmp wcscmpi
      LIBS(ce)     = libcoredll.a libcoredll6.a libmingw32.a libceoldname.a libm.a

* the exact pthread surface libc++ requires, extracted from
  `libcxx/include/__thread/support/pthread.h` (the only thread header used
  in our `LIBCXX_HAS_PTHREAD_API` configuration) and `libcxxabi/src/`.

## 1. Taken unchanged from the mingwrt submodule (compiled by wince-crt)

| mingwrt file | role | note |
|---|---|---|
| `atexit.c` | private atexit table (coredll has none), `_cexit`, `__dll_exit` | also provides `exit` semantics used by our crt0 |
| `abort.c` | `abort()` (TerminateProcess; coredll has no abort) | |
| `assert.c` | `assert()` handler | |
| `coredll_stubs.c` | `setlocale` (C-only), `_errno`, `_get_osfhandle` | |
| `isascii.c`, `strcasecmp.c`, `strncasecmp.c` | moldname-family CRT bits | |
| `mingwex/mbrtowc.c`, `mbsinit.c`, `wcrtomb.c`, `btowc.c`, `wctob.c` | DBCS-aware multibyte over WinCE ACP | `mbrtowc.c` also provides `mbsrtowcs`/`mbrlen` |
| `mingwex/wince/*` ( ctype ×11, `_tolower/_toupper`, time ×7 incl. `strftime`/`wcsftime`, `bsearch`, `mb_cur_max`) | the classic WinCE CRT gap fillers (coredll exports none of the narrow `is*`, no ANSI time) | `mb_cur_max.c` is authoritative: real `GetCPInfo` answer; the old hardcoded `crt_extra.c` in the port layer was deleted in favor of it |
| `mingwex/stdio/*` (pformat + family ×8) | CeGCC C99 printf engine | coredll has only `_vsnprintf` (desktop semantics) |
| `mingwex/` portable subset (`strtoimax.c`, `gettimeofday.c`, tsearch family, wmem* …) | portable C99 supplement | |
| `include/` (whole tree) | CRT headers for the `__COREDLL__` environment | copied verbatim into the sysroot |
| `coredll.def`, `coredll6.def`, `moldname.def.in` | import-library definitions | consumed by **llvm-dlltool** (not binutils dlltool) |

Nothing above is modified; the submodule stays a submodule.

## 2. Adapted (mingwrt original exists but cannot be used unchanged)

| wince-crt file | mingwrt original | what differs and why |
|---|---|---|
| `lib/crt0.c` | `crt3.c` | `crt3.o` calls `__gccmain()` (libgcc walking `__CTOR_LIST__`, i.e. GCC `.ctors`) and `_pei386_runtime_relocator()` (GNU pseudo-reloc).  Neither exists in an LLVM link: clang emits COFF `.CRT$XCU` constructor sections and LLD resolves dllimport references directly.  Our crt0 runs the `.CRT$XCU` walk (crtxcu.c), keeps `__atexit_init`/`_cexit` (atexit.c, reused), and adds a `mainCRTStartup` entry. |
| `lib/dllcrt0.c` | `dllcrt1.c` (→ CeGCC `dllcrt3.o`) | same constructor-mechanism substitution for `DllMainCRTStartup`. |
| `lib/crtxcu.c` | (role of `gccmain.c`) | `gccmain.c` is bound to `__CTOR_LIST__`/`__DTOR_LIST__`; the COFF equivalent `.CRT$XCU`/`.CRT$XTU` table walker has no mingwrt source.  This is the ".ctors → .CRT$XCU bridge" — precisely the piece with standalone value. |
| `lib/winmain.c` | `main.c` + `winmain_ce.c` + `dllmain.c` | same linker-driven adapter selection as the CeGCC manifest (`winmain_ce.o` for CE); ours additionally dispatches the WinCE loader's wide-argument entry to `main()` and provides the default `DllMain`.  mingwrt's `main.c` is the desktop `GetCommandLineA` flavor and is **not** in the CE manifest. |
| `lib/init_args.c` | `__mainArgs` in `winmain_ce.c` | mingwrt's parser has no backslash/quote rules; ours implements the `CommandLineToArgvW` rules and the `__p___argc/__p___argv/__p__environ/__p___fmode` accessors the mingw-runtime headers reference.  Substituting `winmain_ce.o` as-is would lose both. |

## 3. No mingwrt counterpart (must live in the port layer)

| wince-crt file | why nothing in mingwrt covers it |
|---|---|
| `lib/pthread.c` + `lib/pthread.h` | mingwrt's only thread code is `mthr.c`/`mthr_init.c` (the *mingwthr* DLL: TLS-key destructors for **GCC's** EH) and `mthr_stub.c` (its no-op) — no pthread API.  The pthread surface actually required by libc++ (`__thread/support/pthread.h`) is exactly: `pthread_create/join/detach/self/equal`, `pthread_mutex_{init,destroy,lock,trylock,unlock}` (+`mutexattr_{init,destroy,settype}`), `pthread_cond_{init,destroy,wait,timedwait,signal,broadcast}`, `pthread_once`, `pthread_key_create/delete`+`get/setspecific`, `sched_yield`.  libc++abi requires none (Apple-only reference).  libc++ does **not** use `pthread_rwlock` (0 references) — the shim's rwlock is user-API completeness only and is a documented candidate for removal if minimalism is preferred.  Alternative configurations were evaluated: `LIBCXX_HAS_WIN32_THREAD_API` needs SRWLOCK/CONDITION_VARIABLE/INIT_ONCE (do not exist on CE); `LIBCXX_ENABLE_THREADS=OFF` deletes `std::thread`/`std::mutex` semantics (functional downgrade); `LIBCXX_HAS_EXTERNAL_THREAD_API` just moves the same shim to the user's TU.  The shim over coredll (CS/event/semaphore/TlsCall) is therefore the minimal real implementation. |
| `lib/c99_strto.c` | mingwrt's mingwex contains only `strtoimax.c`/`strtoumax.c`; `strtof`/`strtold`/`strtoll`/`strtoull` are **declared in mingwrt's stdlib.h but implemented nowhere in the submodule** (desktop mingw got them from msvcrt).  Four thin wrappers over strtod/strtoimax/strtoumax. |
| `CMakeLists.txt`, `scripts/build-runtimes.sh` | build machinery: llvm-dlltool import libraries, sysroot layout at `<prefix>/wince-sysroot`, compiler-rt/libunwind/libc++abi/libc++ staging under driver-expected names.  mingwrt's configure/Makefile targets GCC + GNU ld exclusively. |
| `test/` | e2e + device verification harness. |

## 4. CeGCC manifest members intentionally not built

| member | reason |
|---|---|
| `gccmain.o` | replaced by `crtxcu.c` (see table 2) |
| `pseudo-reloc.o`, `pseudo-reloc-list.o` | GNU ld dllimport-data workaround; LLD/clang handles dllimport natively |
| `mthr_stub.o` | `__mingwthr_key_dtor` is GCC-EH machinery; the LLVM stack (EHABI + libunwind + the pthread shim's TLS destructors) does not reference it |
| `cpu_features.o` | x86 libgcc feature dispatch; irrelevant on ARM |
| `CRTglob.o`, `CRT_noglob.o`, `CRTfmode.o`, `crtmt.o`, `crtst.o` | opt-in globbing/fmode/thread objects with no consumer in the CE driver defaults |
| `toascii.o`, `iscsym.o`, `iscsymf.o`, `wcscmpi.o` | moldname old-alias extras; `ceoldname.lib` covers the aliases via import-library generation |

## 5. Conclusion

Counting the delta: of the seven C sources in `wince-crt/lib/`, five are
documented adaptations of public-domain mingwrt originals (two of which —
`crt0.c`, `dllcrt0.c` — exist only because the constructor mechanism and
pseudo-reloc differ under LLVM), and two (`pthread.c`, `c99_strto.c`) have
no mingwrt source at all.  Everything else — CRT glue, ctype, time,
printf, multibyte, headers, import definitions — already comes unchanged
from the submodule.  The port layer is therefore already close to the
minimal "existing assets + LLVM-only delta" shape; the audit above is the
contract that keeps it that way (new port-layer code must land in one of
the three tables or not at all).
