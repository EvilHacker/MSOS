
	AREA |.text|, CODE, READONLY

	EXPORT	enterSa1110IdleMode
enterSa1110IdleMode
	; noncachable address (in register space)
	ldr		a1, =0xb0000000

	; jump to the cache-line aligned code
	b		aligned


	AREA |.text|, CODE, READONLY, ALIGN=5

aligned
	; disable clock switching
	mcr 	p15, 0, r0, c15, c2, 2
	
	; load from a noncacheable memory
	ldr 	a2, [a1]
	
	; wait for interrupt
	mcr 	p15, 0, r0, c15, c8, 2
	
	; enable clock switching
	mcr 	p15, 0, r0, c15, c1, 2

	; load from a noncacheable memory
	ldr 	a2, [a1]
	
	; return
	mov		pc, lr


	END