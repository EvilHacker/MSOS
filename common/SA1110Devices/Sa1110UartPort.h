#ifndef _Sa1110UartPort_h_
#define _Sa1110UartPort_h_

#include "../cPrimitiveTypes.h"
#include "../Communication/Stream.h"
#include "../multitasking/InterruptHandler.h"
#include "../multitasking/IntertaskEvent.h"

//------------------------------------------------------------------------------------------------
// * class Sa1110UartPort
//
// Provides an interface to SA1110 serial ports 1, 2, and 3.
//------------------------------------------------------------------------------------------------

class Sa1110UartPort :
	public Stream,
	private InterruptHandler
{
public:
	// constructor and destructor
	enum Port
	{
		port1,
		port2,
		port3
	};
	Sa1110UartPort(Port port);
	~Sa1110UartPort();

	// testing
	inline Bool isInError() const;

	// streaming
	inline UInt read(void *pDestination, UInt length);
	UInt read(void *pDestination, UInt length, TimeValue timeout);
	inline UInt write(const void *pSource, UInt length);
	UInt write(const void *pSource, UInt length, TimeValue timeout);
	inline void flush();

	// error related
	void forceError();
	void reset();

	// configuring
	void configure(
		UInt baudRate,
		UInt numberOfDataBits,
		UInt numberOfStopBits,
		Bool parityEnabled = false,
		Bool evenParity = false);

private:
	// accessing
	inline UInt getInterruptNumber();

	// interrupt handling
	void maskInterrupts();
	Bool handleInterrupt();

	// error handling
	void handleError();

	// register accessing
	enum RegisterAddress
	{
		utcr0 = 0x00,
		utcr1 = 0x04,
		utcr2 = 0x08,
		utcr3 = 0x0C,
		utdr0 = 0x14,
		utsr0 = 0x1C,
		utsr1 = 0x20
	};
	inline UInt readRegister(RegisterAddress address);
	inline void writeRegister(RegisterAddress address, UInt value);

	// representation
	Port port;
	UInt registerBase;

	// receive state
	UInt8 *pReceiveBuffer;
	UInt receiveLength;
	IntertaskEvent receiveEvent;

	// transmit state
	const UInt8 *pTransmitBuffer;
	UInt transmitLength;
	IntertaskEvent transmitEvent;

	// error state
	Bool inError;
	Bool synchronizing;
	IntertaskEvent synchronizationEvent;
};

//------------------------------------------------------------------------------------------------
// * Sa1110UartPort::isInError
//
// Tests whether an error condition has occurred.
//------------------------------------------------------------------------------------------------

inline Bool Sa1110UartPort::isInError() const
{
	return inError;
}

//------------------------------------------------------------------------------------------------
// * Sa1110UartPort::read
//
// Read data from the stream.
//------------------------------------------------------------------------------------------------

inline UInt Sa1110UartPort::read(void *pDestination, UInt length)
{
	return read(pDestination, length, infiniteTime);
}

//------------------------------------------------------------------------------------------------
// * Sa1110UartPort::write
//
// Write data to the stream.
//------------------------------------------------------------------------------------------------

inline UInt Sa1110UartPort::write(const void *pSource, UInt length)
{
	return write(pSource, length, infiniteTime);
}

//------------------------------------------------------------------------------------------------
// * Sa1110UartPort::flush
//
// Sends any buffered data.
//------------------------------------------------------------------------------------------------

inline void Sa1110UartPort::flush()
{
}

//------------------------------------------------------------------------------------------------
// * Sa1110UartPort::getInterruptNumber
//
// Returns the interrupt number used by this port.
//------------------------------------------------------------------------------------------------

inline UInt Sa1110UartPort::getInterruptNumber()
{
	// port1 - interrupt number 15
	// port2 - interrupt number 16
	// port3 - interrupt number 17
	return port + 15;
}

//------------------------------------------------------------------------------------------------
// * Sa1110UartPort::readRegister
//
// Read a serial port register.
//------------------------------------------------------------------------------------------------

inline UInt Sa1110UartPort::readRegister(RegisterAddress address)
{
	const UInt realAddress = registerBase + address;
	return *(volatile UInt *)realAddress;
}

//------------------------------------------------------------------------------------------------
// * Sa1110UartPort::writeRegister
//
// Read a serial port register.
//------------------------------------------------------------------------------------------------

inline void Sa1110UartPort::writeRegister(RegisterAddress address, UInt value)
{
	const UInt realAddress = registerBase + address;
	*(volatile UInt *)realAddress = value;
}

#endif // _Sa1110UartPort_h_
