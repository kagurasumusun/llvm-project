// REQUIRES: arm-registered-target, x86-registered-target

// RUN: %clang -target arm-pc-wince -print-target-triple 2>&1 \
// RUN:   | FileCheck %s --check-prefix=TRIPLE
// TRIPLE: arm-pc-wince

// RUN: %clang -target arm-mingw32ce -print-target-triple 2>&1 \
// RUN:   | FileCheck %s --check-prefix=LEGACY
// LEGACY: arm-unknown-mingw32ce

// RUN: %clang -target arm-pc-wince -c %s -o /dev/null -### 2>&1 \
// RUN:   | FileCheck %s --check-prefix=MSFLAGS

// MSFLAGS: "-funwind-tables=2"
// MSFLAGS: "-fms-extensions"
// MSFLAGS: "-fms-compatibility"
// MSFLAGS-NOT: "-fdelayed-template-parsing"
// MSFLAGS: "-fms-compatibility-version=19.0"
// MSFLAGS: "-fgnu89-inline"
// MSFLAGS: "-fcommon"

// RUN: %clang -target arm-pc-wince -E -dM %s -o - 2>&1 \
// RUN:   | FileCheck %s --check-prefix=DEFINES

// DEFINES-DAG: #define __STDC__ 1
// DEFINES-DAG: #define _ARM_ 1
// DEFINES-DAG: #define _M_ARM 4
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

// A Windows CE triple names no CPU of its own.  What is compiled for comes from
// the architecture, with LLVM's own default where the triple leaves it
// unspecified, and _M_ARM states that same version: see the DEFINES pin above,
// where the bare triple is the v4T default, and CPU-V5TEJ-DEFINES below.  Only
// the soft-float ABI is the platform's rule, which is why it holds for both
// spellings.
// RUN: %clang -target arm-pc-wince -### -c %s -o /dev/null 2>&1 \
// RUN:   | FileCheck %s --check-prefix=CPU
// CPU: arm7tdmi
// CPU: "+soft-float"
// CPU: "+soft-float-abi"
// CPU: "-msoft-float"
// CPU: "-mfloat-abi"
// CPU: "soft"

// The CPU of an architecture that has one is that architecture's default,
// taken from the table every other target uses, and -mcpu= says it directly.
// v5TEJ is asked for by option rather than written into the triple because the
// driver rewrites the architecture name of a triple before the CPU is looked
// up, which takes v5TEJ down to v5E and then answers with v5E's core; see the
// two FIXME'd cases in arm-cortex-cpus-1.c, which are the driver's business on
// every target and neither a property of CE nor something CE fixes.
// RUN: %clang -target arm-pc-wince -march=armv5tej -### -c %s -o /dev/null 2>&1 \
// RUN:   | FileCheck %s --check-prefix=CPU-V5TEJ
// CPU-V5TEJ: arm926ej-s
// RUN: %clang -target armv5tej-pc-wince -E -dM %s -o - 2>&1 \
// RUN:   | FileCheck %s --check-prefix=CPU-V5TEJ-DEFINES
// CPU-V5TEJ-DEFINES: #define _M_ARM 5

// What a CE target presents to a source file is the MinGW set of names with the
// CE identity in place of the emulated MSVCRT, and the version is carried in
// the BCD nibbles the SDK headers compare against.  The table is sorted so that
// each name is checked at a place of its own, __MSVCRT__ included: it is the
// one that must not be there.
// RUN: %clang -target arm-pc-wince5.2 -E -dM %s -o - 2>&1 \
// RUN:   | grep '^#define ' | LC_ALL=C sort \
// RUN:   | FileCheck %s --check-prefix=CE-DEFINES
// CE-DEFINES: #define UNDER_CE 1312
// CE-DEFINES: #define UNICODE 1
// CE-DEFINES: #define WIN32 1
// CE-DEFINES: #define WINCE 1
// CE-DEFINES: #define WINNT 1
// CE-DEFINES: #define _UNICODE 1
// CE-DEFINES: #define _WIN32 1
// CE-DEFINES: #define _WIN32_WCE 1312
// CE-DEFINES: #define __CEGCC_VERSION__ 0x090909
// CE-DEFINES: #define __COREDLL__ 1
// CE-DEFINES: #define __MINGW32CE__ 1
// CE-DEFINES: #define __MINGW32__ 1
// CE-DEFINES-NOT: #define __MSVCRT__ 1
// CE-DEFINES: #define __WINCE__ 1
// RUN: %clang -target arm-pc-wince -mcpu=cortex-a8 -### -c %s -o /dev/null 2>&1 \
// RUN:   | FileCheck %s --check-prefix=CPU-MCPU
// CPU-MCPU: cortex-a8

