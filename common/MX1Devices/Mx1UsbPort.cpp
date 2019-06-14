#include "Mx1UsbPort.h"
#include "Mx1InterruptController.h"
#include "deviceAddresses.h"
#include "../multitasking/TaskScheduler.h"
#include "../multitasking/UninterruptableSection.h"
#include "../PointerArithmetic.h"
#include "../memoryUtilities.h"
#include "../endian.h"
#include "../multitasking/LockedSection.h"
#include "../multitasking/Mutex.h"
#include "../multitasking/sleep.h"

#include "../../Printing/cout.h"
//------------------------------------------------------------------------------------------------
// * Mx1UsbPort::Mx1UsbPort
//
// Constructor.
//------------------------------------------------------------------------------------------------

Mx1UsbPort::Mx1UsbPort() :
	InterruptHandler(InterruptHandler::irqInterruptLevel, 100),
	registerBase(mx1RegistersBase + 0x12000),
	resetPin(Mx1GpioPin::portD, 24)
{
	resetPin.configureAsOutput();
	resetPin.setValue(0);
	sleepForMilliseconds(10);
	resetPin.setValue(1);
	
	{
		// configure USB1 pins
		UninterruptableSection criticalSection;
		*(volatile UInt *)(mx1RegistersBase + 0x1C100) |= (1u << 20); // AFE as output
		*(volatile UInt *)(mx1RegistersBase + 0x1C100) |= (1u << 21); // OE as output
		*(volatile UInt *)(mx1RegistersBase + 0x1C100) |= (1u << 23); // SUSPND as output
		*(volatile UInt *)(mx1RegistersBase + 0x1C100) |= (1u << 26); // VPO as output
		*(volatile UInt *)(mx1RegistersBase + 0x1C100) |= (1u << 27); // VMO as output
		*(volatile UInt *)(mx1RegistersBase + 0x1C100) &= ~(1u << 22); // RCV as input
		*(volatile UInt *)(mx1RegistersBase + 0x1C100) &= ~(1u << 24); // VP as input
		*(volatile UInt *)(mx1RegistersBase + 0x1C100) &= ~(1u << 25); // VM as input
		*(volatile UInt *)(mx1RegistersBase + 0x1C120) &= ~(0xFFu << 20); // clear GUIS_B (20 - 27)
		*(volatile UInt *)(mx1RegistersBase + 0x1C138) &= ~(0xFFu << 20); // clear GPR_B (20 - 27)
	}

	// set error state
	forceError();

	// reset
	usbReset();

	// register interrupt handler
	TaskScheduler::getCurrentTaskScheduler()->addInterruptHandler(this);

	// enable interrupts
	Mx1InterruptController::getCurrentInterruptController()->setToIrq(47);
	Mx1InterruptController::getCurrentInterruptController()->setToIrq(48);
	Mx1InterruptController::getCurrentInterruptController()->setToIrq(49);
	Mx1InterruptController::getCurrentInterruptController()->setToIrq(53);
	Mx1InterruptController::getCurrentInterruptController()->enable(47);
	Mx1InterruptController::getCurrentInterruptController()->enable(48);
	Mx1InterruptController::getCurrentInterruptController()->enable(49);
	Mx1InterruptController::getCurrentInterruptController()->enable(53);
}

//------------------------------------------------------------------------------------------------
// * Mx1UsbPort::~Mx1UsbPort
//
// Destructor.
//------------------------------------------------------------------------------------------------

Mx1UsbPort::~Mx1UsbPort()
{
	// disable interrupts
	Mx1InterruptController::getCurrentInterruptController()->disable(47);
	Mx1InterruptController::getCurrentInterruptController()->disable(48);
	Mx1InterruptController::getCurrentInterruptController()->disable(49);
	Mx1InterruptController::getCurrentInterruptController()->disable(53);

	// unregister interrupt handler
	TaskScheduler::getCurrentTaskScheduler()->removeInterruptHandler(this);
}

//------------------------------------------------------------------------------------------------
// * Mx1UsbPort::configure
//
// Configure UDC controller.
//------------------------------------------------------------------------------------------------

void Mx1UsbPort::configure()
{
	writeRegister(mask, 0);				// unmask all general interrupts
	writeRegister(ep0mask, 0);			// unmask all EP0 interrupts
	writeRegister(ep0mask + 0x30, 0);	// unmask all EP1 interrupts
	writeRegister(ep0mask + 0x30 * 2, 0);	// unmask all EP2 interrupts
}

