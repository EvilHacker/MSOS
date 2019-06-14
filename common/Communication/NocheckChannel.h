#ifndef _NocheckChannel_h_
#define _NocheckChannel_h_

#include "../cPrimitiveTypes.h"
#include "Stream.h"
#include "NocheckReadChannel.h"
#include "NocheckWriteChannel.h"
class NocheckPacketLayer;

//------------------------------------------------------------------------------------------------
// * class NocheckChannel
//
// Represents a bidirectional communication stream.
//------------------------------------------------------------------------------------------------

class NocheckChannel : public Stream
{
public:
	// constructor
	NocheckChannel(NocheckPacketLayer &packetLayer, UInt readChannelId, UInt writeChannelId);

	// testing
	inline Bool isInError() const;

	// streaming
	inline UInt read(void *pDestination, UInt length);
	inline UInt read(void *pDestination, UInt length, TimeValue timeout);
	inline UInt write(const void *pSource, UInt length);
	inline UInt write(const void *pSource, UInt length, TimeValue timeout);
	inline void flush();

	// error related
	inline void forceError();
	inline void reset();

private:
	// representation
	NocheckReadChannel readChannel;
	NocheckWriteChannel writeChannel;
};

//------------------------------------------------------------------------------------------------
// * NocheckChannel::isInError
//
// Tests whether an error condition has occurred.
//------------------------------------------------------------------------------------------------

inline Bool NocheckChannel::isInError() const
{
	return readChannel.isInError() || writeChannel.isInError();
}

//------------------------------------------------------------------------------------------------
// * NocheckChannel::read
//
// Read data from the stream.
//------------------------------------------------------------------------------------------------

inline UInt NocheckChannel::read(void *pDestination, UInt length)
{
	writeChannel.flush();
	return readChannel.read(pDestination, length);
}

//------------------------------------------------------------------------------------------------
// * NocheckChannel::read
//
// Read data from the stream.
//------------------------------------------------------------------------------------------------

inline UInt NocheckChannel::read(void *pDestination, UInt length, TimeValue timeout)
{
	writeChannel.flush();
	return readChannel.read(pDestination, length, timeout);
}

//------------------------------------------------------------------------------------------------
// * NocheckChannel::write
//
// Write data to the stream.
//------------------------------------------------------------------------------------------------

inline UInt NocheckChannel::write(const void *pSource, UInt length)
{
	return writeChannel.write(pSource, length);
}

//------------------------------------------------------------------------------------------------
// * NocheckChannel::write
//
// Write data to the stream.
//------------------------------------------------------------------------------------------------

inline UInt NocheckChannel::write(const void *pSource, UInt length, TimeValue timeout)
{
	return writeChannel.write(pSource, length, timeout);
}

//------------------------------------------------------------------------------------------------
// * NocheckChannel::flush
//
// Sends any buffered data.
//------------------------------------------------------------------------------------------------

inline void NocheckChannel::flush()
{
	writeChannel.flush();
}

//------------------------------------------------------------------------------------------------
// * NocheckChannel::forceError
//
// Force an error condition.
//------------------------------------------------------------------------------------------------

inline void NocheckChannel::forceError()
{
	readChannel.forceError();
	writeChannel.forceError();
}

//------------------------------------------------------------------------------------------------
// * NocheckChannel::reset
//
// Resets the stream and clears errors.
//------------------------------------------------------------------------------------------------

inline void NocheckChannel::reset()
{
	readChannel.reset();
	writeChannel.reset();
}

#endif // _NocheckChannel_h_
