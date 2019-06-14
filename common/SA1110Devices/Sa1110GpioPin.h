#ifndef _Sa1110GpioPin_h_
#define _Sa1110GpioPin_h_

#include "../cPrimitiveTypes.h"
#include "Sa1110DeviceAddresses.h"

//------------------------------------------------------------------------------------------------
// * class Sa1110GpioPin
//
// Provides an interface to SA1110 GPIO pins.
//------------------------------------------------------------------------------------------------

class Sa1110GpioPin
{
public:
	// constructor
	inline Sa1110GpioPin(UInt pinNumber);

	// testing
	inline Bool isInterruptPending() const;

	// accessing
	inline UInt getPinNumber() const;
	inline UInt getValue() const;
	inline void setValue(UInt value);
	inline UInt getInterruptNumber() const;

	// configuring
	void configureAsInput();
	void configureAsOutput();
	void configureAsRisingEdgeInterrupt();
	void configureAsFallingEdgeInterrupt();
	void configureAsAllEdgeInterrupt();
	void configureAsAlternateInputFunction();
	void configureAsAlternateOutputFunction();

	// behaviour
	inline void clearInterrupt();

private:
	enum RegisterAddress
	{
		gplr = sa1110SystemControlBase + 0x40000, // level (read only)
		gpdr = sa1110SystemControlBase + 0x40004, // direction
		gpsr = sa1110SystemControlBase + 0x40008, // set (write only)
		gpcr = sa1110SystemControlBase + 0x4000C, // clear (write only)
		grer = sa1110SystemControlBase + 0x40010, // rising-edge detect
		gfer = sa1110SystemControlBase + 0x40014, // falling-edge detect
		gedr = sa1110SystemControlBase + 0x40018, // edge detect status
		gafr = sa1110SystemControlBase + 0x4001C  // alternate function
	};

	// representation
	UInt pinNumber;
};

//------------------------------------------------------------------------------------------------
// * Sa1110GpioPin::Sa1110GpioPin
//
// Constructor.
//------------------------------------------------------------------------------------------------

inline Sa1110GpioPin::Sa1110GpioPin(UInt pinNumber)
{
	this->pinNumber = pinNumber;
}

//------------------------------------------------------------------------------------------------
// * Sa1110GpioPin::isInterruptPending
//
// Checks whether an edge transition has been detected.
//------------------------------------------------------------------------------------------------

inline Bool Sa1110GpioPin::isInterruptPending() const
{
	return ((*(volatile UInt *)gedr >> pinNumber) & 1) != 0;
}

//------------------------------------------------------------------------------------------------
// * Sa1110GpioPin::getPinNumber
//
// Gets the GPIO pin number.
//------------------------------------------------------------------------------------------------

inline UInt Sa1110GpioPin::getPinNumber() const
{
	return pinNumber;
}

//------------------------------------------------------------------------------------------------
// * Sa1110GpioPin::getValue
//
// Gets the value of the pin.
//------------------------------------------------------------------------------------------------

inline UInt Sa1110GpioPin::getValue() const
{
	return (*(volatile UInt *)gplr >> pinNumber) & 1;
}

//------------------------------------------------------------------------------------------------
// * Sa1110GpioPin::setValue
//
// Sets the value of the pin.
//------------------------------------------------------------------------------------------------

inline void Sa1110GpioPin::setValue(UInt value)
{
	const UInt mask = 1 << pinNumber;
	if(value == 0)
	{
		// clear
		*(volatile UInt *)gpcr = mask;
	}
	else
	{
		// set
		*(volatile UInt *)gpsr = mask;
	}
}

//------------------------------------------------------------------------------------------------
// * Sa1110GpioPin::getInterruptNumber
//
// Returns the SA1110 Interrupt Controller interrupt number for this pin.
//------------------------------------------------------------------------------------------------

inline UInt Sa1110GpioPin::getInterruptNumber() const
{
	if(pinNumber <= 10)
	{
		return pinNumber;
	}
	else
	{
		return 11;
	}
}

//------------------------------------------------------------------------------------------------
// * Sa1110GpioPin::clearInterrupt
//
// Clears a pending interrupt.
//------------------------------------------------------------------------------------------------

inline void Sa1110GpioPin::clearInterrupt()
{
	*(volatile UInt *)gedr = 1 << pinNumber;
}

#endif // _Sa1110GpioPin_h_
