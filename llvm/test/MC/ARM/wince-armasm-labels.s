@ RUN: llvm-mc -triple arm-pc-wince -masm-armasm -filetype=obj -o %t.o %s
@ RUN: llvm-readobj --symbols %t.o | FileCheck %s
@ RUN: llvm-readobj --sections %t.o | FileCheck %s --check-prefix=SEC


@ CHECK:      Name: armasm_proc
@ CHECK:      StorageClass: External
@ CHECK:      Name: data_table
@ CHECK:      Name: armasm_end

@ SEC:      Name: .text
@ SEC:      Name: .data

        AREA    |.text|, CODE, READONLY

        EXPORT  armasm_proc
        ALIGN   2

armasm_proc PROC
        mov     r0, #1
        mov     r1, #2
        bx      lr
armasm_proc ENDP

        AREA    |.data|, DATA, READWRITE

data_table
        DCD     1, 2, 3
        DCW     0x1234
        DCB     "hi", 0
        SPACE   4
        FILL    4, 0xFF
        ALIGN   2

value_size EQU 8
        DCD     value_size
        DCD     &FF
        DCW     %10101010
        DCD     2_1010

        AREA    |.text|, CODE, READONLY
armasm_end
        END
