#include "Sa1110UartPort.h"
#include "Sa1110DeviceAddresses.h"
#include "Sa1110InterruptController.h"
#include "../../multitasking/TaskScheduler.h"
#include "../../multitasking/UninterruptableSection.h"
#include "../../multitasking/sleep.h"

//------------------------------------------------------------------------------------------------
// * Sa1110UartPort::Sa1110UartPort
//
// Constructor.
//------------------------------------------------------------------------------------------------

Sa1110UartPort::Sa1110UartPort(Sa1110UartPort::Port port) :
	port(port)
{
	receiveLength = 0;
	transmitLength = 0;
	inError = false;
	synchronizing = false;

	// port dependent initialization
	switch(port)
	{
		case port1:
		{
			// base address of all UART registers for this port
			registerBase = sa1110PeripheralControlBase + 0x10000;

			// use TXD1 and RXD1 pins instead of GPIO 14 and 15
			*(volatile UInt *)(sa1110SystemControlBase + 0x60008) &= ~(1u << 13);

			// select UART instead of GPCLK
			*(volatile UInt *)(sa1110PeripheralControlBase + 0x20060) = 0x01;
			break;
		}
		case port2:
		{
			// base address of all UART registers for this port
			registerBase = sa1110PeripheralControlBase + 0x30000;

			// select UART instead of IrDA
			*(volatile UInt *)(sa1110PeripheralControlBase + 0x30010) = 0;
			break;
		}
		case port3:
		{
			// base address of all UART registers for this port
			registerBase = sa1110PeripheralControlBase + 0x50000;
			break;
		}
	}

	// register interrupt handler
	TaskScheduler::getCurrentTaskScheduler()->addInterruptHandler(this);

	// default configuration (maximum rate)
	configure(115200, 8, 1, false, false);

	// enable the interrupt
	Sa1110InterruptController::getCurrentInterruptController()->setToIrq(getInterruptNumber());
	Sa1110InterruptController::getCurrentInterruptController()->enable(getInterruptNumber());
}

//------------------------------------------------------------------------------------------------
// * Sa1110UartPort::Sa1110UartPort
//
// Destructor.
//------------------------------------------------------------------------------------------------

Sa1110UartPort::~Sa1110UartPort()
{
	// disable the interrupt
	Sa1110InterruptController::getCurrentInterruptController()->disable(getInterruptNumber());

	// turn off the serial port
	writeRegister(utcr3, 0);

	// unregister interrupt handler
	TaskScheduler::getCurrentTaskScheduler()->removeInterruptHandler(this);
}

//------------------------------------------------------------------------------------------------
// * Sa1110UartPort::read
//
// Read data from the stream.
//------------------------------------------------------------------------------------------------

UInt Sa1110UartPort::read(void *pDestination, UInt length, TimeValue timeout)
{
	// do not continue if we are in an error state
	if(isInError())
	{
		return 0;
	}

	// setup receive state
	pReceiveBuffer = (UInt8 *)pDestination;
	receiveLength = length;

	// enable receiver interrupt
	maskInterrupts();

	// wait for completion of the read
	if(receiveEvent.wait(timeout))
	{
		// read timed-out, cancel read and return number of bytes actually read
		UninterruptableSection critialSection;
		const UInt actualLength = length - receiveLength;
		receiveLength = 0;
		maskInterrupts();
		return actualLength;
	}
	else
	{
		// entire read completed
		return length;
	}
}

//------------------------------------------------------------------------------------------------
// * Sa1110UartPort::write
//
// Write data to the stream.
//------------------------------------------------------------------------------------------------

UInt Sa1110UartPort::write(const void *pSource, UInt length, TimeValue timeout)
{
	// do not continue if we are in an error state
	if(isInError())
	{
		return 0;
	}

	// setup transmit state
	pTransmitBuffer = (UInt8 *)pSource;
	transmitLength = length;

	// enable transmitter interrupt
	maskInterrupts();

	// wait for completion of the write
	if(transmitEvent.wait(timeout))
	{
		// write timed-out, cancel write and return number of bytes actually written
		UninterruptableSection critialSection;
		const UInt actualLength = length - transmitLength;
		transmitLength = 0;
		maskInterrupts();
		return actualLength;
	}
	else
	{
		// entire write completed
		return length;
	}
}

//------------------------------------------------------------------------------------------------
// * Sa1110UartPort::forceError
//
// Force an error condition.
//------------------------------------------------------------------------------------------------

void Sa1110UartPort::forceError()
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
// * Sa1110UartPort::reset
//
// Resets the stream and clears errors.
//------------------------------------------------------------------------------------------------

