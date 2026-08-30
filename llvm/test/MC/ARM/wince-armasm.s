@ RUN: llvm-mc -triple arm-pc-wince -masm-armasm -filetype=obj -o %t.o %s
@ RUN: llvm-readobj --sections --symbols %t.o | FileCheck %s
@ RUN: llvm-objdump -d %t.o | FileCheck %s --check-prefix=CODE

@ Windows CE armasm dialect (llvm-mc -masm-armasm, clang -masm=armasm).
@ This pins down the part the in-tree ARM armasm parser extension actually
@ implements: the structural directives AREA (including armasm's |name|
@ spelling), ALIGN, EXPORT, IMPORT and END, on top of the regular ARM
@ instruction parser.
@
@ It deliberately does not claim more: armasm's own statement syntax
@ (column-0 labels without a trailing ':', ';' comments, DCD/DCB/SPACE/EQU,
@ macros, conditional assembly) is NOT handled here.  Full Platform Builder
@ sources are translated to GNU syntax by
@ utils/wince/armasm/armasm-convert.py; see utils/wince/README.md.

@ The AREA name is the pipe form Platform Builder uses; CODE maps it to a
@ CNT_CODE | MEM_EXECUTE | MEM_READ section (no synthesized .text.<name>).
@ CHECK:      Sections [
@ CHECK:      Name: .text

@ ALIGN 2 is the armasm exponent form (2^2 bytes) for the *location
@ counter*, not the COFF section Alignment characteristic.  Do not
@ FileCheck section Alignment here.

@ EXPORT marks the symbol external, IMPORT declares an undefined external.
@ Both stay ordinary COFF externals.
@ CHECK:      Symbols [
@ CHECK:      Name: armasm_fn
@ CHECK:      StorageClass: External
@ CHECK:      Name: SomeCEApi
@ CHECK:      StorageClass: External

        AREA    |.text|, CODE, READONLY
        ALIGN   2
        EXPORT  armasm_fn
        IMPORT  SomeCEApi

armasm_fn:
        mov     r0, #1
        bl      SomeCEApi
        bx      lr

        END

@ CODE:      armasm_fn:
@ CODE:      mov r0, #1
@ CODE:      bx lr
