#ifndef _CheckWriteChannel_h_
#define _CheckWriteChannel_h_

#include "../cPrimitiveTypes.h"
#include "CheckUnidirectionalChannel.h"
class CheckPacket;
class CheckPacketLayer;

//------------------------------------------------------------------------------------------------
// * class CheckWriteChannel
//
// Represents a write-only communication stream.
//------------------------------------------------------------------------------------------------

class CheckWriteChannel : public CheckUnidirectionalChannel
{
public:
	// constructor and destructor
	CheckWriteChannel(CheckPacketLayer &packetLayer, UInt channelId);
	~CheckWriteChannel();

	// streaming
	inline UInt read(void *pDestination, UInt length);
	inline UInt read(void *pDestination, UInt length, TimeValue timeout);
	inline UInt write(const void *pSource, UInt length);
	UInt write(const void *pSource, UInt length, TimeValue timeout);
	void flush();

	// error related
	void reset();

private:
	// state of current packet being written to
	UInt workingPacketDataSize;
};

//------------------------------------------------------------------------------------------------
// * CheckWriteChannel::read
//
// Read data from the channel.
//------------------------------------------------------------------------------------------------

inline UInt CheckWriteChannel::read(void *pDestination, UInt length)
{
	pDestination = pDestination;
	length = length;
	return 0;
}

//------------------------------------------------------------------------------------------------
// * CheckWriteChannel::read
//
// Read data from the channel.
//------------------------------------------------------------------------------------------------

inline UInt CheckWriteChannel::read(void *pDestination, UInt length, TimeValue timeout)
{
	pDestination = pDestination;
	length = length;
	timeout = timeout;
	return 0;
}

//------------------------------------------------------------------------------------------------
// * CheckWriteChannel::write
//
// Writes data to the channel.
//------------------------------------------------------------------------------------------------

inline UInt CheckWriteChannel::write(const void *pSource, UInt length)
{
	return write(pSource, length, defaultTimeout);
}

#endif // _CheckWriteChannel_h_
