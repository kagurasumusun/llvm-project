@ Input to the CE .pdata link test (wince-pdata.test).  Linked SECOND so it
@ occupies the LOWER .text addresses; the sort must still order entries by
@ pFuncStart.
@
@ Layout of .text (the kernel reads the PDATA_EH pair 8 bytes before
@ pFuncStart): [__C_specific_handler][8-byte PDATA_EH pair][function].
@ .thumb_func marks the NEXT symbol as a Thumb function, so it must sit
@ directly above the function label (data labels stay unmarked).

	.syntax unified
	.thumb
	.text
	.p2align 2
	.globl	__C_specific_handler
	.thumb_func
__C_specific_handler:
	bx	lr

	.p2align 2
	.globl	second
Lsecond_data:
	.long	__C_specific_handler
	.long	Lsecond_data
	.thumb_func
second:
	.seh_proc second
	.seh_handler __C_specific_handler, %except
	push	{r4, r5, lr}
	.seh_save_regs {r4, r5, lr}
	.seh_endprologue
	movs	r0, #2
	movs	r1, #3
	pop	{r4, r5, pc}
	.seh_endproc
