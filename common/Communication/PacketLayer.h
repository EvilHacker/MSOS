#ifndef _PacketLayer_h_
#define _PacketLayer_h_

#include "../cPrimitiveTypes.h"
#include "../multitasking/Mutex.h"
#include "Stream.h"
class UnidirectionalChannel;

//------------------------------------------------------------------------------------------------
// * class PacketLayer
//
// Multiplexes multiple logical channel over one physical connection.
//------------------------------------------------------------------------------------------------

class PacketLayer
{
public:
	virtual ~PacketLayer() {};

	// modifying channels
	virtual void addChannel(UnidirectionalChannel *pChannel) = 0;
	virtual void removeChannel(UnidirectionalChannel *pChannel) = 0;
	virtual Stream * createChannel(UInt readChannelId, UInt writeChannelId) = 0;
	
	// friends
	friend class UnidirectionalChannel;
	
protected:
	// synchronizers
	Mutex channelsMutex;
	Mutex writeAndResetMutex;
	
	// representation
	UInt32 thisConnectionId;
	UInt32 otherConnectionId;

	// querying
	inline UInt getMaximumNumberOfChannels() const;
	enum { maximumNumberOfChannels = 16 };
};

//------------------------------------------------------------------------------------------------
// * PacketLayer::getMaximumNumberOfChannels
//
// Returns the maximum number of channel supported by the packet layer.
//------------------------------------------------------------------------------------------------

inline UInt PacketLayer::getMaximumNumberOfChannels() const
{
	return maximumNumberOfChannels;
}

#endif // _PacketLayer_h_
