/// Windows CE x86 target (the CeGCC i386-mingw32ce configuration).
/// REQUIRES: x86-registered-target

/// The triple is recognized; the legacy CeGCC spelling maps to it.
// RUN: %clang -target i386-pc-wince -print-target-triple 2>&1 \
// RUN:   | FileCheck %s --check-prefix=TRIPLE
// TRIPLE: i386-pc-wince

// RUN: %clang -target i386-mingw32ce -print-target-triple 2>&1 \
// RUN:   | FileCheck %s --check-prefix=TRIPLE

/// Predefined macros: CeGCC gcc/config/i386/mingw32ce.h parity, supplied by
/// addWinCEDefines (clang/lib/Basic/Targets/OSTargets.cpp) plus the i386-only
/// ones in clang/lib/Basic/Targets/X86.h.  The Win32 calling-convention
/// keywords are rewritten to cdecl (what windef.h does under MSVC "for WinCE");
/// UNDER_CE and _WIN32_WCE both carry the CE version.
///
/// Each macro gets its own FileCheck pass over one shared dump.  clang prints
/// -dM output sorted by macro name and FileCheck only ever scans forward, so a
/// single ordered check list silently depends on that sort: UNDER_CE (U=0x55)
/// is printed before _WIN32_WCE (_=0x5F), and listing them the other way round
/// made the second check unmatchable even though both macros are defined.
// RUN: %clang -target i386-pc-wince -E -dM %s -o %t
// RUN: FileCheck %s --check-prefix=DEF-WCEVER --input-file %t
// RUN: FileCheck %s --check-prefix=DEF-UNDERCE --input-file %t
// RUN: FileCheck %s --check-prefix=DEF-X86 --input-file %t
// RUN: FileCheck %s --check-prefix=DEF-WINCE --input-file %t
// RUN: FileCheck %s --check-prefix=DEF-MINGWCE --input-file %t
// RUN: FileCheck %s --check-prefix=DEF-MINGW32 --input-file %t
// RUN: FileCheck %s --check-prefix=DEF-COREDLL --input-file %t
// RUN: FileCheck %s --check-prefix=DEF-CEGCCVER --input-file %t
// RUN: FileCheck %s --check-prefix=DEF-STDCALL --input-file %t
// RUN: FileCheck %s --check-prefix=DEF-UNICODE1 --input-file %t
// RUN: FileCheck %s --check-prefix=DEF-UNICODE2 --input-file %t

// DEF-WCEVER: #define _WIN32_WCE 1536
// DEF-UNDERCE: #define UNDER_CE 1536
// DEF-X86: #define _X86_ 1
// DEF-WINCE: #define WINCE 1
// DEF-MINGWCE: #define __MINGW32CE__ 1
// DEF-MINGW32: #define __MINGW32__ 1
// DEF-COREDLL: #define __COREDLL__ 1
// DEF-CEGCCVER: #define __CEGCC_VERSION__ 0x090909
// DEF-STDCALL: #define __stdcall __attribute__((__cdecl__))
// DEF-UNICODE1: #define _UNICODE 1
// DEF-UNICODE2: #define UNICODE 1

/// The calling conventions are accepted and ignored (single cdecl ABI).
/// Asserted through the emitted symbol rather than through the absence of a
/// warning: __stdcall is predefined to __attribute__((__cdecl__)) above, so the
/// object must carry the cdecl-decorated _f and not a stdcall _f@4.  A
/// NOT-only check list would also pass vacuously if the compile produced no
/// output at all, and a requested FileCheck prefix whose only check is a -NOT
/// is exactly the shape that makes FileCheck error out rather than no-op.
// RUN: %clang -target i386-pc-wince -c %s -o %t.o
// RUN: llvm-nm %t.o | FileCheck %s --check-prefix=CONV
// CONV: _f
// CONV-NOT: @4

int __stdcall f(void) { return 0; }

/// Linker invocation mirrors the ARM flavor (with the i386 builtins).
// RUN: %clang -target i386-pc-wince %s -o /dev/null -### 2>&1 \
// RUN:   | FileCheck %s --check-prefix=LINK
// LINK: lld-link
// LINK: /subsystem:windowsce
// LINK: crt3.o
// LINK: libclang_rt.builtins-i386.a
