#include "cPrimitiveTypes.h"
#include "../../devices/deviceAddresses.h"
#include "../../ArmDevices/AddressTranslationTableBuilder.h"
#include "../../ArmRuntime/runtime.h"

// use all of SDRAM except the upper 16K used for MMU tables
const UInt applicationLimit = sdramLimit - 16 * 1024;

//------------------------------------------------------------------------------------------------
// * getPreMmuStackTop
//
// Return the initial stack pointer.
// This function is kept sufficiently simple as to not use any stack space itself.
//------------------------------------------------------------------------------------------------

extern "C" UInt getPreMmuStackTop()
{
	// this stack is only temporary
	return sdramPhysicalBase + 0x100000;
}

//------------------------------------------------------------------------------------------------
// * preMmuStartup
//
// Perform any boot time initialization.
// The MMU has not yet been initialized.
// The heap has not been initialized, do not use new and delete.
// Static variable constructors have not been called at this time.
// This code will run directly from within its load region.
// There should be no external references to code or data in an execution region.
// Global variables should not be referenced. External functions should not be called.
//------------------------------------------------------------------------------------------------

extern "C" void preMmuStartup()
{
}

//------------------------------------------------------------------------------------------------
// * buildAddressTranslationTables
//
// Build the level 1 and level 2 MMU address translation tables.
// The base address of the level 1 table is returned.
// The MMU has not yet been initialized.
// The heap has not been initialized, do not use new and delete.
// Static variable constructors have not been called at this time.
// This code will run directly from within its load region.
// There should be no external references to code or data in an execution region.
// Global variables should not be referenced. External functions should not be called.
//------------------------------------------------------------------------------------------------

extern "C" void *buildAddressTranslationTables()
{
	#if defined(__TARGET_CPU_ARM920T)
		// put L1 and L2 translation tables at the end of SDRAM
		AddressTranslationTableBuilder ttb((void *)sdramPhysicalLimit, false);
		const UInt publicDomain = 0;

		// map external devices
		ttb.map(sdramBase, sdramPhysicalBase, sdramSize,
			publicDomain, AddressTranslationTableBuilder::readWriteAccessPermission, true, true);
		ttb.map(flashBase, flashPhysicalBase, flashSize,
			publicDomain, AddressTranslationTableBuilder::readWriteAccessPermission, false, false);

		// map MX1 internal register spaces
		ttb.map(mx1RegistersBase, mx1RegistersPhysicalBase, mx1RegistersSize,
			publicDomain, AddressTranslationTableBuilder::readWriteAccessPermission, false, false);

		return ttb.getL1Table();
	#endif

	#if defined(__TARGET_CPU_SA_1100)
		// put L1 and L2 translation tables at the end of SDRAM
		AddressTranslationTableBuilder ttb((void *)sdramPhysicalLimit, false);
		const UInt publicDomain = 0;

		// map external devices
		ttb.map(sdramBase, sdramPhysicalBase, sdramSize,
			publicDomain, AddressTranslationTableBuilder::readWriteAccessPermission, true, true);
		ttb.map(flashBase, flashPhysicalBase, flashSize,
			publicDomain, AddressTranslationTableBuilder::readWriteAccessPermission, false, false);
		ttb.map(visionProcessorBase, visionProcessorBase, visionProcessorSize,
			publicDomain, AddressTranslationTableBuilder::readWriteAccessPermission, false, false);
		ttb.map(usb2ControllerBase, usb2ControllerBase, usb2ControllerSize,
			publicDomain, AddressTranslationTableBuilder::readWriteAccessPermission, false, false);
		ttb.map(usb2ControllerCachedBase, usb2ControllerCachedBase, usb2ControllerSize,
			publicDomain, AddressTranslationTableBuilder::readWriteAccessPermission, true, true);

		// map SA-1110 internal register spaces
		ttb.map(sa1110PeripheralControlBase, sa1110PeripheralControlBase, sa1110PeripheralControlSize,
			publicDomain, AddressTranslationTableBuilder::readWriteAccessPermission, false, false);
		ttb.map(sa1110SystemControlBase, sa1110SystemControlBase, sa1110SystemControlSize,
			publicDomain, AddressTranslationTableBuilder::readWriteAccessPermission, false, false);
		ttb.map(sa1110MemoryControlBase, sa1110MemoryControlBase, sa1110MemoryControlSize,
			publicDomain, AddressTranslationTableBuilder::readWriteAccessPermission, false, false);
		ttb.map(sa1110LcdAndDmaControlBase, sa1110LcdAndDmaControlBase, sa1110LcdAndDmaControlSize,
			publicDomain, AddressTranslationTableBuilder::readWriteAccessPermission, false, false);
		ttb.map(sa1110CacheFlushBase, sa1110CacheFlushBase, sa1110CacheFlushSize,
			publicDomain, AddressTranslationTableBuilder::readWriteAccessPermission, true, true);
	
		return ttb.getL1Table();
	#endif
}

