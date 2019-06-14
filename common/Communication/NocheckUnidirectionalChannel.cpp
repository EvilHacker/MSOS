#include "NocheckUnidirectionalChannel.h"
#include "../memoryUtilities.h"
#include "../pointerArithmetic.h"

//------------------------------------------------------------------------------------------------
// * NocheckUnidirectionalChannel::NocheckUnidirectionalChannel
//
// Constructor.
//------------------------------------------------------------------------------------------------

NocheckUnidirectionalChannel::NocheckUnidirectionalChannel(NocheckPacketLayer &packetLayer, UInt channelId) :
	packetLayer(packetLayer),
	releaseQueue(1),
	waitQueue(1)
{
	inError = true;
	this->channelId = channelId;
	workingPacket.setChannelId(channelId);
}

//------------------------------------------------------------------------------------------------
// * NocheckUnidirectionalChannel::~NocheckUnidirectionalChannel
//
// Destructor.
//------------------------------------------------------------------------------------------------

NocheckUnidirectionalChannel::~NocheckUnidirectionalChannel()
{
	reset();
	
	// make sure that a pending read/write does not block
	forceError();
}

//------------------------------------------------------------------------------------------------
// * NocheckUnidirectionalChannel::forceError
//
// Force an error condition.
//------------------------------------------------------------------------------------------------

void NocheckUnidirectionalChannel::forceError()
{
	// check if an error has occurred
	if(!isInError())
	{
		// remove the channel from the packet layer
		packetLayer.removeChannel(this);

		// flag that we are in an error state
		inError = true;

		// make sure that a pending read on the channel does not block
		if(releaseQueue.isEmpty())
		{
			// add a dummy entry to unblock a reading/writing task
			releaseQueue.addLast(null);
		}
		
		// make sure that a pending read on the packetlayer does not block
		if(waitQueue.isEmpty())
		{
			// add a dummy entry to unblock a reading/writing task
			waitQueue.addLast(null);
		}
	}
}

//------------------------------------------------------------------------------------------------
// * NocheckUnidirectionalChannel::reset
//
// Resets the stream and clears errors.
//------------------------------------------------------------------------------------------------

void NocheckUnidirectionalChannel::reset()
{
	while (!releaseQueue.isEmpty())
	{
		releaseQueue.removeLast();
	}
	while (!waitQueue.isEmpty())
	{
		waitQueue.removeLast();
	}
	
	inError = false;
}

//------------------------------------------------------------------------------------------------
// * NocheckUnidirectionalChannel::waitPacket
//
// Wait until channel register request for the packet.
//------------------------------------------------------------------------------------------------

NocheckPacket * NocheckUnidirectionalChannel::waitPacket()
{
	return waitQueue.removeFirst();
}

//------------------------------------------------------------------------------------------------
// * NocheckUnidirectionalChannel::waitPacket
//
// Notify channel that request completed.
//------------------------------------------------------------------------------------------------

void NocheckUnidirectionalChannel::releasePacket(NocheckPacket * pPacket)
{
	releaseQueue.addLast(pPacket);
}

//------------------------------------------------------------------------------------------------
// * NocheckUnidirectionalChannel::sendPacket
//
// Sends <pPacket> to the the packet layer.
//------------------------------------------------------------------------------------------------

void NocheckUnidirectionalChannel::sendPacket(NocheckPacket *pPacket)
{
	// send the packet
	packetLayer.sendPacket(pPacket);
}

//------------------------------------------------------------------------------------------------
// * NocheckUnidirectionalChannel::receivePacket
//
// Receives <pPacket> from the the packet layer.
//------------------------------------------------------------------------------------------------

void NocheckUnidirectionalChannel::receivePacket(NocheckPacket *pPacket, TimeValue timeout)
{
	// add packet to request queue
	if (waitQueue.addLast(pPacket, timeout))
	{
		forceError();
	}

	// wait for packet
	while (!isInError())
	{
		NocheckPacket * pReturnPacket = null;
		if (releaseQueue.removeFirst(&pReturnPacket, timeout))
		{
			forceError();
		}

		// packet completed
		if (pReturnPacket == pPacket)
		{
			break;
		}
	}
}
