#ifndef _Mx1GpioPin_h_
#define _Mx1GpioPin_h_

#include "../cPrimitiveTypes.h"

//------------------------------------------------------------------------------------------------
// * class Mx1GpioPin
//
// Provides an interface to MX1 GPIO pins.
//------------------------------------------------------------------------------------------------

class Mx1GpioPin
{
public:
	// constructor
	enum Port
	{
		portA,
		portB,
		portC,
		portD
	};
	inline Mx1GpioPin(Port portNumber, UInt pinNumber);

	// testing
	inline Bool isInterruptPending() const;

	// accessing
	inline Port getPortNumber() const;
	inline UInt getPinNumber() const;
	inline UInt getValue() const;
	void setValue(UInt value);
	inline UInt getInterruptNumber() const;

	// configuring
	void configureAsInput();
	void configureAsOutput();
	void configureAsRisingEdgeInterrupt();
	void configureAsFallingEdgeInterrupt();
	void enablePullResistor(Bool pullUpOrDownEnabled);

	// behaviour
	inline void clearInterrupt();

private:
	// types
	struct Mx1GpioPinRegisters
	{
		UInt ddir;
		UInt ocr1;
		UInt ocr2;
		UInt iconfa1;
		UInt iconfa2;
		UInt iconfb1;
		UInt iconfb2;
		UInt dr;
		UInt gius;
		UInt ssr;
		UInt icr1;
		UInt icr2;
		UInt imr;
		UInt isr;
		UInt gpr;
		UInt swr;
		UInt puen;
	};

	// representation
	volatile Mx1GpioPinRegisters *pRegisters;
	Port portNumber;
	UInt8 pinNumber;
};

#include "deviceAddresses.h"

//------------------------------------------------------------------------------------------------
// * Mx1GpioPin::Mx1GpioPin
//
// Constructor.
//------------------------------------------------------------------------------------------------

inline Mx1GpioPin::Mx1GpioPin(Port portNumber, UInt pinNumber)
{
	this->portNumber = portNumber;
	this->pinNumber = pinNumber;
	pRegisters = (Mx1GpioPinRegisters *)(mx1RegistersBase + 0x1C000 + ((UInt)portNumber << 8));
}

//------------------------------------------------------------------------------------------------
// * Mx1GpioPin::isInterruptPending
//
// Checks whether an edge transition has been detected.
//------------------------------------------------------------------------------------------------

inline Bool Mx1GpioPin::isInterruptPending() const
{
	return ((pRegisters->isr >> pinNumber) & 1) != 0;
}

//------------------------------------------------------------------------------------------------
// * Mx1GpioPin::getPortNumber
//
// Gets the GPIO port number.
//------------------------------------------------------------------------------------------------

inline Mx1GpioPin::Port Mx1GpioPin::getPortNumber() const
{
	return portNumber;
}

//------------------------------------------------------------------------------------------------
// * Mx1GpioPin::getPinNumber
//
// Gets the GPIO pin number.
//------------------------------------------------------------------------------------------------

inline UInt Mx1GpioPin::getPinNumber() const
{
	return pinNumber;
}

//------------------------------------------------------------------------------------------------
// * Mx1GpioPin::getValue
//
// Gets the value of the pin.
//------------------------------------------------------------------------------------------------

inline UInt Mx1GpioPin::getValue() const
{
	return (pRegisters->ssr >> pinNumber) & 1;
}

//------------------------------------------------------------------------------------------------
// * Mx1GpioPin::getInterruptNumber
//
// Returns the MX1 Interrupt Controller interrupt number for this pin.
//------------------------------------------------------------------------------------------------

inline UInt Mx1GpioPin::getInterruptNumber() const
{
	static const UInt8 interruptNumbers[] = {11, 12, 13, 62};
	return interruptNumbers[portNumber];
}

//------------------------------------------------------------------------------------------------
// * Mx1GpioPin::clearInterrupt
//
// Clears a pending interrupt.
//------------------------------------------------------------------------------------------------

inline void Mx1GpioPin::clearInterrupt()
{
	pRegisters->isr = 1 << pinNumber;
}

#endif // _Mx1GpioPin_h_
