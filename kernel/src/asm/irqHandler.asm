	.global _irq_handler_asm

	.global irqHandler

INTCPS_SIR_IRQ_ADDR .word 0x48200040

_irq_handler_asm:
	CPS		#0x1F						; switch to system-mode

	STMFD	SP, { R0 - R14 }^			; store user registers
	SUB		SP, SP, #60					; decrement stack-pointer: 15 * 4 bytes = 60bytes
	MOV		R2, SP						; need pointer to user-registers which are stored in  in r2
	MOV 	R3, LR						; store IRQ link-register because will BL to irqHandler c function

	LDR		R0, INTCPS_SIR_IRQ_ADDR		; load address of IRQ_ADDR to R0
	LDR 	R0, [ R0 ]					; load content of address to R0

	MRS		R1, CPSR					; need user cpsr in r1

	BL		irqHandler					; branch AND link to irq parent handler

	; NOTE: in case of scheduling we won't arrive here

	MOV		LR, R3						; restore link-register (overridden by BL to irqHandler)
	LDMFD	SP!, { R0 - R3 }			; restore registers and return.

	CPS		#0x12						; back to IRQ-mode

 	SUBS	PC, LR, #4					; return from IRQ
