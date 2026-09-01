@ RUN: llvm-mc -triple arm-pc-wince -masm-armasm -filetype=obj -o %t.o %s
@ RUN: llvm-objdump -s --section=.data %t.o | FileCheck %s
@ RUN: llvm-readobj --symbols %t.o | FileCheck %s --check-prefix=SYM
@ RUN: llvm-readobj --symbols %t.o | false

@ The armasm data and variable statements, and where their implementation
@ lives - nothing here has a private parser path of its own:
@   - DCD/DCW/DCQ and their 'U' spellings are aliases for MASM's DD/DW/DQ
@     (MasmParser's own value parsing, including the "name DCDU 4" infix
@     label form).  LLVM MC never aligns a data emission, so the 'U' forms
@     emit the same bytes; the dump below is a stream with no padding
@     anywhere, which is that claim.
@   - DCB/DCBU (quoted strings) and the DCFS/DCFD float forms stay real
@     handlers in the extension: MASM's value parsing has no string form and
@     no single-precision narrowing.
@   - EQU is MASM's EQU (a constant name, redefinition is an error), GBLA
@     declares the name as an ordinary redefinable symbol, and SETA gives it
@     a value - so it reads back from any expression, including the
@     right-hand side of the next SETA, exactly as .set would.

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

; EQU is MASM's constant equate: readable from an expression, not redefinable.
val	EQU	0x1234
	DCD	val

	END

; CHECK:	Contents of section .data:
; CHECK-NEXT:	 0000 61626364 65665544 33221188 77887766  abcdefUD3"..w.wf
; CHECK-NEXT:	 0010 55443322 11000000 3f000000 3f000000  UD3"....?...?...
; CHECK-NEXT:	 0020 000000e0 3fefbead de000000 00090000  ....?...........
; CHECK-NEXT:	 0030 00686934 120000                             .hi4...
; The names are ordinary local symbols in the object file, not parser scratch:
; without that, the two words above could not be attributed to anything.
; SYM-DAG:  Name: a
; SYM-DAG:  Name: b
; SYM-DAG:  Name: n
; SYM-DAG:  Name: val
