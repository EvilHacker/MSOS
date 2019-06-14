#include "Mx1TerminalUartPort.h"
#include "Mx1InterruptController.h"
#include "deviceAddresses.h"
#include "../../multitasking/sleep.h"
#include "../../multitasking/TaskScheduler.h"
#include "../../multitasking/UninterruptableSection.h"
#include "../../memoryUtilities.h"

//------------------------------------------------------------------------------------------------
// * Mx1TerminalUartPort::Mx1TerminalUartPort
//
// Constructor.
//------------------------------------------------------------------------------------------------

Mx1TerminalUartPort::Mx1TerminalUartPort(Mx1TerminalUartPort::Port port, UInt baudRate) :
	port(port)
{
	inError = false;
	receiveLength = 0;
	transmitLength = 0;
	recvReadIndex = 0;
	recvBufferSize = 0;

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

	// default configuration (maximum rate)
	configure(baudRate, 8, 1, false, false);

	// enable interrupts
	Mx1InterruptController::getCurrentInterruptController()->setToIrq(getTransmitterInterruptNumber());
	Mx1InterruptController::getCurrentInterruptController()->setToIrq(getReceiverInterruptNumber());
	Mx1InterruptController::getCurrentInterruptController()->setToIrq(getControlInterruptNumber());
	Mx1InterruptController::getCurrentInterruptController()->enable(getTransmitterInterruptNumber());
	Mx1InterruptController::getCurrentInterruptController()->enable(getReceiverInterruptNumber());
	Mx1InterruptController::getCurrentInterruptController()->enable(getControlInterruptNumber());
}

//------------------------------------------------------------------------------------------------
// * Mx1TerminalUartPort::Mx1TerminalUartPort
//
// Destructor.
//------------------------------------------------------------------------------------------------

Mx1TerminalUartPort::~Mx1TerminalUartPort()
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
// * Mx1TerminalUartPort::read
//
// Read data from the stream.
//------------------------------------------------------------------------------------------------

UInt Mx1TerminalUartPort::read(void *pDestination, UInt length, TimeValue timeout)
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
				writeRegister(ucr2, 0);
				
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
// * Mx1TerminalUartPort::write
//
// Write data to the stream.
//------------------------------------------------------------------------------------------------

UInt Mx1TerminalUartPort::write(const void *pSource, UInt length, TimeValue timeout)
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
// * Mx1TerminalUartPort::configure
//
// Configures the parameters of the serial port.
//------------------------------------------------------------------------------------------------

void Mx1TerminalUartPort::configure(
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

	// set baud rate (assuming PERCLK1 is 8.000MHz)
	writeRegister(ubir, (UInt64)baudRate * 0x10000 / 500000 - 1);
	writeRegister(ubmr, 0xFFFF);

	maskInterrupts();
}

//------------------------------------------------------------------------------------------------
// * Mx1TerminalUartPort::maskInterrupts
//
// Enables or disables transmitter and receiver interrupts as required.
//------------------------------------------------------------------------------------------------

void Mx1TerminalUartPort::maskInterrupts(const Bool readEnabled)
{
	UninterruptableSection criticalSection;

	// start with default value
	UInt cr1 = 0x0005;

	// check if there is a read error
	if(readEnabled)
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
// * Mx1TerminalUartPort::handleInterrupt
//
// Interrupt handler.
//------------------------------------------------------------------------------------------------

Bool Mx1TerminalUartPort::handleInterrupt()
{
	// determine if this is a control interrupt
	if(Mx1InterruptController::getCurrentInterruptController()->isPending(getControlInterruptNumber()))
	{
		// flush receive FIFO
		while((readRegister(urxd) & 0x8000) != 0);
		
		// clear idle and break status
		writeRegister(usr2, 0x1004);
		
		return true;
	}
	
	// determine if this is a receiver interrupt
	if(Mx1InterruptController::getCurrentInterruptController()->isPending(getReceiverInterruptNumber()))
	{
		// clear idle status
		writeRegister(usr2, 0x1000);

		// receive data
		UInt data;
		while(((data = readRegister(urxd)) & 0x8000) != 0)
		{
			receiveEvent.signal();
			
			// check for hardware detected errors
			if((data & 0x3400) != 0)
			{
				inError = true;
				
				// flush receive FIFO
				while((readRegister(urxd) & 0x8000) != 0);

				// disable receive interrupts
				maskInterrupts(false);
				break;
			}

			// read byte
			if (recvBufferSize < sizeof(recvBuffer))
			{
				recvBuffer[recvBufferSize++] = data;
			}

			// disable receive interrupts if buffer is 80% full
			if (recvBufferSize > (sizeof(recvBuffer) * 4 / 5))
			{
				// disable receive interrupts
				maskInterrupts(false);
				break;
			}
		}
		
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
