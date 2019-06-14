#ifndef _Mx1I2cPort_h_
#define _Mx1I2cPort_h_

#include "../cPrimitiveTypes.h"
#include "../multitasking/InterruptHandler.h"
#include "../multitasking/IntertaskEvent.h"
#include "../multitasking/Mutex.h"
#include "Mx1DeviceAddresses.h"

//------------------------------------------------------------------------------------------------
// * class Mx1I2cPort
//
// Provides an interface to the MX1 I2C port.
//------------------------------------------------------------------------------------------------

class Mx1I2cPort :
	private InterruptHandler
{
public:
	// constructor and destructor
	Mx1I2cPort();
	~Mx1I2cPort();

	// streaming
	UInt read(UInt address, void *pDestination, UInt length, TimeValue timeout);
	UInt write(UInt address, const void *pSource, UInt length, TimeValue timeout);

	// configuring
	void configure(UInt baudRate);

private:
	// accessing
	inline UInt getInterruptNumber();

	// interrupt handling
	Bool handleInterrupt();

	// register accessing
	enum RegisterAddress
	{
		iadr = mx1RegistersBase + 0x17000,
		ifdr = mx1RegistersBase + 0x17004,
		i2cr = mx1RegistersBase + 0x17008,
		i2sr = mx1RegistersBase + 0x1700C,
		i2dr = mx1RegistersBase + 0x17010
	};
	inline UInt readRegister(RegisterAddress address);
	inline void writeRegister(RegisterAddress address, UInt value);

	// transfer state
	Mutex transferMutex;
	Bool receiving;
	Bool firstRead;
	UInt8 *pTransferBuffer;
	UInt transferLength;
	IntertaskEvent transferEvent;
};

//------------------------------------------------------------------------------------------------
// * Mx1I2cPort::getInterruptNumber
//
// Returns the interrupt number used by this port.
//------------------------------------------------------------------------------------------------

inline UInt Mx1I2cPort::getInterruptNumber()
{
	return 39;
}

//------------------------------------------------------------------------------------------------
// * Mx1I2cPort::readRegister
//
// Read an I2C register.
//------------------------------------------------------------------------------------------------

inline UInt Mx1I2cPort::readRegister(RegisterAddress address)
{
	return *(volatile UInt *)address;
}

//------------------------------------------------------------------------------------------------
// * Mx1I2cPort::writeRegister
//
// Write an I2C register.
//------------------------------------------------------------------------------------------------

inline void Mx1I2cPort::writeRegister(RegisterAddress address, UInt value)
{
	*(volatile UInt *)address = value;
}

#endif // _Mx1I2cPort_h_