// RUN: %clang -target arm-pc-wince %s -o /dev/null -### 2>&1 \
// RUN:   | FileCheck %s --check-prefix=LINK
// LINK: lld-link
// LINK: "-auto-import"
// LINK: "-runtime-pseudo-reloc"
// LINK: /subsystem:windowsce
// LINK-NOT: /subsystem:windowsce,
// LINK: /entry:WinMainCRTStartup
// LINK: /base:0x10000
// LINK: /fixed
// LINK: crt3.o
// LINK: libakari.a
// LINK: libclang_rt.builtins-arm.a
// LINK-NOT: libposix.a
// LINK-NOT: libpthread.a
// LINK: libcoredll6.a
// LINK-NOT: libcoredll6-x86.a

// The mingw32ce layer this tool chain used to name -- the mingwrt CRT glue
// libmingw32.a, its C-library supplement libmingwex.a, its thread glue
// libmingwthrd.a and the desktop-name redirect libceoldname.a -- is not on the
// link line at all: the CE SDK ships the startup layer as libakari.a, and
// naming an archive that is not there hid the library that was really missing.
// RUN: %clang -target arm-pc-wince %s -o /dev/null -### 2>&1 \
// RUN:   | FileCheck %s --check-prefix=LINKRETIRED
// LINKRETIRED: lld-link
// LINKRETIRED-NOT: libmingw32.a
// LINKRETIRED-NOT: libmingwex.a
// LINKRETIRED-NOT: libmingwthrd.a
// LINKRETIRED-NOT: libceoldname.a

// RUN: %clang -target arm-pc-wince -lcommctrl -liphlpapi %s -o /dev/null -### 2>&1 \
// RUN:   | FileCheck %s --check-prefix=LLIB
// LLIB: libcommctrl.a
// LLIB: libiphlpapi.a

// RUN: %clang -target arm-pc-wince5.0 %s -o /dev/null -### 2>&1 \
// RUN:   | FileCheck %s --check-prefix=LINK50
// LINK50: lld-link
// LINK50: /subsystem:windowsce,5.0
// LINK50: libcoredll.a
// LINK50-NOT: libcoredll6.a
// LINK50-NOT: libcoredll4.a

// RUN: %clang -target arm-pc-wince4.2 %s -o /dev/null -### 2>&1 \
// RUN:   | FileCheck %s --check-prefix=LINK42
// LINK42: lld-link
// LINK42: /subsystem:windowsce,4.2
// LINK42: libcoredll4.a
// LINK42-NOT: libcoredll.a
// LINK42-NOT: libcoredll6.a

// RUN: not %clang -target arm-pc-wince3.0 %s -o /dev/null -### 2>&1 \
// RUN:   | FileCheck %s --check-prefix=LINK30
// LINK30: error: unsupported Windows CE version 3
// LINK30-NOT: libcoredll3.a

// Compact 2013 (8.0) is the last release of the family, and it still links
// against the import library of the CE 6 generation.
// RUN: %clang -target arm-pc-wince8.0 %s -o /dev/null -### 2>&1 \
// RUN:   | FileCheck %s --check-prefix=LINK80
// LINK80: lld-link
// LINK80: /subsystem:windowsce,8.0
// LINK80: libcoredll6.a

// The build number of a version is not part of the subsystem version field.
// RUN: %clang -target arm-pc-wince6.0.19041 %s -o /dev/null -### 2>&1 \
// RUN:   | FileCheck %s --check-prefix=LINK60
// LINK60: /subsystem:windowsce,6.0

// RUN: not %clang -target arm-pc-wince9.0 %s -o /dev/null -### 2>&1 \
// RUN:   | FileCheck %s --check-prefix=LINK90
// LINK90: error: unsupported Windows CE version 9
// LINK90-NOT: libcoredll

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