//------------------------------------------------------------------------------------------------
// * Mx1UsbPort::usbReset
//
// Reset routine.
//------------------------------------------------------------------------------------------------

void Mx1UsbPort::usbReset()
{
	inConfigured = false;
	
	// soft reset
	writeRegister(enab, 0x80000000);
	
	// wait for controller to reset
	while (true)
	{
		if ((readRegister(enab)&0x80000000) == 0)
		{
			break;
		}
	}

	// wait for controller to enable
	while (true)
	{
		if ((readRegister(enab)&0x40000000) != 0)
		{
			// big endians
			writeRegister(enab, 0x40000001);
			break;
		}
	}

	// check front-end logic is disabled
	while (true)
	{
		if ((readRegister(ctrl)&0x00000008) == 0)
		{
			break;
		}
	}

	// wait for controller is ready to be configured
	while (true)
	{
		if ((readRegister(dadr)&0x80000000) != 0)
		{
			break;
		}
	}
	
	// load configuration for ALL endpoints
	static const UInt8 epbuf[55] =	
	{
		0x00, 0x00, 0x08, 0x00, 0x00,
		0x14, 0x10, 0x20, 0xC0, 0x01,
		0x24, 0x14, 0x20, 0xC0, 0x02,
		0x34, 0x10, 0x08, 0xC0, 0x03,
		0x44, 0x14, 0x08, 0xC0, 0x04,
		0x54, 0x10, 0x08, 0xC0, 0x05,
		0x18, 0x10, 0x08, 0xC0, 0x01,
		0x28, 0x14, 0x08, 0xC0, 0x02,
		0x38, 0x10, 0x08, 0xC0, 0x03,
		0x48, 0x14, 0x08, 0xC0, 0x04,
		0x58, 0x10, 0x08, 0xC0, 0x05
	};

	for (int i = 0; i < 55; i++)
	{
		writeRegister(ddat, epbuf[i]);
		
		// wait for busy flag is cleared
		while ((readRegister(dadr)&0x40000000) != 0);
	}
	
	// wait for controller is to be configured
	while (true)
	{
		if ((readRegister(dadr)&0x80000000) == 0)
		{
			break;
		}
	}
	
	// mask interrupts
	writeRegister(intr, 0x800000FF);		// clear general interrupts
	writeRegister(mask, 0x800000FF);		// mask all general interrupts

	writeRegister(ep0intr, 0x000001FF);	// clear EP interrupts
	writeRegister(ep0mask, 0x000001FF);	// mask all EP interrupts
	writeRegister(ep0intr + 0x30, 0x000001FF);	// clear EP interrupts
	writeRegister(ep0mask + 0x30, 0x000001FF);	// mask all EP interrupts
	writeRegister(ep0intr + 0x30 * 2, 0x000001FF);	// clear EP interrupts
	writeRegister(ep0mask + 0x30 * 2, 0x000001FF);	// mask all EP interrupts
	writeRegister(ep0intr + 0x30 * 3, 0x000001FF);	// clear EP interrupts
	writeRegister(ep0mask + 0x30 * 3, 0x000001FF);	// mask all EP interrupts
	writeRegister(ep0intr + 0x30 * 4, 0x000001FF);	// clear EP interrupts
	writeRegister(ep0mask + 0x30 * 4, 0x000001FF);	// mask all EP interrupts
	writeRegister(ep0intr + 0x30 * 5, 0x000001FF);	// clear EP interrupts
	writeRegister(ep0mask + 0x30 * 5, 0x000001FF);	// mask all EP interrupts

	writeRegister(ep0stat, 0x0002); // ep0 - CONTROL
	writeRegister(ep0stat + 0x30, 0x0052); // ep1 - OUT
	writeRegister(ep0stat + 0x30 * 2, 0x00D2); // ep2 - IN
	writeRegister(ep0stat + 0x30 * 3, 0x0012); // ep3 - OUT
	writeRegister(ep0stat + 0x30 * 4, 0x0092); // ep4 - IN
	writeRegister(ep0stat + 0x30 * 5, 0x0012); // ep5 - OUT
		
	writeRegister(ep0fctrl, 0x08000000); // ep0
	writeRegister(ep0fctrl + 0x30, 0x08000000); // ep1
	writeRegister(ep0fctrl + 0x30 * 2, 0x08000000); // ep2
	writeRegister(ep0fctrl + 0x30 * 3, 0x08000000); // ep3
	writeRegister(ep0fctrl + 0x30 * 4, 0x08000000); // ep4
	writeRegister(ep0fctrl + 0x30 * 5, 0x08000000); // ep5

	writeRegister(ep0falrm, 0x0000); // ep0
	writeRegister(ep0falrm + 0x30, 0x0000); // ep1
	writeRegister(ep0falrm + 0x30 * 2, 0x0000); // ep2
	writeRegister(ep0falrm + 0x30 * 3, 0x0000); // ep3
	writeRegister(ep0falrm + 0x30 * 4, 0x0000); // ep4
	writeRegister(ep0falrm + 0x30 * 5, 0x0000); // ep5
	
	// wait for controller is to be configured
	while (true)
	{
		if ((readRegister(intr)&0x00000010) == 0)
		{
			break;
		}
	}

	// enable controller
	writeRegister(ctrl, 0x1A);
	configure();
}

