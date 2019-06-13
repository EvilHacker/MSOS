	AREA	|.text|, CODE, READONLY


	EXPORT	switchTasks
switchTasks
	; save all registers of the current task (lr contains caller's pc)
	str		lr, [sp, #-4]!
	stmdb	sp!, {r0-r12, lr}
	mrs		lr, cpsr
	str		lr, [sp, #-4]!

	; save the current task's stack pointer
	str		sp, [a1]

	; get the new task's stack pointer
	ldr		sp, [a2]

	; restore all registers of the new task and return
	ldr		lr, [sp], #4
	msr		cpsr_cxfs, lr
	ldmia	sp!, {r0-r12, lr, pc}


	END
