#include "Usb2Port.h"
#include "deviceAddresses.h"
#include "../Sa1110Devices/Sa1110InterruptController.h"
#include "../multitasking/TaskScheduler.h"
#include "../multitasking/UninterruptableSection.h"
#include "../multitasking/LockedSection.h"
#include "../multitasking/sleep.h"
#include "../memoryUtilities.h"
#include "../PointerArithmetic.h"
#include "../ArmDevices/AddressTranslationTableBuilder.h"

//------------------------------------------------------------------------------------------------
// * Usb2Port::Usb2Port
//
// Constructor.
//------------------------------------------------------------------------------------------------

Usb2Port::Usb2Port() :
	InterruptHandler(InterruptHandler::irqInterruptLevel, 101),
	wakeUpPin(25),
	flagAInterruptPin(10),//19
	flagBInterruptPin(23),//17
	flagCInterruptPin(27),//18
	intFlagInterruptPin(14)//24
{
	// disable the interrupts
	Sa1110InterruptController::getCurrentInterruptController()->disable(11);
	Sa1110InterruptController::getCurrentInterruptController()->disable(10);

	// initialization
	recvBufferSize = 0;
	setupDataCounter = 0;
	readRequestEnabled = false;
	setupRequestEnabled = false;
	
	// set an error state
	forceError();
	
	// set up interrupt for transmission
	flagAInterruptPin.configureAsInput();
	flagAInterruptPin.configureAsRisingEdgeInterrupt();
	flagAInterruptPin.clearInterrupt();

	flagBInterruptPin.configureAsInput();
	flagBInterruptPin.configureAsRisingEdgeInterrupt();
	flagBInterruptPin.clearInterrupt();

	// set up interrupt for receiving
	flagCInterruptPin.configureAsInput();
	
	intFlagInterruptPin.configureAsInput();
	intFlagInterruptPin.configureAsFallingEdgeInterrupt();
	intFlagInterruptPin.clearInterrupt();
	
	// wake up pin configuration
	wakeUpPin.configureAsOutput();
	wakeUpPin.setValue(1);
	
	// register interrupt handler
	TaskScheduler::getCurrentTaskScheduler()->addInterruptHandler(this);

	// enable the interrupts
	Sa1110InterruptController::getCurrentInterruptController()->setToIrq(10);
	Sa1110InterruptController::getCurrentInterruptController()->setToIrq(11);
	Sa1110InterruptController::getCurrentInterruptController()->enable(10);
	Sa1110InterruptController::getCurrentInterruptController()->enable(11);

	// reset entire chip
	Sa1110GpioPin resetPin(5);//14
	resetPin.configureAsOutput();
	resetPin.setValue(0);
	sleepForMilliseconds(10);
	resetPin.setValue(1);
	sleepForMilliseconds(10);
}

//------------------------------------------------------------------------------------------------
// * Usb2Port::Usb2Port
//
// Destructor.
//------------------------------------------------------------------------------------------------

Usb2Port::~Usb2Port()
{
	// disable the interrupts
	Sa1110InterruptController::getCurrentInterruptController()->disable(10);
	Sa1110InterruptController::getCurrentInterruptController()->disable(11);
	
	// unregister interrupt handler
	TaskScheduler::getCurrentTaskScheduler()->removeInterruptHandler(this);
}

//------------------------------------------------------------------------------------------------
// * Usb2Port::verifySettingsWithHubSpeed
//
// Update USB2 device registers
//------------------------------------------------------------------------------------------------

