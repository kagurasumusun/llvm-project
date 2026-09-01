@ RUN: llvm-mc -triple arm-pc-wince -masm-armasm -filetype=obj -o %t.o %s
@ RUN: llvm-objdump -s --section=.data %t.o | FileCheck %s
@ RUN: llvm-readobj --symbols %t.o | FileCheck %s --check-prefix=SYM

@ The armasm data and variable statements the in-tree parser answers with the
@ *same* code the generic AsmParser uses for its own directives, rather than a
@ private re-implementation:
@   - DCBU/DCWU/DCDU/DCQU/DCFU/DCFSU/DCFDU, the forms armasm allows at an
@     unaligned address.  LLVM MC never aligns a data emission in the first
@     place, so the only difference from DCB/DCW/DCD/DCQ/DCF is the name; the
@     dump below is a stream with no padding anywhere, which is that claim.
@     Going through the extension (and not only adding the names to the generic
@     directive table, which is keyed on a leading '.') is also what makes
@     "name DCDU 4" - a label in front of the directive - parse.
@   - GBLA declaration and SETA assignment, which make the name an ordinary
@     symbol with a value exactly as .set does.  That is what lets it be read
@     back from any expression, including the right-hand side of the next SETA.

        AREA    |.data|, DATA, READWRITE
; A quoted armasm string in both the aligned and the unaligned spelling, then
; one value per width.  The dump below is the proof that nothing is padded:
; these widths simply accumulate.
	DCB	"abc"
	DCBU	"de"
	DCWU	0x5566
	DCDU	0x11223344
	DCW	0x7788
	DCQU	0x1122334455667788
	DCFU	0.5
	DCFSU	0.5
	DCFDU	0.5
a	DCDU	0xdeadbeef
; GBLA declares and initialises to zero (its ",N" is an element count, not a
; value); SETA is what gives the name one.  Both words below are the same
; symbol read at two points, which only works because the assignment went to
; the assembler's symbol table rather than anything the parser keeps itself.
n	GBLA	1
	DCD	n
n	SETA	9
	DCD	n
b	DCBU	"hi"

	END

; CHECK:	Contents of section .data:
; CHECK-NEXT:	 0000 61626364 65665544 33221188 77887766  abcdefUD3"..w.wf
; CHECK-NEXT:	 0010 55443322 11000000 3f000000 3f000000  UD3"....?...?...
; CHECK-NEXT:	 0020 000000e0 3fefbead de000000 00090000  ....?...........
; CHECK-NEXT:	 0030 006869                               .hi
; The names are ordinary local symbols in the object file, not parser scratch:
; without that, the two words above could not be attributed to anything.
; SYM-DAG:  Name: a
; SYM-DAG:  Name: b
; SYM-DAG:  Name: n
