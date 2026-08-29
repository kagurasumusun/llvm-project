@ Input to the CE .pdata link test (wince-pdata.test).  Linked SECOND so it
@ occupies the LOWER .text addresses and its .pdata record is emitted
@ before first's; the sort must put first (lower address) first anyway.

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
	.thumb_func
Lsecond_data:
	.long	1
	.long	__C_specific_handler
	.long	Lsecond_data
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
