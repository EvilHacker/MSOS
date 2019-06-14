#include "Stream.h"

//------------------------------------------------------------------------------------------------
// * Stream::read
//
// Read data from the stream with the default timeout.
//------------------------------------------------------------------------------------------------

UInt Stream::read(void *pDestination, UInt length)
{
	return read(pDestination, length, defaultTimeout);
}

//------------------------------------------------------------------------------------------------
// * Stream::write
//
// Write data to the stream with the default timeout.
//------------------------------------------------------------------------------------------------

UInt Stream::write(const void *pSource, UInt length)
{
	return write(pSource, length, defaultTimeout);
}
