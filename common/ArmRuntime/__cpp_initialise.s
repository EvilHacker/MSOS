	AREA	|.text|, CODE, READONLY

	IMPORT	|C$$pi_ctorvec$$Base|
	IMPORT	|C$$pi_ctorvec$$Limit|

	EXPORT	__cpp_initialise
__cpp_initialise
	; save registers
	stmdb	sp!, {v1, v2, lr}
	ldr		v1, =|C$$pi_ctorvec$$Base|
	ldr		v2, =|C$$pi_ctorvec$$Limit|

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
