#include "Sa1110GpioOutput.h"

//------------------------------------------------------------------------------------------------
// * Sa1110GpioOutput::Sa1110GpioOutput
//
// Constructor.
// The <onValue> should be 1 for an active high device or 0 for an active low device.
//------------------------------------------------------------------------------------------------

Sa1110GpioOutput::Sa1110GpioOutput(UInt gpioPinNumber, UInt onValue) :
	Sa1110GpioPin(gpioPinNumber),
	onValue(onValue)
{
	// turn the device off
	turnOff();
	configureAsOutput();
}

//------------------------------------------------------------------------------------------------
// * Sa1110GpioOutput::~Sa1110GpioOutput
//
// Destructor.
//------------------------------------------------------------------------------------------------

Sa1110GpioOutput::~Sa1110GpioOutput()
{
	// tristate the GPIO pin
	configureAsInput();
}
