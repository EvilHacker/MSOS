#include "NocheckChannel.h"

//------------------------------------------------------------------------------------------------
// * NocheckChannel::NocheckChannel
//
// Constructor.
//------------------------------------------------------------------------------------------------

NocheckChannel::NocheckChannel(NocheckPacketLayer &packetLayer, UInt readChannelId, UInt writeChannelId) :
	readChannel(packetLayer, readChannelId),
	writeChannel(packetLayer, writeChannelId)
{
	defaultTimeout = infiniteTime;
}