//------------------------------------------------------------------------------------------------
// * Mx1UsbPort::handleInterrupt
//
// Interrupt handler.
//------------------------------------------------------------------------------------------------

Bool Mx1UsbPort::handleInterrupt()
{
	if (Mx1InterruptController::getCurrentInterruptController()->isPending(53))
	{
		return usbInterrupt();
	}
			
	if (Mx1InterruptController::getCurrentInterruptController()->isPending(47))
	{
		return ep0Interrupt();
	}
	
	if (Mx1InterruptController::getCurrentInterruptController()->isPending(48))
	{
		return ep1Interrupt();
	}
	
	if (Mx1InterruptController::getCurrentInterruptController()->isPending(49))
	{
		return ep2Interrupt();
	}

	return false;
}

//------------------------------------------------------------------------------------------------
// * Mx1UsbPort::usbInterrupt
//
// Common usb interrupt handler.
//------------------------------------------------------------------------------------------------

Bool Mx1UsbPort::usbInterrupt()
{
	// check general USB interrupts
	const UInt interrupt = readRegister(intr);
	// cout << " intr=" << interrupt;
	
	if ((interrupt&0x0001) != 0)
	{
		if ((readRegister(stat)&0x60) == 0x20)
		{
			inConfigured = true;
		}
	}
	
	if ((interrupt&0x0004) != 0)
	{
		// device suspended
	}
	
	if ((interrupt&0x0008) != 0)
	{
		// device wakeup
	}
	
	if ((interrupt&0x0010) != 0)
	{
		// reset start
		usbReset();
	}
	
	if ((interrupt&0x0040) != 0)
	{
		// linux code copy ...
		writeRegister(ctrl, (readRegister(ctrl)|0x0040));    // signal CMD_OVER
	}

	// interrupt handled
	writeRegister(intr, interrupt);
	return true;
}

//------------------------------------------------------------------------------------------------
// * Mx1UsbPort::ep0Interrupt
//
// Ep0 interrupt handler.
//------------------------------------------------------------------------------------------------

