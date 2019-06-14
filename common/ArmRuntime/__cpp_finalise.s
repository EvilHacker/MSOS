	AREA	|.text|, CODE, READONLY

	IMPORT	|C$$pi_dtorvec$$Base|
	IMPORT	|C$$pi_dtorvec$$Limit|

	EXPORT	__cpp_finalise
	EXPORT	__cpp_finalise_dtors
__cpp_finalise
__cpp_finalise_dtors
	; save registers
	stmdb	sp!, {v1, v2, lr}
	ldr		v1, =|C$$pi_dtorvec$$Base|
	ldr		v2, =|C$$pi_dtorvec$$Limit|

	b		compare
loop
	ldr		a1, [v1, #0]
	mov		lr, pc
	add		pc, a1, v1
	add		v1, v1, #4
compare
	cmp		v1, v2
	bne		loop

	; restore register and return
	ldmia	sp!, {v1, v2, pc}


	END
