#include "Sa1110UsbPort.h"
#include "Sa1110InterruptController.h"
#include "Sa1110MemoryCache.h"
#include "../multitasking/TaskScheduler.h"
#include "../multitasking/UninterruptableSection.h"
#include "../PointerArithmetic.h"
#include "../memoryUtilities.h"
#include "../multitasking/LockedSection.h"
#include "../multitasking/Mutex.h"
#include "../multitasking/sleep.h"
#include "../../Printing/cout.h"

// Device descriptor
const UInt8 globalDeviceDescriptor[] =
{
	_DeviceDescriptorSize,			// bLength
	_DeviceDescriptor,              // bDescriptorType
	0x00, 0x01,                     // bcdUSB
	0xFF,                           // bDeviceClass
	0xFF,                           // bDeviceSubClass - 0xFF
	0xFF,                           // bDeviceProtocol
	_MaxPacketSizeEndpoint0,  		// bMaxPacketSize0
	0xB3, 0x0F,                     // idVendor
	0x01, 0x00,                     // idDevice
	0x00, 0x01,                     // bcdDevice
	0x00,                           // iManufacturer
	0x00,                           // iProduct
	0x00,                           // iSerialNumber
	0x01,                           // bNumConfigurations
};

// Device configuration, interface and ... descriptors
const UInt8 globalConfigurationDescriptor[] =
{
	_ConfigurationDescriptorSize,	// bLength
	_ConfigurationDescriptor,       // bDescriptorType
	0x20, 0x00,						// wTotalLength
	0x01,                           // bNumInterfaces
	0x01,                           // bConfigurationValue
	0x00,                           // iConfiguration
	0x40,                           // bmAttributes - 0x40
	0x00, /* Self powered */        // MaxPower

	_InterfaceDescriptorSize,		// bLength
	_InterfaceDescriptor,           // bDescriptorType
	0x00,                           // bInterfaceNumber
	0x00,                           // bAlternateSetting
	0x02,                           // bNumEndpoint
	0xFF,                           // bInterfaceClass - 0x00
	0xFF,                           // bInterfaceSubClass
	0xFF,                           // bInterfaceProtocol - 0xFF
	0x00,                           // iInterface ?1
	
	_EndpointDescriptorSize,		// bLength
	_EndpointDescriptor,            // bDescriptorType
	0x01,                           // bEndpointAddress (0x00: OUT)
	0x02,                           // bmAttributes (0:Control, 1:Iso, 2:Bulk, 3:Int)
	0x08, 0x00,						// wMaxPacketSize
	0x00,                           // bInterval
	
	_EndpointDescriptorSize,		// bLength
	_EndpointDescriptor,            // bDescriptorType
	0x82,                           // bEndpointAddress (0x80: IN)
	0x02,                           // bmAttributes (0:Control, 1:Iso, 2:Bulk, 3:Int)
	0x10, 0x00,						// wMaxPacketSize
	0x00,                           // bInterval
};

//------------------------------------------------------------------------------------------------
// * Sa1110UsbPort::Sa1110UsbPort
//
// Constructor.
//------------------------------------------------------------------------------------------------

Sa1110UsbPort::Sa1110UsbPort() :
	InterruptHandler(InterruptHandler::irqInterruptLevel, 100),
	usbControlInput(19),
	usbControlOutput(20)
#if defined(USB_RECYCLE_SUPPORT)
	, recycleTask(*this)
