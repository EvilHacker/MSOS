#ifndef _CheckChannel_h_
#define _CheckChannel_h_

#include "../cPrimitiveTypes.h"
#include "Stream.h"
#include "CheckReadChannel.h"
#include "CheckWriteChannel.h"
class CheckPacketLayer;

//------------------------------------------------------------------------------------------------
// * class CheckChannel
//
// Represents a bidirectional communication stream.
//------------------------------------------------------------------------------------------------

class CheckChannel : public Stream
{
public:
	// constructor
	CheckChannel(CheckPacketLayer &packetLayer, UInt readChannelId, UInt writeChannelId);

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
	CheckReadChannel readChannel;
	CheckWriteChannel writeChannel;
};

//------------------------------------------------------------------------------------------------
// * CheckChannel::isInError
//
// Tests whether an error condition has occurred.
//------------------------------------------------------------------------------------------------

inline Bool CheckChannel::isInError() const
{
	return readChannel.isInError() || writeChannel.isInError();
}

//------------------------------------------------------------------------------------------------
// * CheckChannel::read
//
// Read data from the stream.
//------------------------------------------------------------------------------------------------

inline UInt CheckChannel::read(void *pDestination, UInt length)
{
	writeChannel.flush();
	return readChannel.read(pDestination, length);
}

//------------------------------------------------------------------------------------------------
// * CheckChannel::read
//
// Read data from the stream.
//------------------------------------------------------------------------------------------------

inline UInt CheckChannel::read(void *pDestination, UInt length, TimeValue timeout)
{
	writeChannel.flush();
	return readChannel.read(pDestination, length, timeout);
}

//------------------------------------------------------------------------------------------------
// * CheckChannel::write
//
// Write data to the stream.
//------------------------------------------------------------------------------------------------

inline UInt CheckChannel::write(const void *pSource, UInt length)
{
	return writeChannel.write(pSource, length);
}

//------------------------------------------------------------------------------------------------
// * CheckChannel::write
//
// Write data to the stream.
//------------------------------------------------------------------------------------------------

inline UInt CheckChannel::write(const void *pSource, UInt length, TimeValue timeout)
{
	return writeChannel.write(pSource, length, timeout);
}

//------------------------------------------------------------------------------------------------
// * CheckChannel::flush
//
// Sends any buffered data.
//------------------------------------------------------------------------------------------------

inline void CheckChannel::flush()
{
	writeChannel.flush();
}

//------------------------------------------------------------------------------------------------
// * CheckChannel::forceError
//
// Force an error condition.
//------------------------------------------------------------------------------------------------

inline void CheckChannel::forceError()
{
	readChannel.forceError();
	writeChannel.forceError();
}

//------------------------------------------------------------------------------------------------
// * CheckChannel::reset
//
// Resets the stream and clears errors.
//------------------------------------------------------------------------------------------------

inline void CheckChannel::reset()
{
	readChannel.reset();
	writeChannel.reset();
}

#endif // _CheckChannel_h_
