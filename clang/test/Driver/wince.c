
/// REQUIRES: arm-registered-target, x86-registered-target


// RUN: %clang -target arm-pc-wince -print-target-triple 2>&1 \
// RUN:   | FileCheck %s --check-prefix=TRIPLE
// TRIPLE: arm-pc-wince


// RUN: %clang -target arm-mingw32ce -print-target-triple 2>&1 \
// RUN:   | FileCheck %s --check-prefix=TRIPLE



// RUN: %clang -target arm-pc-wince -c %s -o /dev/null -### 2>&1 \
// RUN:   | FileCheck %s --check-prefix=MSFLAGS


// MSFLAGS: "-funwind-tables=2"
// MSFLAGS: "-fms-extensions"
// MSFLAGS: "-fms-compatibility"
// MSFLAGS-NOT: "-fdelayed-template-parsing"
// MSFLAGS: "-fms-compatibility-version=1900"
// MSFLAGS: "-fgnu89-inline"
// MSFLAGS: "-fcommon"



// RUN: %clang -target arm-pc-wince -E -dM %s -o - 2>&1 \
// RUN:   | FileCheck %s --check-prefix=DEFINES




// DEFINES-DAG: #define __STDC__ 1
// DEFINES-DAG: #define _ARM_ 1
// DEFINES-DAG: #define _M_ARM 5
// DEFINES-DAG: #define _WIN32_WCE 1536
// DEFINES-DAG: #define UNDER_CE 1536
// DEFINES-DAG: #define WINCE 1
// DEFINES-DAG: #define __MINGW32CE__ 1
// DEFINES-NOT: #define _M_ARM_NT


// RUN: %clang -target arm-pc-wince5.0 -E -dM %s -o - 2>&1 \
// RUN:   | FileCheck %s --check-prefix=CE5
// CE5: #define _WIN32_WCE 1280









// RUN: %clang -target arm-pc-wince4.2 -E -dM %s -o - 2>&1 \
// RUN:   | FileCheck %s --check-prefix=CE42
// RUN: %clang -target arm-pc-wince4.20 -E -dM %s -o - 2>&1 \
// RUN:   | FileCheck %s --check-prefix=CE42
// CE42-DAG: #define _WIN32_WCE 1056
// CE42-DAG: #define UNDER_CE 1056


// RUN: %clang -target arm-pc-wince4.1 -E -dM %s -o - 2>&1 \
// RUN:   | FileCheck %s --check-prefix=CE41
// CE41: #define _WIN32_WCE 1040






// RUN: %clang -target arm-pc-wince -### -c %s -o /dev/null 2>&1 \
// RUN:   | FileCheck %s --check-prefix=CPU
// CPU: arm926ej-s
// CPU: "+soft-float"
// CPU: "+soft-float-abi"
// CPU: "-msoft-float"
// CPU: "-mfloat-abi"
// CPU: "soft"




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


// RUN: %clang -target arm-pc-wince5.0 %s -o /dev/null -### 2>&1 \
// RUN:   | FileCheck %s --check-prefix=LINK50
// LINK50: lld-link
// LINK50: libcoredll.a
// LINK50-NOT: libcoredll6.a
// LINK50-NOT: libcoredll4.a


// RUN: %clang -target arm-pc-wince4.2 %s -o /dev/null -### 2>&1 \
// RUN:   | FileCheck %s --check-prefix=LINK42
// LINK42: lld-link
// LINK42: libcoredll4.a
// LINK42-NOT: libcoredll.a
// LINK42-NOT: libcoredll6.a



// RUN: not %clang -target arm-pc-wince3.0 %s -o /dev/null -### 2>&1 \
// RUN:   | FileCheck %s --check-prefix=LINK30
// LINK30: error: unsupported Windows CE version 3
// LINK30-NOT: libcoredll3.a