#endif
{
	// disable usb interrupt
	Sa1110InterruptController::getCurrentInterruptController()->disable(13);
//	usbControlInput.configureAsInput();
	usbControlOutput.configureAsOutput();
	usbControlOutput.setValue(0);
	sleepForMilliseconds(10);
	usbControlOutput.setValue(1);

	const UInt alignment = 32;
	UInt paddedTransmitBufferSize = 
		((_MaxPacketSizeEndpointIn + 1) + (alignment - 1)) & ~(alignment - 1);
	
	pDmaBuffers = new UInt8[ 
		alignment - 1 
		+ paddedTransmitBufferSize						// transmitter
		+ _MaxReceiveSize								// receiver
		];
	
	void * ptemp = (void *)(((UInt)pDmaBuffers + (alignment - 1)) & ~(alignment - 1));
	dmaTransBuffer = (UInt8 *)ptemp;

	ptemp = addToPointer(ptemp, paddedTransmitBufferSize);
	dmaRecvBuffer = (UInt8 *)ptemp;

	// disable DMA channel 0
	writeRegister(dcsr0Clear, 0x7F);
	
	// disable DMA channel 1
	writeRegister(dcsr1Clear, 0x7F);
	
	// device 0 - DMA 0 for read
	writeRegister(ddar0, (udccr + 0x0A15));
		
	// set error state
	forceError();
	
	// init UDC
	resetController();

	// register interrupt handler
	TaskScheduler::getCurrentTaskScheduler()->addInterruptHandler(this);

	// enable usb interrupt
	Sa1110InterruptController::getCurrentInterruptController()->setToIrq(13);
	Sa1110InterruptController::getCurrentInterruptController()->enable(13);

#if defined(USB_RECYCLE_SUPPORT)
	// recycle control
	recycleTask.resume();
#endif
}

//------------------------------------------------------------------------------------------------
// * Sa1110UsbPort::~Sa1110UsbPort
//
// Destructor.
//------------------------------------------------------------------------------------------------

Sa1110UsbPort::~Sa1110UsbPort()
{
	// disable usb interrupt
	Sa1110InterruptController::getCurrentInterruptController()->disable(13);
	
	// unregister interrupt handler
	TaskScheduler::getCurrentTaskScheduler()->removeInterruptHandler(this);
	delete pDmaBuffers;
}

//------------------------------------------------------------------------------------------------
// * Sa1110UsbPort::initReceiveDMA
//
// Destructor.
//------------------------------------------------------------------------------------------------

void Sa1110UsbPort::initReceiveDMA()
{
	dmaRecvReadIndex = 0;
	dmaRecvBufferSize = 0;
	
	Sa1110MemoryCache::getCurrentMemoryCache()->flushDataCacheEntries(dmaRecvBuffer, _MaxReceiveSize);
	UInt addr = (UInt)((UInt)(UInt *)(dmaRecvBuffer) + 0xC0000000);

	// pause the DMA
	while (true)
	{
		writeRegister(dcsr0Clear, 0x01);
		delay();
		
		if ((readRegister(dcsr0Read)&0x01) == 0)
		{
			break;
		}
	}
	
	receiveBank = readRegister(dcsr0Read);
	if ((receiveBank&0x80) == 0)
	{
		writeRegister(dcsr0Clear, 0x11);
		writeRegister(dbsa0, addr);
		writeRegister(dbta0, _MaxReceiveSize);
		writeRegister(dcsr0Write, 0x13);
	}
	else
	{
		writeRegister(dcsr0Clear, 0x41);
		writeRegister(dbsb0, addr);
		writeRegister(dbtb0, _MaxReceiveSize);
		writeRegister(dcsr0Write, 0x43);
	}
}

//------------------------------------------------------------------------------------------------
// * Sa1110UsbPort::initSendDMA
//
// Destructor.
//------------------------------------------------------------------------------------------------

void Sa1110UsbPort::initSendDMA()
{
	UInt8 length = minimum(_MaxPacketSizeEndpointIn, transmitLength);

	// set packet size
	while (true)
	{
		writeRegister(udcimp, (length - 1));
		delay();
		
		if (readRegister(udcimp) == (length - 1))
		{
			break;
		}
	}

	transmitFifoDataLength = 0;
	while (transmitFifoDataLength < length)
	{
		writeRegister(udcdr, (UInt)(pTransmitBuffer[transmitFifoDataLength++]));
	}
}

//------------------------------------------------------------------------------------------------
// * Sa1110UsbPort::controllerReset
//
// Interrupt handler.
//------------------------------------------------------------------------------------------------

