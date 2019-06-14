#ifndef _Mx1UsbPort_h_
#define _Mx1UsbPort_h_

#include "../cPrimitiveTypes.h"
#include "../Communication/Stream.h"
#include "../multitasking/InterruptHandler.h"
#include "../multitasking/IntertaskEvent.h"
#include "../multitasking/Semaphore.h"
#include "../multitasking/Mutex.h"
#include "Mx1GpioPin.h"

//------------------------------------------------------------------------------------------------
// * class Mx1UsbPort
//
// Provides an interface to UDC.
//------------------------------------------------------------------------------------------------

class Mx1UsbPort :
	public Stream, 
	private InterruptHandler
{
public:
	// constructor
	Mx1UsbPort();
	
	// destructor
	~Mx1UsbPort();

	// error status
	inline Bool isInError() const;
	
	// streaming
	inline UInt read(void *pDestination, UInt length);
	UInt read(void *pDestination, UInt length, TimeValue timeout);
	inline UInt write(const void *pSource, UInt length);
	UInt write(const void *pSource, UInt length, TimeValue timeout);
	inline void flush();
	
	// configuration
	void configure();
	void usbReset();

	// error related
	void forceError();
	void reset();

private:
	// accessing
	inline void delay(UInt counter);
	inline UInt readRegister(UInt address);
	inline void writeRegister(UInt address, UInt value);
	
	// partial access
	inline void writeRegister8(UInt address, UInt8 value);
	inline void writeRegister16(UInt address, UInt16 value);

	// interrupt routine
	Bool handleInterrupt();
	Bool usbInterrupt();
	Bool ep0Interrupt();
	Bool ep1Interrupt();
	Bool ep2Interrupt();

	// error handling
	void handleError();
	
	// register accessing
	enum RegisterAddress
	{
		frame = 	0x00,
		spec = 		0x04,
		stat = 		0x08,
		ctrl = 		0x0C,
		dadr = 		0x10,
		ddat = 		0x14,
		intr = 		0x18,
		mask = 		0x1C,
		enab = 		0x24,
		ep0stat = 	0x30,
		ep0intr = 	0x34,
		ep0mask = 	0x38,
		ep0fdat = 	0x3C,
		ep0fstat = 	0x40,
		ep0fctrl = 	0x44,
		ep0lrfp = 	0x48,
		ep0lwfp = 	0x4C,
		ep0falrm = 	0x50,
		ep0frdp = 	0x54,
		ep0fwrp = 	0x58
	};

	// device specific
	UInt registerBase;
	Mx1GpioPin resetPin;

	// error state
	Bool inError;
	Bool inConfigured;
	IntertaskEvent synchronizationEvent;
	
	// types
	typedef struct
	{
		UInt8 requestType;
		UInt8 request;
		UInt16 value;
		UInt16 index;
		UInt16 length;
	} 
	SetupRequest;
	
	// receive state
	Mutex readMutex;
	UInt receiveLength;
	UInt8 *pReceiveBuffer;
	IntertaskEvent receiveEvent;

	UInt8 recvBuffer[0x1000];
	UInt recvBufferSize;
	
	// transmit parameters
	UInt transmitLength;
	UInt8 *pTransmitBuffer;
	IntertaskEvent transmitPacket;
	IntertaskEvent transmitEvent;
	Mutex writeMutex;
	
};

//------------------------------------------------------------------------------------------------
// * Mx1UsbPort::isInError
//
// Tests whether an error condition has occurred.
//------------------------------------------------------------------------------------------------

inline Bool Mx1UsbPort::isInError() const
{
	return inError;
}

//------------------------------------------------------------------------------------------------
// * Mx1UsbPort::read
//
// Read data from the stream.
//------------------------------------------------------------------------------------------------

inline UInt Mx1UsbPort::read(void *pDestination, UInt length)
{
	return read(pDestination, length, infiniteTime);
}

//------------------------------------------------------------------------------------------------
// * Mx1UsbPort::write
//
// Write data to the stream.
//------------------------------------------------------------------------------------------------

inline UInt Mx1UsbPort::write(const void *pSource, UInt length)
{
	return write(pSource, length, infiniteTime);
}

//------------------------------------------------------------------------------------------------
// * Mx1UsbPort::flush
//
// Sends any buffered data.
//------------------------------------------------------------------------------------------------

inline void Mx1UsbPort::flush()
{
}

//------------------------------------------------------------------------------------------------
// * Mx1UsbPort::delay
//
// Delay.
//------------------------------------------------------------------------------------------------

inline void Mx1UsbPort::delay(UInt counter)
{
	while (counter-- != 0);
}

//------------------------------------------------------------------------------------------------
// * Mx1UsbPort::readRegister
//
// Read a serial port register.
//------------------------------------------------------------------------------------------------

inline UInt Mx1UsbPort::readRegister(UInt address)
{
	const UInt realAddress = registerBase + address;
	return *(volatile UInt *)realAddress;
}

//------------------------------------------------------------------------------------------------
// * Mx1UsbPort::writeRegister
//
// Read a serial port register.
//------------------------------------------------------------------------------------------------

inline void Mx1UsbPort::writeRegister(UInt address, UInt value)
{
	const UInt realAddress = registerBase + address;
	*(volatile UInt *)realAddress = value;
}

//------------------------------------------------------------------------------------------------
// * Mx1UsbPort::writeRegister8
//
// Read a serial port register.
//------------------------------------------------------------------------------------------------

inline void Mx1UsbPort::writeRegister8(UInt address, UInt8 value)
{
	const UInt realAddress = registerBase + address;
	*(volatile UInt8 *)realAddress = value;
}

//------------------------------------------------------------------------------------------------
// * Mx1UsbPort::writeRegister16
//
// Read a serial port register.
//------------------------------------------------------------------------------------------------

inline void Mx1UsbPort::writeRegister16(UInt address, UInt16 value)
{
	const UInt realAddress = registerBase + address;
	*(volatile UInt16 *)realAddress = value;
}

#endif // _Mx1UsbPort_h_
