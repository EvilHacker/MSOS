	IMPORT	currentTaskScheduler__13TaskScheduler
	IMPORT	handleInterrupt__13TaskSchedulerFQ216InterruptHandler14InterruptLevel
	IMPORT	returnFromInterrupt__13TaskSchedulerFv
	IMPORT	pCurrentRemoteDebuggerAgent__19RemoteDebuggerAgent, weak
	IMPORT	handleReset__19RemoteDebuggerAgentFPPUi, weak
	IMPORT	handleUndefinedInstruction__19RemoteDebuggerAgentFPPUi, weak
	IMPORT	handleInstructionAccessAbort__19RemoteDebuggerAgentFPPUi, weak
	IMPORT	handleDataAccessAbort__19RemoteDebuggerAgentFPPUi, weak


	; note: code and data related to each interrupt vector is placed in a
	; separtate AREA so that it can be removed by the linker if not used


	AREA	|.bss_softwareInterruptStack|, DATA, READWRITE, NOINIT
softwareInterruptStack
	SPACE	0x1000
softwareInterruptStackTop

	AREA	|.bss_undefinedInstructionStack|, DATA, READWRITE, NOINIT
undefinedInstructionStack
	SPACE	0x1000
undefinedInstructionStackTop

	AREA	|.bss_abortStack|, DATA, READWRITE, NOINIT
resetStack
instructionAccessAbortStack
dataAccessAbortStack
	SPACE	0x1000
resetStackTop
instructionAccessAbortStackTop
dataAccessAbortStackTop

	AREA	|.bss_irqStack|, DATA, READWRITE, NOINIT
irqStack
	SPACE	0x1000
irqStackTop

	AREA	|.bss_fiqStack|, DATA, READWRITE, NOINIT
fiqStack
	SPACE	0x1000
fiqStackTop


	AREA	|.bss_oldResetHandler|, DATA, READWRITE, NOINIT
	EXPORT	oldResetHandler
oldResetHandler
	DCD		0

	AREA	|.bss_oldSoftwareInterruptHandler|, DATA, READWRITE, NOINIT
	EXPORT	oldSoftwareInterruptHandler
oldSoftwareInterruptHandler
	DCD		0

	AREA	|.bss_oldUndefinedInstructionHandler|, DATA, READWRITE, NOINIT
	EXPORT	oldUndefinedInstructionHandler
oldUndefinedInstructionHandler
	DCD		0

	AREA	|.bss_oldInstructionAccessAbortHandler|, DATA, READWRITE, NOINIT
	EXPORT	oldInstructionAccessAbortHandler
oldInstructionAccessAbortHandler
	DCD		0

	AREA	|.bss_oldDataAccessAbortHandler|, DATA, READWRITE, NOINIT
	EXPORT	oldDataAccessAbortHandler
oldDataAccessAbortHandler
	DCD		0

	AREA	|.bss_oldIrqHandler|, DATA, READWRITE, NOINIT
	EXPORT	oldIrqHandler
oldIrqHandler
	DCD		0

	AREA	|.bss_oldFiqHandler|, DATA, READWRITE, NOINIT
	EXPORT	oldFiqHandler
oldFiqHandler
	DCD		0


;------------------------------------------------------------------------------------------------
; * handleReset
;------------------------------------------------------------------------------------------------

	AREA	|.text_handleReset|, CODE, READONLY

	EXPORT	handleReset
handleReset
	; set the stack pointer
	ldr		sp, =resetStackTop

	; save registers
	sub		lr, lr, #4
	stmdb	sp!, {a1, a2, a3, a4, ip, lr}

	; call the C++ handler and return from the interrupt
	ldr		a1, =pCurrentRemoteDebuggerAgent__19RemoteDebuggerAgent
	add		a2, sp, #(5 * 4)
	ldr		a1, [a1]
	ldr		lr, =returnFromInterrupt
	b		handleReset__19RemoteDebuggerAgentFPPUi


;------------------------------------------------------------------------------------------------
; * handleSoftwareInterrupt
;------------------------------------------------------------------------------------------------

	AREA	|.text_handleSoftwareInterrupt|, CODE, READONLY

	EXPORT	handleSoftwareInterrupt
