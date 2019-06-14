	AREA	|.text|, CODE, READONLY

;------------------------------------------------------------------------------------------------
; * flushSa1110DataCache
;------------------------------------------------------------------------------------------------

	EXPORT	flushSa1110DataCache
flushSa1110DataCache

	; write back changes in the cache
	mov		a1, #0xE0000000
	mov		a2, #0x2000
writeBack
	ldr		a3, [a1], #0x20
	subs	a2, a2, #0x20
	bne		writeBack

	; flush entire data cache
	mcr		p15, 0, r0, c7, c6, 0

	; return
	mov		pc, lr


	END