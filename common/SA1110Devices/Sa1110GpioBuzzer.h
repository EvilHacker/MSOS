#ifndef _Sa1110GpioBuzzer_h_
#define _Sa1110GpioBuzzer_h_

#include "../cPrimitiveTypes.h"
#include "../multitasking/TaskScheduler.h"
#include "Sa1110GpioPin.h"

//------------------------------------------------------------------------------------------------
// * class Sa1110GpioBuzzer
//
// Used to Generates tones via a speaker connected to a GPIO pin.
//------------------------------------------------------------------------------------------------

class Sa1110GpioBuzzer
{
public:
	// constructor and destructor
	Sa1110GpioBuzzer(UInt pinNumber);
	~Sa1110GpioBuzzer();

	// querying
	inline Timer *getReferenceTimer() const;
	inline TimeValue getReferenceFrequency() const;

	// behaviour
	void generateTone(UInt duration, UInt period);

private:
	// representation
	Sa1110GpioPin outputPin;

	// pin values
	enum
	{
		lowValue = 0,
		highValue = 1 - lowValue
	};
};

//------------------------------------------------------------------------------------------------
// * Sa1110GpioBuzzer::getReferenceTimer
//
// Returns the reference clock frequency from which all generated frequencies are derived from.
//------------------------------------------------------------------------------------------------

inline Timer *Sa1110GpioBuzzer::getReferenceTimer() const
{
	return TaskScheduler::getCurrentTaskScheduler()->getTimer();
}

//------------------------------------------------------------------------------------------------
// * Sa1110GpioBuzzer::getReferenceFrequency
//
// Returns the reference clock frequency from which all generated frequencies are derived from.
//------------------------------------------------------------------------------------------------

inline TimeValue Sa1110GpioBuzzer::getReferenceFrequency() const
{
	return TaskScheduler::getCurrentTaskScheduler()->getTimer()->getFrequency();
}

#endif // _Sa1110GpioBuzzer_h_
