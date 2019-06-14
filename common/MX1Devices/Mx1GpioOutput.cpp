#include "Mx1GpioOutput.h"

//------------------------------------------------------------------------------------------------
// * Mx1GpioOutput::Mx1GpioOutput
//
// Constructor.
// The <onValue> should be 1 for an active high device or 0 for an active low device.
//------------------------------------------------------------------------------------------------

Mx1GpioOutput::Mx1GpioOutput(Port portNumber, UInt pinNumber, UInt onValue) :
	Mx1GpioPin(portNumber, pinNumber),
	onValue(onValue)
{
	// turn the device off
	turnOff();
	configureAsOutput();
}

//------------------------------------------------------------------------------------------------
// * Mx1GpioOutput::~Mx1GpioOutput
//
// Destructor.
//------------------------------------------------------------------------------------------------

Mx1GpioOutput::~Mx1GpioOutput()
{
	// tristate the GPIO pin
	configureAsInput();
}