void Usb2Port::verifySettingsWithHubSpeed()
{
	UInt fnaddress = readRegister(fnaddr);
	if ((fnaddress & 0x80) != 0)
	{
		// set packet sizes for high speed
		writeRegister(ep8pktlenl, highSpeedPacketSizeLsb);
		writeRegister(ep8pktlenh, 0x20 | highSpeedPacketSizeMsb);
		// 16-bit data bus
		writeRegister(ep2pktlenl, highSpeedPacketSizeLsb);
		writeRegister(ep2pktlenh, 0x30 | highSpeedPacketSizeMsb);
		
		// 2IN
		writeRegister(ep6pktlenl, highSpeedPacketSizeLsb);
		writeRegister(ep6pktlenh, 0x30 | highSpeedPacketSizeMsb);
		
		packetSize = (UInt)highSpeedPacketSizeLsb | ((UInt)highSpeedPacketSizeMsb << 8);
	}
	else
	{
		// set packet sizes for full speed
		writeRegister(ep8pktlenl, fullSpeedPacketSizeLsb);
		writeRegister(ep8pktlenh, 0x20 | fullSpeedPacketSizeMsb);
		// 16-bit data bus
		writeRegister(ep2pktlenl, fullSpeedPacketSizeLsb);
		writeRegister(ep2pktlenh, 0x30 | fullSpeedPacketSizeMsb);
		
		// 2IN
		writeRegister(ep6pktlenl, fullSpeedPacketSizeLsb);
		writeRegister(ep6pktlenh, 0x30 | fullSpeedPacketSizeMsb);
		
		packetSize = (UInt)fullSpeedPacketSizeLsb | ((UInt)fullSpeedPacketSizeMsb << 8);
	}
}

//------------------------------------------------------------------------------------------------
// * Usb2Port::configureRegisters
//
// Update USB2 device registers
//------------------------------------------------------------------------------------------------

void Usb2Port::configureRegisters()
{
	// OUT: EP8 - FLAGC EF
	writeRegister(flagscd, 0x0B);
	
	// configure USB2 fifos flags
	// OUT: EP8
	// bit# - value - description
	// 7		1		valid (active)
	// 6		0		direction (out)
	// 5,4		10		bulk
	// 3		0		size (512 bytes)
	// 2		0		stall bit
	// 1,0		10		2x buffer
	writeRegister(ep8cfg, 0xA2);

	// IN:  EP2 - FLAGA EF
	// IN:  EP6 - FLAGB EF
	writeRegister(flagsab, 0xA8);

	// IN:  EP2, EP6
	// bit# - value - description
	// 7		1		valid (active)
	// 6		1		direction (in)
	// 5,4		10		bulk
	// 3		0		size (512 bytes)
	// 2		0		stall bit
	// 1,0		11		3x buffer
	writeRegister(ep2cfg, 0xE3);
	writeRegister(ep6cfg, 0xE3);
}

//------------------------------------------------------------------------------------------------
// * Usb2Port::writeDescriptor
//
// Write USB2 device descriptor
//------------------------------------------------------------------------------------------------

