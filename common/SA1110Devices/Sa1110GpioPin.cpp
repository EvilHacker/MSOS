#include "Sa1110GpioPin.h"
#include "../multitasking/UninterruptableSection.h"

//------------------------------------------------------------------------------------------------
// * Sa1110GpioPin::configureAsInput
//
// Configure the function of the pin.
//------------------------------------------------------------------------------------------------

void Sa1110GpioPin::configureAsInput()
{
	UninterruptableSection criticalSection;
	const UInt mask = 1 << pinNumber;
	*(volatile UInt *)gafr &= ~mask;
	*(volatile UInt *)grer &= ~mask;
	*(volatile UInt *)gfer &= ~mask;
	*(volatile UInt *)gpdr &= ~mask;
}

//------------------------------------------------------------------------------------------------
// * Sa1110GpioPin::configureAsOutput
//
// Configure the function of the pin.
//------------------------------------------------------------------------------------------------

void Sa1110GpioPin::configureAsOutput()
{
	UninterruptableSection criticalSection;
	const UInt mask = 1 << pinNumber;
	*(volatile UInt *)gafr &= ~mask;
	*(volatile UInt *)grer &= ~mask;
	*(volatile UInt *)gfer &= ~mask;
	*(volatile UInt *)gpdr |= mask;
}

//------------------------------------------------------------------------------------------------
// * Sa1110GpioPin::configureAsRisingEdgeInterrupt
//
// Configure the function of the pin.
//------------------------------------------------------------------------------------------------

void Sa1110GpioPin::configureAsRisingEdgeInterrupt()
{
	UninterruptableSection criticalSection;
	const UInt mask = 1 << pinNumber;
	*(volatile UInt *)gafr &= ~mask;
	*(volatile UInt *)gpdr &= ~mask;
	*(volatile UInt *)grer |= mask;
	*(volatile UInt *)gfer &= ~mask;
}

//------------------------------------------------------------------------------------------------
// * Sa1110GpioPin::configureAsFallingEdgeInterrupt
//
// Configure the function of the pin.
//------------------------------------------------------------------------------------------------

void Sa1110GpioPin::configureAsFallingEdgeInterrupt()
{
	UninterruptableSection criticalSection;
	const UInt mask = 1 << pinNumber;
	*(volatile UInt *)gafr &= ~mask;
	*(volatile UInt *)gpdr &= ~mask;
	*(volatile UInt *)grer &= ~mask;
	*(volatile UInt *)gfer |= mask;
}

//------------------------------------------------------------------------------------------------
// * Sa1110GpioPin::configureAsAllEdgeInterrupt
//
// Configure the function of the pin.
//------------------------------------------------------------------------------------------------

void Sa1110GpioPin::configureAsAllEdgeInterrupt()
{
	UninterruptableSection criticalSection;
	const UInt mask = 1 << pinNumber;
	*(volatile UInt *)gafr &= ~mask;
	*(volatile UInt *)gpdr &= ~mask;
	*(volatile UInt *)grer |= mask;
	*(volatile UInt *)gfer |= mask;
}

//------------------------------------------------------------------------------------------------
// * Sa1110GpioPin::configureAsAlternateInputFunction
//
// Configure the function of the pin.
//------------------------------------------------------------------------------------------------

void Sa1110GpioPin::configureAsAlternateInputFunction()
{
	UninterruptableSection criticalSection;
	const UInt mask = 1 << pinNumber;
	*(volatile UInt *)grer &= ~mask;
	*(volatile UInt *)gfer &= ~mask;
	*(volatile UInt *)gpdr &= ~mask;
	*(volatile UInt *)gafr |= mask;
}

//------------------------------------------------------------------------------------------------
// * Sa1110GpioPin::configureAsAlternateOutputFunction
//
// Configure the function of the pin.
//------------------------------------------------------------------------------------------------

void Sa1110GpioPin::configureAsAlternateOutputFunction()
{
	UninterruptableSection criticalSection;
	const UInt mask = 1 << pinNumber;
	*(volatile UInt *)grer &= ~mask;
	*(volatile UInt *)gfer &= ~mask;
	*(volatile UInt *)gafr |= mask;
	*(volatile UInt *)gpdr |= mask;
}
