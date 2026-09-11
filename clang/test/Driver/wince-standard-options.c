// REQUIRES: arm-registered-target, x86-registered-target
// RUN: %clang -target arm-unknown-wince -fsyntax-only -### -x c %s -x c++ %s 2>&1 | FileCheck %s --check-prefix=MIXED
// RUN: %clang --driver-mode=g++ -target arm-unknown-wince -x c -fsyntax-only -### %s 2>&1 | FileCheck %s --check-prefix=C
// RUN: %clang -target arm-unknown-wince -x c++ -fsyntax-only -### %s 2>&1 | FileCheck %s --check-prefix=CXX --implicit-check-not=-fgnu89-inline --implicit-check-not=-fcommon
// RUN: %clang -target arm-unknown-wince -fsyntax-only -### -fno-common -fno-gnu89-inline -fno-ms-compatibility -fno-ms-extensions %s 2>&1 | FileCheck %s --check-prefix=OPTOUT --implicit-check-not=-fcommon --implicit-check-not=-fgnu89-inline --implicit-check-not=-fms-extensions --implicit-check-not='"-fms-compatibility"'
// RUN: %clang -target arm-unknown-wince -fsyntax-only -### -fmsc-version=1916 %s 2>&1 | FileCheck %s --check-prefix=MSC --implicit-check-not=-fms-compatibility-version=1900
// RUN: %clang -target arm-unknown-wince -fsyntax-only -### -fno-short-wchar %s 2>&1 | FileCheck %s --check-prefix=WCHAR --implicit-check-not=-fwchar-type=short
// RUN: %clang -target i386-unknown-wince -fsyntax-only -### %s 2>&1 | FileCheck %s --check-prefix=X86-EH --implicit-check-not=-exception-model=arm
// RUN: %clang -target arm-unknown-wince -### -nostdlib %s -o %t.exe 2>&1 | FileCheck %s --check-prefix=LINK --implicit-check-not=crt3.o --implicit-check-not=libclang_rt.builtins --implicit-check-not=libmingw32.a --implicit-check-not=libcoredll
// RUN: %clang -target arm-unknown-wince -### -nodefaultlibs %s -o %t.exe 2>&1 | FileCheck %s --check-prefix=START --implicit-check-not=libclang_rt.builtins --implicit-check-not=libmingw32.a --implicit-check-not=libcoredll
// RUN: %clang -target arm-unknown-wince -### -nostartfiles %s -o %t.exe 2>&1 | FileCheck %s --check-prefix=LIBS --implicit-check-not=crt3.o
// RUN: %clang -target thumb-unknown-wince -### %s -o %t.exe 2>&1 | FileCheck %s --check-prefix=LIBS --implicit-check-not=libclang_rt.builtins-thumb.a
// RUN: %clang -target arm-unknown-wince -### --rtlib=libgcc %s -o %t.exe 2>&1 | FileCheck %s --check-prefix=LIBGCC --implicit-check-not=--as-needed --implicit-check-not=--no-as-needed
// RUN: %clang -target arm-unknown-wince -### --unwindlib=libunwind %s -o %t.exe 2>&1 | FileCheck %s --check-prefix=UNWIND --implicit-check-not=--as-needed --implicit-check-not=--no-as-needed
// RUN: %clang --driver-mode=g++ -target arm-unknown-wince -### -nostdlib++ %s -o %t.exe 2>&1 | FileCheck %s --check-prefix=LIBS --implicit-check-not=libc++.a --implicit-check-not=libc++abi.a --implicit-check-not=libunwind.a
// RUN: %clang -target arm-unknown-wince -### %s -Wl,/entry:custom -lfoo -Xlinker /opt:ref -l:exact.lib -o %t.exe 2>&1 | FileCheck %s --check-prefix=INPUTS
// RUN: %clang -target arm-unknown-wince -### --ld-path=%clang %s -o %t.exe 2>&1 | FileCheck %s --check-prefix=LDPATH
// RUN: not %clang -target arm-unknown-wince -### -fuse-ld=does-not-exist %s -o %t.exe 2>&1 | FileCheck %s --check-prefix=BAD-LINKER
// A linker named by -fuse-ld= has to be the one that writes CE images, whether
// or not the flavour happens to resolve to a program on this host; --ld-path=
// above is not refused, since naming the program to run is its purpose.
// RUN: not %clang -target arm-unknown-wince -### -fuse-ld=bfd %s -o %t.exe 2>&1 | FileCheck %s --check-prefix=NOT-LINKER
// RUN: %clang -target arm-unknown-wince -### -fuse-ld=lld %s -o %t.exe 2>&1 | FileCheck %s --check-prefix=LLD-LINKER
// RUN: not %clang --driver-mode=g++ -target arm-unknown-wince -### -stdlib=does-not-exist %s -o %t.exe 2>&1 | FileCheck %s --check-prefix=BAD-STDLIB

// MIXED: "-cc1"
// MIXED-SAME: "-fgnu89-inline"
// MIXED-SAME: "-fcommon"
// MIXED: "-cc1"
// MIXED-NOT: "-fgnu89-inline"
// MIXED-NOT: "-fcommon"

// C: "-cc1"
// C-SAME: "-fgnu89-inline"
// C-SAME: "-fcommon"
// CXX: "-cc1"
// OPTOUT: "-cc1"

// MSC: "-fms-compatibility-version=19.16"
// MSC-NOT: "-fms-compatibility-version=
// WCHAR: "-fwchar-type=int"
// X86-EH: "-exception-model=dwarf"

// LINK: lld-link
// START: lld-link
// START-SAME: crt3.o
// LIBS: lld-link
// LIBS-SAME: libclang_rt.builtins-arm.a
// LIBS-SAME: libcoredll6.a
// LIBGCC: lld-link
// LIBGCC-SAME: libgcc.a
// UNWIND: lld-link
// UNWIND-SAME: libunwind.a

// INPUTS: lld-link
// INPUTS-SAME: "/entry:WinMainCRTStartup"
// INPUTS-SAME: "/entry:custom"
// INPUTS-SAME: "libfoo.a"
// INPUTS-SAME: "/opt:ref"
// INPUTS-SAME: "exact.lib"

// LDPATH: "{{[^"]*}}clang{{[^"]*}}" "-auto-import"
// BAD-LINKER: error: invalid linker name in argument '-fuse-ld=does-not-exist'
// NOT-LINKER: error: invalid linker name in argument '-fuse-ld=bfd'
// LLD-LINKER: lld-link
// BAD-STDLIB: error: invalid library name in argument '-stdlib=does-not-exist'

int value;