void Usb2Port::writeDescriptor()
{
	static const UInt8 descriptorTable[] = 
	{
		/* Device Descriptor */   
		DSCR_DEVICE_LEN,	/* Descriptor length */
		DSCR_DEVICE,  		/* Decriptor type */
		0x00,0x02,			/* Specification Version (BCD) */
		0x00,//FF				/* Device class */
		0x00,//FF				/* Device sub-class */
		0x00,//FF				/* Device protocol */
		0x40,				/* Maximum packet size */
		0xB3, 0x0F,			/* Vendor ID */
		0x02, 0x00,			/* Product ID (Sample Device) */
		0x00, 0x00,			/* Product version ID */
		0x00,				/* Manufacturer string index */
		0x00,				/* Product string index */
		0x00,				/* Serial number string index */
		0x01,				/* Number of configurations */
	
		/* Device Qualifier Descriptor */
		DSCR_DEVQUAL_LEN,	/* Descriptor length */
		DSCR_DEVQUAL,		/* Decriptor type */
		0x00, 0x02,			/* Specification Version (BCD) */
		0x00,//FF				/* Device class */
		0x00,//FF				/* Device sub-class */
		0x00,//FF				/* Device protocol */
		0x40,				/* Maximum packet size */
		0x01,				/* Number of configurations */
		0x00,				/* Reserved */
	
		/* High-Speed Configuration Descriptor*/
		DSCR_CONFIG_LEN,	/* Descriptor length */
		DSCR_CONFIG,		/* Descriptor type */
		DSCR_CONFIG_LEN  +	/* Total Length (LSB) */
		 DSCR_INTRFC_LEN +
		// 2IN
		 (3*DSCR_ENDPNT_LEN),
		0x00,				/* Total Length (MSB) */
		0x01,				/* Number of interfaces */
		0x01,				/* Configuration number */
		0x00,				/* Configuration string */
		0xA0,				/* Attributes (b7 - buspwr, b6 - selfpwr, b5 - rwu) */
		0x32,				/* Power requirement (div 2 ma) */
	
		/* Interface Descriptor */
		DSCR_INTRFC_LEN,	/* Descriptor length */
		DSCR_INTRFC,		/* Descriptor type */
		0x00,				/* Zero-based index of this interface */
		0x00,				/* Alternate setting */
		// 2IN
		0x03,				/* Number of end points */ 
		0xFF,				/* Interface class */
		0x00,//FF				/* Interface sub class */
		0x00,//FF				/* Interface protocol */
		0x00,				/* Interface descriptor string index */
	      
		/* Endpoint 8 Descriptor */
		DSCR_ENDPNT_LEN,	/* Descriptor length */
		DSCR_ENDPNT,		/* Descriptor type */
		0x08,				/* Endpoint number, and direction */
		ET_BULK,			/* Endpoint type */
		highSpeedPacketSizeLsb,	/* Maximun packet size (LSB) */
		highSpeedPacketSizeMsb,	/* Max packect size (MSB) */
		0x00,				/* Polling interval */
	
		/* Endpoint 2 Descriptor */
		DSCR_ENDPNT_LEN,	/* Descriptor length */
		DSCR_ENDPNT,		/* Descriptor type */
		0x82,				/* Endpoint number, and direction */
		ET_BULK,			/* Endpoint type */
		highSpeedPacketSizeLsb,	/* Maximun packet size (LSB) */
		highSpeedPacketSizeMsb,	/* Max packect size (MSB) */
		0x00,				/* Polling interval */
	
		// 2IN
		/* Endpoint 6 Descriptor */
		DSCR_ENDPNT_LEN,	/* Descriptor length */
		DSCR_ENDPNT,		/* Descriptor type */
		0x86,				/* Endpoint number, and direction */
		ET_BULK,			/* Endpoint type */
		highSpeedPacketSizeLsb,	/* Maximun packet size (LSB) */
		highSpeedPacketSizeMsb,	/* Max packect size (MSB) */
		0x00,				/* Polling interval */
		/* End of High-Speed Configuration Descriptor */
	
		/* Full-Speed Configuration Descriptor */
		DSCR_CONFIG_LEN,	/* Descriptor length */
		DSCR_CONFIG,		/* Descriptor type */
		 DSCR_CONFIG_LEN +	/* Total Length (LSB) */
		 DSCR_INTRFC_LEN +
		// 2IN
		 (3*DSCR_ENDPNT_LEN),
		0x00,				/* Total Length (MSB) */
		0x01,				/* Number of interfaces */
		0x01,				/* Configuration number */
		0x00,				/* Configuration string */
		0xA0,				/* Attributes (b7 - buspwr, b6 - selfpwr, b5 - rwu) */
		0x32,				/* Power requirement (div 2 ma) */
	
		/* Interface Descriptor */
		DSCR_INTRFC_LEN,	/* Descriptor length */
		DSCR_INTRFC,		/* Descriptor type */
		0x00,				/* Zero-based index of this interface */
		0x00,				/* Alternate setting */
		// 2IN
		0x03,				/* Number of end points */ 
		0xFF,				/* Interface class */
		0x00,//FF				/* Interface sub class */
		0x00,//FF				/* Interface sub sub class */
		0x00,				/* Interface descriptor string index */
	      
		/* Endpoint 8 Descriptor */
		DSCR_ENDPNT_LEN,	/* Descriptor length */
		DSCR_ENDPNT,		/* Descriptor type */
		0x08,				/* Endpoint number, and direction */
		ET_BULK,			/* Endpoint type */
		fullSpeedPacketSizeLsb,	/* Maximun packet size (LSB) */
		fullSpeedPacketSizeMsb,	/* Max packect size (MSB) */
		0x00,				/* Polling interval */
	
		/* Endpoint 2 Descriptor */
		DSCR_ENDPNT_LEN,	/* Descriptor length */
		DSCR_ENDPNT,		/* Descriptor type */
		0x82,				/* Endpoint number, and direction */
		ET_BULK,			/* Endpoint type */
		fullSpeedPacketSizeLsb, /* Maximun packet size (LSB) */
		fullSpeedPacketSizeMsb,	/* Max packect size (MSB) */
		0x00,				/* Polling interval */
	
		// 2IN
		/* Endpoint 6 Descriptor */
		DSCR_ENDPNT_LEN,	/* Descriptor length */
		DSCR_ENDPNT,		/* Descriptor type */
		0x86,				/* Endpoint number, and direction */
		ET_BULK,			/* Endpoint type */
		fullSpeedPacketSizeLsb, /* Maximun packet size (LSB) */
		fullSpeedPacketSizeMsb,	/* Max packect size (MSB) */
		0x00				/* Polling interval */
		/* End of Full-Speed Configuration Descriptor */ 
	};	
	
	const UInt16 descriptorLength = sizeof(descriptorTable);

	UninterruptableSection criticalSection;
	*(volatile UInt8 *)baseCommandAddress = (UInt8)(desc | 0x80);
	*(volatile UInt8 *)baseCommandAddress = (UInt8)((descriptorLength >> 4) & 0xF);
	*(volatile UInt8 *)baseCommandAddress = (UInt8)(descriptorLength & 0xF);
	*(volatile UInt8 *)baseCommandAddress = (UInt8)((descriptorLength >> 12) & 0xF);
	*(volatile UInt8 *)baseCommandAddress = (UInt8)((descriptorLength >> 8) & 0xF);

	for (UInt i = 0; i < descriptorLength; i++)
	{
		const UInt8 byte = descriptorTable[i];
		*(volatile UInt8 *)baseCommandAddress = (UInt8)((byte >> 4) & 0xF);
		*(volatile UInt8 *)baseCommandAddress = (UInt8)(byte & 0xF);
	}
}

