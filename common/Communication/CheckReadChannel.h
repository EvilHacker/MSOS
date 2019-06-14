#ifndef _CheckReadChannel_h_
#define _CheckReadChannel_h_

#include "../cPrimitiveTypes.h"
#include "CheckUnidirectionalChannel.h"
class CheckPacket;
class CheckPacketLayer;

//------------------------------------------------------------------------------------------------
// * class CheckReadChannel
//
// Represents a read-only communication stream.
//------------------------------------------------------------------------------------------------

class CheckReadChannel : public CheckUnidirectionalChannel
{
public:
	// constructor
	CheckReadChannel(CheckPacketLayer &packetLayer, UInt channelId);
	~CheckReadChannel();

	// streaming
	inline UInt read(void *pDestination, UInt length);
	UInt read(void *pDestination, UInt length, TimeValue timeout);
	inline UInt write(const void *pSource, UInt length);
	inline UInt write(const void *pSource, UInt length, TimeValue timeout);
	inline void flush();

	// error related
	void reset();

private:
	// state of current packet being read from
	UInt workingPacketOffset;
	UInt workingPacketDataSize;
};

//------------------------------------------------------------------------------------------------
// * CheckReadChannel::read
//
// Read data from the channel.
//------------------------------------------------------------------------------------------------

inline UInt CheckReadChannel::read(void *pDestination, UInt length)
{
	return read(pDestination, length, defaultTimeout);
}

//------------------------------------------------------------------------------------------------
// * CheckReadChannel::write
//
// Write data to the channel.
//------------------------------------------------------------------------------------------------

inline UInt CheckReadChannel::write(const void *pSource, UInt length)
{
	pSource = pSource;
	length = length;
	return 0;
}

//------------------------------------------------------------------------------------------------
// * CheckReadChannel::write
//
// Write data to the channel.
//------------------------------------------------------------------------------------------------

inline UInt CheckReadChannel::write(const void *pSource, UInt length, TimeValue timeout)
{
	pSource = pSource;
	length = length;
	timeout = timeout;
	return 0;
}

//------------------------------------------------------------------------------------------------
// * CheckReadChannel::flush
//
// Sends any buffered data.
//------------------------------------------------------------------------------------------------

inline void CheckReadChannel::flush()
{
}

#endif // _CheckReadChannel_h_
