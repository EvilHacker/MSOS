#ifndef _Mx1GpioButton_h_
#define _Mx1GpioButton_h_

#include "../../multitasking/Task.h"
#include "../../multitasking/InterruptHandler.h"
#include "../../multitasking/IntertaskEvent.h"
#include "Mx1GpioPin.h"

//------------------------------------------------------------------------------------------------
// * class Mx1GpioButton
//
// Handles the press and release of a button connected to a GPIO pin.
//------------------------------------------------------------------------------------------------

class Mx1GpioButton : public Task, private InterruptHandler
{
public:
	// constructor and destructor
	Mx1GpioButton(
		Mx1GpioPin::Port portNumber,
		UInt pinNumber,
		UInt priority = defaultPriority,
		UInt stackSize = 10000);
	virtual ~Mx1GpioButton();

	// testing
	inline Bool isPressed() const;
	inline Bool isReleased() const;

protected:
	// event handling
	virtual void handlePress();
	virtual void handleRelease();

private:
	// querying
	inline TimeValue getDebounceTime() const;

	// task main function
	void main();

	// interrupt handling
	Bool handleInterrupt();

	// representation
	enum
	{
		pressedValue = 0,
		releasedValue = 1 - pressedValue
	};
	Mx1GpioPin inputPin;
	IntertaskEvent buttonEvent;
	Bool pressed;
};

//------------------------------------------------------------------------------------------------
// * Mx1GpioButton::isPressed
//
// Tests whether the button is pressed.
//------------------------------------------------------------------------------------------------

inline Bool Mx1GpioButton::isPressed() const
{
	return pressed;
}

//------------------------------------------------------------------------------------------------
// * Mx1GpioButton::isReleased
//
// Tests whether the button is released.
//------------------------------------------------------------------------------------------------

inline Bool Mx1GpioButton::isReleased() const
{
	return !pressed;
}

//------------------------------------------------------------------------------------------------
// * Mx1GpioButton::getDebounceTime
//
// Return the debounce period of a button expressed as default timer ticks.
//------------------------------------------------------------------------------------------------

TimeValue Mx1GpioButton::getDebounceTime() const
{
	// 40ms
	return 4000000 / 25;
}

#endif // _Mx1GpioButton_h_