//------------------------------------------------------------------------------------------------
// * getPostMmuStackTop
//
// Return the initial stack pointer.
// This function is kept sufficiently simple as to not use any stack space itself.
//------------------------------------------------------------------------------------------------

extern "C" UInt getPostMmuStackTop()
{
	// this stack is only temporary and is placed within the heap (at the end)
	// after the RTOS is started this stack may be corrupted by memory allocations from the heap
	// we assume that any memory allocations before the RTOS is started will not corrupt this stack
	// each task will have its own stack allocated to it and this initial stack will not be used
	return applicationLimit;
}

//------------------------------------------------------------------------------------------------
// * postMmuStartup
//
// Perform any boot time initialization.
// The MMU has already been initialized.
// Static variable constructors have not been called at this time.
//------------------------------------------------------------------------------------------------

extern "C" void postMmuStartup()
{
	#if defined(__TARGET_CPU_ARM920T)
		// disable all interrupts
		*(volatile UInt *)(mx1RegistersBase + 0x23010) = 0;
		*(volatile UInt *)(mx1RegistersBase + 0x23014) = 0;
	#endif

	#if defined(__TARGET_CPU_SA_1100)
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

		// complete reset of the Vision Processor by giving it several clocks pulses
		for(UInt vpClockPulseCount = 0; vpClockPulseCount < 64; ++vpClockPulseCount)
		{
			// delay and enable output on GPIO pin 26
			*(volatile UInt32 *)(sa1110SystemControlBase + 0x40004) = 1 << 26;

			// clock high
			*(volatile UInt32 *)(sa1110SystemControlBase + 0x40008) = 1 << 26;

			// delay
			*(volatile UInt32 *)(sa1110SystemControlBase + 0x40004) = 1 << 26;

			// clock low
			*(volatile UInt32 *)(sa1110SystemControlBase + 0x4000C) = 1 << 26;
		}

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

		// disable serial port 1 (GPCLK/UART)
		*(volatile UInt32 *)(sa1110PeripheralControlBase + 0x20060) = 0;
		*(volatile UInt32 *)(sa1110PeripheralControlBase + 0x1000C) = 0;

		// disable serial port 2 (HSSP/UART)
		*(volatile UInt32 *)(sa1110PeripheralControlBase + 0x40060) = 0;
		*(volatile UInt32 *)(sa1110PeripheralControlBase + 0x3000C) = 0;

		// disable serial port 3 (UART)
		*(volatile UInt32 *)(sa1110PeripheralControlBase + 0x5000C) = 0;

		// disable serial port 4 (MCP/SSP)
		*(volatile UInt32 *)(sa1110PeripheralControlBase + 0x60000) = 0;
		*(volatile UInt32 *)(sa1110PeripheralControlBase + 0x70064) = 0;
	#endif

	// initialize the heap so that operator new() and operator delete() can be used
	extern UInt8 Image$$sdramZero$$ZI$$Base;
	extern UInt8 Image$$sdramZero$$ZI$$Length;
	initializeHeap(
		(UInt)&Image$$sdramZero$$ZI$$Base + (UInt)&Image$$sdramZero$$ZI$$Length,
		applicationLimit);
}
