#ifndef _Mx1UartPort_h_
#define _Mx1UartPort_h_

#include "../../cPrimitiveTypes.h"
#include "../../Communication/Stream.h"
#include "../../multitasking/InterruptHandler.h"
#include "../../multitasking/IntertaskEvent.h"

//------------------------------------------------------------------------------------------------
// * class Mx1UartPort
//
// Provides an interface to MX1 serial ports 1 and 2.
//------------------------------------------------------------------------------------------------

class Mx1UartPort :
	public Stream,
	private InterruptHandler
{
public:
	// constructor and destructor
	enum Port
	{
		port1,
		port2
	};
	Mx1UartPort(Port port, Bool synchronizeOnReset = true);
	~Mx1UartPort();

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
	inline UInt getTransmitterInterruptNumber();
	inline UInt getReceiverInterruptNumber();
	inline UInt getControlInterruptNumber();

	// interrupt handling
	void maskInterrupts();
	Bool handleInterrupt();

	// error handling
	void handleError();

	// register accessing
	enum RegisterAddress
	{
		urxd = 0x00,
		utxd = 0x40,
		ucr1 = 0x80,
		ucr2 = 0x84,
		ucr3 = 0x88,
		ucr4 = 0x8C,
		ufcr = 0x90,
		usr1 = 0x94,
		usr2 = 0x98,
		uesc = 0x9C,
		utim = 0xA0,
		ubir = 0xA4,
		ubmr = 0xA8,
		ubrc = 0xAC,
		bipr1 = 0xB0,
		bipr2 = 0xB4,
		bipr3 = 0xB8,
		bipr4 = 0xBC,
		bmpr1 = 0xC0,
		bmpr2 = 0xC4,
		bmpr3 = 0xC8,
		bmpr4 = 0xCC,
		uts = 0xD0
	};
	inline UInt readRegister(RegisterAddress address);
	inline void writeRegister(RegisterAddress address, UInt value);

	// representation
	Port port;
	UInt registerBase;
	Bool synchronizeOnReset;

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
// * Mx1UartPort::isInError
//
// Tests whether an error condition has occurred.
//------------------------------------------------------------------------------------------------

inline Bool Mx1UartPort::isInError() const
{
	return inError;
}

//------------------------------------------------------------------------------------------------
// * Mx1UartPort::read
//
// Read data from the stream.
//------------------------------------------------------------------------------------------------

inline UInt Mx1UartPort::read(void *pDestination, UInt length)
{
	return read(pDestination, length, infiniteTime);
}

//------------------------------------------------------------------------------------------------
// * Mx1UartPort::write
//
// Write data to the stream.
//------------------------------------------------------------------------------------------------

inline UInt Mx1UartPort::write(const void *pSource, UInt length)
{
	return write(pSource, length, infiniteTime);
}

//------------------------------------------------------------------------------------------------
// * Mx1UartPort::flush
//
// Sends any buffered data.
//------------------------------------------------------------------------------------------------

inline void Mx1UartPort::flush()
{
}

//------------------------------------------------------------------------------------------------
// * Mx1UartPort::getTransmitterInterruptNumber
//
// Returns an interrupt number used by this port.
//------------------------------------------------------------------------------------------------

inline UInt Mx1UartPort::getTransmitterInterruptNumber()
{
	// port1 - interrupt number 29
	// port2 - interrupt number 23
	static const UInt8 interruptNumbers[] = {29, 23};
	return interruptNumbers[port];
}

//------------------------------------------------------------------------------------------------
// * Mx1UartPort::getReceiverInterruptNumber
//
// Returns an interrupt number used by this port.
//------------------------------------------------------------------------------------------------

inline UInt Mx1UartPort::getReceiverInterruptNumber()
{
	// port1 - interrupt number 30
	// port2 - interrupt number 24
	static const UInt8 interruptNumbers[] = {30, 24};
	return interruptNumbers[port];
}

//------------------------------------------------------------------------------------------------
// * Mx1UartPort::getControlInterruptNumber
//
// Returns an interrupt number used by this port.
//------------------------------------------------------------------------------------------------

inline UInt Mx1UartPort::getControlInterruptNumber()
{
	// port1 - interrupt number 28
	// port2 - interrupt number 22
	static const UInt8 interruptNumbers[] = {28, 22};
	return interruptNumbers[port];
}

//------------------------------------------------------------------------------------------------
// * Mx1UartPort::readRegister
//
// Read a serial port register.
//------------------------------------------------------------------------------------------------

inline UInt Mx1UartPort::readRegister(RegisterAddress address)
{
	const UInt realAddress = registerBase + address;
	return *(volatile UInt *)realAddress;
}

//------------------------------------------------------------------------------------------------
// * Mx1UartPort::writeRegister
//
// Read a serial port register.
//------------------------------------------------------------------------------------------------

inline void Mx1UartPort::writeRegister(RegisterAddress address, UInt value)
{
	const UInt realAddress = registerBase + address;
	*(volatile UInt *)realAddress = value;
}

#endif // _Mx1UartPort_h_
