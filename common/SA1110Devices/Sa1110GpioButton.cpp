#include "Sa1110GpioButton.h"
#include "../multitasking/TaskScheduler.h"
#include "../multitasking/Timer.h"
#include "../multitasking/sleep.h"
#include "Sa1110InterruptController.h"

//------------------------------------------------------------------------------------------------
// * Sa1110GpioButton::Sa1110GpioButton
//
// Constructor.
//------------------------------------------------------------------------------------------------

Sa1110GpioButton::Sa1110GpioButton(UInt gpioPinNumber, UInt priority, UInt stackSize) :
	Task(priority, stackSize),
	inputPin(gpioPinNumber)
{
	// initialize the GPIO pin
	inputPin.configureAsInput();
	inputPin.clearInterrupt();

	// get the initial state of the button
	pressed = inputPin.getValue() == pressedValue;

	// add an interrupt handler and enable the interrupt
	TaskScheduler::getCurrentTaskScheduler()->addInterruptHandler(this);
	Sa1110InterruptController::getCurrentInterruptController()->setToIrq(inputPin.getInterruptNumber());
	Sa1110InterruptController::getCurrentInterruptController()->enable(inputPin.getInterruptNumber());
}

//------------------------------------------------------------------------------------------------
// * Sa1110GpioButton::~Sa1110GpioButton
//
// Destructor.
//------------------------------------------------------------------------------------------------

Sa1110GpioButton::~Sa1110GpioButton()
{
	// tristate the GPIO pin
	inputPin.configureAsInput();

	// remove the interrupt handler
	TaskScheduler::getCurrentTaskScheduler()->removeInterruptHandler(this);

	// stop the task
	suspend();
}

//------------------------------------------------------------------------------------------------
// * Sa1110GpioButton::handlePress
//
// Default handler for button press.
//------------------------------------------------------------------------------------------------

void Sa1110GpioButton::handlePress()
{
}

//------------------------------------------------------------------------------------------------
// * Sa1110GpioButton::handleRelease
//
// Default handler for button release.
//------------------------------------------------------------------------------------------------

void Sa1110GpioButton::handleRelease()
{
}

//------------------------------------------------------------------------------------------------
// * Sa1110GpioButton::main
//
// Task main function for handing button events.
//------------------------------------------------------------------------------------------------

void Sa1110GpioButton::main()
{
	Timer *pTimer = TaskScheduler::getCurrentTaskScheduler()->getTimer();

	// handle button events forever
	while(true)
	{
		// configure the GPIO pin to trigger an interrupt when the button is pressed
		if(pressedValue == 0)
		{
			// enable falling edge interrupt
			inputPin.configureAsFallingEdgeInterrupt();
		}
		else
		{
			// enable rising edge interrupt
			inputPin.configureAsRisingEdgeInterrupt();
		}

		// wait for press
		if(inputPin.getValue() == pressedValue)
		{
			// the button has already been pressed
			buttonEvent.signal();
		}
		buttonEvent.wait();

		// get the time of press
		const TimeValue pressTime = pTimer->getTime();

		// handle press
		pressed = true;
		handlePress();

		// wait for debounce period
		const TimeValue pressDebounceTime = getDebounceTime() - (pTimer->getTime() - pressTime);
		if(pressDebounceTime > 0)
		{
			sleepForTicks(pressDebounceTime, pTimer);
		}

		// configure the GPIO pin to trigger an interrupt when the button is released
		if(releasedValue == 0)
		{
			// enable falling edge interrupt
			inputPin.configureAsFallingEdgeInterrupt();
		}
		else
		{
			// enable rising edge interrupt
			inputPin.configureAsRisingEdgeInterrupt();
		}

		// wait for release
		if(inputPin.getValue() == releasedValue)
		{
			// the button has already been released
			buttonEvent.signal();
		}
		buttonEvent.wait();

		// get the time of release
		const TimeValue releaseTime = pTimer->getTime();

		// handle release
		pressed = false;
		handleRelease();

		// wait for debounce period
		const TimeValue releaseDebounceTime = getDebounceTime() - (pTimer->getTime() - releaseTime);
		if(releaseDebounceTime > 0)
		{
			sleepForTicks(releaseDebounceTime, pTimer);
		}
	}
}

//------------------------------------------------------------------------------------------------
// * Sa1110GpioButton::handleInterrupt
//
// Interrupt handler.
//------------------------------------------------------------------------------------------------

Bool Sa1110GpioButton::handleInterrupt()
{
	// determine if this interrupt is for us
	if(inputPin.isInterruptPending())
	{
		// disable the interrupt
		inputPin.configureAsInput();
		inputPin.clearInterrupt();

		// signal an event
		buttonEvent.signal();
		return true;
	}

	// not our interrupt
	return false;
}