void Sa1110UsbPort::resetController()
{
	// init variables
	askMore = false;
	ep0State = ep0Idle;
	configurationNum = 0;
	initStatus = initNone;
	transmitLengthEndpoint0 = 0;

//	cout << "Reset Controller\r\n";
	
	while (true) 
	{
		if ((readRegister(udccr)&0x02) == 0)
		{
			break;
		}
	}

	// set #29 errata bit
	while (true)
	{
		writeRegister(udccr, 0x80);
		delay();
		
		if (readRegister(udccr) == (UInt)0x80)
		{
			break;
		}
	}
	
	while (true)
	{
		writeRegister(udccr, 0x01);
		delay();
		
		if (readRegister(udccr) == (UInt)0x41u)
		{
			break;
		}
	}
	
	while (true)
	{
		writeRegister(udccr, 0);
		delay();
		
		if (readRegister(udccr) == 0)
		{
			break;
		}
	}

	while (true)
	{
		writeRegister(udcomp, (UInt)_MaxPacketSizeEndpointOut_1);
		delay();
		
		if (readRegister(udcomp) == (UInt)_MaxPacketSizeEndpointOut_1)
		{
			break;
		}
	}
	
	while (true)
	{
		writeRegister(udcsr, (UInt)0x20u);
		delay();
		
		if ((readRegister(udcsr)&0x20) == 0)
		{
			break;
		}
	}
}

//------------------------------------------------------------------------------------------------
// * Sa1110UsbPort::handleInterrupt
//
// Interrupt handler.
//------------------------------------------------------------------------------------------------

Bool Sa1110UsbPort::handleInterrupt()
{
	// determine if this interrupt is for us
	if(Sa1110InterruptController::getCurrentInterruptController()->isPending(13))
	{
		return proceedInterrupt();
	}

	// interrupt not for us
	return false;
}

//------------------------------------------------------------------------------------------------
// * Sa1110UsbPort::proceedInterrupt
//
// Interrupt execution.
//------------------------------------------------------------------------------------------------

Bool Sa1110UsbPort::proceedInterrupt()
{
	UInt ireg = 0;
	ireg = readRegister(udcsr);
	ireg = readRegister(udcsr);
	
	// clear interrupts
	while (true)
	{
		writeRegister(udcsr, ireg);
		delay();
		
		if (readRegister(udcsr) == 0)
		{
			break;
		}
	}
	
	if (ireg != 8)
	{
	//	cout << "I" << ireg << " ";
	}
	
	if ((ireg&0x20) != 0)	
	{
		// reset interrupt
		resetController();
		return true;
	} 

	if ((ireg&0x01) != 0)
	{
		// EP0 signaled
		funcControl();
	}
	
	if ((ireg&0x02) != 0)
	{
		// EP1 signaled
		funcReceive();
	}
	
	if ((ireg&0x04) != 0)
	{
		// EP2 signaled
		funcTransmit();
	}
		
//	if ((ireg&0x10) != 0)
//	{
//		// RESUME signaled
//		while ((readRegister(udccr)&0x04) == 0)
//		{
//			writeRegister(udccr, (readRegister(udccr)|0x04));
//		}
//		
//		while ((readRegister(udccr)&0x04) != 0)
//		{
//			writeRegister(udccr, (readRegister(udccr)&((UInt)~0x04)));
//		}
//		
//		while ((readRegister(udccr)&0x40) != 0)
//		{
//			writeRegister(udccr, (readRegister(udccr)&((UInt)~0x40)));
//		}
//	}
		
//	if ((ireg&0x08) != 0)
//	{
//		// SRM signaled
//		writeRegister(udcsr, 0x08);
//		while ((readRegister(udccr)&0x40) == 0)
//		{
//			writeRegister(udccr, (readRegister(udccr)|0x40));
//		}
//	}
	
	// interrupt handled
	return true;
}

//------------------------------------------------------------------------------------------------
// * Sa1110UsbPort::funcControl
//
// Interrupt handler.
//------------------------------------------------------------------------------------------------

