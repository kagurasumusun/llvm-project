@ RUN: llvm-mc -triple arm-pc-wince -mcpu=arm926ej-s -masm-armasm -filetype=obj -o %t.o %s
@ RUN: llvm-objdump -s --section=.data %t.o | FileCheck %s
@ RUN: llvm-readobj --symbols %t.o | FileCheck %s --check-prefix=SYM


        AREA    |.data|, DATA, READWRITE
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
n	GBLA	1
	DCD	n
n	SETA	9
	DCD	n
b	DCBU	"hi"

val	EQU	0x1234
	DCD	val

	END

; CHECK:	Contents of section .data:
; CHECK-NEXT:	 0000 61626364 65665544 33221188 77887766  abcdefUD3"..w.wf
; CHECK-NEXT:	 0010 55443322 11000000 3f000000 3f000000  UD3"....?...?...
; CHECK-NEXT:	 0020 000000e0 3fefbead de000000 00090000  ....?...........
; CHECK-NEXT:	 0030 00686934 120000                             .hi4...
; SYM-DAG:  Name: a
; SYM-DAG:  Name: b
; SYM-DAG:  Name: n
