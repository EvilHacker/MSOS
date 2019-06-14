#ifndef _Usb2Port_h_
#define _Usb2Port_h_

#include "../cPrimitiveTypes.h"
#include "../Communication/Stream.h"
#include "../multitasking/InterruptHandler.h"
#include "../multitasking/IntertaskEvent.h"
#include "../multitasking/Mutex.h"
#include "../multitasking/MemberTask.h"
#include "../Sa1110Devices/Sa1110GpioPin.h"

#define	DSCR_DEVICE			1   /* Descriptor type: Device */
#define	DSCR_CONFIG			2   /* Descriptor type: Configuration */
#define	DSCR_STRING			3   /* Descriptor type: String */
#define	DSCR_INTRFC			4   /* Descriptor type: Interface */
#define	DSCR_ENDPNT			5   /* Descriptor type: Endpoint */
#define	DSCR_DEVQUAL		6   /* Descriptor type: Device Qualifier */

#define	DSCR_DEVICE_LEN		18
#define	DSCR_CONFIG_LEN		9
#define	DSCR_INTRFC_LEN		9
#define	DSCR_ENDPNT_LEN		7
#define	DSCR_DEVQUAL_LEN	10

#define	ET_BULK				2   /* Endpoint type: Bulk */

//------------------------------------------------------------------------------------------------
// * class Usb2Port
//
// Provides an interface to a CY7C68001 USB II controller.
//------------------------------------------------------------------------------------------------

