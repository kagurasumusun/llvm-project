# Windows CE images bracket .ctors/.dtors into __CTOR_LIST__/__DTOR_LIST__
# (mingwrt's __main walks them for global C++ constructors/destructors).

# REQUIRES: arm
# RUN: llvm-mc -triple=arm-pc-wince -filetype=obj -o %t.obj %s
# RUN: lld-link -wince -entry:WinMainCRTStartup %t.obj -out:%t.exe
# RUN: llvm-objdump -s %t.exe | FileCheck %s

.syntax unified
.arm
.globl WinMainCRTStartup
WinMainCRTStartup:
  bx lr

.data
  .word __CTOR_LIST__
  .word __DTOR_LIST__

.section .ctors.00005, "w"
  .word 2
.section .ctors, "w"
  .word 1
.section .ctors.00100, "w"
  .word 3

.section .dtors, "w"
  .word 4
.section .dtors.00100, "w"
  .word 6
.section .dtors.00005, "w"
  .word 5

# The head sentinel is (uintptr_t)-1, constructors run in reverse priority
# order (highest priority name first, plain .ctors last), and the list is
# null-terminated.  __CTOR_LIST__ points at the head sentinel.
# CHECK:      Contents of section .ctors:
# CHECK-NEXT: ffffffff
# CHECK-NEXT: 03000000
# CHECK-NEXT: 02000000
# CHECK-NEXT: 01000000
# CHECK-NEXT: 00000000

# Destructors run in forward priority order (plain .dtors first).
# CHECK:      Contents of section .dtors:
# CHECK-NEXT: ffffffff
# CHECK-NEXT: 04000000
# CHECK-NEXT: 05000000
# CHECK-NEXT: 06000000
# CHECK-NEXT: 00000000
