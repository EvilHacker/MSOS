#include "Mx1I2cPort.h"
#include "Mx1InterruptController.h"
#include "../multitasking/TaskScheduler.h"
#include "../multitasking/LockedSection.h"
#include "../multitasking/UninterruptableSection.h"

//------------------------------------------------------------------------------------------------
// * Mx1I2cPort::Mx1I2cPort
//
// Constructor.
//------------------------------------------------------------------------------------------------

Mx1I2cPort::Mx1I2cPort()
{
	transferLength = 0;

	// initialize pins
	{
		UninterruptableSection criticalSection;
		*(volatile UInt *)(mx1RegistersBase + 0x1C020) &= ~(3u << 15);
		*(volatile UInt *)(mx1RegistersBase + 0x1C038) &= ~(3u << 15);
	}

	// register interrupt handler
	TaskScheduler::getCurrentTaskScheduler()->addInterruptHandler(this);

	// default configuration
	configure(400000);

	// enable interrupts
	Mx1InterruptController::getCurrentInterruptController()->setToIrq(getInterruptNumber());
	Mx1InterruptController::getCurrentInterruptController()->enable(getInterruptNumber());
}

//------------------------------------------------------------------------------------------------
// * Mx1I2cPort::Mx1I2cPort
//
// Destructor.
//------------------------------------------------------------------------------------------------

Mx1I2cPort::~Mx1I2cPort()
{
	// disable interrupts
	Mx1InterruptController::getCurrentInterruptController()->disable(getInterruptNumber());

	// turn off the I2C port
	writeRegister(i2cr, 0);

	// unregister interrupt handler
	TaskScheduler::getCurrentTaskScheduler()->removeInterruptHandler(this);
}

//------------------------------------------------------------------------------------------------
// * Mx1I2cPort::read
//
// Read data from the port.
// Returns the number of bytes actually read.
//------------------------------------------------------------------------------------------------

UInt Mx1I2cPort::read(UInt address, void *pDestination, UInt length, TimeValue timeout)
{
	LockedSection transferLock(transferMutex);

	// setup receive state
	receiving = true;
	firstRead = true;
	pTransferBuffer = (UInt8 *)pDestination;
	transferLength = length;

	// start, enable interrupts
	writeRegister(i2cr, 0xF0);

	// write slave address
	writeRegister(i2dr, address | 1);

	// wait for completion of the read
	UInt actualLength;
	if(transferEvent.wait(timeout))
	{
		// read timed-out, cancel read and return number of bytes actually read
		UninterruptableSection critialSection;
		actualLength = length - transferLength;
		transferLength = 0;
		writeRegister(i2cr, 0x80);
	}
	else
	{
		// read completed or aborted by slave
		actualLength = length - transferLength;
	}
	
	return actualLength;
}

//------------------------------------------------------------------------------------------------
// * Mx1I2cPort::write
//
// Write data to the port.
// Returns the number of bytes actually written.
//------------------------------------------------------------------------------------------------

UInt Mx1I2cPort::write(UInt address, const void *pSource, UInt length, TimeValue timeout)
{
	LockedSection transferLock(transferMutex);

	// setup transmit state
	receiving = false;
	pTransferBuffer = (UInt8 *)pSource;
	transferLength = length;

	// start, enable interrupts
	writeRegister(i2cr, 0xF0);

	// write slave address
	writeRegister(i2dr, address);

	// wait for completion of the write
	UInt actualLength;
	if(transferEvent.wait(timeout))
	{
		// write timed-out, cancel write and return number of bytes actually written
		UninterruptableSection critialSection;
		actualLength = length - transferLength;
		transferLength = 0;
		writeRegister(i2cr, 0x80);
	}
	else
	{
		// write completed or aborted by slave
		actualLength = length - transferLength;
	}
	
	return actualLength;
}

//------------------------------------------------------------------------------------------------
// * Mx1I2cPort::configure
//
// Configures the parameters of the I2C port.
//------------------------------------------------------------------------------------------------

void Mx1I2cPort::configure(UInt baudRate)
{
	// todo: figure-out how to map <baudRate> to register value
	// hardwire to 96MHz (HCLK) / 240 = 400KHz for now
	// note: the observed frequency seems to be less than specified in document

	// initialize registers
	writeRegister(ifdr, 0x0F);	// 400KHz
//	writeRegister(ifdr, 0x17);	// 100KHz
	writeRegister(iadr, 0x00);
	writeRegister(i2cr, 0x80);
}

//------------------------------------------------------------------------------------------------
// * Mx1I2cPort::handleInterrupt
//
// Interrupt handler.
//------------------------------------------------------------------------------------------------

Bool Mx1I2cPort::handleInterrupt()
{
	// determine if this is an I2C interrupt
	if(Mx1InterruptController::getCurrentInterruptController()->isPending(getInterruptNumber()))
	{
		UInt status = readRegister(i2sr);

		// clear interrupt flags
		writeRegister(i2sr, 0x00);

		// check if arbitration lost
		if((status & 0x10) != 0)
		{
			// error detected, abort transfer
			transferEvent.signal();

			// stop, disable interrupts
			writeRegister(i2cr, 0x80);
		}

		// check if receiving or transmitting
		else if(receiving)
		{
			if(firstRead)
			{
				firstRead = false;

				if(transferLength == 1)
				{
					// nack, receive mode
					writeRegister(i2cr, 0xE8);
				}
				else
				{
					// receive mode
					writeRegister(i2cr, 0xE0);
				}

				// dummy read
				readRegister(i2dr);
			}
			else
			{
				if(transferLength <= 2)
				{
					if(transferLength == 1)
					{
						// stop, disable interrupts
						writeRegister(i2cr, 0x80);
					}
					else //if(transferLength == 2)
					{
						// nack
						writeRegister(i2cr, 0xE8);
					}
				}

				// read one byte
				*(pTransferBuffer++) = readRegister(i2dr);
				if(--transferLength == 0)
				{
					// transfer complete
					transferEvent.signal();
				}
			}
		}
		else
		{
			// check if transfer completed or slave nack
			if((transferLength == 0) || ((status & 0x01) != 0))
			{
				// stop, disable interrupts
				writeRegister(i2cr, 0x80);

				// transfer complete or aborted by slave
				transferEvent.signal();
			}
			else
			{
				// write one byte
				writeRegister(i2dr, *(pTransferBuffer++));
				--transferLength;
			}
		}

		// interrupt handled
		return true;
	}

	// interrupt not for us
	return false;
}
