#include "CheckUnidirectionalChannel.h"
#include "../memoryUtilities.h"
#include "../pointerArithmetic.h"

//------------------------------------------------------------------------------------------------
// * CheckUnidirectionalChannel::CheckUnidirectionalChannel
//
// Constructor.
//------------------------------------------------------------------------------------------------

CheckUnidirectionalChannel::CheckUnidirectionalChannel(CheckPacketLayer &packetLayer, UInt channelId) :
	packetLayer(packetLayer),
	receiveQueue(packetPipelineDepth + 1),
	historyQueue(packetPipelineDepth * 2)
{
	inError = true;
	pWorkingPacket = null;
	this->channelId = channelId;
	sequenceNumber = packetPipelineDepth;
	numberOfPacketsToFreeFromHistory = 0;
}

//------------------------------------------------------------------------------------------------
// * CheckUnidirectionalChannel::~CheckUnidirectionalChannel
//
// Destructor.
//------------------------------------------------------------------------------------------------

CheckUnidirectionalChannel::~CheckUnidirectionalChannel()
{
	// free all packets
	freeAllPackets();
}

//------------------------------------------------------------------------------------------------
// * CheckUnidirectionalChannel::forceError
//
// Force an error condition.
//------------------------------------------------------------------------------------------------

void CheckUnidirectionalChannel::forceError()
{
	// check if an error has occurred
	if(!isInError())
	{
		// remove the channel from the packet layer
		packetLayer.removeChannel(this);

		// flag that we are in an error state
		inError = true;

		// make sure that a pending read/write does not block
		if(receiveQueue.isEmpty())
		{
			// add a dummy entry to unblock a reading/writing task
			receiveQueue.addLast(null);
		}
	}
}

//------------------------------------------------------------------------------------------------
// * CheckUnidirectionalChannel::reset
//
// Resets the stream and clears errors.
//------------------------------------------------------------------------------------------------

void CheckUnidirectionalChannel::reset()
{
	inError = false;
	sequenceNumber = packetPipelineDepth;
	numberOfPacketsToFreeFromHistory = 0;

	// free all packets
	freeAllPackets();
}

//------------------------------------------------------------------------------------------------
// * CheckUnidirectionalChannel::receivePacket
//
// Receives <pPacket> from the the packet layer.
//------------------------------------------------------------------------------------------------

void CheckUnidirectionalChannel::receivePacket(CheckPacket *pPacket)
{
	// check if the sequence is correct
	if(pPacket->getSequenceNumber() == sequenceNumber)
	{
		// increment sequence number
		++sequenceNumber;

		// add the packet into the receive queue
		receiveQueue.addLast(pPacket);

		// remove a packet from the history queue
		++numberOfPacketsToFreeFromHistory;
		freePacketsInHistory();
	}
	else
	{
		// the packet is out of sequence, ignore it
		freePacket(pPacket);
	}
}

//------------------------------------------------------------------------------------------------
// * CheckUnidirectionalChannel::sendPacket
//
// Sends <pPacket> to the the packet layer.
//------------------------------------------------------------------------------------------------

void CheckUnidirectionalChannel::sendPacket(
	CheckPacket *pPacket,
	UInt dataSize)
{
	// fill in packet header
	pPacket->setDataSize(dataSize);
	pPacket->setCrcValue();

	// send the packet
	packetLayer.sendPacket(pPacket);
}

//------------------------------------------------------------------------------------------------
// * CheckUnidirectionalChannel::recordPacket
//
// Records <pPacket> that has already been sent in the history queue.
//------------------------------------------------------------------------------------------------

void CheckUnidirectionalChannel::recordPacket(CheckPacket *pPacket)
{
	// add the packet into the history queue
	historyQueue.addLast(pPacket);
}

//------------------------------------------------------------------------------------------------
// * CheckUnidirectionalChannel::freePacketsInHistory
//
// Attempts to free <numberOfPacketsToFreeFromHistory> packets from the history queue.
//------------------------------------------------------------------------------------------------

void CheckUnidirectionalChannel::freePacketsInHistory()
{
	// attempt to remove the required number of packets
	while(numberOfPacketsToFreeFromHistory > 0)
	{
		// do nothing if the history queue is empty
		if(historyQueue.isEmpty())
		{
			break;
		}

		// free one packet from the history
		--numberOfPacketsToFreeFromHistory;
		freePacket(historyQueue.removeFirst());
	}
}

//------------------------------------------------------------------------------------------------
// * CheckUnidirectionalChannel::resendAllPackets
//
// Send all packets in the history queue.
//------------------------------------------------------------------------------------------------

void CheckUnidirectionalChannel::resendAllPackets()
{
	freePacketsInHistory();
	while(!historyQueue.isEmpty())
	{
		packetLayer.sendPacketFirst(historyQueue.removeLast());
	}
}

//------------------------------------------------------------------------------------------------
// * CheckUnidirectionalChannel::freeAllPackets
//
// Frees all packets contained in the receiver.
//------------------------------------------------------------------------------------------------

void CheckUnidirectionalChannel::freeAllPackets()
{
	if(pWorkingPacket != null)
	{
		freePacket(pWorkingPacket);
		pWorkingPacket = null;
	}
	while(!receiveQueue.isEmpty())
	{
		CheckPacket *pPacket = receiveQueue.removeFirst();
		if(pPacket != null)
		{
			freePacket(pPacket);
		}
	}
	while(!historyQueue.isEmpty())
	{
		freePacket(historyQueue.removeFirst());
	}
}
