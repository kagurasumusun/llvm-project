// REQUIRES: arm-registered-target, x86-registered-target

// RUN: not %clang -target armeb-pc-wince %s -o /dev/null -### 2>&1 \
// RUN:   | FileCheck %s --check-prefix=ARMEB
// ARMEB: error: unsupported architecture 'armeb' for Windows CE target

// RUN: not %clang -target x86_64-pc-wince %s -o /dev/null -### 2>&1 \
// RUN:   | FileCheck %s --check-prefix=X8664
// X8664: error: unsupported architecture 'x86_64' for Windows CE target

// MIPS ran on Windows CE too and is refused by the same rule.  It is run rather
// than described here: a refusal is a driver decision, taken in the tool chain
// the OS selects before any back end is asked for, so the pin needs nothing
// registered and no REQUIRES on a MIPS build.  llvm-mc can write MIPS COFF, but
// lld/COFF applies no MIPS relocations, which is why a refusal is the answer
// rather than a link line that could not be carried out.
// RUN: not %clang -target mipsel-pc-wince %s -o /dev/null -### 2>&1 \
// RUN:   | FileCheck %s --check-prefix=MIPSEL
// MIPSEL: error: unsupported architecture 'mipsel' for Windows CE target

// SuperH ran on CE as well, but LLVM has no SuperH target and no Triple
// architecture for it, so there is no such triple to refuse here.

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