Bool Mx1UsbPort::ep0Interrupt()
{
	static const UInt usbDevDscptr[] = 
	{
		// big endians
    	0x12010001, 0x00000008, 0xB30F0100, 0x00010000, 0x00010000
    };
/*   
	const UInt8 globalDeviceDescriptor[] =
	{
		0x12,							// bLength
		0x01,              				// bDescriptorType
		0x00, 0x01,                     // bcdUSB
		0x00,                           // bDeviceClass
		0x00,                           // bDeviceSubClass - 0xFF
		0x00,                           // bDeviceProtocol
		0x08,  							// bMaxPacketSize0

		0xB3, 0x0F,                     // idVendor
		0x01, 0x00,                     // idDevice
		0x00, 0x01,                     // bcdDevice
		0x00,                           // iManufacturer
		0x00,                           // iProduct
		0x00,                           // iSerialNumber
		0x01,                           // bNumConfigurations
	};
*/
	static const UInt usbConfDscptr[] = 
	{
		// big endians
    	0x09022000, 0x01010040, 0x00090400, 0x0002FFFF,
	    0xFF000705, 0x01022000, 0x00070582, 0x02200000
	};
/*
	const UInt8 globalConfigurationDescriptor[] =
	{
		0x09,							// bLength
		0x02,							// bDescriptorType
		0x20, 0x00,						// wTotalLength
		0x01,                           // bNumInterfaces
		0x01,                           // bConfigurationValue
		0x00,                           // iConfiguration
		0x40,                           // bmAttributes - 0x40
		0x00,       					// MaxPower
	
		0x09,							// bLength
		0x04,				 			// bDescriptorType
		0x00,                           // bInterfaceNumber
		0x00,                           // bAlternateSetting
		0x02,                           // bNumEndpoint
		0xFF,                           // bInterfaceClass - 0x00
		0xFF,                           // bInterfaceSubClass
		0xFF,                           // bInterfaceProtocol - 0xFF
		0x00,                           // iInterface ?1
		
		0x07,							// bLength
		0x05,            				// bDescriptorType
		0x01,                           // bEndpointAddress (0x00: OUT)
		0x02,                           // bmAttributes (0:Control, 1:Iso, 2:Bulk, 3:Int)
		0x20, 0x00,						// wMaxPacketSize
		0x00,                           // bInterval
		
		0x07,							// bLength
		0x05,            				// bDescriptorType
		0x82,                           // bEndpointAddress (0x80: IN)
		0x02,                           // bmAttributes (0:Control, 1:Iso, 2:Bulk, 3:Int)
		0x20, 0x00,						// wMaxPacketSize
		0x00,                           // bInterval
	};
*/	
	const UInt interrupt = readRegister(ep0intr);
	const UInt status = readRegister(ep0stat);
	// const UInt fstatus = readRegister(ep0fstat);
	// cout << " ep0I=" << interrupt << " ep0S=" << status << " ep0fstat=" << fstatus;
	
	if ((interrupt&0x0002) != 0)
	{
		// check number of bytes in FIFO
		if ((status&0x007F0000) == 0x80000)
		{
			// device request is coming
			UInt32 setupWord;
			SetupRequest setupBuffer;
			
			// read device request data from FIFO
			int j = 0;
			for (int i = 0; i < 2; i++)
			{
			    setupWord = readRegister(ep0fdat);
			    ((UInt8 *)(&setupBuffer))[j++] = (UInt8)(setupWord >> 24);
			    ((UInt8 *)(&setupBuffer))[j++] = (UInt8)(setupWord >> 16);
			    ((UInt8 *)(&setupBuffer))[j++] = (UInt8)(setupWord >> 8);
			    ((UInt8 *)(&setupBuffer))[j++] = (UInt8)setupWord;
			}
			
		//	cout << " Request " << setupBuffer.requestType << " " 
		//		 << setupBuffer.request << " " << setupBuffer.value  << " " 
		//		 << setupBuffer.index   << " " << setupBuffer.length << "\n";
			
			// decode device request
			if (((setupBuffer.requestType)&0x60) == 0 && 
				(setupBuffer.request) == 6)
			{
				switch ((setupBuffer.value >> 8))
				{
					case 0x01:  
						// device descriptor
						writeRegister(ep0fdat, usbDevDscptr[0]);
						writeRegister(ep0fctrl, 0x28000000);
						writeRegister(ep0fdat, usbDevDscptr[1]);
						
						if (setupBuffer.length > 8)
						{
							writeRegister(ep0fdat, usbDevDscptr[2]);
							writeRegister(ep0fctrl, 0x28000000);
							writeRegister(ep0fdat, usbDevDscptr[3]);
							
							writeRegister(ep0fctrl, 0x28000000);
							writeRegister16((ep0fdat + 2), (usbDevDscptr[4] >> 16));
						}
						
					break;
					case 0x02: 
						// configuration descriptor
						writeRegister(ep0fdat, usbConfDscptr[0]);
						writeRegister(ep0fctrl, 0x28000000);
						writeRegister(ep0fdat, usbConfDscptr[1]);
						
						if (setupBuffer.length > 8)
						{
							if (setupBuffer.length == 9)
							{
								writeRegister(ep0fctrl, 0x28000000);
								writeRegister8((ep0fdat + 3), (usbConfDscptr[2] >> 24));
							}
							else
							{
								writeRegister(ep0fdat, usbConfDscptr[2]);
								writeRegister(ep0fctrl, 0x28000000);
								writeRegister(ep0fdat, usbConfDscptr[3]);
								
								writeRegister(ep0fdat, usbConfDscptr[4]);
								writeRegister(ep0fctrl, 0x28000000);
								writeRegister(ep0fdat, usbConfDscptr[5]);
								
								writeRegister(ep0fdat, usbConfDscptr[6]);
								writeRegister(ep0fctrl, 0x28000000);
								writeRegister(ep0fdat, usbConfDscptr[7]);
								
								writeRegister(ep0stat, (readRegister(ep0stat)|4));
							}
						}
					break;
				}
			}
			
			if (((setupBuffer.requestType)&0x60) == 0x40 &&
				(setupBuffer.request) == 1)
			{
				forceError();
				if (inConfigured)
				{
					transmitLength = 0;
					receiveLength = 0;
					recvBufferSize = 0;
					
					receiveEvent.clear();
					transmitEvent.clear();
					transmitPacket.clear();
					
					// flush a r/x FIFOs
					writeRegister(ep0stat + 0x30, 
						(readRegister(ep0stat + 0x30)|2));
					writeRegister(ep0stat + 0x30 * 2, 
						(readRegister(ep0stat + 0x30 * 2)|2));
					
					// resynchronization completed
					synchronizationEvent.signal();
				}
			}
		}
	}

	// interrupt handled
	writeRegister(ep0intr, interrupt);
	return true;
}

