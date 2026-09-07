// REQUIRES: arm-registered-target, x86-registered-target

// RUN: not %clang -target armeb-pc-wince %s -o /dev/null -### 2>&1 \
// RUN:   | FileCheck %s --check-prefix=ARMEB
// ARMEB: error: unsupported architecture 'armeb' for Windows CE target

// RUN: not %clang -target x86_64-pc-wince %s -o /dev/null -### 2>&1 \
// RUN:   | FileCheck %s --check-prefix=X8664
// X8664: error: unsupported architecture 'x86_64' for Windows CE target

// RUN: %clang -target arm-pc-wince %s -o /dev/null -### 2>&1 \
// RUN:   | FileCheck %s --check-prefix=OKARM
// OKARM-NOT: unsupported architecture
// OKARM: lld-link
// OKARM: libclang_rt.builtins-arm.a
// OKARM: libcoredll6.a

// RUN: %clang -target i386-pc-wince %s -o /dev/null -### 2>&1 \
// RUN:   | FileCheck %s --check-prefix=OKX86
// OKX86-NOT: unsupported architecture
// OKX86: lld-link
// OKX86: libclang_rt.builtins-i386.a
// OKX86: libcoredll6-x86.a
