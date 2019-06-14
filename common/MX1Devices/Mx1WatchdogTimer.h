#ifndef _Mx1WatchdogTimer_h_
#define _Mx1WatchdogTimer_h_

#if !defined(INCLUDE_DEBUGGER)

#include "../cPrimitiveTypes.h"
#include "../multitasking/Mutex.h"
#include "Mx1DeviceAddresses.h"
class WatchdogSection;
class LinkedList;

//------------------------------------------------------------------------------------------------
// * class Mx1WatchdogTimer
//
// Provides an interface to the MX1 Watchdog Timer.
//------------------------------------------------------------------------------------------------

class Mx1WatchdogTimer
{
public:
	// querying
	inline static Mx1WatchdogTimer *getCurrentWatchdogTimer();

private:
	// constructor and destructor
	Mx1WatchdogTimer();
	~Mx1WatchdogTimer();

	// adding/removing watchdog sections
	void addSection(WatchdogSection *pSection);
	void removeSection(WatchdogSection *pSection);

	// updating
	void update();

	// sort function
	static SInt compareWatchdogSections(
		const Link *pSection1,
		const Link *pSection2);

	// register accessing
	enum RegisterAddress
	{
		wcr = mx1RegistersBase + 0x1000,
		wsr = mx1RegistersBase + 0x1004,
		wstr = mx1RegistersBase + 0x1008
	};
	inline UInt readRegister(RegisterAddress address);
	inline void writeRegister(RegisterAddress address, UInt value);

	// representation
	Mutex sectionsListMutex;
	LinkedList sectionsList;

	// singleton
	static Mx1WatchdogTimer currentWatchdogTimer;

	// friends
	friend class WatchdogSection;
};

//------------------------------------------------------------------------------------------------
// * Mx1WatchdogTimer::getCurrentWatchdogTimer
//
// Read a Watchdog Timer register.
//------------------------------------------------------------------------------------------------

inline Mx1WatchdogTimer *Mx1WatchdogTimer::getCurrentWatchdogTimer()
{
	return &currentWatchdogTimer;
}

//------------------------------------------------------------------------------------------------
// * Mx1WatchdogTimer::readRegister
//
// Read a Watchdog Timer register.
//------------------------------------------------------------------------------------------------

inline UInt Mx1WatchdogTimer::readRegister(RegisterAddress address)
{
	return *(volatile UInt *)address;
}

//------------------------------------------------------------------------------------------------
// * Mx1WatchdogTimer::writeRegister
//
// Writes a Watchdog Timer register.
//------------------------------------------------------------------------------------------------

inline void Mx1WatchdogTimer::writeRegister(RegisterAddress address, UInt value)
{
	*(volatile UInt *)address = value;
}

#endif // !defined(INCLUDE_DEBUGGER)

#endif // _Mx1WatchdogTimer_h_
