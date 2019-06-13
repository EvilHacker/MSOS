#ifndef _InterruptHandler_h_
#define _InterruptHandler_h_

#include "../cPrimitiveTypes.h"
#include "../Collections/Link.h"

//------------------------------------------------------------------------------------------------
// * class InterruptHandler
//
// Handles interrupts.
//------------------------------------------------------------------------------------------------

class InterruptHandler : public Link
{
public:
	// types
	enum InterruptLevel
	{
		defaultInterruptLevel = 0,
		#if defined(_MSC_VER) && defined(_M_ARM) || defined(__ARMCC_VERSION)
			// ARM has two interrupt levels: IRQ and FIQ
			irqInterruptLevel = 0,
			fiqInterruptLevel,
		#endif
		numberOfInterruptLevels
	};

	// accessing
	inline InterruptLevel getLevel() const;
	inline UInt getPriority() const;

protected:
	// constructor
	InterruptHandler(InterruptLevel level = defaultInterruptLevel, UInt priority = 0);

	// interrupt handling
	virtual Bool handleInterrupt() = 0;

private:
	// list node accessing
	inline InterruptHandler *getNextHandler() const;

	// representation
	InterruptLevel level;
	UInt priority;

	// friends
	friend class TaskScheduler;
};

//------------------------------------------------------------------------------------------------
// * InterruptHandler::InterruptHandler
//
// Constructor.
//------------------------------------------------------------------------------------------------

inline InterruptHandler::InterruptHandler(InterruptLevel level, UInt priority) :
	level(level),
	priority(priority)
{
}

//------------------------------------------------------------------------------------------------
// * InterruptHandler::getLevel
//
// Accessor.
//------------------------------------------------------------------------------------------------

inline InterruptHandler::InterruptLevel InterruptHandler::getLevel() const
{
	return level;
}

//------------------------------------------------------------------------------------------------
// * InterruptHandler::getPriority
//
// Accessor.
//------------------------------------------------------------------------------------------------

inline UInt InterruptHandler::getPriority() const
{
	return priority;
}

//------------------------------------------------------------------------------------------------
// * InterruptHandler::getNextTask
//
// Returns the next interrupt handler in the list.
//------------------------------------------------------------------------------------------------

inline InterruptHandler *InterruptHandler::getNextHandler() const
{
	return (InterruptHandler *)Link::getNext();
}

#endif // _InterruptHandler_h_
