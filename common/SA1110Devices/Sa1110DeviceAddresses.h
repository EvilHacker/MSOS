#ifndef _Sa1110DeviceAddresses_h_
#define _Sa1110DeviceAddresses_h_

enum
{
	// 1MB flash
	flashSize = 0x00100000,
	flashBase = 0x04000000,
	flashLimit = flashBase + flashSize,
	flashPhysicalBase = 0x00000000,
	flashPhysicalLimit = flashPhysicalBase + flashSize,


	// 8MB SDRAM
	sdramSize = 0x00800000,
	sdramBase = 0x00000000,
	sdramLimit = sdramBase + sdramSize,
	sdramPhysicalBase = 0xC0000000,
	sdramPhysicalLimit = sdramPhysicalBase + sdramSize,


	// SA-1110 peripheral control registers
	sa1110PeripheralControlSize = 0x00100000,
	sa1110PeripheralControlBase = 0x80000000,
	sa1110PeripheralControlLimit = sa1110PeripheralControlBase + sa1110PeripheralControlSize,

	// SA-1110 system control registers
	sa1110SystemControlSize = 0x00100000,
	sa1110SystemControlBase = 0x90000000,
	sa1110SystemControlLimit = sa1110SystemControlBase + sa1110SystemControlSize,

	// SA-1110 memory control registers
	sa1110MemoryControlSize = 0x00100000,
	sa1110MemoryControlBase = 0xA0000000,
	sa1110MemoryControlLimit = sa1110MemoryControlBase + sa1110MemoryControlSize,

	// SA-1110 LCD and DMA control registers
	sa1110LcdAndDmaControlSize = 0x00200000,
	sa1110LcdAndDmaControlBase = 0xB0000000,
	sa1110LcdAndDmaControlLimit = sa1110LcdAndDmaControlBase + sa1110LcdAndDmaControlSize,

	// SA-1110 cache flush space
	sa1110CacheFlushSize = 0x00100000,
	sa1110CacheFlushBase = 0xE0000000,
	sa1110CacheFlushLimit = sa1110CacheFlushBase + sa1110CacheFlushSize,


	// Vision Processor ASIC registers
	visionProcessorSize = 0x00100000,
	visionProcessorBase = 0x08000000,
	visionProcessorLimit = visionProcessorBase + visionProcessorSize,


	// USB 2 controller registers
	usb2ControllerSize = 0x00100000,
	usb2ControllerBase = 0x48000000,
	usb2ControllerCachedBase = usb2ControllerBase + usb2ControllerSize,
	usb2ControllerLimit = usb2ControllerCachedBase + usb2ControllerSize
};

#endif // _Sa1110DeviceAddresses_h_