// RUN: %clang -target arm-pc-wince -L/opt/wince-sdk/lib %s -o /dev/null -### 2>&1 \
// RUN:   | FileCheck %s --check-prefix=LLIBPATH
// LLIBPATH: /libpath:/opt/wince-sdk/lib
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

// -mconsole and -mwindows pick the CRT startup routine.  There is no CE
// console subsystem, so /subsystem: stays the same either way.
// RUN: %clang -target arm-pc-wince -mconsole %s -o /dev/null -### 2>&1 \
// RUN:   | FileCheck %s --check-prefix=CONSOLE
// CONSOLE: lld-link
// CONSOLE: /subsystem:windowsce
// CONSOLE: /entry:mainCRTStartup
// CONSOLE-NOT: /entry:WinMainCRTStartup

// RUN: %clang -target arm-pc-wince -mwindows %s -o /dev/null -### 2>&1 \
// RUN:   | FileCheck %s --check-prefix=WINDOWSAPP
// WINDOWSAPP: lld-link
// WINDOWSAPP: /subsystem:windowsce
// WINDOWSAPP: /entry:WinMainCRTStartup

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

// A DLL is not a process, so the mode flags have nothing to choose.
// RUN: %clang -target arm-pc-wince -shared -mconsole %s -o /dev/null -### 2>&1 \
// RUN:   | FileCheck %s --check-prefix=DLLCONSOLE
// DLLCONSOLE: /dll
// DLLCONSOLE: /entry:DllMainCRTStartup

// RUN: %clang -target arm-pc-wince -mthreads %s -o /dev/null -### 2>&1 \
// RUN:   | FileCheck %s --check-prefix=THREADS
// THREADS: "-D_MT"
// THREADS-NOT: libmingwthrd.a
// THREADS: libpthread.a

// RUN: %clang -target arm-pc-wince -pthread %s -o /dev/null -### 2>&1 \
// RUN:   | FileCheck %s --check-prefix=PTHREAD
// PTHREAD: "-D_MT"
// PTHREAD-NOT: libmingwthrd.a
// PTHREAD: libpthread.a

// RUN: %clang --driver-mode=g++ -target arm-pc-wince -x c++ %s -o /dev/null -### 2>&1 \
// RUN:   | FileCheck %s --check-prefix=CXX
// CXX: libc++.a
// CXX: libc++abi.a
// CXX: libunwind.a

// A sysroot built by a GNU package with --prefix=/usr keeps its headers and
// libraries below usr/, and the driver has to look there as well as at the root
// of the sysroot.
// RUN: %clang -target arm-pc-wince --sysroot=%S/Inputs/wince-usr-sysroot %s \
// RUN:   -c -o /dev/null -### 2>&1 | FileCheck %s --check-prefix=USRINC
// USRINC: /wince-usr-sysroot/include"
// USRINC: /wince-usr-sysroot/usr/include"
// RUN: %clang -target arm-pc-wince --sysroot=%S/Inputs/wince-usr-sysroot %s \
// RUN:   -o /dev/null -### 2>&1 | FileCheck %s --check-prefix=USRLIB
// USRLIB: /wince-usr-sysroot/lib"
// USRLIB: /wince-usr-sysroot/usr/lib"

// The name of the coredll import library belongs to the SDK rather than to the
// operating system, so a sysroot that spells it another way is followed.
// RUN: %clang -target arm-pc-wince6.0 --sysroot=%S/Inputs/wince-msvc-sysroot %s \
// RUN:   -o /dev/null -### 2>&1 | FileCheck %s --check-prefix=SDKCOREDLL
// SDKCOREDLL: coredll.lib
// SDKCOREDLL-NOT: libcoredll

// And where one directory carries both an ARM and an x86 library, the CPU of
// the target decides which of them the link gets.
// RUN: %clang -target i386-pc-wince6.0 --sysroot=%S/Inputs/wince-two-cpus-sysroot %s \
// RUN:   -o /dev/null -### 2>&1 | FileCheck %s --check-prefix=TWOCPUX86
// TWOCPUX86: libcoredll6-x86.a
// TWOCPUX86-NOT: libcoredll6.a
// RUN: %clang -target arm-pc-wince6.0 --sysroot=%S/Inputs/wince-two-cpus-sysroot %s \
// RUN:   -o /dev/null -### 2>&1 | FileCheck %s --check-prefix=TWOCPUARM
// TWOCPUARM: libcoredll6.a

