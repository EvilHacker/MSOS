#ifndef _NocheckWriteChannel_h_
#define _NocheckWriteChannel_h_

#include "../cPrimitiveTypes.h"
#include "NocheckUnidirectionalChannel.h"
class NocheckPacket;
class NocheckPacketLayer;

//------------------------------------------------------------------------------------------------
// * class NocheckWriteChannel
//
// Represents a write-only communication stream.
//------------------------------------------------------------------------------------------------

class NocheckWriteChannel : public NocheckUnidirectionalChannel
{
public:
	// constructor and destructor
	NocheckWriteChannel(NocheckPacketLayer &packetLayer, UInt channelId);
	virtual ~NocheckWriteChannel();

	// streaming
	inline UInt read(void *pDestination, UInt length);
	inline UInt read(void *pDestination, UInt length, TimeValue timeout);
	inline UInt write(const void *pSource, UInt length);
	UInt write(const void *pSource, UInt length, TimeValue timeout);
	void flush() {};

	// error related
	void reset();
};

//------------------------------------------------------------------------------------------------
// * NocheckWriteChannel::read
//
// Read data from the channel.
//------------------------------------------------------------------------------------------------

inline UInt NocheckWriteChannel::read(void *pDestination, UInt length)
{
	pDestination = pDestination;
	length = length;
	return 0;
}

//------------------------------------------------------------------------------------------------
// * NocheckWriteChannel::read
//
// Read data from the channel.
//------------------------------------------------------------------------------------------------

inline UInt NocheckWriteChannel::read(void *pDestination, UInt length, TimeValue timeout)
{
	pDestination = pDestination;
	length = length;
	timeout = timeout;
	return 0;
}

//------------------------------------------------------------------------------------------------
// * NocheckWriteChannel::write
//
// Writes data to the channel.
//------------------------------------------------------------------------------------------------

inline UInt NocheckWriteChannel::write(const void *pSource, UInt length)
{
	return write(pSource, length, defaultTimeout);
}

#endif // _NocheckWriteChannel_h_