void Sa1110UsbPort::funcControl()
{
	UInt ep0 = readRegister(udccs0);

//	cout << "E0" << ep0 << " ";
	
	if ((ep0&0x20) != 0)
	{
		// SE detected
		while (true)
		{
			writeRegister(udccs0, 0x80);
			delay();
			
			ep0 = readRegister(udccs0);
			if ((ep0&0x20) == 0)
			{
				break;
			}
		}

		ep0State = ep0Idle;
		if ((ep0&0x01) == 0)
		{
			return;
		}
	}
	
	if (ep0State == ep0Idle)
	{
		if ((ep0&0x01) != 0)
		{
			// something is coming
			if (readRegister(udcwc) == 8)
			{
				// request
				readSetupData();
			}
		}
		else
		{
			if((ep0&0x08) != 0)
			{
				// FSTALL detected
//				writeRegister(udccs0, 0);
			}
			
			if((ep0&0x04) != 0)
			{
				// stall detected
				while (true)
				{
					writeRegister(udccs0, 0x04);
//					forceError();
					delay(10);
					
					if ((readRegister(udccs0)&0x04) == 0)
					{
						break;
					}
				}
			}
		}
	}
	else if (ep0State == ep0Indata) 
	{
		// transfer next chunk of data
		continueSendingData();
	}
	else if (ep0State == ep0Xfer)
	{
		// complete transfer
		xferProcess();
	}
}

//------------------------------------------------------------------------------------------------
// * Sa1110UsbPort::readSetupData
//
// Process SETUP token
//------------------------------------------------------------------------------------------------

void Sa1110UsbPort::readSetupData()
{
	// Read setup buffer
	UInt i = 0;
	do
	{
		((UInt8 *)(&setupBuffer))[i] = (UInt8)readRegister(udcd0);
		delay(10);
		
		i = 8 - readRegister(udcwc);
	}
	while (i != 8);

//	cout << "Request " << setupBuffer.requestType << " " 
//		 << setupBuffer.request << " " << setupBuffer.value  << " " 
//		 << setupBuffer.index   << " " << setupBuffer.length << "\r\n";
	
	if (setupBuffer.length != 0)	
	{
		// OPR is set - clear it
		while (true)
		{
			writeRegister(udccs0, 0x40);
			delay(10);
			
			if ((readRegister(udccs0)&0x01) == 0)
			{
				break;
			}
		}
	}
	
	// filter standard requests
	if (((setupBuffer.requestType)&0x60) == 0)
	{
		// Standard request
		switch(setupBuffer.request)
		{
		case _GetDescriptor:
			// get device descriptors		
			getDescriptor();
			break;
		
		case _GetConfiguration:
			// send configuration status
			sendDataEndPoint0(&configurationNum, 1);
			break;
		
		case _SetAddress:
			// clear OPR and set DE
			while (true)
			{
				writeRegister(udccs0, 0x50);
				delay(10);
				
				UInt treg = readRegister(udccs0);
				if ((treg&0x01) == 0 && (treg&0x10) != 0)
				{
					break;
				}
			}
			
			writeRegister(udcar, ((setupBuffer.value)&0x7f));
			
			// Change init state
			if (initStatus == initNone)
			{
				initStatus = initStart;
				#if defined(USB_RECYCLE_SUPPORT)
					recycleEvent.signal();
				#endif
			}
			
			break;
		
		case _SetConfiguration:
			configurationNum = (setupBuffer.value&0xFF);
			if (configurationNum == 1)
			{
				// clear OPR and set DE
				while (true)
				{
					writeRegister(udccs0, 0x50);
					delay(10);
					
					UInt treg = readRegister(udccs0);
					if ((treg&0x01) == 0 && (treg&0x10) != 0)
					{
						break;
					}
				}
			}
			
			break;
			
		case _ClearFeature:
		
			// clear feature
			while (true)
			{
				writeRegister(udccs0, 0x50);
				delay(10);
				
				UInt treg = readRegister(udccs0);
				if ((treg&0x01) == 0 && (treg&0x10) != 0)
				{
					break;
				}
			}
			
			if ((setupBuffer.index) == 0x01)  /* OUT */
			{
				while (true)
				{
					if ((readRegister(udccs1)&0x10) != 0)
					{
						break;
					}
					
					writeRegister(udccs1, 0x10);
					delay();
				}
				
				while (true)
				{
					if ((readRegister(udccs1)&0x10) == 0)
					{
						break;
					}
					
					writeRegister(udccs1, 0);
					delay();
				}
				
				while (true)
				{
					if ((readRegister(udccs1)&0x08) == 0)
					{
						break;
					}
					
					writeRegister(udccs1, 0x08);
					delay();
				}
			}
	
			if ((setupBuffer.index) == 0x82)   /* IN */
			{	
				while (true)
				{
					if ((readRegister(udccs2)&0x20) != 0)
					{
						break;
					}
					
					writeRegister(udccs2, 0x20);
					delay();
				}
				
				while (true)
				{
					if ((readRegister(udccs2)&0x20) == 0)
					{
						break;
					}
					
					writeRegister(udccs2, 0);
					delay();
				}
				
				while (true)
				{
					if ((readRegister(udccs2)&0x10) == 0)
					{
						break;
					}
					
					writeRegister(udccs2, 0x10);
					delay();
				}
			}
			
			break;
			
		default:
			// unknown request
			writeRegister(udccs0, 0x50);
			break;
		}
	}
	
	// specific vendor requests
	if (((setupBuffer.requestType)&0x60) == 0x40)
	{
		// Vendor request
		switch (setupBuffer.request)
		{
		case _ResyncDevice:
			funcResync();
			break;
		}
	}
}

