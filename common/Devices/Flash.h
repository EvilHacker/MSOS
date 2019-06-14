#ifndef _Flash_h_
#define _Flash_h_

#include "../cPrimitiveTypes.h"

//------------------------------------------------------------------------------------------------
// * class Flash
//
// Provides an interface to Flash memory.
//------------------------------------------------------------------------------------------------

class Flash
{
public:
	// destructor
	virtual ~Flash();

	// accessing
	inline static Flash *getCurrentFlash();

	// flash operations
	virtual void write(void *destination, const void *source, UInt size) = 0;
	virtual void read(void *destination, const void *source, UInt size) = 0;
	virtual void erase(void *destination) = 0;		

protected:
	// singleton
	static Flash *pCurrentFlash;
};

//------------------------------------------------------------------------------------------------
// * Flash::getCurrentFlash
//
// Returns the singleton instance.
//------------------------------------------------------------------------------------------------

inline Flash *Flash::getCurrentFlash()
{
	return pCurrentFlash;
}

#endif // _Flash_h_
