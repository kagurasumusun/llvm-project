// REQUIRES: arm-registered-target, x86-registered-target
// RUN: not %clang -cc1as -triple x86_64-pc-windows -masm=armasm -filetype obj %s -o %t.obj 2>&1 | FileCheck %s --check-prefix=X86
// RUN: not %clang -cc1as -triple arm-unknown-linux-gnueabi -masm=armasm -filetype obj %s -o %t.obj 2>&1 | FileCheck %s --check-prefix=ELF
// X86: error: unsupported option '-masm=armasm' for target 'x86_64-pc-windows-msvc'
// ELF: error: unsupported option '-masm=armasm' for target 'arm-unknown-linux-gnueabi'