//------------------------------------------------------------------------------------------------
// * Sa1110UsbPort::funcResync
//
// Function
//------------------------------------------------------------------------------------------------

void Sa1110UsbPort::funcResync()
{
	// make sure error is set
	forceError();
	if (configurationNum == 1)
	{
		transmitFifoDataLength = 0;
		transmitLength = 0;
		receiveLength = 0;

		transmitEvent.clear();
		receiveEvent.clear();
		
		initReceiveDMA();
		synchronizationEvent.signal();
		
		// clear OPR and set DE
		while (true)
		{
			writeRegister(udccs0, 0x50);
			delay(10);
			
			UInt treg = readRegister(udccs0);
			if ((treg&0x01) == 0 && (treg&0x10) != 0)
			{
				break;
			}
		}
		
		// resynchronization completed
	}
}

//------------------------------------------------------------------------------------------------
// * Sa1110UsbPort::getDescriptor
//
// Function
//------------------------------------------------------------------------------------------------

void Sa1110UsbPort::getDescriptor()
{
	switch ((UInt8) (setupBuffer.value>>8))
	{
	case _DeviceDescriptor:
		// Send device descriptor
		sendDataEndPoint0(globalDeviceDescriptor, sizeof(globalDeviceDescriptor));
		break;
	
	case _ConfigurationDescriptor:
		// Send configuration descriptor
		sendDataEndPoint0(globalConfigurationDescriptor, sizeof(globalConfigurationDescriptor));
				
		// device is ready to communicate
		if ((initStatus == initStart) && 
			(setupBuffer.length > sizeof(globalConfigurationDescriptor)))
		{
			initStatus = initLoad;
		}
		
		break;
	}
}

//------------------------------------------------------------------------------------------------
// * Sa1110UsbPort::sendData
//
// Function
//------------------------------------------------------------------------------------------------

void Sa1110UsbPort::sendDataEndPoint0(const UInt8 *buffer, UInt size)
{
	// Provide only size requested by host
	transmitEndpoint0 = (UInt8 *)buffer;
	transmitLengthEndpoint0 = minimum(setupBuffer.length, size);
	
	askMore = (setupBuffer.length > size)?(true):(false);
	ep0State = ep0Indata;
	continueSendingData();
}

//------------------------------------------------------------------------------------------------
// * Sa1110UsbPort::continueSendingData
//
// Function
//------------------------------------------------------------------------------------------------