//------------------------------------------------------------------------------------------------
// * Mx1UsbPort::ep1Interrupt
//
// Ep1 (OUT) interrupt handler.
//------------------------------------------------------------------------------------------------

Bool Mx1UsbPort::ep1Interrupt()
{
	const UInt interrupt = readRegister(ep0intr + 0x30);
	// cout << " ep1I=" << interrupt;

	if ((interrupt&0x05) != 0)
	{
		// EOF received
		UInt counter = (((readRegister(ep0stat + 0x30)) >> 16) & 0x7F);
		while (counter != 0)
		{
			const UInt receiveDword = readRegister(ep0fdat + 0x30);
			if (counter >= 4)
			{
				recvBuffer[recvBufferSize++] = (UInt8)(receiveDword >> 24);
				recvBuffer[recvBufferSize++] = (UInt8)(receiveDword >> 16);
				recvBuffer[recvBufferSize++] = (UInt8)(receiveDword >> 8);
				recvBuffer[recvBufferSize++] = (UInt8)(receiveDword);
				counter -= 4;
			}
			else
			{
				switch(counter)
				{
				case 3:
					recvBuffer[recvBufferSize++] = (UInt8)(receiveDword >> 24);
					recvBuffer[recvBufferSize++] = (UInt8)(receiveDword >> 16);
					recvBuffer[recvBufferSize++] = (UInt8)(receiveDword >> 8);
					break;
				case 2:
					recvBuffer[recvBufferSize++] = (UInt8)(receiveDword >> 24);
					recvBuffer[recvBufferSize++] = (UInt8)(receiveDword >> 16);
					break;
				case 1:
					recvBuffer[recvBufferSize++] = (UInt8)(receiveDword >> 24);
					break;
				default:
					break;
				};
				
				counter = 0;
			}
		}
		
		receiveEvent.signal();
	}

	// interrupt handled
	writeRegister(ep0intr + 0x30, interrupt);
	return true;
}

//------------------------------------------------------------------------------------------------
// * Mx1UsbPort::ep2Interrupt
//
// Ep2 (IN) interrupt handler.
//------------------------------------------------------------------------------------------------

Bool Mx1UsbPort::ep2Interrupt()
{
	UInt interrupt = readRegister(ep0intr + 0x60);
	// cout << " ep2I=" << interrupt;
	
	// check that current packet is completed
	if ((interrupt&0x01) == 0x01)
	{
		transmitPacket.signal();
	}
	
	// check that current transfer is completed
	if ((interrupt&0x04) == 0x04)
	{
		transmitEvent.signal();
	}

	// interrupt handled
	writeRegister(ep0intr + 0x60, interrupt);
	return true;
}

//------------------------------------------------------------------------------------------------
// * Mx1UsbPort::handleError
//
// Error handler.
//------------------------------------------------------------------------------------------------

