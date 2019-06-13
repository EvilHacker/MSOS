#ifndef _Timer_h_
#define _Timer_h_

#include "../cPrimitiveTypes.h"
#include "../Collections/LinkedList.h"
#include "TimeValue.h"
class TimeInterval;

//------------------------------------------------------------------------------------------------
// * class Timer
//
// Keeps track of time.
//------------------------------------------------------------------------------------------------

class Timer
{
public:
	// types
	typedef ::TimeValue TimeValue;

	// destructor
	virtual ~Timer();

	// querying
	virtual TimeValue getFrequency() const = 0;
	virtual TimeValue convertSeconds(UInt seconds) const = 0;
	virtual TimeValue convertMilliseconds(UInt milliseconds) const = 0;
	virtual TimeValue convertMicroseconds(UInt microseconds) const = 0;
	virtual TimeValue getTime() const = 0;

protected:
	// constructor
	inline Timer();

	// interval accessing
	inline TimeInterval *getFirstInterval();
	inline void addInterval(TimeInterval *pInterval);
	inline void removeInterval(TimeInterval *pInterval);

	// time change handling
	void tick();

	// interval change handling
	virtual void updateIntervals() = 0;

	// friends
	friend class TimeInterval;

private:
	// sort function
	static SInt compareIntervals(
		const Link *pInterval1,
		const Link *pInterval2);

	// representation
	LinkedList intervalList;
};

#include "TimeInterval.h"

//------------------------------------------------------------------------------------------------
// * Timer::Timer
//
// Constructor.
//------------------------------------------------------------------------------------------------

inline Timer::Timer()
{
}

//------------------------------------------------------------------------------------------------
// * Timer::getFirstInterval
//
// Returns the next interval that will be expired.
//------------------------------------------------------------------------------------------------

inline TimeInterval *Timer::getFirstInterval()
{
	return (TimeInterval *)intervalList.getFirst();
}

//------------------------------------------------------------------------------------------------
// * Timer::addInterval
//
// Adds the specified <pInterval> to the list in time order.
// The <pInterval> will be added after any intervals that expire before it.
//------------------------------------------------------------------------------------------------

inline void Timer::addInterval(TimeInterval *pInterval)
{
	intervalList.addSorted(pInterval, &Timer::compareIntervals);
}

//------------------------------------------------------------------------------------------------
// * Timer::removeInterval
//
// Removes the specified <pInterval> from the list.
//------------------------------------------------------------------------------------------------

inline void Timer::removeInterval(TimeInterval *pInterval)
{
	intervalList.remove(pInterval);
}

#endif // _Timer_h_