void Sa1110UsbPort::continueSendingData()
{
	
	if ((readRegister(udccs0)&0x04) != 0)
	{
		// STALL detected
		ep0State = ep0Idle;
		writeRegister(udccs0, 0x04);
		return;
	}
	
	if ((readRegister(udccs0)&0x02) != 0)
	{
		// IPR is not clear
		return;
	}
	
	// check if endpoint is large enough to accept bytes left
	UInt8 packSize = minimum(transmitLengthEndpoint0, (UInt8)_MaxPacketSizeEndpoint0);
	transmitLengthEndpoint0 -= packSize;
	
//	cout << "continueSendingData() " << (UInt)packSize << " \r\n";
	if (packSize != 0)
	{
		const SInt maxRc = 100;
		const SInt maxError = 20;

		SInt rc, rerror;
		UInt oldc, c = 0;
		UInt8 * ptr = transmitEndpoint0;
		do
		{
			rc = maxRc;
			rerror = maxError;
			
			oldc = c;
			ptr = transmitEndpoint0 + c;
			writeRegister(udcd0, *ptr);
			c = readRegister(udcwc);
			while (oldc == c)
			{
				if (--rc < 0)
				{
					// missed write
					writeRegister(udcd0, *ptr);
					rc = maxRc;
					if (--rerror < 0)
					{
						// force stall and return
						writeRegister(udccs0, 0x8);
						return;
					}
				}
				c = readRegister(udcwc);
			}
		}
		while (packSize != c);
		transmitEndpoint0 += packSize;
	}

	delay(10);
	
	UInt rudccs0 = 0;
	if (transmitLengthEndpoint0 == 0)
	{
		if (askMore)
		{
			if (packSize < (UInt8)_MaxPacketSizeEndpoint0)
			{
				ep0State = ep0Xfer;
			}
		}
		else
		{
			rudccs0 |= 0x10;
			ep0State = ep0Idle;
//			while (true)
//			{
//				writeRegister(udccs0, 0x10);
//				delay(10);
//				
//				if ((readRegister(udccs0)&0x10) != 0)
//				{
//					break;
//				}
//			}
		}
	}
	
	rudccs0 |= 0x02;
	while (true)
	{
		writeRegister(udccs0, rudccs0);
		delay(10);
		
		if ((readRegister(udccs0)&0x02) != 0)
		{
			break;
		}
	}
}

//------------------------------------------------------------------------------------------------
// * Sa1110UsbPort::xferProcess
//
// Function
//------------------------------------------------------------------------------------------------

void Sa1110UsbPort::xferProcess()
{
//	cout << "xferProcess() \r\n";
	
	if ((readRegister(udccs0)&0x04) != 0)
	{
		// STALL detected
		ep0State = ep0Idle;
		writeRegister(udccs0, 0x04);
	}
	
	if ((readRegister(udccs0)&0x02) != 0)
	{
		// IPR is not clear
		return;
	}

	// IPR is clear - set data end
	ep0State = ep0Idle;
	while (true)
	{
		writeRegister(udccs0, 0x10);
		delay(10);
		
		if ((readRegister(udccs0)&0x10) != 0)
		{
			break;
		}
	}
}

//------------------------------------------------------------------------------------------------
// * Sa1110UsbPort::read
//
// Read data from the stream.
//------------------------------------------------------------------------------------------------

UInt Sa1110UsbPort::read(void *pDestination, UInt length)
{
	// do not continue if we are in an error state
	if(isInError())
	{
		return 0;
	}
	
	LockedSection readLock(readMutex);

	pReceiveBuffer = (UInt8 *)pDestination;
	receiveLength = length;
	
	while (receiveLength != 0)
	{
		if (dmaRecvReadIndex == dmaRecvBufferSize)
		{
			{
				UninterruptableSection criticalSection;
				
				if(isInError())
				{
					return length - receiveLength;
				}
				
				// go DMA
				initReceiveDMA();
			}
			
			// wait for completion of the read
			receiveEvent.wait();
			if (isInError())
			{
				return length - receiveLength;
			}
		}
		else
		{
			{
				UninterruptableSection criticalSection;
				
				UInt sizeExist = dmaRecvBufferSize - dmaRecvReadIndex;
				UInt sizeRead = minimum(receiveLength, sizeExist);
				
				// copy data
				memoryCopy(pReceiveBuffer, 
					(void *)&((dmaRecvBuffer)[dmaRecvReadIndex]), 
					sizeRead);
					
				// adjust buffers
				pReceiveBuffer += sizeRead;
				receiveLength -= sizeRead;
				
				dmaRecvReadIndex += sizeRead;
			}
		}
	}

	return length;
}