handleSoftwareInterrupt
	; set the stack pointer
	ldr		sp, =softwareInterruptStackTop

	; save registers
	stmdb	sp!, {a1, a2, a3, a4, ip, lr}

	IF {TRUE} ; just return if there is no old handler to call

	; return from the interrupt
	b		returnFromInterrupt

	ELSE ; only if there is an old handler to call

	; determine if the interrupt should be handled by us or by the old handler
	ldr		a1, [lr, #-4]
	bics	a1, a1, #0xff000000
	beq		returnFromInterrupt

	; call old handler
	ldr		a1, =oldSoftwareInterruptHandler
	ldr		a1, [a1]
	str		a1, [sp, #(5 * 4)]
	ldmia	sp!, {a1, a2, a3, a4, ip, pc}

	ENDIF


;------------------------------------------------------------------------------------------------
; * handleUndefinedInstruction
;------------------------------------------------------------------------------------------------

	AREA	|.text_handleUndefinedInstruction|, CODE, READONLY

	EXPORT	handleUndefinedInstruction
handleUndefinedInstruction
	; set the stack pointer
	ldr		sp, =undefinedInstructionStackTop

	; save registers
	sub		lr, lr, #4
	stmdb	sp!, {a1, a2, a3, a4, ip, lr}

	; call the C++ handler and return from the interrupt
	ldr		a1, =pCurrentRemoteDebuggerAgent__19RemoteDebuggerAgent
	add		a2, sp, #(5 * 4)
	ldr		a1, [a1]
	ldr		lr, =returnFromInterrupt
	b		handleUndefinedInstruction__19RemoteDebuggerAgentFPPUi


;------------------------------------------------------------------------------------------------
; * handleInstructionAccessAbort
;------------------------------------------------------------------------------------------------

	AREA	|.text_handleInstructionAccessAbort|, CODE, READONLY

	EXPORT	handleInstructionAccessAbort
handleInstructionAccessAbort
	; set the stack pointer
	ldr		sp, =instructionAccessAbortStackTop

	; save registers
	sub		lr, lr, #4
	stmdb	sp!, {a1, a2, a3, a4, ip, lr}

	; call the C++ handler and return from the interrupt
	ldr		a1, =pCurrentRemoteDebuggerAgent__19RemoteDebuggerAgent
	add		a2, sp, #(5 * 4)
	ldr		a1, [a1]
	ldr		lr, =returnFromInterrupt
	b		handleInstructionAccessAbort__19RemoteDebuggerAgentFPPUi


;------------------------------------------------------------------------------------------------
; * handleDataAccessAbort
;------------------------------------------------------------------------------------------------

	AREA	|.text_handleDataAccessAbort|, CODE, READONLY

	EXPORT	handleDataAccessAbort
handleDataAccessAbort
	; set the stack pointer
	ldr		sp, =dataAccessAbortStackTop

	; save registers
	sub		lr, lr, #8
	stmdb	sp!, {a1, a2, a3, a4, ip, lr}

	; call the C++ handler and return from the interrupt
	ldr		a1, =pCurrentRemoteDebuggerAgent__19RemoteDebuggerAgent
	add		a2, sp, #(5 * 4)
	ldr		a1, [a1]
	ldr		lr, =returnFromInterrupt
	b		handleDataAccessAbort__19RemoteDebuggerAgentFPPUi


;------------------------------------------------------------------------------------------------
; * handleIrq
;------------------------------------------------------------------------------------------------

	AREA	|.text_handleIrq|, CODE, READONLY

	EXPORT	handleIrq
handleIrq
	; set the stack pointer
	ldr		sp, =irqStackTop

	; save registers
	sub		lr, lr, #4
	stmdb	sp!, {a1, a2, a3, a4, ip, lr}

	; call the C++ handler and return from the interrupt
	ldr		a1, =currentTaskScheduler__13TaskScheduler
	ldr		a2, =0
	ldr		lr, =returnFromInterrupt
	b		handleInterrupt__13TaskSchedulerFQ216InterruptHandler14InterruptLevel


;------------------------------------------------------------------------------------------------
; * handleFiq
;------------------------------------------------------------------------------------------------

	AREA	|.text_handleFiq|, CODE, READONLY

	EXPORT	handleFiq
handleFiq
	; set the stack pointer
	ldr		sp, =fiqStackTop

	; save registers
	sub		lr, lr, #4
	stmdb	sp!, {a1, a2, a3, a4, ip, lr}

	; call the C++ handler
	ldr		a1, =currentTaskScheduler__13TaskScheduler
	ldr		a2, =1
	bl		handleInterrupt__13TaskSchedulerFQ216InterruptHandler14InterruptLevel

	; return from the interrupt
	;b		returnFromInterrupt


;------------------------------------------------------------------------------------------------
; * returnFromInterrupt
;
; This is the epilog for an interrupt handler and
; can only be called from an interrupt mode.
; This procedure will either
; - return from the interrupt normally or
; - switch to a different task.
; On entry:
;	spsr = cpsr of interrupted mode
;	[sp, #00] = a1 of interrupted mode
;	[sp, #04] = a2 of interrupted mode
;	[sp, #08] = a3 of interrupted mode
;	[sp, #12] = a4 of interrupted mode
;	[sp, #16] = ip of interrupted mode
;	[sp, #20] = pc of interrupted mode
;	all other non-banked registers must contain interrupted mode values
;------------------------------------------------------------------------------------------------

returnFromInterrupt
	; return from interrupt if it is nested (spsr not user nor system mode)
	mrs		a1, spsr
	ands	a1, a1, #0xF
	cmpne	a1, #0xF
	ldmneia	sp!, {a1, a2, a3, a4, ip, pc}^

	; critical section, protect against nested interrupts by disabling them
	; (this is only required if the FIQ handler schedules tasks)
	mrs		a1, cpsr
	orr		a1, a1, #0xC0
	msr		cpsr_c, a1

	; notify the scheduler that a return from interrupt is occurring
	ldr		a1, =currentTaskScheduler__13TaskScheduler
	bl		returnFromInterrupt__13TaskSchedulerFv

	; the sheduler returns the following information to us:
	; 	a1 - pointer to the location storing the sp of the task being switched from
	; 	a2 - pointer to the location storing the sp of the task being switched to
	; if no task switch should be performed then a1 and a2 will be null (zero)

	; return back to the interrupted task if no task switch should be performed
	cmp		a1, #0
	ldmeqia	sp!, {a1, a2, a3, a4, ip, pc}^

	; get the switched-from-task's sp (user mode sp)
	stmia	a1, {sp}^
	;nop	; required if the next instruction sources a banked register
	ldr		lr, [a1]

	; save the state of the switched-from-task
	; push pc(r15), lr(r14), r12, r11, r10, r9, r8, r7, r6, r5, r4, r3, r2, r1, r0, cpsr
	ldr		ip, [sp, #(4 * 4)]
	sub		lr, lr, #(11 * 4)
	stmia	lr, {r4-r12, lr}^
	;nop	; required if the next instruction sources a banked register
	mrs		a3, spsr
	ldmia	sp!, {r4, r5, r6, r7, r8, r9}
	stmdb	lr!, {r4, r5, r6, r7}
	str		a3, [lr, #-4]!
	str		r9, [lr, #(15 * 4)]

	; set the switched-from-task's stack pointer
	str		lr, [a1]

	; get the switched-to-task's stack pointer
	ldr		a1, [a2]

	; set the switched-to-task's sp (user mode sp)
	add		a3, a1, #(16 * 4)
	str		a3, [a2]
	ldmia	a2, {sp}^
	;nop	; required if the next instruction sources a banked register

	; restore the state of the switched-to-task and switch to it
	; pop cpsr, r0, r1, r2, r3, r4, r5, r6, r7, r8, r9, r10, r11, r12, lr(r14), pc(r15)
	ldr		lr, [a1], #4
	msr		spsr_cxfs, lr
	ldr		lr, [a1, #(14 * 4)]
	ldmia	a1, {r0-r12, lr}^
	nop		; required if the next instruction sources a banked register
	movs	pc, lr


;------------------------------------------------------------------------------------------------
; * simulateIrq
;
; This function simulates an IRQ exception.
;------------------------------------------------------------------------------------------------

	AREA	|.text_simulateIrq|, CODE, READONLY

	EXPORT	simulateIrq
simulateIrq
	mov		a1, lr
	mrs		a2, cpsr
	bic		a3, a2, #2_00011111
	orr		a3, a3, #2_10010010
	msr		cpsr_c, a3
	msr		spsr_cxsf, a2
	add		lr, a1, #4
	b		handleIrq


;------------------------------------------------------------------------------------------------
; * simulateFiq
;
; This function simulates an FIQ exception.
;------------------------------------------------------------------------------------------------

	AREA	|.text_simulateFiq|, CODE, READONLY

	EXPORT	simulateFiq
simulateFiq
	mov		a1, lr
	mrs		a2, cpsr
	mov		a3, #2_11010001
	msr		cpsr_c, a3
	msr		spsr_cxsf, a2
	add		lr, a1, #4
	b		handleFiq


	END
