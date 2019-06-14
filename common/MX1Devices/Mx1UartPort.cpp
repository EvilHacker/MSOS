#include "Mx1UartPort.h"
#include "Mx1InterruptController.h"
#include "deviceAddresses.h"
#include "../multitasking/TaskScheduler.h"
#include "../multitasking/UninterruptableSection.h"
#include "../multitasking/sleep.h"

//------------------------------------------------------------------------------------------------
// * Mx1UartPort::Mx1UartPort
//
// Constructor.
//------------------------------------------------------------------------------------------------

Mx1UartPort::Mx1UartPort(Mx1UartPort::Port port, Bool synchronizeOnReset) :
	port(port),
	synchronizeOnReset(synchronizeOnReset)
{
	receiveLength = 0;
	transmitLength = 0;
	inError = false;
	synchronizing = false;

	// port dependent initialization
	switch(port)
	{
		default:
		case port1:
		{
			// base address of all UART registers for this port
			registerBase = mx1RegistersBase + 0x6000;

			// initialize Tx and Rx pins
			{
				UninterruptableSection criticalSection;
				*(volatile UInt *)(mx1RegistersBase + 0x1C200) |=  (1u << 11);
				*(volatile UInt *)(mx1RegistersBase + 0x1C200) &= ~(2u << 11);
				*(volatile UInt *)(mx1RegistersBase + 0x1C220) &= ~(3u << 11);
				*(volatile UInt *)(mx1RegistersBase + 0x1C238) &= ~(3u << 11);
			}
			break;
		}
		case port2:
		{
			// base address of all UART registers for this port
			registerBase = mx1RegistersBase + 0x7000;

			// initialize Tx and Rx pins
			{
				UninterruptableSection criticalSection;
				*(volatile UInt *)(mx1RegistersBase + 0x1C100) |=  (1u << 30);
				*(volatile UInt *)(mx1RegistersBase + 0x1C100) &= ~(2u << 30);
				*(volatile UInt *)(mx1RegistersBase + 0x1C120) &= ~(3u << 30);
				*(volatile UInt *)(mx1RegistersBase + 0x1C138) &= ~(3u << 30);
			}
			break;
		}
	}

	// register interrupt handler
	TaskScheduler::getCurrentTaskScheduler()->addInterruptHandler(this);

	// default configuration
	configure(115200, 8, 1, false, false);

	// enable interrupts
	Mx1InterruptController::getCurrentInterruptController()->setToIrq(getTransmitterInterruptNumber());
	Mx1InterruptController::getCurrentInterruptController()->setToIrq(getReceiverInterruptNumber());
	Mx1InterruptController::getCurrentInterruptController()->setToIrq(getControlInterruptNumber());
	Mx1InterruptController::getCurrentInterruptController()->enable(getTransmitterInterruptNumber());
	Mx1InterruptController::getCurrentInterruptController()->enable(getReceiverInterruptNumber());
	Mx1InterruptController::getCurrentInterruptController()->enable(getControlInterruptNumber());
}

//------------------------------------------------------------------------------------------------
// * Mx1UartPort::Mx1UartPort
//
// Destructor.
//------------------------------------------------------------------------------------------------

Mx1UartPort::~Mx1UartPort()
{
	// disable interrupts
	Mx1InterruptController::getCurrentInterruptController()->disable(getControlInterruptNumber());
	Mx1InterruptController::getCurrentInterruptController()->disable(getReceiverInterruptNumber());
	Mx1InterruptController::getCurrentInterruptController()->disable(getTransmitterInterruptNumber());

	// turn off the serial port
	writeRegister(ucr2, 0);

	// unregister interrupt handler
	TaskScheduler::getCurrentTaskScheduler()->removeInterruptHandler(this);
}

//------------------------------------------------------------------------------------------------
// * Mx1UartPort::read
//
// Read data from the stream.
//------------------------------------------------------------------------------------------------

