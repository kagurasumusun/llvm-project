@ Input to the CE .pdata link test (wince-pdata.test).  Linked FIRST but
@ occupies the HIGHER .text addresses; the .pdata sort must place the
@ lower entry (second, from the other object) first.
@
@ Layout mirrors wince-pdata-second.s: [8-byte PDATA_EH pair][function],
@ with .thumb_func directly above the function label.

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
