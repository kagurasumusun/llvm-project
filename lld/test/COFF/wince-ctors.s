
# REQUIRES: arm
# RUN: llvm-mc -triple=arm-pc-wince -filetype=obj -o %t.obj %s
# RUN: lld-link -wince -subsystem:windowsce -nodefaultlib -entry:WinMainCRTStartup %t.obj -out:%t.exe
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

# CHECK:      Contents of section .ctors:
# CHECK-NEXT: ffffffff 03000000 02000000 01000000
# CHECK-NEXT: 00000000

# CHECK:      Contents of section .dtors:
# CHECK-NEXT: ffffffff 04000000 05000000 06000000
# CHECK-NEXT: 00000000
