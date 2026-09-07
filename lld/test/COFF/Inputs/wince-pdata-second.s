
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
