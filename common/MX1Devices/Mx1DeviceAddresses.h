#ifndef _Mx1DeviceAddresses_h_
#define _Mx1DeviceAddresses_h_

enum
{
	// 1MB flash
	flashSize = 0x00100000,
	flashBase = 0x10000000,
	flashLimit = flashBase + flashSize,
	flashPhysicalBase = 0x10000000,
	flashPhysicalLimit = flashPhysicalBase + flashSize,


	// 8MB SDRAM
	sdramSize = 0x00800000,
	sdramBase = 0x00000000,
	sdramLimit = sdramBase + sdramSize,
	sdramPhysicalBase = 0x08000000,
	sdramPhysicalLimit = sdramPhysicalBase + sdramSize,


	// MX1 internal registers (156KB used but we map 1MB)
	mx1RegistersSize = 0x00100000,
	mx1RegistersBase = 0x20000000,
	mx1RegistersLimit = mx1RegistersBase + mx1RegistersSize,
	mx1RegistersPhysicalBase = 0x00200000,
	mx1RegistersPhysicalLimit = mx1RegistersPhysicalBase + mx1RegistersSize
};

#endif // _Mx1DeviceAddresses_h_