//------------------------------------------------------------------------------------------------
// * Sa1110UsbPort::funcReceive
//
// vendor request routine process
//------------------------------------------------------------------------------------------------

void Sa1110UsbPort::funcReceive()
{
	UInt ep1 = readRegister(udccs1);

	// check receive packet RPC
	if((readRegister(udccs1)&0x02) != 0)
	{
		// disable DMA 0
//		writeRegister(dcsr0Clear, 0x7F);
		
		UInt ii = _MaxReceiveSize;
		if ((receiveBank&0x80) == 0)
		{
			ii = ii - readRegister(dbta0);
		}
		else
		{
			ii = ii - readRegister(dbtb0);
		}

		// check for hardware detected errors
		if ((readRegister(udccs1)&0x04) == 0)
		{
			while (true)
			{
				// receive one byte
				if ((readRegister(udccs1)&0x20) == 0)
				{
					break;
				}
				
				((UInt8 *)dmaRecvBuffer)[ii++] = (UInt8)readRegister(udcdr);
			}
			 
			if (!isInError())
			{
				dmaRecvReadIndex = minimum(ii, receiveLength);
				if (dmaRecvReadIndex > 0)
				{
					dmaRecvBufferSize = ii;
				}

				memoryCopy(pReceiveBuffer, dmaRecvBuffer, dmaRecvReadIndex);
				
				receiveLength -= dmaRecvReadIndex;
				pReceiveBuffer += dmaRecvReadIndex;
			}
		}
		else
		{
			// RPE detected
//			forceError(); // signal error
		}
	
		if (!isInError())
		{
			if (receiveLength == 0)
			{
				receiveEvent.signal();
			}
			else
			{
				initReceiveDMA();
			}
		}
	}
	
	if ((readRegister(udccs1)&0x08) != 0)
	{
		// STALL detected
		while (true)
		{
			if ((readRegister(udccs1)&0x08) == 0)
			{
				break;
			}
			writeRegister(udccs1, 0x08);
			delay();
		}
		
//		forceError();
	}
	
	if (transmitFifoDataLength != 0)
	{
//		cout << "Collision\r\n";
		initSendDMA();
	}
	
	while (true)
	{
		writeRegister(udccs1, 0x02);
		delay();
		
		if ((readRegister(udccs1)&0x02) == 0)
		{
			break;
		}
	}
}

//------------------------------------------------------------------------------------------------
// * Sa1110UsbPort::write
//
// Write data to the stream.
//------------------------------------------------------------------------------------------------

UInt Sa1110UsbPort::write(const void *pSource, UInt length)
{
	if (length == 0)
	{
		return 0;
	}

	if((length & _MaxPacketSizeEndpointIn_1) == 0)
	{
		return
			write(pSource, length - 1) +
			write(addToPointer(pSource, (length - 1)), 1);
	}
	
	// do not continue if we are in an error state
	if(isInError())
	{
		return 0;
	}
	
	LockedSection writeLock(writeMutex);

	{
		UninterruptableSection criticalSection;
		
		if(isInError())
		{
			return 0;
		}
		
		// setup transmit state
		pTransmitBuffer = (UInt8 *)pSource;
		transmitLength = length;
			
		// go DMA
		initSendDMA();
	}

	// wait for completion of the write
	transmitEvent.wait();

	return length;
}

//------------------------------------------------------------------------------------------------
// * Sa1110UsbPort::funcTransmit
//
// vendor request routine process
//------------------------------------------------------------------------------------------------

