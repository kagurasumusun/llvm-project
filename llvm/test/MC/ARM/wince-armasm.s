@ RUN: llvm-mc -triple arm-pc-wince -masm-armasm -filetype=obj -o %t.o %s
@ RUN: llvm-readobj --sections --symbols %t.o | FileCheck %s
@ RUN: llvm-objdump -d %t.o | FileCheck %s --check-prefix=CODE

@ Windows CE armasm dialect (llvm-mc -masm-armasm, clang -masm=armasm).
@ This pins down the part the in-tree ARM armasm parser extension actually
@ implements: the structural directives AREA (including armasm's |name|
@ spelling), ALIGN, EXPORT, IMPORT and END, on top of the regular ARM
@ instruction parser.
@
@ It deliberately does not claim more than it implements: armasm's statement
@ syntax (a label in front of a directive with no trailing ':', ';' comments,
@ DCD/DCB/SPACE/EQU and the '*U' data spellings, GBLA/SETA variables,
@ IF/ELSEIF/ELSE/ENDIF) is covered by wince-armasm-labels.s,
@ wince-armasm-data.s and wince-armasm-cond.s.  What is still not handled here
@ is armasm's macro processor (MACRO/MEND, WHILE/WEND, GET/INCLUDE), which is
@ what the converter in the toolchain builder
@ (kagurasumusun/cellvm-build:armasm/armasm-convert.py) expands before the
@ file reaches this parser; those directives are diagnosed rather than
@ ignored.

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

@ llvm-objdump -d on IMAGE_FILE_MACHINE_ARM prints "<name>:" and does
@ not resolve the bl reloc to SomeCEApi (the IMPORT is an undefined
@ external; the immediate encodes as bl to the next insn).  bx lr is
@ shown as <unknown> (e12fff1e) on this machine type.
@ CODE:      <armasm_fn>:
@ CODE:      mov r0, #1
@ CODE:      bl