// RUN: %clang -target i386-pc-wince %s -o /dev/null -### 2>&1 \
// RUN:   | FileCheck %s --check-prefix=LINKX86
// RUN: %clang -target i386-pc-wince6.0 %s -o /dev/null -### 2>&1 \
// RUN:   | FileCheck %s --check-prefix=LINKX86
// LINKX86: lld-link
// LINKX86: libclang_rt.builtins-i386.a
// LINKX86: libcoredll6-x86.a
// LINKX86-NOT: libcoredll6.a
// LINKX86-NOT: libclang_rt.builtins-arm.a



// RUN: %clang -target i386-pc-wince5.0 %s -o /dev/null -### 2>&1 \
// RUN:   | FileCheck %s --check-prefix=LINKX86CE5
// RUN: %clang -target i386-pc-wince4.2 %s -o /dev/null -### 2>&1 \
// RUN:   | FileCheck %s --check-prefix=LINKX86CE4
// LINKX86CE5: libcoredll.a
// LINKX86CE5-NOT: libcoredll6-x86.a
// LINKX86CE4: libcoredll4.a
// LINKX86CE4-NOT: libcoredll6-x86.a



// RUN: %clang -target arm-pc-wince -L/opt/easyrpg-deps/lib %s -o /dev/null -### 2>&1 \
// RUN:   | FileCheck %s --check-prefix=LLIBPATH
// LLIBPATH: /libpath:/opt/easyrpg-deps/lib
// LLIBPATH: /libpath:
// LLIBPATH: libcoredll6.a



// RUN: %clang -target arm-pc-wince -gcodeview -c %s -o /dev/null -### 2>&1 \
// RUN:   | FileCheck %s --check-prefix=CV
// CV: "-gcodeview"



// RUN: %clang -target arm-pc-wince -g %s -o /dev/null -### 2>&1 \
// RUN:   | FileCheck %s --check-prefix=LINKDEBUG
// LINKDEBUG: lld-link
// LINKDEBUG: -debug

// RUN: %clang -target arm-pc-wince %s -o /dev/null -### 2>&1 \
// RUN:   | FileCheck %s --check-prefix=LINKNODEBUG
// LINKNODEBUG: lld-link
// LINKNODEBUG-NOT: -debug



// RUN: %clang -target arm-pc-wince -mconsole %s -o /dev/null -### 2>&1 \
// RUN:   | FileCheck %s --check-prefix=CONSOLE
// CONSOLE: lld-link
// CONSOLE: /subsystem:windowsce
// CONSOLE: /entry:WinMainCRTStartup


// RUN: %clang -target arm-pc-wince -pg %s -o /dev/null -### 2>&1 \
// RUN:   | FileCheck %s --check-prefix=PG
// PG: lld-link
// PG: gcrt3.o
// PG: libgmon.a



// RUN: %clang -target arm-pc-wince -shared %s -o /dev/null -### 2>&1 \
// RUN:   | FileCheck %s --check-prefix=DLL
// DLL: lld-link
// DLL: /dll
// DLL: /entry:DllMainCRTStartup
// DLL: /base:0x10000000
// DLL: dllcrt3.o
// DLL-NOT: /fixed




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


// RUN: %clang -target arm-pc-wince -x c++ %s -o /dev/null -### 2>&1 \
// RUN:   | FileCheck %s --check-prefix=CXX
// CXX: libc++.a
// CXX: libc++abi.a
// CXX: libunwind.a

int x;


// RUN: %clang_cl --target=arm-pc-wince -c %s -o /dev/null -### 2>&1 \
// RUN:   | FileCheck %s --check-prefix=CLMODE
// CLMODE: "-fms-extensions"
// CLMODE: "-fms-compatibility"

// RUN: %clang_cl --target=arm-pc-wince %s -o /dev/null -fuse-ld=lld -### 2>&1 \
// RUN:   | FileCheck %s --check-prefix=CLLINK
// CLLINK: lld-link
// CLLINK: /subsystem:windowsce