class Usb2Port :
	public Stream,
	private InterruptHandler
{
public:
	// constructor and destructor
	Usb2Port();
	virtual ~Usb2Port();

	// testing
	inline Bool isInError() const;

	// streaming
	UInt read(void *pDestination, UInt length);
	inline UInt read(void *pDestination, UInt length, TimeValue timeout);
	UInt write(const void *pSource, UInt length);
	inline UInt write(const void *pSource, UInt length, TimeValue timeout);
	inline void flush();

	// error related
	void forceError();
	void reset();
	
private:
	// interrupt handling
	Bool handleInterrupt();
	void handleFlagAInterrupt();
	void handleFlagBInterrupt();
	void handleIntFlagInterrupt();
	
	Sa1110GpioPin wakeUpPin;
	Sa1110GpioPin flagAInterruptPin;
	Sa1110GpioPin flagBInterruptPin;
	Sa1110GpioPin flagCInterruptPin;
	Sa1110GpioPin intFlagInterruptPin;
	
	// device configuration
	void configureRegisters();
	void verifySettingsWithHubSpeed();
	void writeDescriptor();
	
	// error handling
	void handleError();

	enum PacketSize
	{
		fullSpeedPacketSizeLsb = 0x40,
		fullSpeedPacketSizeMsb = 0x00,
		highSpeedPacketSizeLsb = 0x00,
		highSpeedPacketSizeMsb = 0x02
	};
	
	enum AccessAddresses
	{
		baseFifo2Address	= usb2ControllerCachedBase + 0x0000,
//		baseFifo2Address	= usb2ControllerBase + 0x0000,
		baseFifo4Address	= usb2ControllerBase + 0x1000,
		baseFifo6Address	= usb2ControllerCachedBase + 0x2000,
//		baseFifo6Address	= usb2ControllerBase + 0x2000,
		baseFifo8Address	= usb2ControllerBase + 0x3000,
		baseCommandAddress	= usb2ControllerBase + 0x4000
	};

	// register accessing
	enum RegisterAddress
	{
		// General Configuration
		ifconfig		= 0x01,
		flagsab			= 0x02,
		flagscd			= 0x03,
		polar			= 0x04,
		revid			= 0x05,

		// Endpoint Configuration
		ep2cfg			= 0x06,
		ep4cfg			= 0x07,
		ep6cfg			= 0x08,
		ep8cfg			= 0x09,
		ep2pktlenh		= 0x0a,
		ep2pktlenl		= 0x0b,
		ep4pktlenh		= 0x0c,
		ep4pktlenl		= 0x0d,
		ep6pktlenh		= 0x0e,
		ep6pktlenl		= 0x0f,
		ep8pktlenh		= 0x10,
		ep8pktlenl		= 0x11,
		ep2pfh			= 0x12,
		ep2pfl			= 0x13,
		ep4pfh			= 0x14,
		ep4pfl			= 0x15,
		ep6pfh			= 0x16,
		ep6pfl			= 0x17,
		ep8pfh			= 0x18,
		ep8pfl			= 0x19,
		ep2isoinpkts	= 0x1a,
		ep4isoinpkts	= 0x1b,
		ep6isoinpkts	= 0x1c,
		ep8isoinpkts	= 0x1d,
		
		// Flags
		ep24flags		= 0x1e,
		ep68flags		= 0x1f,
		
		// Inpktend / flush
		inpktendflush	= 0x20,
		
		// Usb configuration
		usbframeh		= 0x2a,
		usbframel		= 0x2b,
		microframe		= 0x2c,
		fnaddr			= 0x2d,
		
		// Interrupts
		intenable		= 0x2e,
		
		// Descriptor RAM
		desc			= 0x30, // size = 500 bytes
		
		// Endpoint 0
		ep0buf			= 0x31, // size = 64 bytes
		setup			= 0x32, // size = 8 bytes
		ep0bc			= 0x33
	};

	// read register interrupt event
	IntertaskEvent readEvent;
	Bool readRequestEnabled;
	UInt readRegisterValue;
	UInt packetSize;
	
	// registers access
	UInt readRegister(RegisterAddress address);
	void writeRegister(RegisterAddress address, UInt value);
	inline void delay(const int usDelay = 1);

	UInt8 setupBuffer[8];
	UInt setupDataCounter;
	bool setupRequestEnabled;

	// receive state
	Mutex readMutex;
	UInt receiveLength;
	UInt8 *pReceiveBuffer;
	IntertaskEvent receiveEvent;

	UInt8 recvBuffer[0x1000];
	UInt recvBufferSize;
	
	// transmit state
	Mutex writeMutex;
	UInt transmitLength;
	UInt writeBlockSize;
	const UInt8 *pTransmitBuffer;
	IntertaskEvent transmitFifo2Empty;
	IntertaskEvent transmitFifo6Empty;

	// suspend mode
	Bool suspended;

	// error state
	Bool inError;
	IntertaskEvent synchronizationEvent;
};

//------------------------------------------------------------------------------------------------
// * Usb2Port::isInError
//
// Tests whether an error condition has occurred.
//------------------------------------------------------------------------------------------------

inline Bool Usb2Port::isInError() const
{
	return inError;
}

//------------------------------------------------------------------------------------------------
// * Usb2Port::read
//
// Read data from the stream.
//------------------------------------------------------------------------------------------------

UInt Usb2Port::read(void *pDestination, UInt length, TimeValue timeout)
{
	return read(pDestination, length);
}

//------------------------------------------------------------------------------------------------
// * Usb2Port::write
//
// Write data to the stream.
//------------------------------------------------------------------------------------------------

UInt Usb2Port::write(const void *pSource, UInt length, TimeValue timeout)
{
	return write(pSource, length);
}

//------------------------------------------------------------------------------------------------
// * Usb2Port::flush
//
// Sends any buffered data.
//------------------------------------------------------------------------------------------------

inline void Usb2Port::flush()
{
}

//------------------------------------------------------------------------------------------------
// * Usb2Port::delay
//
// Delay in us
//------------------------------------------------------------------------------------------------

inline void Usb2Port::delay(const int usDelay)
{ 
	// delay
	const int d = 56 * usDelay; // clocks
	for (int c = 0; c < d; c++);
}

#endif // _Usb2Port_h_
