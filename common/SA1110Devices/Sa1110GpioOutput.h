#ifndef _Sa1110GpioOutput_h_
#define _Sa1110GpioOutput_h_

#include "Sa1110GpioPin.h"

//------------------------------------------------------------------------------------------------
// * class Sa1110GpioOutput
//
// Controls an output device connected to a SA-1110 GPIO pin.
// The device can be turned on or off and can be active high or low.
//------------------------------------------------------------------------------------------------

class Sa1110GpioOutput : private Sa1110GpioPin
{
public:
	// constructor and destructor
	Sa1110GpioOutput(UInt gpioPinNumber, UInt onValue = 1);
	~Sa1110GpioOutput();

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
// * Sa1110GpioOutput::isOn
//
// Tests whether the device is on.
//------------------------------------------------------------------------------------------------

inline Bool Sa1110GpioOutput::isOn() const
{
	return getValue() == getOnValue();
}

//------------------------------------------------------------------------------------------------
// * Sa1110GpioOutput::isOff
//
// Tests whether the device is off.
//------------------------------------------------------------------------------------------------

inline Bool Sa1110GpioOutput::isOff() const
{
	return getValue() == getOffValue();
}

//------------------------------------------------------------------------------------------------
// * Sa1110GpioOutput::turnOn
//
// Turns the device on.
//------------------------------------------------------------------------------------------------

inline void Sa1110GpioOutput::turnOn()
{
	setValue(getOnValue());
}

//------------------------------------------------------------------------------------------------
// * Sa1110GpioOutput::turnOff
//
// Turns the device off.
//------------------------------------------------------------------------------------------------

inline void Sa1110GpioOutput::turnOff()
{
	setValue(getOffValue());
}

//------------------------------------------------------------------------------------------------
// * Sa1110GpioOutput::getOnValue
//
// Tests whether the LED is on.
//------------------------------------------------------------------------------------------------

inline UInt Sa1110GpioOutput::getOnValue() const
{
	return onValue;
}

//------------------------------------------------------------------------------------------------
// * Sa1110GpioOutput::getOffValue
//
// Tests whether the device is off.
//------------------------------------------------------------------------------------------------

inline UInt Sa1110GpioOutput::getOffValue() const
{
	return 1 - onValue;
}

#endif // _Sa1110GpioOutput_h_
