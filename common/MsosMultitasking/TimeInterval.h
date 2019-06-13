#ifndef _TimeInterval_h_
#define _TimeInterval_h_

#include "../cPrimitiveTypes.h"
#include "../Collections/Link.h"
#include "TimeValue.h"
class Timer;

//------------------------------------------------------------------------------------------------
// * class TimeInterval
//
// An instance of this class can be used to determine if a time interval has elapsed.
//------------------------------------------------------------------------------------------------

class TimeInterval : public Link
{
public:
	// types
	typedef ::TimeValue TimeValue;

	// constructors and destructors
	TimeInterval(Timer &timer);
	virtual ~TimeInterval();

	// testing
	inline Bool isExpired() const;

	// querying
	inline TimeValue getExpiryTime() const;

	// timing
	inline void beginTimingFor(TimeValue tickCount);
	void beginTimingUntil(TimeValue expiryTime);

protected:
	// behaviour
	virtual void handleExpiry() = 0;

private:
	// behaviour
	void expire();

	// list node accessing
	inline TimeInterval *getPreviousInterval() const;
	inline TimeInterval *getNextInterval() const;
	inline void setPreviousInterval(TimeInterval *pPreviousInterval);
	inline void setNextInterval(TimeInterval *pNextInterval);

	// representation
	Timer &timer;
	TimeValue expiryTime;
	Bool expired;

	// friends
	friend class Timer;
};

#include "Timer.h"

//------------------------------------------------------------------------------------------------
// * TimeInterval::isExpired
//
// Test whether this time interval has expired.
//------------------------------------------------------------------------------------------------

inline Bool TimeInterval::isExpired() const
{
	return expired;
}

//------------------------------------------------------------------------------------------------
// * TimeInterval::getExpiryTime
//
// Returns the time at which this interval will expire.
//------------------------------------------------------------------------------------------------

TimeValue TimeInterval::getExpiryTime() const
{
	return expiryTime;
}

//------------------------------------------------------------------------------------------------
// * TimeInterval::beginTimingFor
//
// Starts timing now.
//------------------------------------------------------------------------------------------------

inline void TimeInterval::beginTimingFor(TimeValue tickCount)
{
	beginTimingUntil(timer.getTime() + tickCount);
}

//------------------------------------------------------------------------------------------------
// * TimeInterval::getPreviousInterval
//
// Returns the previous interval in the list.
//------------------------------------------------------------------------------------------------

inline TimeInterval *TimeInterval::getPreviousInterval() const
{
	return (TimeInterval *)Link::getPrevious();
}

//------------------------------------------------------------------------------------------------
// * TimeInterval::getNextInterval
//
// Returns the next interval in the list.
//------------------------------------------------------------------------------------------------

inline TimeInterval *TimeInterval::getNextInterval() const
{
	return (TimeInterval *)Link::getNext();
}

//------------------------------------------------------------------------------------------------
// * TimeInterval::setPreviousInterval
//
// Changes the interval that comes before this one in the list.
//------------------------------------------------------------------------------------------------

inline void TimeInterval::setPreviousInterval(TimeInterval *pPreviousInterval)
{
	Link::setPrevious(pPreviousInterval);
}

//------------------------------------------------------------------------------------------------
// * TimeInterval::setNextInterval
//
// Changes the interval that comes after this one in the list.
//------------------------------------------------------------------------------------------------

inline void TimeInterval::setNextInterval(TimeInterval *pNextInterval)
{
	Link::setNext(pNextInterval);
}

#endif // _TimeInterval_h_