UInt Mx1UartPort::read(void *pDestination, UInt length, TimeValue timeout)
{
	// do not continue if we are in an error state
	if(length == 0 || isInError())
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
// * Mx1UartPort::write
//
// Write data to the stream.
//------------------------------------------------------------------------------------------------

UInt Mx1UartPort::write(const void *pSource, UInt length, TimeValue timeout)
{
	// do not continue if we are in an error state
	if(length == 0 || isInError())
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
// * Mx1UartPort::forceError
//
// Force an error condition.
//------------------------------------------------------------------------------------------------

void Mx1UartPort::forceError()
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
// * Mx1UartPort::reset
//
// Resets the stream and clears errors.
//------------------------------------------------------------------------------------------------

void Mx1UartPort::reset()
{
	// make sure that all reads and writes are terminated by forcing an error condition
	forceError();

	if(synchronizeOnReset)
	{
		// get default timer and frequency
		Timer *pTimer = TaskScheduler::getCurrentTaskScheduler()->getTimer();
		const TimeValue frequency = pTimer->getFrequency();
	
		// wait for the transmit FIFO to empty
		while((readRegister(usr2) & 0x4000) == 0)
		{
			// wait before checking again
			sleepForTicks(frequency / (115200 / 10), pTimer);
		}
	
		// assert break for a minimal time
		synchronizing = true;
		maskInterrupts();
		sleepForTicks(frequency / (9600 / 10), pTimer);
	
		// wait for break from other side
		if(synchronizationEvent.waitForSeconds(1))
		{
			// synchronization timed-out, remain in error, deassert break for a minimal time
			synchronizing = false;
			maskInterrupts();
			sleepForTicks(frequency / (9600 / 10), pTimer);
		}
		else
		{
			// synchronization completed, clear error
			inError = false;
			synchronizing = false;
		}
	}
	else
	{
		// clear error condition
		inError = false;
	}
}

//------------------------------------------------------------------------------------------------
// * Mx1UartPort::configure
//
// Configures the parameters of the serial port.
//------------------------------------------------------------------------------------------------

void Mx1UartPort::configure(
	UInt baudRate,
	UInt numberOfDataBits,
	UInt numberOfStopBits,
	Bool parityEnabled,
	Bool evenParity)
{
	// reset serial port
	writeRegister(ucr2, 0);

	// set line parameters
	UInt cr2 = 0x5007;
	if(numberOfDataBits == 8)
	{
		cr2 |= 0x0020;
	}
	if(numberOfStopBits == 2)
	{
		cr2 |= 0x0040;
	}
	if(parityEnabled)
	{
		cr2 |= 0x0100;
		if(!evenParity)
		{
			cr2 |= 0x0080;
		}
	}

	// initialize control registers
	writeRegister(ucr1, 0x0005);
	writeRegister(ucr2, cr2);
	writeRegister(ucr3, 0x0000);
	writeRegister(ucr4, 0x8044);
	writeRegister(ufcr, 0x4290);

	// set baud rate (assuming System PLL is 96.000MHz)
	const UInt pclkdiv1 = (*(volatile UInt *)(mx1RegistersBase + 0x1B020) & 0xF) + 1;
	writeRegister(ubir, pclkdiv1 * 0x100000 * (UInt64)baudRate / 96000000 - 1);
	writeRegister(ubmr, 0xFFFF);

	maskInterrupts();
}

//------------------------------------------------------------------------------------------------
// * Mx1UartPort::maskInterrupts
//
// Enables or disables transmitter and receiver interrupts as required.
// Also asserts outgoing break signal as required.
//------------------------------------------------------------------------------------------------

void Mx1UartPort::maskInterrupts()
{
	UninterruptableSection criticalSection;

	// start with default value
	UInt cr1 = 0x0005;

	// check if synchronizing
	if(synchronizing)
	{
		// enable receiver interrupt and assert break
		cr1 |= 0x1210;
	}

	// check if there is a pending read
	if(receiveLength != 0)
	{
		// enable receiver interrupt
		cr1 |= 0x1200;
	}

	// check if there is a pending write
	if(transmitLength != 0)
	{
		// enable transmitter interrupt
		cr1 |= 0x2000;
	}

	// write the register
	writeRegister(ucr1, cr1);
}

//------------------------------------------------------------------------------------------------
// * Mx1UartPort::handleInterrupt
//
// Interrupt handler.
//------------------------------------------------------------------------------------------------

Bool Mx1UartPort::handleInterrupt()
{
	// determine if this is a control interrupt
	if(Mx1InterruptController::getCurrentInterruptController()->isPending(getControlInterruptNumber()))
	{
		// flush receive FIFO
		while((readRegister(urxd) & 0x8000) != 0);
		
		// clear idle and break status
		writeRegister(usr2, 0x1004);
		
		// error condition
		handleError();

		// notify of synchronization
		synchronizationEvent.signal();

		// interrupt handled
		return true;
	}

	// determine if this is a receiver interrupt
	if(Mx1InterruptController::getCurrentInterruptController()->isPending(getReceiverInterruptNumber()))
	{
		// clear idle status
		writeRegister(usr2, 0x1000);

		// check if synchronizing
		if(synchronizing)
		{
			// flush receive FIFO
			while((readRegister(urxd) & 0x8000) != 0);
		}
		else
		{
			// receive data
			UInt data;
			while(((data = readRegister(urxd)) & 0x8000) != 0)
			{
				// check for hardware detected errors
				if((data & 0x3400) != 0)
				{
					// flush receive FIFO
					while((readRegister(urxd) & 0x8000) != 0);

					// handle the error condition
					handleError();

					break;
				}

				// receive one byte
				*(pReceiveBuffer++) = data;

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

		// interrupt handled
		return true;
	}

	// determine if this is a transmitter interrupt
	if(Mx1InterruptController::getCurrentInterruptController()->isPending(getTransmitterInterruptNumber()))
	{
		// transmit data
		while((readRegister(usr1) & 0x2000) != 0)
		{
			// transmit one byte
			writeRegister(utxd, *(pTransmitBuffer++));

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

		// interrupt handled
		return true;
	}

	// interrupt not for us
	return false;
}

//------------------------------------------------------------------------------------------------
// * Mx1UartPort::handleError
//
// Error handler.
//------------------------------------------------------------------------------------------------

void Mx1UartPort::handleError()
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

	// disable transmitter and receiver interrupts
	maskInterrupts();
}
