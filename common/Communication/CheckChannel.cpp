#include "CheckChannel.h"

//------------------------------------------------------------------------------------------------
// * CheckChannel::CheckChannel
//
// Constructor.
//------------------------------------------------------------------------------------------------

CheckChannel::CheckChannel(CheckPacketLayer &packetLayer, UInt readChannelId, UInt writeChannelId) :
	readChannel(packetLayer, readChannelId),
	writeChannel(packetLayer, writeChannelId)
{
	defaultTimeout = infiniteTime;
}
