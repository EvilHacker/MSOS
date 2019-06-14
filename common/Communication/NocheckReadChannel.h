#ifndef _NocheckReadChannel_h_
#define _NocheckReadChannel_h_

#include "../cPrimitiveTypes.h"
#include "NocheckUnidirectionalChannel.h"
class NocheckPacket;
class NocheckPacketLayer;

//------------------------------------------------------------------------------------------------
// * class NocheckReadChannel
//
// Represents a read-only communication stream.
//------------------------------------------------------------------------------------------------

class NocheckReadChannel : public NocheckUnidirectionalChannel
{
public:
	// constructor
	NocheckReadChannel(NocheckPacketLayer &packetLayer, UInt channelId);
	virtual ~NocheckReadChannel();

	// streaming
	inline UInt read(void *pDestination, UInt length);
	UInt read(void *pDestination, UInt length, TimeValue timeout);
	inline UInt write(const void *pSource, UInt length);
	inline UInt write(const void *pSource, UInt length, TimeValue timeout);
	inline void flush() {};

	// error related
	void reset();
};

//------------------------------------------------------------------------------------------------
// * NocheckReadChannel::read
//
// Read data from the channel.
//------------------------------------------------------------------------------------------------

inline UInt NocheckReadChannel::read(void *pDestination, UInt length)
{
	return read(pDestination, length, defaultTimeout);
}

//------------------------------------------------------------------------------------------------
// * NocheckReadChannel::write
//
// Write data to the channel.
//------------------------------------------------------------------------------------------------

inline UInt NocheckReadChannel::write(const void *pSource, UInt length)
{
	pSource = pSource;
	length = length;
	return 0;
}

//------------------------------------------------------------------------------------------------
// * NocheckReadChannel::write
//
// Write data to the channel.
//------------------------------------------------------------------------------------------------

inline UInt NocheckReadChannel::write(const void *pSource, UInt length, TimeValue timeout)
{
	pSource = pSource;
	length = length;
	timeout = timeout;
	return 0;
}

#endif // _NocheckReadChannel_h_
