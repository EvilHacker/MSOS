#ifndef _Sa1110WatchdogTimer_h_
#define _Sa1110WatchdogTimer_h_

#if !defined(INCLUDE_DEBUGGER)

#include "../cPrimitiveTypes.h"
#include "../multitasking/Task.h"
#include "../multitasking/Mutex.h"
#include "Sa1110DeviceAddresses.h"
class WatchdogSection;
class LinkedList;

//------------------------------------------------------------------------------------------------
// * class Sa1110WatchdogTimer
//
// Provides an interface to the StrongArm Watchdog Timer.
//------------------------------------------------------------------------------------------------

class Sa1110WatchdogTimer : private Task
{
public:
	// querying
	inline static Sa1110WatchdogTimer *getCurrentWatchdogTimer();

private:
	// constructor and destructor
	Sa1110WatchdogTimer();
	~Sa1110WatchdogTimer();

	// adding/removing watchdog sections
	void addSection(WatchdogSection *pSection);
	void removeSection(WatchdogSection *pSection);

	// updating
	void update();
	void main();

	// sort function
	static SInt compareWatchdogSections(
		const Link *pSection1,
		const Link *pSection2);

	// register accessing
	enum RegisterAddress
	{
		osmr3 = sa1110SystemControlBase + 0x0C,
		oscr = sa1110SystemControlBase + 0x10,
		ower = sa1110SystemControlBase + 0x18,
		rsrr = sa1110SystemControlBase + 0x30000
	};
	inline UInt readRegister(RegisterAddress address);
	inline void writeRegister(RegisterAddress address, UInt value);

	// representation
	Mutex sectionsListMutex;
	LinkedList sectionsList;
	static const TimeValue kickTime = 3686400 * 1; // 1 second
	static const TimeValue resetTime = 3686400 * 1.1; // 1.1 seconds

	// singleton
	static Sa1110WatchdogTimer currentWatchdogTimer;

	// friends
	friend class WatchdogSection;
};

//------------------------------------------------------------------------------------------------
// * Sa1110WatchdogTimer::getCurrentWatchdogTimer
//
// Read a Watchdog Timer register.
//------------------------------------------------------------------------------------------------

inline Sa1110WatchdogTimer *Sa1110WatchdogTimer::getCurrentWatchdogTimer()
{
	return &currentWatchdogTimer;
}

//------------------------------------------------------------------------------------------------
// * Sa1110WatchdogTimer::readRegister
//
// Read a Watchdog Timer register.
//------------------------------------------------------------------------------------------------

inline UInt Sa1110WatchdogTimer::readRegister(RegisterAddress address)
{
	return *(volatile UInt *)address;
}

//------------------------------------------------------------------------------------------------
// * Sa1110WatchdogTimer::writeRegister
//
// Writes a Watchdog Timer register.
//------------------------------------------------------------------------------------------------

inline void Sa1110WatchdogTimer::writeRegister(RegisterAddress address, UInt value)
{
	*(volatile UInt *)address = value;
}

#endif // !defined(INCLUDE_DEBUGGER)

#endif // _Sa1110WatchdogTimer_h_