void Sa1110UsbPort::funcTransmit()
{
	UInt ep2 = readRegister(udccs2);
	
	// check transmit packet TPC
	if ((readRegister(udccs2)&0x02) != 0)
	{
		if ((readRegister(udccs2)&0x0C) == 0)
		{
			transmitLength -= transmitFifoDataLength;
			pTransmitBuffer += transmitFifoDataLength;
			transmitFifoDataLength = 0;
		}
		else
		{
			// TUR or TPE detected
//			forceError(); // signal error
		}
		
		if ((readRegister(udccs2)&0x10) != 0)
		{
			// STALL detected
			while (true)
			{
				writeRegister(udccs2, 0x10);
				delay();
				
				if ((readRegister(udccs2)&0x10) == 0)
				{
					break;
				}
			}
			
//			forceError(); // signal error
		}
		
		while (true)
		{
			writeRegister(udccs2, 0x02);
			delay();
			
			if ((readRegister(udccs2)&0x02) == 0)
			{
				break;
			}
		}
			
		if (!isInError())
		{
			if (transmitLength == 0)
			{
				transmitEvent.signal();
			}
			else
			{
				initSendDMA();
			}
		}
	}
}

//------------------------------------------------------------------------------------------------
// * Sa1110UsbPort::forceError
//
// Force an error condition.
//------------------------------------------------------------------------------------------------

void Sa1110UsbPort::forceError()
{
	// check if an error has occurred
	if(!isInError())
	{
		// force an error
		UninterruptableSection criticalSection;
		handleError();
	}
}

//------------------------------------------------------------------------------------------------
// * Sa1110UsbPort::handleError
//
// Error handler.
//------------------------------------------------------------------------------------------------

void Sa1110UsbPort::handleError()
{
	// set flag to indicate an error
	inError = true;
	
	// terminate current transmissions
	receiveEvent.signal();
	transmitEvent.signal();
}

//------------------------------------------------------------------------------------------------
// * Sa1110UsbPort::reset
//
// Resets the stream and clears errors.
//------------------------------------------------------------------------------------------------

void Sa1110UsbPort::reset()
{
	// make sure that all reads and write are terminated by forcing an error condition
	forceError();

	// wait for completion of synchronization
	if(synchronizationEvent.waitForSeconds(10))
	{
		return;
	}

	inError = false;
	
	// discard first packet
//	UInt8 dummy;
//	read(&dummy, sizeof(dummy));
//	write(&dummy, sizeof(dummy));

//	cout << "Usb Reset Exit\n";
}

//------------------------------------------------------------------------------------------------
// * Sa1110UsbPort::recycleConnection
//
// Emulate disconnect / connect sequence
//------------------------------------------------------------------------------------------------

void Sa1110UsbPort::recycleConnection()
{
	// disable usb interrupt
	Sa1110InterruptController::getCurrentInterruptController()->disable(13);
	
	// set error state
	forceError();
	
	// emulate disconnect
	usbControlOutput.setValue(0);
	
//	cout << "Recycle Connection\n";
	
	// wait for 10 us
	sleepForMicroseconds(10);
	
	// emulate connect
	usbControlOutput.setValue(1);	
	
	// init UDC
	resetController();

	// enable usb interrupt
	Sa1110InterruptController::getCurrentInterruptController()->setToIrq(13);
	Sa1110InterruptController::getCurrentInterruptController()->enable(13);
}

//------------------------------------------------------------------------------------------------
// * UsbDebugTask::main
//
// main
//------------------------------------------------------------------------------------------------

#if defined(USB_RECYCLE_SUPPORT)
void Sa1110UsbPort::UsbRecycleTask::main()
{
	while (true)
	{
		// wait for signal to recycle
		usbPort.recycleEvent.wait();
		
		// wait for transaction
		sleepForSeconds(5);
		if (usbPort.initStatus != initStart)
		{
			continue;
		}
		
		// recycling support
		usbPort.recycleConnection();
	}
}
#endif
