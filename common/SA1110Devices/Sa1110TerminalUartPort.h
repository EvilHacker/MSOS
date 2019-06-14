#ifndef _Sa1110TerminalUartPort_h_
#define _Sa1110TerminalUartPort_h_

#include "../../cPrimitiveTypes.h"
#include "../communication/Stream.h"
#include "../../multitasking/InterruptHandler.h"
#include "../../multitasking/IntertaskEvent.h"

//------------------------------------------------------------------------------------------------
// * class Sa1110TerminalUartPort
//
// Provides an interface to SA1110 serial ports 1, 2, and 3.
//------------------------------------------------------------------------------------------------

class Sa1110TerminalUartPort :
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
	Sa1110TerminalUartPort(Port port, UInt baudRate);
	~Sa1110TerminalUartPort();
	
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
	inline UInt getInterruptNumber();

	// interrupt handling
	void maskInterrupts(const Bool readEnabled = true);
	Bool handleInterrupt();

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
// * Sa1110TerminalUartPort::isInError
//
// Return last IO operation completion status
//------------------------------------------------------------------------------------------------

inline Bool Sa1110TerminalUartPort::isInError() const
{
	return inError;
}

//------------------------------------------------------------------------------------------------
// * Sa1110TerminalUartPort::read
//
// Read data from the stream.
//------------------------------------------------------------------------------------------------

inline UInt Sa1110TerminalUartPort::read(void *pDestination, UInt length)
{
	return read(pDestination, length, infiniteTime);
}

//------------------------------------------------------------------------------------------------
// * Sa1110TerminalUartPort::write
//
// Write data to the stream.
//------------------------------------------------------------------------------------------------

inline UInt Sa1110TerminalUartPort::write(const void *pSource, UInt length)
{
	return write(pSource, length, infiniteTime);
}

//------------------------------------------------------------------------------------------------
// * Sa1110TerminalUartPort::getInterruptNumber
//
// Returns the interrupt number used by this port.
//------------------------------------------------------------------------------------------------

inline UInt Sa1110TerminalUartPort::getInterruptNumber()
{
	// port1 - interrupt number 15
	// port2 - interrupt number 16
	// port3 - interrupt number 17
	return port + 15;
}

//------------------------------------------------------------------------------------------------
// * Sa1110TerminalUartPort::readRegister
//
// Read a serial port register.
//------------------------------------------------------------------------------------------------

inline UInt Sa1110TerminalUartPort::readRegister(RegisterAddress address)
{
	const UInt realAddress = registerBase + address;
	return *(volatile UInt *)realAddress;
}

//------------------------------------------------------------------------------------------------
// * Sa1110TerminalUartPort::writeRegister
//
// Read a serial port register.
//------------------------------------------------------------------------------------------------

inline void Sa1110TerminalUartPort::writeRegister(RegisterAddress address, UInt value)
{
	const UInt realAddress = registerBase + address;
	*(volatile UInt *)realAddress = value;
}

#endif // _TerminalUartPort_h_
