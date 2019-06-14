#include "Mx1GpioPin.h"
#include "../multitasking/UninterruptableSection.h"

//------------------------------------------------------------------------------------------------
// * Mx1GpioPin::setValue
//
// Sets the value of the pin.
//------------------------------------------------------------------------------------------------

void Mx1GpioPin::setValue(UInt value)
{
	const UInt mask = 1 << pinNumber;

	UninterruptableSection criticalSection;
	if(value == 0)
	{
		// clear
		pRegisters->dr &= ~mask;
	}
	else
	{
		// set
		pRegisters->dr |= mask;
	}
}

//------------------------------------------------------------------------------------------------
// * Mx1GpioPin::configureAsInput
//
// Configure the function of the pin.
//------------------------------------------------------------------------------------------------

void Mx1GpioPin::configureAsInput()
{
	const UInt mask = 1 << pinNumber;

	UninterruptableSection criticalSection;
	pRegisters->imr &= ~mask;
	pRegisters->ddir &= ~mask;
	pRegisters->gius |= mask;
}

//------------------------------------------------------------------------------------------------
// * Mx1GpioPin::configureAsOutput
//
// Configure the function of the pin.
//------------------------------------------------------------------------------------------------

void Mx1GpioPin::configureAsOutput()
{
	const UInt mask = 1 << pinNumber;

	UninterruptableSection criticalSection;
	pRegisters->imr &= ~mask;
	if(pinNumber <= 15)
	{
		pRegisters->ocr1 = pRegisters->ocr1
			| (3 << (pinNumber << 1));
	}
	else
	{
		pRegisters->ocr2 = pRegisters->ocr2
			| (3 << ((pinNumber & 0xF) << 1));
	}
	pRegisters->ddir |= mask;
	pRegisters->gius |= mask;
}

//------------------------------------------------------------------------------------------------
// * Mx1GpioPin::configureAsRisingEdgeInterrupt
//
// Configure the function of the pin.
//------------------------------------------------------------------------------------------------

void Mx1GpioPin::configureAsRisingEdgeInterrupt()
{
	const UInt mask = 1 << pinNumber;

	UninterruptableSection criticalSection;
	pRegisters->ddir &= ~mask;
	if(pinNumber <= 15)
	{
		pRegisters->icr1 = pRegisters->icr1
			& ~(3 << (pinNumber << 1));
	}
	else
	{
		pRegisters->icr2 = pRegisters->icr2
			& ~(3 << ((pinNumber & 0xF) << 1));
	}
	pRegisters->imr |= mask;
}

//------------------------------------------------------------------------------------------------
// * Mx1GpioPin::configureAsFallingEdgeInterrupt
//
// Configure the function of the pin.
//------------------------------------------------------------------------------------------------

void Mx1GpioPin::configureAsFallingEdgeInterrupt()
{
	const UInt mask = 1 << pinNumber;

	UninterruptableSection criticalSection;
	pRegisters->ddir &= ~mask;
	if(pinNumber <= 15)
	{
		pRegisters->icr1 = pRegisters->icr1
			& ~(2 << (pinNumber << 1))
			| (1 << (pinNumber << 1));
	}
	else
	{
		pRegisters->icr2 = pRegisters->icr2
			& ~(2 << ((pinNumber & 0xF) << 1))
			| (1 << ((pinNumber & 0xF) << 1));
	}
	pRegisters->imr |= mask;
}

//------------------------------------------------------------------------------------------------
// * Mx1GpioPin::enablePullResistor
//
// Enables or disables an internal pull-up or pull-down resistor.
//------------------------------------------------------------------------------------------------

void Mx1GpioPin::enablePullResistor(Bool pullUpOrDownEnabled)
{
	const UInt mask = 1 << pinNumber;

	UninterruptableSection criticalSection;
	if(pullUpOrDownEnabled)
	{
		// enable pull-up/down
		pRegisters->puen |= mask;
	}
	else
	{
		// disable pull-up/down
		pRegisters->puen &= ~mask;
	}
}
