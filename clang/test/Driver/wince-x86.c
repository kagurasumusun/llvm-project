/// Windows CE x86 target (the CeGCC i386-mingw32ce configuration).
/// REQUIRES: x86-registered-target

/// The triple is recognized; the legacy CeGCC spelling maps to it.
// RUN: %clang -target i386-pc-wince -print-target-triple 2>&1 \
// RUN:   | FileCheck %s --check-prefix=TRIPLE
// TRIPLE: i386-pc-wince

// RUN: %clang -target i386-mingw32ce -print-target-triple 2>&1 \
// RUN:   | FileCheck %s --check-prefix=TRIPLE

/// Predefined macros: CeGCC gcc/config/i386/mingw32ce.h.  The Win32
/// calling-convention keywords are rewritten to cdecl (what windef.h does
/// under MSVC "for WinCE"); UNDER_CE/_WIN32_WCE carry the CE version.
// RUN: %clang -target i386-pc-wince -E -dM %s -o - 2>&1 \
// RUN:   | FileCheck %s --check-prefix=DEFINES
// DEFINES: #define _WIN32_WCE 1536
// DEFINES: #define UNDER_CE 1536
// DEFINES: #define _X86_ 1
// DEFINES: #define WINCE 1
// DEFINES: #define __MINGW32CE__ 1
// DEFINES: #define __MINGW32__ 1
// DEFINES: #define __COREDLL__ 1
// DEFINES: #define __CEGCC_VERSION__ 0x090909
// DEFINES: #define __stdcall __attribute__((__cdecl__))
// DEFINES: #define _UNICODE 1
// DEFINES: #define UNICODE 1

/// The calling conventions are accepted and ignored (single cdecl ABI).
// RUN: %clang -target i386-pc-wince -c %s -o /dev/null 2>&1 \
// RUN:   | FileCheck %s --check-prefix=CONV
// CONV-NOT: warning

int __stdcall f(void) { return 0; }

/// Linker invocation mirrors the ARM flavor (with the i386 builtins).
// RUN: %clang -target i386-pc-wince %s -o /dev/null -### 2>&1 \
// RUN:   | FileCheck %s --check-prefix=LINK
// LINK: lld-link
// LINK: /subsystem:windowsce
// LINK: crt3.o
// LINK: clang_rt.builtins-i386.lib
