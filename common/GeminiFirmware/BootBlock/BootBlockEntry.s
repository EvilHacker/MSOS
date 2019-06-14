	AREA	|$startup|, CODE, READONLY

	IMPORT	|Load$$sdramInit$$Base|
	IMPORT	|Image$$sdramInit$$Base|
	IMPORT	|Image$$sdramInit$$Length|

	IMPORT	|Image$$sdramZero$$ZI$$Base|
	IMPORT	|Image$$sdramZero$$ZI$$Length|

	; from startup.cpp
	IMPORT  getPreMmuStackTop
	IMPORT  preMmuStartup
	IMPORT  buildAddressTranslationTables
	IMPORT  getPostMmuStackTop
	IMPORT  postMmuStartup

	; C++ runtime
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
	; temporary vector table
	ldr		pc, =%f1
	ldr		pc, =%f1
	ldr		pc, =%f1
	ldr		pc, =%f1
	ldr		pc, =%f1
	ldr		pc, =%f1
	ldr		pc, =%f1

	; make sure we are running from the linked address and not from some alias
	ldr		pc, =%f1
1
	; switch to system mode with interrupts disabled
	mov		a1, #2_11011111
	msr		cpsr_c, a1

	IF {CPU} = "ARM920T"

	; initialize clocks
	;	OSC32       =     32768Hz
	;	MCU PLL     = 201326592Hz = (1024 * OSC32 / 1) * (6 + (0 / 1))
	;	FCLK        = 201326592Hz = MCU PLL / 1
	;	System PLL  =  96000009Hz = (1024 * OSC32 / 4) * (11 + (421 / 948))
	;	HCLK = BCLK =  96000009Hz = System PLL / 1
	;	CLK48       =  48000004Hz = System PLL / 2
	;	PERCLK1     =   8000001Hz = System PLL / 12
	;	PERCLK2     =   8000001Hz = System PLL / 12
	;	PERCLK3     =   8000001Hz = System PLL / 12
	ldr		a1, =0x0021B000
	ldr		a2, =0x000B00BB
	str		a2, [a1, #0x20]
	ldr		a2, =0x00011800
	str		a2, [a1, #0x04]
	ldr		a2, =0x00000040
	str		a2, [a1, #0x08]
	ldr		a2, =0x0FB32DA5
	str		a2, [a1, #0x0C]
	ldr		a2, =0x00000000
	str		a2, [a1, #0x10]
	ldr		a2, =0x07600003
	str		a2, [a1, #0x00]


	; configure access to flash (chip select 0)
	ldr		a1, =0x00220000
	ldr		a2, [a1]
	bic		a2, a2, #0x00003F00
	orr		a2, a2, #0x00000B00
	str		a2, [a1]


	; initialize SDRAM (MT48LC4M32LFFC-8)
	;	4Mx32 (16MBytes)
	;	2 bank address bits
	;	12 row address bits
	;	8 column address bits,
	;	4096 rows refreshed every 64ms
	;	interleaved bank addressing
	;	CAS latency 2
	ldr		a1, =0x00221000
	ldr		a2, =0x910A0200
	str		a2, [a1]
	ldr		a3, =0x08400000
	ldr		a4, [a3]
	ldr		a2, =0xA10A0200
	str		a2, [a1]
	mov		a4, #8
1	subs	a4, a4, #1
	str		a4, [a3]
	bne		%b1
	ldr		a2, =0xB10A0200
	str		a2, [a1]
	ldr		a3, =0x08223000
	ldr		a4, [a3]
	ldr		a2, =0x810A8200
	str		a2, [a1]

	ENDIF


	IF {CPU} = "SA-1100"

	; make sure the processor core is in a hard reset state
	;  		        IZFRSBLDPWCAM
	ldr		a1, =2_00000001110000
	mcr		p15, 0, a1, c1, c0
	ldr		a1, =0
	mcr		p15, 0, a1, c5, c0
	mcr		p15, 0, a1, c6, c0
	mcr		p15, 0, r0, c7, c7
	mcr		p15, 0, r0, c7, c8
	mcr		p15, 0, r0, c8, c7
	mcr		p15, 0, r0, c9, c0
	mcr		p15, 0, r0, c9, c0, 4
	mcr		p15, 0, a1, c13, c0
	mcr		p15, 0, a1, c14, c3
	mcr		p15, 0, a1, c14, c8
	mcr 	p15, 0, r0, c15, c2, 2


; select only one of the following clock frequencies:
;clockFrequency EQU 58982400
;clockFrequency EQU 73728000
;clockFrequency EQU 88473600
;clockFrequency EQU 103219200
;clockFrequency EQU 117964800
;clockFrequency EQU 132710400
;clockFrequency EQU 147456000
clockFrequency EQU 162201600
;clockFrequency EQU 176947200
;clockFrequency EQU 191692800
;clockFrequency EQU 206438400
;clockFrequency EQU 221184000

	; set the core clock frequency
	ldr		a1, =0x90020000
	ldr		a2, =(clockFrequency / 3686400 / 4) - 4
	str		a2, [a1, #0x14]

	; enable clock switching
	mcr 	p15, 0, r0, c15, c1, 2

	; wait 200us for clock to stabilize
	ldr		a1, =0x90000000
	ldr		a2, [a1, #0x10]
	add		a2, a2, #0x300
1	ldr		a3, [a1, #0x10]
	cmp		a2, a3
	bpl		%b1


	; configure and enable access to flash (static bank 0) and VP-ASIC (static bank 1)
	ldr		a1, =0xA0000000
	ldr		a2, =0x631EC754
	str		a2, [a1, #0x10]

	; configure and enable access to USB 2 controller (static bank 4, 5)
	ldr		a2, =0x210D210D
	str		a2, [a1, #0x2C]

	; change SDRAM states (self-refresh -> power-down -> PWRDWNX -> idle)
	ldr		a2, [a1, #0x1C]
	orr		a2, a2, #0x00200000
	str		a2, [a1, #0x1C]
	bic		a2, a2, #0x80000000
	str		a2, [a1, #0x1C]
	orr		a2, a2, #0x00100000
	bic		a2, a2, #0x00200000
	str		a2, [a1, #0x1C]
	nop
	nop

	; configure access to SDRAM (DRAM bank 0)
	ldr		a2, =0xAAAAAAA7
	str		a2, [a1, #0x04]
	ldr		a2, =0xAAAAAAAA
	str		a2, [a1, #0x08]
	ldr		a2, =0xAAAAAAAA
	str		a2, [a1, #0x0C]
	ldr		a2, =0x04340131
	str		a2, [a1, #0x1C]
	IF clockFrequency > 200000000
		ldr		a2, =0xB144B144 ; CAS latency of 3
	ELSE
		ldr		a2, =0xA144A144 ; CAS latency of 2
	ENDIF
	str		a2, [a1, #0x00]

	; do several refresh cycles to SDRAM
	ldr		a3, =0xC0000000
	mov		a4, #8
1	subs	a4, a4, #1
	str		a4, [a3]
	bne		%b1

	; enable SDRAM (DRAM bank 0)
	ldr		a2, [a1, #0x00]
	orr		a2, a2, #0x00000001
	str		a2, [a1, #0x00]

	; enable SDRAM auto power-down (DRAM bank 0)
	ldr		a2, [a1, #0x1C]
	orr		a2, a2, #0x30000000
	str		a2, [a1, #0x1C]

	ENDIF


	; initialization before MMU is enabled
	bl		getPreMmuStackTop
	mov		sp, a1
	bl		preMmuStartup


	; build memory address translation tables
	bl		buildAddressTranslationTables

	; Enable MMU and caches
	mcr		p15, 0, a1, c2, c0
	ldr		a1, =0x55555555
	mcr		p15, 0, a1, c3, c0
	IF {CPU} = "ARM920T"
	;  		       in                 IZFRSBLDPWCAM
	ldr		a1, =2_11000000000000000001000001111111
	ENDIF
	IF {CPU} = "SA-1100"
	;  		                          IZFRSBLDPWCAM
	ldr		a1, =2_00000000000000000001000001111111
	ENDIF
	mcr		p15, 0, a1, c1, c0

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


	; initialization after MMU is enabled
	bl		getPostMmuStackTop
	mov		sp, a1
	bl		postMmuStartup


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
;	a1 = corrupted
;	a2 = corrupted
;------------------------------------------------------------------------------------------------

	IF {CPU} = "ARM920T"

flushAllCaches
	mov		a1, #0
	mov		a2, #8
segmentLoop
indexLoop
	mcr		p15, 0, a1, c7, c14, 2
	adds	a1, a1, #0x04000000
	bcc		indexLoop
	subs	a2, a2, #1
	add		a1, a1, #0x00000020
	bne		segmentLoop
	mcr		p15, 0, a2, c7, c10, 4
	mcr		p15, 0, a2, c7, c5, 0
	mov		pc, lr

	ENDIF


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

	IF {CPU} = "SA-1100"

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

	ENDIF


	END
