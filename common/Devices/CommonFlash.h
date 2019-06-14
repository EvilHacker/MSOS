#ifndef _CommonFlash_h_
#define _CommonFlash_h_

#include "Flash.h"
#include "../multitasking/Mutex.h"
#include "../multitasking/LockedSection.h"
#include "../memoryUtilities.h"
#include "../pointerArithmetic.h"

//------------------------------------------------------------------------------------------------
// * class CommonFlash
//
// Provides an interface to Flash memory.
//------------------------------------------------------------------------------------------------

template<class FlashWord>
class CommonFlash : public Flash
{
public:
	// flash operations
	virtual void write(void *destination, const void *source, UInt size);
	virtual void read(void *destination, const void *source, UInt size);

protected:
	// flash operations
	virtual void writeWord(void *destination, FlashWord value) = 0;

	// representation
	Mutex flashMutex;
};

//------------------------------------------------------------------------------------------------
// * CommonFlash::write
//
// Copies <size> bytes from <source> to <destination> in flash.
//------------------------------------------------------------------------------------------------

template<class FlashWord>
void CommonFlash<FlashWord>::write(void *destination, const void *source, UInt size)
{
	LockedSection flashLock(flashMutex);

	// check for missaligned start
	if(((UInt)destination & (sizeof(FlashWord) - 1)) != 0)
	{
		// write a word to flash but only change the last few bytes
		const UInt addressModulo = (UInt)destination & (sizeof(FlashWord) - 1);
		const UInt numberOfBytesChanging = minimum(sizeof(FlashWord) - addressModulo, size);

		FlashWord value = *subtractFromPointer((FlashWord *)destination, addressModulo);
		memoryCopy((UInt8 *)(&value) + addressModulo, source, numberOfBytesChanging);
		writeWord(subtractFromPointer(destination, addressModulo), value);

		// advance to the next aligned value
		destination = (UInt8 *)destination + numberOfBytesChanging;
		source = (UInt8 *)source + numberOfBytesChanging;
		size -= numberOfBytesChanging;
	}

	while(size >= sizeof(FlashWord))
	{
		// get the next flash word
		FlashWord value;
		memoryCopy(&value, source, sizeof(value));

		// write one flash word
		writeWord(destination, value);

		// advance to the next flash word
		destination = (FlashWord *)destination + 1;
		source = (FlashWord *)source + 1;
		size -= sizeof(FlashWord);
	}

	// check for missaligned end
	if(size != 0)
	{
		FlashWord value = *(FlashWord *)destination;
		memoryCopy(&value, source, size);
		writeWord(destination, value); 
	}
}


//------------------------------------------------------------------------------------------------
// * CommonFlash::read
//
// Copies <size> bytes from <source> in flash to <destination>.
//------------------------------------------------------------------------------------------------

template<class FlashWord>
void CommonFlash<FlashWord>::read(void *destination, const void *source, UInt size)
{
	LockedSection flashLock(flashMutex);

	memoryCopy(destination, source, size);
}

#endif // _CommonFlash_h_