//------------------------------------------------------------------------------------------------
// * Usb2Port::readRegister
//
// Read a USB 2 register.
//------------------------------------------------------------------------------------------------

UInt Usb2Port::readRegister(RegisterAddress address)
{
	{
		// disable int interrupt
		UninterruptableSection criticalSection;
		readRequestEnabled = true;
		*(volatile UInt8 *)baseCommandAddress = address | 0xC0;
	}
	
	// wait for data
	readEvent.wait();
	return readRegisterValue;
}

//------------------------------------------------------------------------------------------------
// * Usb2Port::writeRegister
//
// Write a USB 2 register.
//------------------------------------------------------------------------------------------------

void Usb2Port::writeRegister(RegisterAddress address, UInt value)
{
	// disable int interrupt
	UninterruptableSection criticalSection;
	*(volatile UInt8 *)baseCommandAddress = address | 0x80;
	*(volatile UInt8 *)baseCommandAddress = (value >> 4) & 0xF;
	*(volatile UInt8 *)baseCommandAddress = value & 0xF;
}

//------------------------------------------------------------------------------------------------
// * Usb2Port::read
//
// Read data from the stream.
//------------------------------------------------------------------------------------------------

UInt Usb2Port::read(void *pDestination, UInt length)
{
	// do not continue if we are in an error state or nothing to read
	if(isInError())
	{
		return 0;
	}
	
	LockedSection readLock(readMutex);

	// setup receive state
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
				return length - receiveLength;
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
	
	return length;
}

//------------------------------------------------------------------------------------------------
// * Usb2Port::write
//
// Write data to the stream.
//------------------------------------------------------------------------------------------------