// An architecture CE never ran on is refused by the tool chain rather than by a
// back end, so pinning the refusal here depends on no target beyond the two the
// file already requires.  Big-endian ARM is the case to ask about, and the
// reason the refusal gives is with the diagnostic rather than in it.
// RUN: not %clang -target armeb-pc-wince %s -o /dev/null -### 2>&1 \
// RUN:   | FileCheck %s --check-prefix=NOARMEB
// NOARMEB: unsupported architecture 'armeb' for Windows CE target

int x;

// RUN: %clang_cl --target=arm-pc-wince -c %s -o /dev/null -### 2>&1 \
// RUN:   | FileCheck %s --check-prefix=CLMODE
// CLMODE: "-fms-extensions"
// CLMODE: "-fms-compatibility"

// RUN: %clang_cl --target=arm-pc-wince %s -o /dev/null -fuse-ld=lld -### 2>&1 \
// RUN:   | FileCheck %s --check-prefix=CLLINK
// CLLINK: lld-link
// CLLINK: /subsystem:windowsce

// RUN: %clang -target arm-pc-wince -x assembler -masm=armasm %s -c -o /dev/null -### 2>&1 \
// RUN:   | FileCheck %s --check-prefix=ASMARM
// ASMARM: "-cc1as"
// ASMARM: "-masm=armasm"

// RUN: %clang -target arm-pc-wince -x assembler %s -c -o /dev/null -### 2>&1 \
// RUN:   | FileCheck %s --check-prefix=ASMGNU
// ASMGNU: "-cc1as"
// ASMGNU-NOT: "-masm=armasm"

// A CE target is one of the Windows OSes for the object format and for what
// follows from it, so an image nobody named gets the PE default name rather
// than the ELF one.  A runtime facility is a different question and the desktop
// one is not assumed of CE: no __cxa_atexit is asked of a runtime that has no
// such symbol, and a sanitizer is refused for lack of a runtime for it, as on
// any target without one, rather than taken from a desktop Windows.
// RUN: %clang -target arm-pc-wince -### -nostdlib %s 2>&1 \
// RUN:   | FileCheck %s --check-prefix=IMAGENAME
// IMAGENAME: "/out:a.exe"
// IMAGENAME-NOT: a.out

// RUN: %clang -target arm-pc-wince -### -c %s -o /dev/null 2>&1 \
// RUN:   | FileCheck %s --check-prefix=CXA
// CXA: "-fno-use-cxa-atexit"

// RUN: not %clang -target arm-pc-wince -fsanitize=address \
// RUN:   -c %s -o /dev/null 2>&1 | FileCheck %s --check-prefix=NO-SANITIZER
// NO-SANITIZER: unsupported option '-fsanitize=address'

// The same goes for which function-ABI name -mabi= is checked against on a
// 32-bit x86 CE target: the API is called the way the object format family
// calls it, so ms is the default and sysv the unsupported one.
// RUN: %clang -target i386-unknown-wince -mabi=ms -fsyntax-only %s
// RUN: not %clang -target i386-unknown-wince -mabi=sysv -fsyntax-only %s 2>&1 \
// RUN:   | FileCheck %s --check-prefix=MABI
// MABI: unsupported option '-mabi=' for target 'i386-unknown-wince'

