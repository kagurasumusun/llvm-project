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
// MSFLAGS: "-fgnu89-inline"

/// WinCE predefined macros.  The default deployment is Windows Embedded CE
/// 6.0 (0x600 == 1536).
// RUN: %clang -target arm-pc-wince -E -dM %s -o - 2>&1 \
// RUN:   | FileCheck %s --check-prefix=DEFINES
// DEFINES: #define _ARM_ 1
// DEFINES: #define _WIN32_WCE 1536
// DEFINES: #define UNDER_CE 1536
// DEFINES: #define WINCE 1
// DEFINES: #define __MINGW32CE__ 1
// DEFINES-NOT: #define _M_ARM_NT

/// WinCE version from the triple feeds _WIN32_WCE (0x500 == 1280).
// RUN: %clang -target arm-pc-wince5.0 -E -dM %s -o - 2>&1 \
// RUN:   | FileCheck %s --check-prefix=CE5
// CE5: #define _WIN32_WCE 1280

/// The default CPU is the ARMv5TE baseline (arm926ej-s / i.MX28), soft-float
/// (the WinCE/COREDLL FP ABI).
// RUN: %clang -target arm-pc-wince -### -c %s -o /dev/null 2>&1 \
// RUN:   | FileCheck %s --check-prefix=CPU
// CPU: arm926ej-s
// CPU: "-mfloat-abi=soft"

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
// LINK: mingw32.lib
// LINK: clang_rt.builtins-arm.lib
// LINK: ceoldname.lib
// LINK: mingwex.lib
// LINK: coredll.lib

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
// THREADS: mingwthrd.lib
// THREADS: pthread.lib

// RUN: %clang -target arm-pc-wince -pthread %s -o /dev/null -### 2>&1 \
// RUN:   | FileCheck %s --check-prefix=PTHREAD
// PTHREAD: "-D_MT"
// PTHREAD: mingwthrd.lib
// PTHREAD: pthread.lib

/// C++ pulls in libc++/libc++abi/libunwind (GNU-named sysroot archives).
// RUN: %clang -target arm-pc-wince -x c++ %s -o /dev/null -### 2>&1 \
// RUN:   | FileCheck %s --check-prefix=CXX
// CXX: c++.lib
// CXX: c++abi.lib
// CXX: unwind.lib

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
