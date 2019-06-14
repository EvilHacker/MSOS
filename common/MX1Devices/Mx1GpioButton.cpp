#include "Mx1GpioButton.h"
#include "../multitasking/TaskScheduler.h"
#include "../multitasking/Timer.h"
#include "../multitasking/sleep.h"
#include "Mx1InterruptController.h"

//------------------------------------------------------------------------------------------------
// * Mx1GpioButton::Mx1GpioButton
//
// Constructor.
//------------------------------------------------------------------------------------------------

Mx1GpioButton::Mx1GpioButton(
	Mx1GpioPin::Port portNumber,
	UInt pinNumber,
	UInt priority,
	UInt stackSize) :
		Task(priority, stackSize),
		inputPin(portNumber, pinNumber)
{
	// initialize the GPIO pin
	inputPin.configureAsInput();
	inputPin.clearInterrupt();

	// get the initial state of the button
	pressed = inputPin.getValue() == pressedValue;

	// add an interrupt handler and enable the interrupt
	TaskScheduler::getCurrentTaskScheduler()->addInterruptHandler(this);
	Mx1InterruptController::getCurrentInterruptController()->setToIrq(inputPin.getInterruptNumber());
	Mx1InterruptController::getCurrentInterruptController()->enable(inputPin.getInterruptNumber());
}

//------------------------------------------------------------------------------------------------
// * Mx1GpioButton::~Mx1GpioButton
//
// Destructor.
//------------------------------------------------------------------------------------------------

Mx1GpioButton::~Mx1GpioButton()
{
	// tristate the GPIO pin
	inputPin.configureAsInput();

	// disable the interrupt and remove the interrupt handler
	TaskScheduler::getCurrentTaskScheduler()->removeInterruptHandler(this);

	// stop the task
	suspend();
}

//------------------------------------------------------------------------------------------------
// * Mx1GpioButton::handlePress
//
// Default handler for button press.
//------------------------------------------------------------------------------------------------

void Mx1GpioButton::handlePress()
{
}

//------------------------------------------------------------------------------------------------
// * Mx1GpioButton::handleRelease
//
// Default handler for button release.
//------------------------------------------------------------------------------------------------

void Mx1GpioButton::handleRelease()
{
}

//------------------------------------------------------------------------------------------------
// * Mx1GpioButton::main
//
// Task main function for handing button events.
//------------------------------------------------------------------------------------------------

void Mx1GpioButton::main()
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
// * Mx1GpioButton::handleInterrupt
//
// Interrupt handler.
//------------------------------------------------------------------------------------------------

Bool Mx1GpioButton::handleInterrupt()
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
