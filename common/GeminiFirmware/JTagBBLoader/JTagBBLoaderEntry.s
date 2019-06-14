	AREA	|$startup|, CODE, READONLY

	IMPORT	|Load$$sdramInit$$Base|
	IMPORT	|Image$$sdramInit$$Base|
	IMPORT	|Image$$sdramInit$$Length|

	IMPORT	|Image$$sdramZero$$ZI$$Base|
	IMPORT	|Image$$sdramZero$$ZI$$Length|

	IMPORT  getInitialStackTop
	IMPORT  startup
	IMPORT	__cpp_initialise
	IMPORT	__cpp_finalise
	IMPORT	main
	IMPORT	handleFatalError


;------------------------------------------------------------------------------------------------
; * __main
;
; Main entry point of firmware.
;------------------------------------------------------------------------------------------------

	ENTRY

	EXPORT	__main
	EXPORT	_main
__main
_main
	; make sure we are running from the linked address and not from some alias
	ldr		pc, =%f1
	dcd		0x00A01100
1
	; switch to system mode and disable IRQ and FIQ
	mov		a1, #2_11011111
	msr		cpsr_c, a1


	; copy load regions to execution regions
	ldr		a1, =|Load$$sdramInit$$Base|
	ldr		a2, =|Image$$sdramInit$$Base|
	ldr		a3, =|Image$$sdramInit$$Length|
	bl		copyRegion

	; zero initialize execution regions
	ldr		a1, =|Image$$sdramZero$$ZI$$Base|
	ldr		a2, =|Image$$sdramZero$$ZI$$Length|
	bl		zeroRegion

	; ensure that instruction and data caches are coherent by flushing both of them
	bl		flushAllCaches


	; set the stack pointer
	bl		getInitialStackTop
	mov		sp, a1

	; perform startup initialization
	bl		startup
	
	; construct static objects
	bl		__cpp_initialise

	; run the application
	bl		main

	; destruct static objects
	bl		__cpp_finalise

	; halt
infiniteLoop
	bl		handleFatalError
	b		infiniteLoop


;------------------------------------------------------------------------------------------------
; * copyRegion
;
; Copies a region from its load address to execution address.
; The stack is not used.
; On entry:
;	a1 = region load address
;	a2 = region execution address
;	a3 = region length (should be non-zero)
; On exit:
;	a1 = end of region load address
;	a2 = end of region execution address
;	a3 = 0
;------------------------------------------------------------------------------------------------

copyRegion
	ldr		r3, [a1], #4
	str		r3, [a2], #4
	subs	a3, a3, #4
	bgt		copyRegion
	mov		pc, lr


;------------------------------------------------------------------------------------------------
; * zeroRegion
;
; Zero initializes an execution region.
; The stack is not used.
; On entry:
;	a1 = region execution address
;	a2 = region length (should be non-zero)
; On exit:
;	a1 = end of region execution address
;	a2 = 0
;	a3 = 0
;------------------------------------------------------------------------------------------------

zeroRegion
	mov		a3, #0
zeroLoop
	str		a3, [a1], #4
	subs	a2, a2, #4
	bgt		zeroLoop
	mov		pc, lr


;------------------------------------------------------------------------------------------------
; * flushAllCaches
;
; Flush all caches.
; The stack is not used.
; On exit:
;	a1 = #0xE0000000
;	a2 = 0
;	a3 = 0
;------------------------------------------------------------------------------------------------

flushAllCaches
	mov		a1, #0xE0000000
	mov		a2, #8192
dataCacheWriteBack
	ldr		a3, [a1], #32
	subs	a2, a2, #32
	bne		dataCacheWriteBack
	mcr		p15, 0, r0, c7, c7, 0
	mcr		p15, 0, r0, c7, c10, 4
	mov		pc, lr


	END
