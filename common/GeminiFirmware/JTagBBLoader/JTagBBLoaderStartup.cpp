#include "cPrimitiveTypes.h"
#include "../devices/deviceAddresses.h"
#include "../../ArmRuntime/runtime.h"
#include "../../pointerArithmetic.h"
#define USING_ANGEL_DEBUGGER

#if defined(USING_ANGEL_DEBUGGER)
	// with the Angel debugger we reserve the upper 128K of SDRAM
	const UInt applicationLimit = sdramLimit - 128 * 1024;
#else
	// use all of SDRAM except the upper 16K used for MMU tables
	const UInt applicationLimit = sdramLimit - 16 * 1024;
#endif

//------------------------------------------------------------------------------------------------
// * getInitialStackTop
//
// Return the initial stack pointer.
// This function is kept sufficiently simple as to not use any stack space itself.
//------------------------------------------------------------------------------------------------

extern "C" UInt getInitialStackTop()
{
	// this stack is only temporary and is placed within the heap (at the end)
	// after the RTOS is started this stack may be corrupted by memory allocations from the heap
	// we assume that any memory allocations before the RTOS is started will not corrupt this stack
	// each task will have its own stack allocated to it and this initial stack will not be used
	return applicationLimit;
}

//------------------------------------------------------------------------------------------------
// * startup
//
// Perform any boot time initialization.
// Stack and MMU have already been initialized.
// Static variable constructors have not been called at this time.
//------------------------------------------------------------------------------------------------

extern "C" void startup()
{
	// mask all interrupts through interrupt controller
	*(volatile UInt32 *)(sa1110SystemControlBase + 0x50004) = 0;
	// enable interrupt masking during idle mode
	*(volatile UInt32 *)(sa1110SystemControlBase + 0x5000C) = 1;

	// tristate GPIO pins and disable and clear interrupts
	*(volatile UInt32 *)(sa1110SystemControlBase + 0x4001C) = 0;
	*(volatile UInt32 *)(sa1110SystemControlBase + 0x40010) = 0;
	*(volatile UInt32 *)(sa1110SystemControlBase + 0x40014) = 0;
	*(volatile UInt32 *)(sa1110SystemControlBase + 0x40004) = 0;
	*(volatile UInt32 *)(sa1110SystemControlBase + 0x40018) = ~0;

	// disable and clear real-time clock interrupts
	*(volatile UInt32 *)(sa1110SystemControlBase + 0x10010) = 0x0;
	*(volatile UInt32 *)(sa1110SystemControlBase + 0x10010) = 0x3;

	// disable and clear OS timer interrupts
	*(volatile UInt32 *)(sa1110SystemControlBase + 0x0001C) = 0x0;
	*(volatile UInt32 *)(sa1110SystemControlBase + 0x00014) = 0xF;

	// disable and clear DMA interrupts on all 6 channels
	*(volatile UInt32 *)(sa1110LcdAndDmaControlBase + 0x000004) = 0x00;
	*(volatile UInt32 *)(sa1110LcdAndDmaControlBase + 0x000008) = 0x7F;
	*(volatile UInt32 *)(sa1110LcdAndDmaControlBase + 0x000024) = 0x00;
	*(volatile UInt32 *)(sa1110LcdAndDmaControlBase + 0x000028) = 0x7F;
	*(volatile UInt32 *)(sa1110LcdAndDmaControlBase + 0x000044) = 0x00;
	*(volatile UInt32 *)(sa1110LcdAndDmaControlBase + 0x000048) = 0x7F;
	*(volatile UInt32 *)(sa1110LcdAndDmaControlBase + 0x000064) = 0x00;
	*(volatile UInt32 *)(sa1110LcdAndDmaControlBase + 0x000068) = 0x7F;
	*(volatile UInt32 *)(sa1110LcdAndDmaControlBase + 0x000084) = 0x00;
	*(volatile UInt32 *)(sa1110LcdAndDmaControlBase + 0x000088) = 0x7F;
	*(volatile UInt32 *)(sa1110LcdAndDmaControlBase + 0x0000A4) = 0x00;
	*(volatile UInt32 *)(sa1110LcdAndDmaControlBase + 0x0000A8) = 0x7F;

	// LCD is already disabled at power up

	// disable serial port 0 (USB)
	*(volatile UInt32 *)(sa1110PeripheralControlBase + 0x00000) = 0xF9;

	#if !defined(USING_ANGEL_DEBUGGER)
		// disable serial port 1 (GPCLK/UART)
		*(volatile UInt32 *)(sa1110PeripheralControlBase + 0x20060) = 0;
		*(volatile UInt32 *)(sa1110PeripheralControlBase + 0x1000C) = 0;
	#endif

	// disable serial port 2 (HSSP/UART)
	*(volatile UInt32 *)(sa1110PeripheralControlBase + 0x40060) = 0;
	*(volatile UInt32 *)(sa1110PeripheralControlBase + 0x3000C) = 0;

	// disable serial port 3 (UART)
	*(volatile UInt32 *)(sa1110PeripheralControlBase + 0x5000C) = 0;

	// disable serial port 4 (MCP/SSP)
	*(volatile UInt32 *)(sa1110PeripheralControlBase + 0x60000) = 0;
	*(volatile UInt32 *)(sa1110PeripheralControlBase + 0x70064) = 0;

	// initialize the heap so that operator new() and operator delete() can be used
	extern UInt8 Image$$sdramZero$$ZI$$Base;
	extern UInt8 Image$$sdramZero$$ZI$$Length;
	initializeHeap(
		(UInt)&Image$$sdramZero$$ZI$$Base + (UInt)&Image$$sdramZero$$ZI$$Length,
		applicationLimit);
}
