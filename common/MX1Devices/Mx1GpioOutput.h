#ifndef _Mx1GpioOutput_h_
#define _Mx1GpioOutput_h_

#include "Mx1GpioPin.h"

//------------------------------------------------------------------------------------------------
// * class Mx1GpioOutput
//
// Controls an output device connected to a GPIO pin.
// The device can be turned on or off and can be active high or low.
//------------------------------------------------------------------------------------------------

class Mx1GpioOutput : public Mx1GpioPin
{
public:
	// constructor and destructor
	typedef Mx1GpioPin::Port Port;
	Mx1GpioOutput(Port portNumber, UInt pinNumber, UInt onValue = 1);
	~Mx1GpioOutput();

	// testing
	inline Bool isOn() const;
	inline Bool isOff() const;

	// behaviour
	inline void turnOn();
	inline void turnOff();

private:
	// accessing
	inline UInt getOnValue() const;
	inline UInt getOffValue() const;

	// representation
	UInt onValue;
};

//------------------------------------------------------------------------------------------------
// * Mx1GpioOutput::isOn
//
// Tests whether the device is on.
//------------------------------------------------------------------------------------------------

inline Bool Mx1GpioOutput::isOn() const
{
	return getValue() == getOnValue();
}

//------------------------------------------------------------------------------------------------
// * Mx1GpioOutput::isOff
//
// Tests whether the device is off.
//------------------------------------------------------------------------------------------------

inline Bool Mx1GpioOutput::isOff() const
{
	return getValue() == getOffValue();
}

//------------------------------------------------------------------------------------------------
// * Mx1GpioOutput::turnOn
//
// Turns the device on.
//------------------------------------------------------------------------------------------------

inline void Mx1GpioOutput::turnOn()
{
	setValue(getOnValue());
}

//------------------------------------------------------------------------------------------------
// * Mx1GpioOutput::turnOff
//
// Turns the device off.
//------------------------------------------------------------------------------------------------

inline void Mx1GpioOutput::turnOff()
{
	setValue(getOffValue());
}

//------------------------------------------------------------------------------------------------
// * Mx1GpioOutput::getOnValue
//
// Tests whether the LED is on.
//------------------------------------------------------------------------------------------------

inline UInt Mx1GpioOutput::getOnValue() const
{
	return onValue;
}

//------------------------------------------------------------------------------------------------
// * Mx1GpioOutput::getOffValue
//
// Tests whether the device is off.
//------------------------------------------------------------------------------------------------

inline UInt Mx1GpioOutput::getOffValue() const
{
	return 1 - onValue;
}

#endif // _Mx1GpioOutput_h_
