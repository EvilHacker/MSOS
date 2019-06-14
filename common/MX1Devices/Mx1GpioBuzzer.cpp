#include "../cPrimitiveTypes.h"
#include "Mx1GpioBuzzer.h"
#include "../multitasking/TaskScheduler.h"
#include "../multitasking/sleep.h"

//------------------------------------------------------------------------------------------------
// * Mx1GpioBuzzer::Mx1GpioBuzzer
//
// Constructor.
//------------------------------------------------------------------------------------------------

Mx1GpioBuzzer::Mx1GpioBuzzer(
	Mx1GpioPin::Port portNumber,
	UInt pinNumber) :
		outputPin(portNumber, pinNumber)
{
	// initialize the GPIO pin
	outputPin.setValue(lowValue);
	outputPin.configureAsOutput();
}

//------------------------------------------------------------------------------------------------
// * Mx1GpioBuzzer::~Mx1GpioBuzzer
//
// Destructor.
//------------------------------------------------------------------------------------------------

Mx1GpioBuzzer::~Mx1GpioBuzzer()
{
	// tri-state the GPIO pin
	outputPin.configureAsInput();
}

//------------------------------------------------------------------------------------------------
// * Mx1GpioBuzzer::generateTone
//
// Generates a tone with a specified <period> and <duration>.
// The actual duration will be rounded down to a multiple of the <period>.
//------------------------------------------------------------------------------------------------

void Mx1GpioBuzzer::generateTone(UInt duration, UInt period)
{
	// switch to a higher priority so that the frequency will be accurate
	Task *pCurrentTask = TaskScheduler::getCurrentTaskScheduler()->getCurrentTask();
	const UInt savedPriority = pCurrentTask->getPriority();
	pCurrentTask->setPriority(maximum(savedPriority, Task::realtimePriority + 1000));

	// oscillate the output pin
	const UInt highDuration = period / 2;
	const UInt lowDuration = period - highDuration;
	TimeValue edgeTime = getReferenceTimer()->getTime();
	while(duration >= period)
	{
		duration -= period;

		// one pulse
		outputPin.setValue(highValue);
		sleepUntil(edgeTime += highDuration, getReferenceTimer());
		outputPin.setValue(lowValue);
		sleepUntil(edgeTime += lowDuration, getReferenceTimer());
	};

	// restore the previous priority
	pCurrentTask->setPriority(savedPriority);
}
