@ RUN: llvm-mc -triple arm-pc-wince -masm-armasm -filetype=obj -o %t.o %s
@ RUN: llvm-objdump -s --section=.data %t.o | FileCheck %s
@ RUN: llvm-readobj --symbols %t.o | FileCheck %s --check-prefix=SYM

@ armasm's IF/ELSEIF/ELSE/ENDIF are aliases for the generic .if/.elseif/
@ .else/.endif, so they inherit that implementation's nesting and its treatment
@ of the branch that is not taken.  Two properties of that implementation are
@ worth pinning down, because they are what makes the alias honest rather than
@ a superficial spelling difference:
@   - the untaken branch is dropped before it is parsed, so the label and the
@     value inside it come to nothing at all (see not_taken / zero_case);
@   - the condition is an ordinary expression, so a name declared by GBLA and
@     written by SETA works in it exactly as it works in a DCD operand.

; GBLA declares a variable and initialises it to zero - its ",N" is an element
; count, not a value - so the first condition below is false and the ELSE is
; what runs.
        AREA    |.data|, DATA, READWRITE
n	GBLA	1
	IF	n == 1
taken_at_declare
	DCD	0xAAAAAAAA
	ELSE
taken_at_zero
	DCD	0xBBBBBBBB
	ENDIF
n	SETA	1
	IF	n == 1
taken_after_seta
	DCD	n
	ELSE
not_taken
	DCD	0xCCCCCCCC
	ENDIF
	IF	n == 0
zero_case
	DCD	0x1
ELSEIF	n == 1
one_case
	DCD	0x2
	ENDIF
	END

; CHECK:	Contents of section .data:
; CHECK-NEXT:	 0000 bbbbbbbb 01000000 02000000           ............
; the labels of the taken branches exist, the skipped ones do not
; SYM-DAG:  Name: taken_at_zero
; SYM-DAG:  Name: taken_after_seta
; SYM-DAG:  Name: one_case
; SYM-NOT:  Name: taken_at_declare
; SYM-NOT:  Name: not_taken
; SYM-NOT:  Name: zero_case
