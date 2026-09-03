/// Windows CE driver tests (GCC-style clang driver).
/// REQUIRES: arm-registered-target, x86-registered-target

/// The triple is recognized and canonicalized.
// RUN: %clang -target arm-pc-wince -print-target-triple 2>&1 \
// RUN:   | FileCheck %s --check-prefix=TRIPLE
// TRIPLE: arm-pc-wince

/// The legacy CeGCC spelling maps to the same target.
// RUN: %clang -target arm-mingw32ce -print-target-triple 2>&1 \
// RUN:   | FileCheck %s --check-prefix=TRIPLE

/// -fms-extensions (__declspec) and -fms-compatibility (libc++ ctype
/// using-declarations) are on. Delayed template parsing is clang-cl only.
// RUN: %clang -target arm-pc-wince -c %s -o /dev/null -### 2>&1 \
// RUN:   | FileCheck %s --check-prefix=MSFLAGS
// The cc1 arg order places -funwind-tables=2 before the -fms* flags, so the
// checks below mirror that order.
// MSFLAGS: "-funwind-tables=2"
// MSFLAGS: "-fms-extensions"
// MSFLAGS: "-fms-compatibility"
// MSFLAGS-NOT: "-fdelayed-template-parsing"
// MSFLAGS: "-fms-compatibility-version=1900"
// MSFLAGS: "-fgnu89-inline"
// MSFLAGS: "-fcommon"

/// WinCE predefined macros.  The default deployment is Windows Embedded CE
/// 6.0 (0x600 == 1536).
// RUN: %clang -target arm-pc-wince -E -dM %s -o - 2>&1 \
// RUN:   | FileCheck %s --check-prefix=DEFINES
// Macro definition order is not semantically meaningful, so the checks are
// order-independent (DAG).  _M_ARM_NT must never appear (WinCE != Windows NT).
// MSVCCompat is on for w32api (__declspec), but C keeps __STDC__ like
// CeGCC's GCC did - gnulib's cdefs.h errors out without it.
// DEFINES-DAG: #define __STDC__ 1
// DEFINES-DAG: #define _ARM_ 1
// DEFINES-DAG: #define _M_ARM 5
// DEFINES-DAG: #define _WIN32_WCE 1536
// DEFINES-DAG: #define UNDER_CE 1536
// DEFINES-DAG: #define WINCE 1
// DEFINES-DAG: #define __MINGW32CE__ 1
// DEFINES-NOT: #define _M_ARM_NT

/// WinCE version from the triple feeds _WIN32_WCE (0x500 == 1280).
// RUN: %clang -target arm-pc-wince5.0 -E -dM %s -o - 2>&1 \
// RUN:   | FileCheck %s --check-prefix=CE5
// CE5: #define _WIN32_WCE 1280

/// _WIN32_WCE uses Microsoft's "VRR" encoding: V is the major version and RR
/// is the *two-digit decimal* CE core version, one decimal digit per hex
/// nibble.  Windows CE 4.2 is core version 4.20, so the macro is 0x0420 ==
/// 1056 - precisely what w32api compares against (aygshell.h gates CE 4.2
/// features on `_WIN32_WCE >= 0x0420`, shellapi.h on `>= 0x420`).  Encoding it
/// as major*0x100 + minor instead yielded 0x0402 == 1026; since 1026 < 1056,
/// every CE 4.1/4.2-gated API was silently disabled for a versioned triple.
/// A single-digit minor is read as the tens digit, so 4.2 and 4.20 agree.
// RUN: %clang -target arm-pc-wince4.2 -E -dM %s -o - 2>&1 \
// RUN:   | FileCheck %s --check-prefix=CE42
// RUN: %clang -target arm-pc-wince4.20 -E -dM %s -o - 2>&1 \
// RUN:   | FileCheck %s --check-prefix=CE42
// CE42-DAG: #define _WIN32_WCE 1056
// CE42-DAG: #define UNDER_CE 1056

/// CE 4.1 is core version 4.10 -> 0x0410 == 1040.
// RUN: %clang -target arm-pc-wince4.1 -E -dM %s -o - 2>&1 \
// RUN:   | FileCheck %s --check-prefix=CE41
// CE41: #define _WIN32_WCE 1040

/// The default CPU is the ARMv5TE baseline (arm926ej-s / i.MX28), soft-float
/// (the WinCE/COREDLL FP ABI).  FloatABI::Soft maps to both the
/// "+soft-float" and "+soft-float-abi" target features (software FP
/// operations and software FP argument passing - the defining
/// characteristics of the armel ABI).
// RUN: %clang -target arm-pc-wince -### -c %s -o /dev/null 2>&1 \
// RUN:   | FileCheck %s --check-prefix=CPU
// CPU: arm926ej-s
// CPU: "+soft-float"
// CPU: "+soft-float-abi"
// CPU: "-msoft-float"
// CPU: "-mfloat-abi"
// CPU: "soft"

