#ifndef _Sa1110GpioButton_h_
#define _Sa1110GpioButton_h_

#include "../multitasking/Task.h"
#include "../multitasking/InterruptHandler.h"
#include "../multitasking/IntertaskEvent.h"
#include "Sa1110GpioPin.h"

//------------------------------------------------------------------------------------------------
// * class Sa1110GpioButton
//
// Handles the press and release of a button connected to a GPIO pin.
//------------------------------------------------------------------------------------------------

class Sa1110GpioButton : public Task, private InterruptHandler
{
public:
	// constructor and destructor
	Sa1110GpioButton(
		UInt gpioPinNumber,
		UInt priority = defaultPriority,
		UInt stackSize = 10000);
	virtual ~Sa1110GpioButton();

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
	Sa1110GpioPin inputPin;
	IntertaskEvent buttonEvent;
	Bool pressed;
};

//------------------------------------------------------------------------------------------------
// * Sa1110GpioButton::isPressed
//
// Tests whether the button is pressed.
//------------------------------------------------------------------------------------------------

inline Bool Sa1110GpioButton::isPressed() const
{
	return pressed;
}

//------------------------------------------------------------------------------------------------
// * Sa1110GpioButton::isReleased
//
// Tests whether the button is released.
//------------------------------------------------------------------------------------------------

inline Bool Sa1110GpioButton::isReleased() const
{
	return !pressed;
}

//------------------------------------------------------------------------------------------------
// * Sa1110GpioButton::getDebounceTime
//
// Return the debounce period of a button expressed as default timer ticks.
//------------------------------------------------------------------------------------------------

TimeValue Sa1110GpioButton::getDebounceTime() const
{
	// 40ms
	return 3686400 / 25;
}

#endif // _Sa1110GpioButton_h_
