@ RUN: llvm-mc -triple arm-pc-wince -masm-armasm -filetype=obj -o %t.o %s
@ RUN: llvm-readobj --sections --symbols %t.o | FileCheck %s
@ RUN: llvm-objdump -d %t.o | FileCheck %s --check-prefix=CODE


@ CHECK:      Sections [
@ CHECK:      Name: .text


@ CHECK:      Symbols [
@ CHECK-DAG:  Name: armasm_fn
@ CHECK-DAG:  Name: SomeCEApi

        AREA    |.text|, CODE, READONLY
        ALIGN   2
        EXPORT  armasm_fn
        IMPORT  SomeCEApi

armasm_fn:
        mov     r0, #1
        bl      SomeCEApi
        bx      lr
        DCD     SomeCEApi

        END

@ CODE:      <armasm_fn>:
@ CODE:      mov r0, #1
@ CODE:      bl
