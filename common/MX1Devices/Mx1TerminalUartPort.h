#ifndef _Mx1TerminalUartPort_h_
#define _Mx1TerminalUartPort_h_

#include "../../cPrimitiveTypes.h"
#include "../communication/Stream.h"
#include "../../multitasking/InterruptHandler.h"
#include "../../multitasking/IntertaskEvent.h"

//------------------------------------------------------------------------------------------------
// * class Mx1TerminalUartPort
//
// Provides an interface to Mx1 serial ports 1 and 2.
//------------------------------------------------------------------------------------------------

class Mx1TerminalUartPort :
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
	Mx1TerminalUartPort(Port port, UInt baudRate);
	~Mx1TerminalUartPort();
	
	// error report
	inline Bool isInError() const;

	// streaming
	inline UInt read(void *pDestination, UInt length);
	UInt read(void *pDestination, UInt length, TimeValue timeout);
	inline UInt write(const void *pSource, UInt length);
	UInt write(const void *pSource, UInt length, TimeValue timeout);
	
	// not implemented
	inline void forceError() {};
	inline void reset() {};
	inline void flush() {};

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
	void maskInterrupts(const Bool readEnabled = true);
	Bool handleInterrupt();

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

	// receive state
	UInt8 *pReceiveBuffer;
	UInt receiveLength;
	IntertaskEvent receiveEvent;

	UInt recvReadIndex;
	UInt recvBufferSize;
	UInt8 recvBuffer[10000];

	// transmit state
	const UInt8 *pTransmitBuffer;
	UInt transmitLength;
	IntertaskEvent transmitEvent;
	
	// ASCII mode operation support
	Bool inError;
};

//------------------------------------------------------------------------------------------------
// * Mx1TerminalUartPort::isInError
//
// Return last IO operation completion status
//------------------------------------------------------------------------------------------------

inline Bool Mx1TerminalUartPort::isInError() const
{
	return inError;
}

//------------------------------------------------------------------------------------------------
// * Mx1TerminalUartPort::read
//
// Read data from the stream.
//------------------------------------------------------------------------------------------------

inline UInt Mx1TerminalUartPort::read(void *pDestination, UInt length)
{
	return read(pDestination, length, infiniteTime);
}

//------------------------------------------------------------------------------------------------
// * Mx1TerminalUartPort::write
//
// Write data to the stream.
//------------------------------------------------------------------------------------------------

inline UInt Mx1TerminalUartPort::write(const void *pSource, UInt length)
{
	return write(pSource, length, infiniteTime);
}

//------------------------------------------------------------------------------------------------
// * Mx1TerminalUartPort::getTransmitterInterruptNumber
//
// Returns an interrupt number used by this port.
//------------------------------------------------------------------------------------------------

inline UInt Mx1TerminalUartPort::getTransmitterInterruptNumber()
{
	// port1 - interrupt number 29
	// port2 - interrupt number 23
	static const UInt8 interruptNumbers[] = {29, 23};
	return interruptNumbers[port];
}

//------------------------------------------------------------------------------------------------
// * Mx1TerminalUartPort::getReceiverInterruptNumber
//
// Returns an interrupt number used by this port.
//------------------------------------------------------------------------------------------------

inline UInt Mx1TerminalUartPort::getReceiverInterruptNumber()
{
	// port1 - interrupt number 30
	// port2 - interrupt number 24
	static const UInt8 interruptNumbers[] = {30, 24};
	return interruptNumbers[port];
}

//------------------------------------------------------------------------------------------------
// * Mx1TerminalUartPort::getControlInterruptNumber
//
// Returns an interrupt number used by this port.
//------------------------------------------------------------------------------------------------

inline UInt Mx1TerminalUartPort::getControlInterruptNumber()
{
	// port1 - interrupt number 28
	// port2 - interrupt number 22
	static const UInt8 interruptNumbers[] = {28, 22};
	return interruptNumbers[port];
}

//------------------------------------------------------------------------------------------------
// * Mx1TerminalUartPort::readRegister
//
// Read a serial port register.
//------------------------------------------------------------------------------------------------

inline UInt Mx1TerminalUartPort::readRegister(RegisterAddress address)
{
	const UInt realAddress = registerBase + address;
	return *(volatile UInt *)realAddress;
}

//------------------------------------------------------------------------------------------------
// * Mx1TerminalUartPort::writeRegister
//
// Read a serial port register.
//------------------------------------------------------------------------------------------------

inline void Mx1TerminalUartPort::writeRegister(RegisterAddress address, UInt value)
{
	const UInt realAddress = registerBase + address;
	*(volatile UInt *)realAddress = value;
}

#endif // _Mx1TerminalUartPort_h_