/// Linker invocation: lld-link with the WinCE image defaults, the CeGCC
/// startfile/library order, and the GNU-named sysroot the sysroot script
/// assembles from the unmodified mingwrt/w32api trees.
// RUN: %clang -target arm-pc-wince %s -o /dev/null -### 2>&1 \
// RUN:   | FileCheck %s --check-prefix=LINK
// LINK: lld-link
// LINK: "-wince"
// LINK: "-auto-import"
// LINK: "-runtime-pseudo-reloc"
// LINK: /subsystem:windowsce
// LINK: /entry:WinMainCRTStartup
// LINK: /base:0x10000
// LINK: /fixed
// LINK: crt3.o
// LINK: libmingw32.a
// LINK: libclang_rt.builtins-arm.a
// LINK: libceoldname.a
// LINK: libmingwex.a
// LINK-NOT: libposix.a
// LINK-NOT: libpthread.a
// LINK: libcoredll6.a
// LINK-NOT: libcoredll6-x86.a

// RUN: %clang -target arm-pc-wince -lcommctrl -liphlpapi %s -o /dev/null -### 2>&1 \
// RUN:   | FileCheck %s --check-prefix=LLIB
// LLIB: libcommctrl.a
// LLIB: libiphlpapi.a

/// CE 5.0 triple selects the CE 5.0 COREDLL surface.
// RUN: %clang -target arm-pc-wince5.0 %s -o /dev/null -### 2>&1 \
// RUN:   | FileCheck %s --check-prefix=LINK50
// LINK50: lld-link
// LINK50: libcoredll.a
// LINK50-NOT: libcoredll6.a
// LINK50-NOT: libcoredll4.a

/// CE 4.x triples select the CE 4.x (CE.net) COREDLL surface.
// RUN: %clang -target arm-pc-wince4.2 %s -o /dev/null -### 2>&1 \
// RUN:   | FileCheck %s --check-prefix=LINK42
// LINK42: lld-link
// LINK42: libcoredll4.a
// LINK42-NOT: libcoredll.a
// LINK42-NOT: libcoredll6.a

/// CE 3.0 is out of scope (def set and header floor are 4.x+; mingwrt
/// ships no coredll3.def): the link fails with an explicit diagnostic.
// RUN: not %clang -target arm-pc-wince3.0 %s -o /dev/null -### 2>&1 \
// RUN:   | FileCheck %s --check-prefix=LINK30
// LINK30: error: unsupported Windows CE version 3
// LINK30-NOT: libcoredll3.a

/// The COREDLL import surface is arch-specific as well as version-specific.
/// x86 coredll exports the SEH/C++ EH runtime in-DLL (_except_handler3,
/// _except_handler4_common, _local_unwind2/_local_unwind4, _EH_prolog,
/// _EH_prolog2, _SEH_prolog/_SEH_epilog, __abnormal_termination,
/// _CxxThrowException, _setjmp3), the x86 compiler helpers (_chkstk,
/// _alloca_probe*, _ftol*, _all*/_aull*, the _CI* math intrinsics, _inp/_outp)
/// and the MSVC/__thiscall QAE/UAE/AAE spellings of the C++ EH/RTTI exports.
/// The ARM def has none of those (it forwards FP to FPCRT and uses
/// __C_specific_handler for SEH), so linking libcoredll6.a into an x86 image
/// left every __try/__except unresolved even though CodeGen emitted the
/// platform-native x86 SEH correctly.  mingwrt builds libcoredll6-x86.a from
/// coredll6-x86.def for an i386-*-mingw32ce target.
// RUN: %clang -target i386-pc-wince %s -o /dev/null -### 2>&1 \
// RUN:   | FileCheck %s --check-prefix=LINKX86
// RUN: %clang -target i386-pc-wince6.0 %s -o /dev/null -### 2>&1 \
// RUN:   | FileCheck %s --check-prefix=LINKX86
// LINKX86: lld-link
// LINKX86: libclang_rt.builtins-i386.a
// LINKX86: libcoredll6-x86.a
// LINKX86-NOT: libcoredll6.a
// LINKX86-NOT: libclang_rt.builtins-arm.a

/// CE 4.x/5.0 have no x86 def yet (no x86 CE 4/5 shared source is available
/// to parse), so those generations keep the version-selected ARM-parsed
/// surface whatever the architecture.
// RUN: %clang -target i386-pc-wince5.0 %s -o /dev/null -### 2>&1 \
// RUN:   | FileCheck %s --check-prefix=LINKX86CE5
// RUN: %clang -target i386-pc-wince4.2 %s -o /dev/null -### 2>&1 \
// RUN:   | FileCheck %s --check-prefix=LINKX86CE4
// LINKX86CE5: libcoredll.a
// LINKX86CE5-NOT: libcoredll6-x86.a
// LINKX86CE4: libcoredll4.a
// LINKX86CE4-NOT: libcoredll6-x86.a

/// -L adds a library search path for lld-link (translated to /libpath:,
/// ahead of the sysroot lib dir); dropped, third-party -L deps are not found.
// RUN: %clang -target arm-pc-wince -L/opt/easyrpg-deps/lib %s -o /dev/null -### 2>&1 \
// RUN:   | FileCheck %s --check-prefix=LLIBPATH
// LLIBPATH: /libpath:/opt/easyrpg-deps/lib
// LLIBPATH: /libpath:
// LLIBPATH: libcoredll6.a