static void burstWrite(void *pDestination, const void *pSource, UInt length)
{
	const UInt offset = (const UInt)(((UInt)pSource)&3u);
	if (offset == 0)
	{
		asm
		{
	begin:
			ldmia	pSource!, {a3, a4, ip, lr}
			stmia	pDestination, {a3, a4, ip, lr}
			subs	length, length, #16
			bne		begin
		}
	}
	else
	{
		UInt16 word = 0;
		const UInt8 * data = (UInt8 *)pSource;
		for (UInt i = 0; i < length; i += 2)
		{
			word = 0; word |= data[i + 1]; word <<= 8; word |= data[i];
			*(volatile UInt16 *)pDestination = word;
		}
	}
}

UInt Usb2Port::write(const void *pSource, UInt length)
{
	// do not continue if we are in an error state or nothing to write
	if (isInError())
	{
		return 0;
	}

	LockedSection writeLock(writeMutex);
	
	// setup transmit state
	pTransmitBuffer = (UInt8 *)pSource;
	transmitLength = length;
	
	// variables
	UInt size;
	UInt sendLength;
	UInt padding[4];
	UInt blockLength2 = 0;
	UInt blockLength6 = 0;

	Bool ep2 = true;
	Bool flush2 = false;
	Bool flush6 = false;
	Bool complete2 = false;
	Bool complete6 = false;
	Bool lastPacket = false;

	// operation contants
	const UInt8 *pTransmit2Buffer = pTransmitBuffer;
	const UInt8 *pTransmit6Buffer = pTransmitBuffer + writeBlockSize;
	const UInt8 *pTransmitBufferEnd = pTransmitBuffer + transmitLength;
	const UInt blockSizeMinusPacket = writeBlockSize - packetSize;
	const UInt numberOfBlocks = transmitLength / writeBlockSize;
	const UInt numberOfBlockedBytes = numberOfBlocks * writeBlockSize;
	const Bool evenNumber = ((numberOfBlocks&1u) == 0) ? true : false;

	// conditioned initialization
	if (transmitLength == numberOfBlockedBytes)
	{
		flush6 = complete6 = !evenNumber;
	}
	else
	{
		flush6 = complete6 = evenNumber;
	}

	// before transfer all fifos are empty
	transmitFifo2Empty.signal();
	transmitFifo6Empty.signal();
	
	// transmitt loop
	while (transmitLength != 0)
	{
		if (ep2)
		{
			ep2 = false;
			if (pTransmit2Buffer < pTransmitBufferEnd)
			{
				size = (UInt)pTransmitBufferEnd - (UInt)pTransmit2Buffer;
				if (size > packetSize)
				{
					lastPacket = false;
					sendLength = packetSize;
					if (blockLength2 == blockSizeMinusPacket)
					{
						lastPacket = true;
					}
				}
				else
				{
					lastPacket = true;
					sendLength = size;
				}

				delay();
				transmitFifo2Empty.wait();
				if (isInError())
				{
					return length - transmitLength;
				}
				
				size = (sendLength & 0xFFF0);
				if (size != 0)
				{
					burstWrite((void *)baseFifo2Address, pTransmit2Buffer, size);
					pTransmit2Buffer += size;
				}
				
				transmitLength -= sendLength;
				blockLength2 += size;
				
				if (lastPacket)
				{
					size = (sendLength & 0xF);
					if (size != 0)
					{
						memoryCopy(padding, pTransmit2Buffer, size);
						burstWrite((void *)baseFifo2Address, padding, 16);
						pTransmit2Buffer += size;
						blockLength2 += 16;
						sendLength &= 0xFFF0;
						sendLength += 16;
					}

					if (blockLength2 == writeBlockSize)
					{
						pTransmit2Buffer += writeBlockSize;
						blockLength2 = 0;
					}
					else
					{
						flush2 = true;
						if (sendLength == packetSize)
						{
							complete2 = true;
						}
					}
				}
			}
		}
		else
		{
			ep2 = true;
			if (pTransmit6Buffer < pTransmitBufferEnd)
			{
				size = (UInt)pTransmitBufferEnd - (UInt)pTransmit6Buffer;
				if (size > packetSize)
				{
					lastPacket = false;
					sendLength = packetSize;
					if (blockLength6 == blockSizeMinusPacket)
					{
						lastPacket = true;
					}
				}
				else
				{
					lastPacket = true;
					sendLength = size;
				}

				delay();
				transmitFifo6Empty.wait();
				if (isInError())
				{
					return length - transmitLength;
				}
				
				size = (sendLength & 0xFFF0);
				if (size != 0)
				{
					burstWrite((void *)baseFifo6Address, pTransmit6Buffer, size);
					pTransmit6Buffer += size;
				}
				
				transmitLength -= sendLength;
				blockLength6 += size;

				if (lastPacket)
				{
					size = (sendLength & 0xF);
					if (size != 0)
					{
						memoryCopy(padding, pTransmit6Buffer, size);
						burstWrite((void *)baseFifo6Address, padding, 16);
						pTransmit6Buffer += size;
						blockLength6 += 16;
						sendLength &= 0xFFF0;
						sendLength += 16;
					}
				
					if (blockLength6 == writeBlockSize)
					{
						pTransmit6Buffer += writeBlockSize;
						blockLength6 = 0;
					}
					else
					{
						flush6 = true;
						if (sendLength == packetSize)
						{
							complete6 = true;
						}
					}
				}
			}
		}
	}

	// endpoint 2 flush required
	if (flush2)
	{
		if (complete2)
		{
			delay(0);
			transmitFifo2Empty.wait();
			if (isInError())
			{
				return length - transmitLength;
			}
			
			*(volatile UInt16 *)baseFifo2Address = 0;
		}
		// force to send a packet
		writeRegister(inpktendflush, 2);
	}
	transmitFifo2Empty.wait();
	if (isInError())
	{
		return length - transmitLength;
	}
	
	// endpoint 6 flush required
	if (flush6)
	{
		if (complete6)
		{
			delay(5);
			transmitFifo6Empty.wait();
			if (isInError())
			{
				return length - transmitLength;
			}

			*(volatile UInt16 *)baseFifo6Address = 0;
		}
		// force to send a packet
		writeRegister(inpktendflush, 6);
	}
	transmitFifo6Empty.wait();
	if (isInError())
	{
		return length - transmitLength;
	}

	return length;
}

