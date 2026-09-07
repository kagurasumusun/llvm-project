/// REQUIRES: x86-registered-target

// RUN: %clang -target i386-pc-wince -print-target-triple 2>&1 \
// RUN:   | FileCheck %s --check-prefix=TRIPLE
// TRIPLE: i386-pc-wince

// RUN: %clang -target i386-mingw32ce -print-target-triple 2>&1 \
// RUN:   | FileCheck %s --check-prefix=TRIPLE

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

// RUN: %clang -target i386-pc-wince -c %s -o %t.o
// RUN: llvm-nm %t.o | FileCheck %s --check-prefix=CONV
// CONV: _f
// CONV-NOT: @4

int __stdcall f(void) { return 0; }

// RUN: %clang -target i386-pc-wince %s -o /dev/null -### 2>&1 \
// RUN:   | FileCheck %s --check-prefix=LINK
// LINK: lld-link
// LINK: /subsystem:windowsce
// LINK: crt3.o
// LINK: libclang_rt.builtins-i386.a
