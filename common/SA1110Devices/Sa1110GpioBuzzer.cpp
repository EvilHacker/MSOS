#include "../cPrimitiveTypes.h"
#include "Sa1110GpioBuzzer.h"
#include "../multitasking/TaskScheduler.h"
#include "../multitasking/sleep.h"

//------------------------------------------------------------------------------------------------
// * Sa1110GpioBuzzer::Sa1110GpioBuzzer
//
// Constructor.
//------------------------------------------------------------------------------------------------

Sa1110GpioBuzzer::Sa1110GpioBuzzer(UInt pinNumber) :
	outputPin(pinNumber)
{
	// initialize the GPIO pin
	outputPin.setValue(lowValue);
	outputPin.configureAsOutput();
}

//------------------------------------------------------------------------------------------------
// * Sa1110GpioBuzzer::~Sa1110GpioBuzzer
//
// Destructor.
//------------------------------------------------------------------------------------------------

Sa1110GpioBuzzer::~Sa1110GpioBuzzer()
{
	// tri-state the GPIO pin
	outputPin.configureAsInput();
}

//------------------------------------------------------------------------------------------------
// * Sa1110GpioBuzzer::generateTone
//
// Generates a tone with a specified <period> and <duration>.
// The actual duration will be rounded down to a multiple of the <period>.
//------------------------------------------------------------------------------------------------

void Sa1110GpioBuzzer::generateTone(UInt duration, UInt period)
{
	// switch to a higher priority so that the frequency will be accurate
	Task *pCurrentTask = TaskScheduler::getCurrentTaskScheduler()->getCurrentTask();
	const UInt savedPriority = pCurrentTask->getPriority();
	pCurrentTask->setPriority(maximum(savedPriority, Task::realtimePriority + 1000));

	// oscillate the output pin
	while(duration >= period)
	{
		duration -= period;

		// one pulse
		outputPin.setValue(highValue);
		sleepForTicks(period / 2, getReferenceTimer());
		outputPin.setValue(lowValue);
		sleepForTicks((period + 1) / 2, getReferenceTimer());
	};

	// restore the previous priority
	pCurrentTask->setPriority(savedPriority);
}
