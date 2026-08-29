@ Input to the CE .pdata link test (wince-pdata.test).  Linked FIRST so it
@ occupies the HIGHER .text addresses; the .pdata sort must still place
@ first (lower address) before second (higher address).

	.syntax unified
	.thumb
	.text
	.p2align 2
	.globl	first
	.thumb_func
	@ Compiler (ARMAsmPrinter) output shape for an SEH function:
	@ [SEH scope table][PDATA_EH pair][function].  The pair's second word
	@ points at the scope table (handler data); the pair must be exactly
	@ the 8 bytes before the first instruction.
Lfirst_data:
	.long	1
	.long	__C_specific_handler
	.long	Lfirst_data
first:
	.seh_proc first
	.seh_handler __C_specific_handler, %except
	push	{r4, lr}
	.seh_save_regs {r4, lr}
	.seh_endprologue
	movs	r0, #1
	pop	{r4, pc}
	.seh_endproc
