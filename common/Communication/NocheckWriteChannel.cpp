#include "NocheckWriteChannel.h"
#include "../memoryUtilities.h"
#include "../pointerArithmetic.h"

//------------------------------------------------------------------------------------------------
// * NocheckWriteChannel::NocheckWriteChannel
//
// Constructor.
//------------------------------------------------------------------------------------------------

NocheckWriteChannel::NocheckWriteChannel(NocheckPacketLayer &packetLayer, UInt channelId) :
	NocheckUnidirectionalChannel(packetLayer, channelId)
{
	reset();
}

//------------------------------------------------------------------------------------------------
// * NocheckWriteChannel::~NocheckWriteChannel
//
// Destructor.
//------------------------------------------------------------------------------------------------

NocheckWriteChannel::~NocheckWriteChannel()
{
	// remove the channel from the packet layer
	packetLayer.removeChannel(this);
}

//------------------------------------------------------------------------------------------------
// * NocheckWriteChannel::write
//
// Write data to the channel.
//------------------------------------------------------------------------------------------------

UInt NocheckWriteChannel::write(const void *pSource, UInt length, TimeValue timeout)
{
	// do not continue if we are in an error state
	if(isInError())
	{
		return 0;
	}
	
	if (length == 0)
	{
		return 0;
	}
	
	// write all data at once
	workingPacket.setDataSize(length);
	workingPacket.setPacketData(pSource);
	
	// send data 
	sendPacket(&workingPacket);

	return length;
}

//------------------------------------------------------------------------------------------------
// * NocheckWriteChannel::reset
//
// Resets the stream and clears errors.
//------------------------------------------------------------------------------------------------

void NocheckWriteChannel::reset()
{
	NocheckUnidirectionalChannel::reset();
	
	// add the channel to the packet layer
	packetLayer.addChannel(this);
}
