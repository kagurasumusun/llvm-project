/// Windows CE driver tests (GCC-style clang driver).
/// REQUIRES: arm-registered-target

/// The triple is recognized and canonicalized.
// RUN: %clang -target arm-pc-wince -print-target-triple 2>&1 \
// RUN:   | FileCheck %s --check-prefix=TRIPLE
// TRIPLE: arm-pc-wince

/// The legacy CeGCC spelling maps to the same target.
// RUN: %clang -target arm-mingw32ce -print-target-triple 2>&1 \
// RUN:   | FileCheck %s --check-prefix=TRIPLE

/// MSVC compatibility defaults: -fms-extensions/-fms-compatibility and
/// delayed template parsing are on for this target.
// RUN: %clang -target arm-pc-wince -c %s -o /dev/null -### 2>&1 \
// RUN:   | FileCheck %s --check-prefix=MSFLAGS
// MSFLAGS: "-fms-extensions"
// MSFLAGS: "-fms-compatibility"
// MSFLAGS: "-fdelayed-template-parsing"
// MSFLAGS: "-fms-compatibility-version=1900"
// MSFLAGS: "-funwind-tables"

/// WinCE predefined macros.
// RUN: %clang -target arm-pc-wince -E -dM %s -o - 2>&1 \
// RUN:   | FileCheck %s --check-prefix=DEFINES
// DEFINES: #define _ARM_ 1
// DEFINES: #define _WIN32_WCE 1280
// DEFINES: #define UNDER_CE 1280
// DEFINES: #define WINCE 1
// DEFINES: #define __MINGW32CE__ 1
// DEFINES-NOT: #define _M_ARM_NT

/// WinCE version from the triple feeds _WIN32_WCE (0x600 == 1536).
// RUN: %clang -target arm-pc-wince6.0 -E -dM %s -o - 2>&1 \
// RUN:   | FileCheck %s --check-prefix=CE6
// CE6: #define _WIN32_WCE 1536

/// The default CPU is the ARMv4T baseline (arm7tdmi).
// RUN: %clang -target arm-pc-wince -### -c %s -o /dev/null 2>&1 \
// RUN:   | FileCheck %s --check-prefix=CPU
// CPU: arm7tdmi

/// Linker invocation: lld-link with the WinCE image defaults.
// RUN: %clang -target arm-pc-wince %s -o /dev/null -### 2>&1 \
// RUN:   | FileCheck %s --check-prefix=LINK
// LINK: lld-link
// LINK: /subsystem:windowsce
// LINK: /entry:WinMainCRTStartup
// LINK: /base:0x10000
// LINK: /fixed
// LINK: crt0.obj
// LINK: wce.lib
// LINK: mingwex.lib
// LINK: clang_rt.builtins-arm.lib
// LINK: ceoldname.lib
// LINK: coredll.lib

/// DLL link: dllcrt0.obj, DLL base, no /fixed.
// RUN: %clang -target arm-pc-wince -shared %s -o /dev/null -### 2>&1 \
// RUN:   | FileCheck %s --check-prefix=DLL
// DLL: lld-link
// DLL: /dll
// DLL: /base:0x10000000
// DLL: dllcrt0.obj
// DLL-NOT: /fixed

/// C++ pulls in libc++/libc++abi/libunwind.
// RUN: %clang -target arm-pc-wince -x c++ %s -o /dev/null -### 2>&1 \
// RUN:   | FileCheck %s --check-prefix=CXX
// CXX: -lc++
// CXX: -lc++abi
// CXX: -lunwind

int x;