// Which C++ unwinding format is in use is fixed by the platform, not chosen: on CE's
// ARM and Thumb images the tables are the .ARM.exidx ones the EHABI personality walks
// (llvm::Triple::getDefaultExceptionHandling), and the two models that would replace
// them name the desktop MSVC runtime's tables and .eh_frame respectively, neither of
// which a CE image carries.  Refusing them here is what lets libc++abi's and
// libunwind's configuration headers stay free of CE overrides, and SjLj -- which needs
// no tables at all, and is why CE has its own longjmp patterns -- stays legal.
// RUN: not %clang -target arm-pc-wince -fseh-exceptions -fsyntax-only %s 2>&1 \
// RUN:   | FileCheck %s --check-prefix=EHSEH
// The triple the front end reports is the one the driver settled after handling the
// ARM options, which spells a bare "arm" by the architecture it defaults to; only the
// OS component matters to this check, and it stays the spelling that was asked for.
// EHSEH: error: invalid exception model 'seh' for target '{{.*}}-pc-wince'
// RUN: not %clang -target arm-pc-wince -fdwarf-exceptions -fsyntax-only %s 2>&1 \
// RUN:   | FileCheck %s --check-prefix=EHMODEL
// RUN: not %clang -target armv7-pc-windowsce -mthumb -fseh-exceptions -fsyntax-only %s 2>&1 \
// RUN:   | FileCheck %s --check-prefix=EHMODEL
// RUN: not %clang -target thumbv7-pc-wince -fdwarf-exceptions -fsyntax-only %s 2>&1 \
// RUN:   | FileCheck %s --check-prefix=EHMODEL
// EHMODEL: error: invalid exception model '{{(seh|dwarf)}}' for target
// RUN: %clang -target arm-pc-wince -fsjlj-exceptions -fsyntax-only %s
// RUN: %clang -target arm-pc-wince -fexceptions -fcxx-exceptions -fsyntax-only %s
// 32-bit x86 CE keeps DWARF CFI, which is that CPU's own default for COFF here, so
// nothing about the ARM rule above may reach it.
// RUN: %clang -target i386-unknown-wince -fdwarf-exceptions -fsyntax-only %s

// A CE target presents the Win32 API the two Windows entries ask about, as the
// desktop one does, and TARGET_OS_WINCE is what tells it apart from the desktop;
// UEFI, being firmware, has an entry of its own and stays zero.
// RUN: %clang -target arm-pc-wince -dM -E -fdefine-target-os-macros %s -o - 2>&1 \
// RUN:   | FileCheck %s --check-prefix=OSMACROS
// OSMACROS-DAG: #define TARGET_OS_WINCE 1
// OSMACROS-DAG: #define TARGET_OS_WIN32 1
// OSMACROS-DAG: #define TARGET_OS_WINDOWS 1
// OSMACROS-DAG: #define TARGET_OS_UEFI 0

// The tool names this target is reached by add nothing but
// --target=arm-pc-wince, so a Makefile written for the CeGCC line stays
// unmodified and the GCC options it passes have to be taken: -mwin32 is in
// GCC's own MinGW option file and the poke-function-name pair in its ARM one,
// and such a Makefile may carry either.  They are dropped, with nothing of
// them reaching cc1, as the options of clang's ignored m group are.
// RUN: %clang -target arm-pc-wince -mwin32 -mpoke-function-name -### -c %s \
// RUN:   -o /dev/null 2>&1 | FileCheck %s --check-prefix=GCCIGNORED
// GCCIGNORED: "-cc1"
// GCCIGNORED-NOT: unknown argument
// GCCIGNORED-NOT: unused argument
// GCCIGNORED-NOT: mwin32
// GCCIGNORED-NOT: poke-function-name
// RUN: %clang -target arm-pc-wince -mno-poke-function-name -### -c %s \
// RUN:   -o /dev/null 2>&1 | FileCheck %s --check-prefix=GCCIGNORED
// GCC's -mno-win32 is the one GCC spelling of that pair this target does not
// take: it asks for the Microsoft Windows macros to be left out, and a CE
// target cannot stop being one, so the request fails as any unknown option
// does rather than being dropped.
// RUN: not %clang -target arm-pc-wince -mno-win32 -c %s -o /dev/null -### \
// RUN:   2>&1 | FileCheck %s --check-prefix=NOWIN32
// NOWIN32: error: unknown argument: '-mno-win32'
// The same command line reaching a link, which is what a Makefile that hands
// its CFLAGS to both steps produces.
// RUN: %clang -target arm-pc-wince -mwin32 -### %s -o /dev/null 2>&1 \
// RUN:   | FileCheck %s --check-prefix=GCCIGNOREDLINK
// GCCIGNOREDLINK: lld-link
// GCCIGNOREDLINK-NOT: unknown argument
// GCCIGNOREDLINK-NOT: unused argument
