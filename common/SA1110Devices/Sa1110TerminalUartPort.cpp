#include "Sa1110TerminalUartPort.h"
#include "Sa1110DeviceAddresses.h"
#include "Sa1110InterruptController.h"
#include "../../multitasking/sleep.h"
#include "../../multitasking/TaskScheduler.h"
#include "../../multitasking/UninterruptableSection.h"
#include "../../memoryUtilities.h"

//------------------------------------------------------------------------------------------------
// * Sa1110TerminalUartPort::Sa1110TerminalUartPort
//
// Constructor.
//------------------------------------------------------------------------------------------------

Sa1110TerminalUartPort::Sa1110TerminalUartPort(Sa1110TerminalUartPort::Port port, UInt baudRate) :
	port(port)
{
	receiveLength = 0;
	transmitLength = 0;
	inError = false;
	recvReadIndex = 0;
	recvBufferSize = 0;

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
	configure(baudRate, 8, 1, false, false);

	// enable the interrupt
	Sa1110InterruptController::getCurrentInterruptController()->setToIrq(getInterruptNumber());
	Sa1110InterruptController::getCurrentInterruptController()->enable(getInterruptNumber());
}

//------------------------------------------------------------------------------------------------
// * Sa1110TerminalUartPort::Sa1110TerminalUartPort
//
// Destructor.
//------------------------------------------------------------------------------------------------

Sa1110TerminalUartPort::~Sa1110TerminalUartPort()
{
	// disable the interrupt
	Sa1110InterruptController::getCurrentInterruptController()->disable(getInterruptNumber());

	// turn off the serial port
	writeRegister(utcr3, 0);

	// unregister interrupt handler
	TaskScheduler::getCurrentTaskScheduler()->removeInterruptHandler(this);
}

//------------------------------------------------------------------------------------------------
// * Sa1110TerminalUartPort::read
//
// Read data from the stream.
//------------------------------------------------------------------------------------------------

UInt Sa1110TerminalUartPort::read(void *pDestination, UInt length, TimeValue timeout)
{
	// setup receive state
	pReceiveBuffer = (UInt8 *)pDestination;
	receiveLength = length;

	// read loop
	while (receiveLength != 0)
	{
		if (recvBufferSize == 0)
		{
			inError = false;
			
			// enable receive interrupts
			maskInterrupts();
			
			// wait for data
			if (receiveEvent.wait(timeout))
			{
				inError = true;
			}
			
			if (inError)
			{
				// handle UART error
				writeRegister(utcr3, 0);
				
				// wait 0.5s 
				sleepForMilliseconds(500);
				break;
			}
		}

		{
			UninterruptableSection criticalSection;
			UInt size = minimum(receiveLength, (recvBufferSize - recvReadIndex));
			
			// copy data
			memoryCopy(pReceiveBuffer, (void *)&((recvBuffer)[recvReadIndex]), size);
					
			recvReadIndex += size;
			if (recvReadIndex == recvBufferSize)
			{
				recvBufferSize = 0;
				recvReadIndex = 0;
			}
				
			// adjust buffers
			receiveLength -= size;
			pReceiveBuffer += size;
		}
	}
	
	return length - receiveLength;
}

//------------------------------------------------------------------------------------------------
// * Sa1110TerminalUartPort::write
//
// Write data to the stream.
//------------------------------------------------------------------------------------------------

UInt Sa1110TerminalUartPort::write(const void *pSource, UInt length, TimeValue timeout)
{
	// setup transmit state
	pTransmitBuffer = (UInt8 *)pSource;
	transmitLength = length;

	// enable transmitter interrupt
	maskInterrupts();

	// wait for completion of the write
	transmitEvent.wait();
	return length;
}

//------------------------------------------------------------------------------------------------
// * Sa1110TerminalUartPort::configure
//
// Configures the parameters of the serial port.
//------------------------------------------------------------------------------------------------

void Sa1110TerminalUartPort::configure(
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
// * Sa1110TerminalUartPort::maskInterrupts
//
// Enables or disables transmitter and receiver interrupts as required.
//------------------------------------------------------------------------------------------------

void Sa1110TerminalUartPort::maskInterrupts(const Bool readEnabled)
{
	UninterruptableSection criticalSection;

	// start with both receiver and transmitter enabled
	UInt cr3 = 0x03;

	// check if there is a read error
	if(readEnabled)
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
// * Sa1110TerminalUartPort::handleInterrupt
//
// Interrupt handler.
//------------------------------------------------------------------------------------------------

Bool Sa1110TerminalUartPort::handleInterrupt()
{
	// determine if this interrupt is for us
	if(Sa1110InterruptController::getCurrentInterruptController()->isPending(getInterruptNumber()))
	{
		// receive data
		UInt sr1;
		while(((sr1 = readRegister(utsr1)) & 0x02) != 0)
		{
			receiveEvent.signal();
			
			// check for hardware detected errors
			if ((sr1 & 0x70) != 0)
			{
				inError = true;
				
				// disable receive interrupts
				maskInterrupts(false);
				break;
			}
			
			// read byte
			if (recvBufferSize < sizeof(recvBuffer))
			{
				recvBuffer[recvBufferSize++] = readRegister(utdr0);
			}
			
			// disable receive interrupts if buffer is 80% full
			if (recvBufferSize > (sizeof(recvBuffer) * 4 / 5))
			{
				maskInterrupts(false);
				break;
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
			// clear the interrupts
			writeRegister(utsr0, sr0);
		}

		// interrupt handled
		return true;
	}

	// interrupt not for us
	return false;
}