/// -gcodeview is accepted for the CE target (CodeView debug info); the
/// cc1 gets -gcodeview.
// RUN: %clang -target arm-pc-wince -gcodeview -c %s -o /dev/null -### 2>&1 \
// RUN:   | FileCheck %s --check-prefix=CV
// CV: "-gcodeview"

/// -g maps to lld-link -debug (the image carries the objects' debug
/// info; the MSVC convention) and is absent without -g.
// RUN: %clang -target arm-pc-wince -g %s -o /dev/null -### 2>&1 \
// RUN:   | FileCheck %s --check-prefix=LINKDEBUG
// LINKDEBUG: lld-link
// LINKDEBUG: -debug

// RUN: %clang -target arm-pc-wince %s -o /dev/null -### 2>&1 \
// RUN:   | FileCheck %s --check-prefix=LINKNODEBUG
// LINKNODEBUG: lld-link
// LINKNODEBUG-NOT: -debug

/// -mconsole: CE images are always subsystem 9 and always enter through
/// WinMainCRTStartup (main() is bridged by mingwrt's winmain_ce.o).
// RUN: %clang -target arm-pc-wince -mconsole %s -o /dev/null -### 2>&1 \
// RUN:   | FileCheck %s --check-prefix=CONSOLE
// CONSOLE: lld-link
// CONSOLE: /subsystem:windowsce
// CONSOLE: /entry:WinMainCRTStartup

/// -pg: gcrt3.o startfile + the sampling profiler archive.
// RUN: %clang -target arm-pc-wince -pg %s -o /dev/null -### 2>&1 \
// RUN:   | FileCheck %s --check-prefix=PG
// PG: lld-link
// PG: gcrt3.o
// PG: libgmon.a

/// DLL link: dllcrt3.o, DllMainCRTStartup (CeGCC LINK_SPEC), DLL base,
/// no /fixed.
// RUN: %clang -target arm-pc-wince -shared %s -o /dev/null -### 2>&1 \
// RUN:   | FileCheck %s --check-prefix=DLL
// DLL: lld-link
// DLL: /dll
// DLL: /entry:DllMainCRTStartup
// DLL: /base:0x10000000
// DLL: dllcrt3.o
// DLL-NOT: /fixed

/// -mthreads (or -pthread) selects -D_MT at compile time and links
/// mingwrt's thread glue plus the pthreads4w static library at link time
/// (CeGCC order: %{mthreads:-lmingwthrd} -lmingw32 ...).
// RUN: %clang -target arm-pc-wince -mthreads %s -o /dev/null -### 2>&1 \
// RUN:   | FileCheck %s --check-prefix=THREADS
// THREADS: "-D_MT"
// THREADS: libmingwthrd.a
// THREADS: libpthread.a

// RUN: %clang -target arm-pc-wince -pthread %s -o /dev/null -### 2>&1 \
// RUN:   | FileCheck %s --check-prefix=PTHREAD
// PTHREAD: "-D_MT"
// PTHREAD: libmingwthrd.a
// PTHREAD: libpthread.a

/// C++ pulls in libc++/libc++abi/libunwind (GNU-named sysroot archives).
// RUN: %clang -target arm-pc-wince -x c++ %s -o /dev/null -### 2>&1 \
// RUN:   | FileCheck %s --check-prefix=CXX
// CXX: libc++.a
// CXX: libc++abi.a
// CXX: libunwind.a

int x;

/// clang-cl mode on WinCE: same toolchain selection and image defaults.
// RUN: %clang_cl --target=arm-pc-wince -c %s -o /dev/null -### 2>&1 \
// RUN:   | FileCheck %s --check-prefix=CLMODE
// CLMODE: "-fms-extensions"
// CLMODE: "-fms-compatibility"

// RUN: %clang_cl --target=arm-pc-wince %s -o /dev/null -fuse-ld=lld -### 2>&1 \
// RUN:   | FileCheck %s --check-prefix=CLLINK
// CLLINK: lld-link
// CLLINK: /subsystem:windowsce

/// The armasm dialect is opt-in on WinCE: it reaches -cc1as when it is asked
/// for, and stays out of the command line otherwise (Platform Builder sources
/// are translated to GNU syntax by utils/wince/armasm/armasm-convert.py, so
/// the dialect must not be forced on every .s).
// RUN: %clang -target arm-pc-wince -x assembler -masm=armasm %s -c -o /dev/null -### 2>&1 \
// RUN:   | FileCheck %s --check-prefix=ASMARM
// ASMARM: "-cc1as"
// ASMARM: "-masm=armasm"

// RUN: %clang -target arm-pc-wince -x assembler %s -c -o /dev/null -### 2>&1 \
// RUN:   | FileCheck %s --check-prefix=ASMGNU
// ASMGNU: "-cc1as"
// ASMGNU-NOT: "-masm=armasm"

