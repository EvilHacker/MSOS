#include "NocheckReadChannel.h"
#include "../memoryUtilities.h"
#include "../pointerArithmetic.h"

//------------------------------------------------------------------------------------------------
// * NocheckReadChannel::NocheckReadChannel
//
// Constructor.
//------------------------------------------------------------------------------------------------

NocheckReadChannel::NocheckReadChannel(NocheckPacketLayer &packetLayer, UInt channelId) :
	NocheckUnidirectionalChannel(packetLayer, channelId)
{
	reset();
}

//------------------------------------------------------------------------------------------------
// * NocheckReadChannel::~NocheckReadChannel
//
// Destructor.
//------------------------------------------------------------------------------------------------

NocheckReadChannel::~NocheckReadChannel()
{
	// remove the channel from the packet layer
	packetLayer.removeChannel(this);
}

//------------------------------------------------------------------------------------------------
// * NocheckReadChannel::read
//
// Read data from the channel.
//------------------------------------------------------------------------------------------------

UInt NocheckReadChannel::read(void *pDestination, UInt length, TimeValue timeout)
{
	// do not continue if we are in an error state
	if(isInError())
	{
		return 0;
	}

	// keep reading in pieces until the entire amount has been read
	UInt remainingLength = length;
	while(remainingLength > 0)
	{
		workingPacket.setPacketData(pDestination);
		workingPacket.setDataSize(remainingLength);

		// wait for data to transfer
		receivePacket(&workingPacket, timeout);
		if(isInError())
		{
			return length - remainingLength;
		}
		
		// requested length always >= received data size
		pDestination = addToPointer(pDestination, workingPacket.getDataSize());
		remainingLength -= workingPacket.getDataSize();
	}

	return length;
}

//------------------------------------------------------------------------------------------------
// * NocheckReadChannel::reset
//
// Resets the stream and clears errors.
//------------------------------------------------------------------------------------------------

void NocheckReadChannel::reset()
{
	NocheckUnidirectionalChannel::reset();

	// add the channel to the packet layer
	packetLayer.addChannel(this);
}