void Sa1110UartPort::reset()
{
	// make sure that all reads and write are terminated by forcing an error condition
	forceError();

	// wait for the transmit FIFO to empty
	while((readRegister(utsr1) & 0x01) != 0)
	{
		// wait before checking again
		Timer *pTimer = TaskScheduler::getCurrentTaskScheduler()->getTimer();
		sleepForTicks(10 * pTimer->getFrequency() / 115200, pTimer);
	}

	// set flag to indicate that we are resynchronizing
	synchronizing = true;

	// start transmitting a break
	writeRegister(utcr3, 0x0F);

	// wait for completion of synchronization
	if(synchronizationEvent.waitForSeconds(10))
	{
		// synchronization timed-out, remain in error
		synchronizing = false;
		forceError();
	}

	// wait for the transmit FIFO to empty
	while((readRegister(utsr1) & 0x01) != 0)
	{
		// wait before checking again
		Timer *pTimer = TaskScheduler::getCurrentTaskScheduler()->getTimer();
		sleepForTicks(10 * pTimer->getFrequency() / 115200, pTimer);
	}

	// stop the break
	maskInterrupts();
}

//------------------------------------------------------------------------------------------------
// * Sa1110UartPort::configure
//
// Configures the parameters of the serial port.
//------------------------------------------------------------------------------------------------

void Sa1110UartPort::configure(
	UInt baudRate,
	UInt numberOfDataBits,
	UInt numberOfStopBits,
	Bool parityEnabled,
	Bool evenParity)
{
	// turn serial port off
	writeRegister(utcr3, 0);

	// clear interrupt status bits
	writeRegister(utsr0, 0xFF);

	// set line parameters
	UInt cr0 = 0;
	if(numberOfDataBits == 8)
	{
		cr0 |= 0x08;
	}
	if(numberOfStopBits == 2)
	{
		cr0 |= 0x04;
	}
	if(parityEnabled)
	{
		cr0 |= 0x01;
		if(evenParity)
		{
			cr0 |= 0x02;
		}
	}
	writeRegister(utcr0, cr0);

	// set baud rate
	const UInt clockDivider = (3686400 / 16 / baudRate) - 1;
	writeRegister(utcr1, clockDivider >> 8);
	writeRegister(utcr2, clockDivider);

	// turn serial port on
	maskInterrupts();
}

//------------------------------------------------------------------------------------------------
// * Sa1110UartPort::maskInterrupts
//
// Enables or disables transmitter and receiver interrupts as required.
//------------------------------------------------------------------------------------------------

void Sa1110UartPort::maskInterrupts()
{
	UninterruptableSection criticalSection;

	// start with both receiver and transmitter enabled
	UInt cr3 = 0x03;

	// check if there is a pending read
	if(receiveLength != 0)
	{
		// enable receiver interrupt
		cr3 |= 0x08;
	}

	// check if there is a pending write
	if(transmitLength != 0)
	{
		// enable transmitter interrupt
		cr3 |= 0x10;
	}

	// write the register
	writeRegister(utcr3, cr3);
}

//------------------------------------------------------------------------------------------------
// * Sa1110UartPort::handleInterrupt
//
// Interrupt handler.
//------------------------------------------------------------------------------------------------

Bool Sa1110UartPort::handleInterrupt()
{
	// determine if this interrupt is for us
	if(Sa1110InterruptController::getCurrentInterruptController()->isPending(getInterruptNumber()))
	{
		// receive data
		if(receiveLength != 0)
		{
			UInt sr1;
			while(((sr1 = readRegister(utsr1)) & 0x02) != 0)
			{
				// receive one byte
				*(pReceiveBuffer++) = readRegister(utdr0);

				// check for hardware detected errors
				if((sr1 & 0x70) != 0)
				{
					// handle the error condition
					handleError();

					break;
				}

				// check if read completed
				if(--receiveLength == 0)
				{
					// signal completion of a read
					receiveEvent.signal();

					// disable receiver interrupt
					maskInterrupts();

					break;
				}
			}
		}

		// transmit data
		if(transmitLength != 0)
		{
			while((readRegister(utsr1) & 0x04) != 0)
			{
				// transmit one byte
				writeRegister(utdr0, *(pTransmitBuffer++));
	
				// check if write completed
				if(--transmitLength == 0)
				{
					// signal completion of write
					transmitEvent.signal();
	
					// disable transmitter interrupt
					maskInterrupts();
	
					break;
				}
			}
		}

		// check for other interrupt flags
		UInt sr0 = readRegister(utsr0);
		if(sr0 != 0)
		{
			// check for end of break
			if(synchronizing && (sr0 & 0x10) != 0)
			{
				// synchronization complete
				inError = false;
				synchronizing = false;
				synchronizationEvent.signal();
			}

			// check for begin of break
			if((sr0 & 0x08) != 0)
			{
				// synchronization initiated
				handleError();

				// flush receive FIFO
				while((readRegister(utsr1) & 0x02) != 0)
				{
					readRegister(utdr0);
				}
			}

			// clear the interrupts
			writeRegister(utsr0, sr0);
		}

		// interrupt handled
		return true;
	}

	// interrupt not for us
	return false;
}

//------------------------------------------------------------------------------------------------
// * Sa1110UartPort::handleError
//
// Error handler.
//------------------------------------------------------------------------------------------------

void Sa1110UartPort::handleError()
{
	// set flag to indicate an error
	inError = true;

	// terminate current transmissions
	if(receiveLength != 0)
	{
		receiveLength = 0;
		receiveEvent.signal();
	}
	if(transmitLength != 0)
	{
		transmitLength = 0;
		transmitEvent.signal();
	}
}
