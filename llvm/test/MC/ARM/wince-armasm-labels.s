@ RUN: llvm-mc -triple arm-pc-wince -masm-armasm -filetype=obj -o %t.o %s
@ RUN: llvm-readobj --symbols %t.o | FileCheck %s
@ RUN: llvm-readobj --sections %t.o | FileCheck %s --check-prefix=SEC

@ armasm *statement* syntax, on top of the structural directives covered by
@ llvm/test/MC/ARM/wince-armasm.s:
@   - ';' starts a comment.  This is an addition, not a replacement: the
@     GNU-syntax comment character for this target stays '@', which is what
@     the RUN lines above use.
@   - a label written in front of a directive, with no ':' after it
@     ("name PROC", "name DCD 1")
@   - a label standing alone on its line
@   - the data directives DCD/DCW/DCB/DCQ/SPACE/FILL and the EQU constant
@   - the armasm integer literals &FF [hex], %1010 [binary] and n_xxxx
@     [base n]
@
@ The CHECK lines pin down symbol and section names only.  The emitted bytes
@ are deliberately not checked: this file has not yet been assembled by a
@ real build, so byte-level expectations would be guesses.
@
@ Covered by sibling tests, not this file: DCFU/DCFSU/DCFDU/DCQU and the other
@ '*U' data forms, GBLA/LCLA and SETA, and IF/ELSEIF/ELSE/ENDIF
@ (wince-armasm-data.s, wince-armasm-cond.s).  Still not handled by the in-tree
@ parser, and diagnosed rather than ignored: armasm's macro processor
@ (MACRO/MEND, WHILE/WEND, GET/INCLUDE) and the SETS/:DEF: spellings that go
@ with it - those need the converter in the toolchain builder
@ (kagurasumusun/cellvm-build:armasm/armasm-convert.py).

@ CHECK:      Name: armasm_proc
@ CHECK:      StorageClass: External
@ CHECK:      Name: data_table
@ CHECK:      Name: armasm_end

@ SEC:      Name: .text
@ SEC:      Name: .data

        AREA    |.text|, CODE, READONLY   ; armasm comments start with ';'

        EXPORT  armasm_proc               ; the EXPORT survives the PROC below
        ALIGN   2                         ; armasm exponent form: 2^2 = 4

armasm_proc PROC                          ; "name PROC" - no ':' after the name
        mov     r0, #1
        mov     r1, #2
        bx      lr
armasm_proc ENDP                          ; "name ENDP" closes that procedure

        AREA    |.data|, DATA, READWRITE

data_table                                ; a label standing on its own line
        DCD     1, 2, 3
        DCW     0x1234
        DCB     "hi", 0
        SPACE   4
        FILL    4, 0xFF
        ALIGN   2

value_size EQU 8                          ; EQU defines a constant, not a label
        DCD     value_size
        DCD     &FF                         ; armasm hex
        DCW     %10101010                   ; armasm binary
        DCD     2_1010                      ; armasm base-2 (ten)

        AREA    |.text|, CODE, READONLY
armasm_end
        END