//------------------------------------------------------------------------------------------------
// * Usb2Port::forceError
//
// Force an error condition.
//------------------------------------------------------------------------------------------------

void Usb2Port::forceError()
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
// * Usb2Port::reset
//
// Resets the stream and clears errors.
//------------------------------------------------------------------------------------------------

void Usb2Port::reset()
{
	// make sure that all reads and write are terminated by forcing an error condition
	forceError();

	// wait for completion of synchronization
	if(synchronizationEvent.waitForSeconds(10))
	{
		return;
	}

	// update register settings 
	verifySettingsWithHubSpeed();

	// flush FIFOs
	writeRegister(inpktendflush, 0xF0);
	
	// check write position
	while (readMutex.isLocked() || 
		   writeMutex.isLocked())
	{
		sleepForMilliseconds(1);
	}
					
	receiveEvent.clear();
	
	// clear error flag
	inError = false;
}

//------------------------------------------------------------------------------------------------
// * Usb2Port::handleInterrupt
//
// Interrupt handler.
//------------------------------------------------------------------------------------------------

Bool Usb2Port::handleInterrupt()
{
	// determine if this interrupt is for us
	if(intFlagInterruptPin.isInterruptPending())
	{
		// INT interrupt
		handleIntFlagInterrupt();
		return true;
	}

	if(flagAInterruptPin.isInterruptPending())
	{
		// flag B interrupt
		handleFlagAInterrupt();
		return true;
	}

	if(flagBInterruptPin.isInterruptPending())
	{
		// flag B interrupt
		handleFlagBInterrupt();
		return true;
	}
	
	// interrupt not for us
	return false;
}

//------------------------------------------------------------------------------------------------
// * Usb2Port::handleFlagAInterrupt
//
// Interrupt handler.
//------------------------------------------------------------------------------------------------