void Mx1UsbPort::handleError()
{
	// set flag to indicate an error
	inError = true;
	
	// terminate current transmissions
	receiveEvent.signal();
	transmitPacket.signal();
	transmitEvent.signal();
}

//------------------------------------------------------------------------------------------------
// * Mx1UsbPort::forceError
//
// Force an error condition.
//------------------------------------------------------------------------------------------------

void Mx1UsbPort::forceError()
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
// * Mx1UsbPort::reset
//
// Resets the stream and clears errors.
//------------------------------------------------------------------------------------------------

void Mx1UsbPort::reset()
{
	// make sure that all reads and write are terminated by forcing an error condition
	forceError();

	// wait for completion of synchronization
	if(synchronizationEvent.waitForSeconds(10))
	{
		return;
	}
	
	// clear error flag
	inError = false;
}

//------------------------------------------------------------------------------------------------
// * Mx1UsbPort::read
//
// Read data from the stream.
//------------------------------------------------------------------------------------------------

UInt Mx1UsbPort::read(void *pDestination, UInt length, TimeValue timeout)
{
	// do not continue if we are in an error state
	if(isInError())
	{
		return 0;
	}
		
	LockedSection readLock(readMutex);

	pReceiveBuffer = (UInt8 *)pDestination;
	receiveLength = length;
	
	// read loop
	while (receiveLength != 0)
	{
		if (recvBufferSize == 0)
		{
			receiveEvent.wait();
			if (isInError())
			{
				break;
			}
		}

		{
			UninterruptableSection criticalSection;
			const UInt size = minimum(receiveLength, recvBufferSize);
			
			// copy data
			memoryCopy(pReceiveBuffer, recvBuffer, size);
			
			// adjust read buffers
			receiveLength -= size;
			pReceiveBuffer += size;
			recvBufferSize -= size;
			
			// adjust internal buffers
			memoryCopy(recvBuffer, (void *)&((recvBuffer)[size]), recvBufferSize);
		}
	}
	
	return length - receiveLength;
}

//------------------------------------------------------------------------------------------------
// * Mx1UsbPort::write
//
// Write data to the stream.
//------------------------------------------------------------------------------------------------

inline UInt32 changeEndian(register UInt8 *buf)
{
    return (((UInt32)buf[0] << 24) | ((UInt32)buf[1] << 16) | ((UInt32)buf[2] << 8) | (UInt32)buf[3]);
}

UInt Mx1UsbPort::write(const void *pSource, UInt length, TimeValue timeout)
{
	LockedSection writeLock(writeMutex);
	
	if (length == 0)
	{
		return 0;
	}

	if((length & 31) == 0)
	{
		return write(pSource, length - 4) + write(addToPointer(pSource, (length - 4)), 4);
	}

	// do not continue if we are in an error state
	if(isInError())
	{
		return 0;
	}
	
	// setup transmit state
	pTransmitBuffer = (UInt8 *)pSource;
	transmitLength = length;
	
	// transmit loop
	while (transmitLength > 0)
	{
		if (transmitLength >= 32)
		{
			for (UInt i = 0; i < 7; i++)
			{
				writeRegister(ep0fdat + 0x60, changeEndian((UInt8 *)pTransmitBuffer));
				pTransmitBuffer += 4;
			}
			
			writeRegister(ep0fctrl + 0x60, 0x28000000);
			writeRegister(ep0fdat + 0x60, changeEndian((UInt8 *)pTransmitBuffer));
			pTransmitBuffer += 4;
			
			transmitLength -= 32;
			transmitPacket.wait();
			if (isInError())
			{
				break;
			}
			
			if (transmitLength == 0)
			{
				// zero-length packet
				writeRegister(ep0stat + 0x60, (readRegister(ep0stat + 0x60)|4));

				transmitEvent.wait();
				break;
			}
		}
		else
		{
			for (UInt i = 1; i < transmitLength; i++)
			{
				writeRegister8(ep0fdat + 0x63, *pTransmitBuffer++);
			}
			
			writeRegister(ep0fctrl + 0x60, 0x28000000);
			writeRegister8(ep0fdat + 0x63, *pTransmitBuffer++);
			
			transmitLength = 0;
			transmitEvent.wait();
			break;
		}
	}

	// wait for completion of the write
	return length - transmitLength;
}
