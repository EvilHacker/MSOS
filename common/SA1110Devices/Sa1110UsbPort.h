#ifndef _Sa1110UsbPort_h_
#define _Sa1110UsbPort_h_

#include "../cPrimitiveTypes.h"
#include "../Communication/Stream.h"
#include "../multitasking/InterruptHandler.h"
#include "../multitasking/IntertaskEvent.h"
#include "../multitasking/Semaphore.h"
#include "../multitasking/Mutex.h"
#include "Sa1110DeviceAddresses.h"
#include "Sa1110UsbStd.h"
#include "Sa1110GpioPin.h"


//------------------------------------------------------------------------------------------------
// * class Sa1110UsbPort
//
// Provides an interface to UDC.
//------------------------------------------------------------------------------------------------

class Sa1110UsbPort :
	public Stream, 
	private InterruptHandler
{
public:
	// constructor
	Sa1110UsbPort();
	
	// destructor
	~Sa1110UsbPort();
	
	// testing
	inline Bool isInError() const;
	inline Bool isInSync() const;

	// streaming
	UInt read(void *pDestination, UInt length);
	inline UInt read(void *pDestination, UInt length, TimeValue timeout);
	UInt write(const void *pSource, UInt length);
	inline UInt write(const void *pSource, UInt length, TimeValue timeout);
	inline void flush();

	// error related
	void forceError();
	void resetController();
	void reset();

private:
	
	// interrupt handling
	Bool handleInterrupt();
	Bool proceedInterrupt();
	void recycleConnection();

	// error handling
	void handleError();
	void funcControl();
	void funcResync();
		
	// variables
	void * pDmaBuffers;	
	
	// receive state
	UInt receiveBank;
	UInt receiveLength;
	UInt8 *pReceiveBuffer;
	IntertaskEvent receiveEvent;
	Mutex readMutex;
	UInt8 *dmaRecvBuffer;
	UInt dmaRecvReadIndex;
	UInt dmaRecvBufferSize;
	
	void funcReceive();
	void initReceiveDMA();
	
	// receive on ep0 state
	UInt receivePacketSize;
	
	// transmit parameters
	UInt transmitBank;
	UInt transmitLength;
	UInt transmitFifoDataLength;
	UInt8 *pTransmitBuffer;
	IntertaskEvent transmitEvent;
	Mutex writeMutex;
	UInt8 *dmaTransBuffer;
	
	void funcTransmit();
	void initSendDMA();

	// synchrinization parameters
	IntertaskEvent synchronizationEvent;
	
	// transfer
	void getDescriptor();
	void readSetupData();
	void sendDataEndPoint0(const UInt8 *buffer, UInt size);
	void continueSendingData();
	void xferProcess();

	// error state
	Bool askMore;
	Bool inError;
	UInt8 configurationNum;
	
	// initialization stages
	enum InitStatus
	{
		initNone	= 0,
		initStart	= 1,
		initLoad	= 2
	};
	
	InitStatus initStatus;
	
	enum Ep0State
	{
		ep0Idle 	= 0,
		ep0Indata	= 1,
		ep0Xfer		= 2
	};
	
	// parameters
	Ep0State ep0State;
	Sa1110GpioPin usbControlInput;
	Sa1110GpioPin usbControlOutput;
	
	// transmit state
	UInt transmitLengthEndpoint0;
	UInt8 * transmitEndpoint0;

	// parameters
	DeviceRequest setupBuffer;
	
#if defined(USB_RECYCLE_SUPPORT)
	class UsbRecycleTask : public Task
	{
	public:
		// constructor
		UsbRecycleTask(Sa1110UsbPort& usbPort) : Task(lowPriority, 1024), usbPort(usbPort) {};
	
	private:
		// task entry point
		void main();
		
		// representation
		Sa1110UsbPort& usbPort;
	};

	UsbRecycleTask recycleTask;
	IntertaskEvent recycleEvent;
	friend class UsbRecycleTask;
#endif
	
public:	

	// register accessing
	enum RegisterAddress
	{
		ddar0 = sa1110LcdAndDmaControlBase + 0x0,
		dcsr0Write = sa1110LcdAndDmaControlBase + 0x4,
		dcsr0Clear = sa1110LcdAndDmaControlBase + 0x8,
		dcsr0Read = sa1110LcdAndDmaControlBase + 0xc,		
		dbsa0 = sa1110LcdAndDmaControlBase + 0x10,
		dbta0 = sa1110LcdAndDmaControlBase + 0x14,
		dbsb0 = sa1110LcdAndDmaControlBase + 0x18,
		dbtb0 = sa1110LcdAndDmaControlBase + 0x1c,
		
		ddar1 = sa1110LcdAndDmaControlBase + 0x20,
		dcsr1Write = sa1110LcdAndDmaControlBase + 0x24,
		dcsr1Clear = sa1110LcdAndDmaControlBase + 0x28,
		dcsr1Read = sa1110LcdAndDmaControlBase + 0x2c,		
		dbsa1 = sa1110LcdAndDmaControlBase + 0x30,
		dbta1 = sa1110LcdAndDmaControlBase + 0x34,
		dbsb1 = sa1110LcdAndDmaControlBase + 0x38,
		dbtb1 = sa1110LcdAndDmaControlBase + 0x3c,
	
		udccr  = sa1110PeripheralControlBase + 0x0000,
		udcar  = sa1110PeripheralControlBase + 0x0004,
		udcomp = sa1110PeripheralControlBase + 0x0008,
		udcimp = sa1110PeripheralControlBase + 0x000C,
		udccs0 = sa1110PeripheralControlBase + 0x0010,
		udccs1 = sa1110PeripheralControlBase + 0x0014,
		udccs2 = sa1110PeripheralControlBase + 0x0018,
		udcd0  = sa1110PeripheralControlBase + 0x001C,
		udcwc  = sa1110PeripheralControlBase + 0x0020,
		udcdr  = sa1110PeripheralControlBase + 0x0028,
		udcsr  = sa1110PeripheralControlBase + 0x0030
	};

	inline void delay(UInt c = 1);
	inline UInt readRegister(RegisterAddress address);
	inline void writeRegister(RegisterAddress address, UInt value);
};

