	AREA	|.text|, CODE, READONLY


	;void disableInterrupts();
	;void enableInterrupts();
	;UInt getInterruptState();
	;void setInterruptState(UInt state);


	EXPORT	disableInterrupts
disableInterrupts
	mrs		a1, cpsr
	orr		a1, a1, #0xc0
	msr		cpsr_cxsf, a1
	mov		pc, lr

	EXPORT	enableInterrupts
enableInterrupts
	mrs		a1, cpsr
	bic		a1, a1, #0xc0
	msr		cpsr_cxsf, a1
	mov		pc, lr

	EXPORT	getInterruptState
getInterruptState
	mrs		a1, cpsr
	mov		pc, lr

	EXPORT	setInterruptState
setInterruptState
	msr		cpsr_cxsf, a1
	mov		pc, lr


	END
