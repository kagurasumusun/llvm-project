
	.syntax unified
	.thumb
	.text
	.p2align 2
	.globl	first
Lfirst_data:
	.long	__C_specific_handler
	.long	Lfirst_data
	.thumb_func
first:
	.seh_proc first
	.seh_handler __C_specific_handler, %except
	push	{r4, lr}
	.seh_save_regs {r4, lr}
	.seh_endprologue
	movs	r0, #1
	pop	{r4, pc}
	.seh_endproc