//------------------------------------------------------------------------------------------------
// * Sa1110UsbPort::isInError
//
// Tests whether an error condition has occurred.
//------------------------------------------------------------------------------------------------

inline Bool Sa1110UsbPort::isInError() const
{
	return inError;
}

//------------------------------------------------------------------------------------------------
// * UsbPort::read
//
// Read data from the stream.
//------------------------------------------------------------------------------------------------

UInt Sa1110UsbPort::read(void *pDestination, UInt length, TimeValue timeout)
{
	return read(pDestination, length);
}

//------------------------------------------------------------------------------------------------
// * UsbPort::write
//
// Write data to the stream.
//------------------------------------------------------------------------------------------------

UInt Sa1110UsbPort::write(const void *pSource, UInt length, TimeValue timeout)
{
	return write(pSource, length);
}

//------------------------------------------------------------------------------------------------
// * Sa1110UsbPort::flush
//
// Sends any buffered data.
//------------------------------------------------------------------------------------------------

inline void Sa1110UsbPort::flush() { }

//------------------------------------------------------------------------------------------------
// * Sa1110UsbPort::delay
//
// Delay
//------------------------------------------------------------------------------------------------

inline void Sa1110UsbPort::delay(UInt c)
{
	UInt dummy = 0;
	while(c--)
	{
		dummy += readRegister(udcwc);
	};
}

//------------------------------------------------------------------------------------------------
// * Sa1110UsbPort::readRegister
//
// Read a serial port register.
//------------------------------------------------------------------------------------------------

inline UInt Sa1110UsbPort::readRegister(RegisterAddress address)
{
	return *(volatile UInt *)address;
}

//------------------------------------------------------------------------------------------------
// * Sa1110UsbPort::writeRegister
//
// Read a serial port register.
//------------------------------------------------------------------------------------------------

inline void Sa1110UsbPort::writeRegister(RegisterAddress address, UInt value)
{
	*(volatile UInt *)address = value;
}

#endif // _Sa1110UsbPort_h_