void Usb2Port::handleFlagAInterrupt()
{
	// handle transmit interrupt
	transmitFifo2Empty.signal();
	flagAInterruptPin.clearInterrupt();
}

//------------------------------------------------------------------------------------------------
// * Usb2Port::handleFlagBInterrupt
//
// Interrupt handler.
//------------------------------------------------------------------------------------------------

void Usb2Port::handleFlagBInterrupt()
{
	// handle transmit interrupt
	transmitFifo6Empty.signal();
	flagBInterruptPin.clearInterrupt();
}

//------------------------------------------------------------------------------------------------
// * Usb2Port::handleIntFlagInterrupt
//
// Interrupt handler.
//------------------------------------------------------------------------------------------------

void Usb2Port::handleIntFlagInterrupt()
{
	// handle interrupt
	if (readRequestEnabled)
	{
		readRequestEnabled = false;
		readRegisterValue = *(volatile UInt8 *)baseCommandAddress;
		readEvent.signal();
	}
	else if (setupRequestEnabled)
	{
		setupBuffer[setupDataCounter++] = *(volatile UInt8 *)baseCommandAddress;
		if (setupDataCounter < 8)
		{
			*(volatile UInt8 *)baseCommandAddress = setup | 0xC0;
		}
		else
		{
			setupRequestEnabled = false;
			if ((setupBuffer[0] & 0x40) != 0 &&  // vendor request
				 setupBuffer[1] == (UInt8)1)
			{
				writeBlockSize = 0;
				memoryCopy(&writeBlockSize, &setupBuffer[2], 2);
				writeBlockSize <<= 9;
				
				// set error state
				forceError();
				
				// reset buffers
				transmitLength = 0;
				receiveLength = 0;
				recvBufferSize = 0;
				
				// handle vendor request
				writeRegister(ep0bc, 0);
			
				// resynchronization completed
				synchronizationEvent.signal();
			}
			else
			{
				// stall on other requests
				writeRegister(setup, 0xFF);
			}
		}
	}
	else
	{
		UInt8 interruptStatus = *(volatile UInt8 *)baseCommandAddress;
		switch(interruptStatus)
		{
			case 0x01: // ready
				/* configure registers */
				/* save descriptor data */
				{
					writeRegister(ifconfig, 0);
					wakeUpPin.setValue(0);
					configureRegisters();
					writeDescriptor();
					
					// enter low-power mode
					suspended = true;
					writeRegister(ifconfig, 4);
					break;
				}
			case 0x02: // bus activity
				/* move device to low-power mode */
				{
					suspended = !suspended;
					if (suspended)
					{
						// enter low-power mode
						writeRegister(ifconfig, 4);
					}
					else
					{
						// back to full-power mode
						writeRegister(ifconfig, 0);
					}
					break;
				}
			case 0x04: // enumeration completes
				{
					forceError();
					break;
				}
			case 0x20: // receive fifo is not empty
				/* read whole received data into internal buffer */
				{
					while (flagCInterruptPin.getValue() == 0)
					{
						recvBuffer[recvBufferSize++] = *(volatile UInt8 *)baseFifo8Address;
					};
					
					receiveEvent.signal();
					break;
				}
			case 0x40: // ep0buf
				break;
			case 0x80: // setup
				/* endpoint 0 setup */
				/* next eight interrupts are setup data */
				/* parse the interrupt register value */
				{
					setupDataCounter = 0;
					setupRequestEnabled = true;
					*(volatile UInt8 *)baseCommandAddress = setup | 0xC0;
					break;
				}
			default:
				break;
		};
	}
	
	intFlagInterruptPin.clearInterrupt();
}

//------------------------------------------------------------------------------------------------
// * Usb2Port::handleError
//
// Error handler.
//------------------------------------------------------------------------------------------------

void Usb2Port::handleError()
{
	// set flag to indicate an error
	inError = true;

	// terminate current transmissions
	receiveEvent.signal();
	transmitFifo2Empty.signal();
	transmitFifo6Empty.signal();
}
