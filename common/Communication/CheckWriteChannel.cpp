#include "CheckWriteChannel.h"
#include "../memoryUtilities.h"
#include "../pointerArithmetic.h"

//------------------------------------------------------------------------------------------------
// * CheckWriteChannel::CheckWriteChannel
//
// Constructor.
//------------------------------------------------------------------------------------------------

CheckWriteChannel::CheckWriteChannel(CheckPacketLayer &packetLayer, UInt channelId) :
	CheckUnidirectionalChannel(packetLayer, channelId)
{
	reset();
}

//------------------------------------------------------------------------------------------------
// * CheckWriteChannel::~CheckWriteChannel
//
// Destructor.
//------------------------------------------------------------------------------------------------

CheckWriteChannel::~CheckWriteChannel()
{
	// remove the channel from the packet layer
	packetLayer.removeChannel(this);
}

//------------------------------------------------------------------------------------------------
// * CheckWriteChannel::write
//
// Write data to the channel.
//------------------------------------------------------------------------------------------------

UInt CheckWriteChannel::write(const void *pSource, UInt length, TimeValue timeout)
{
	// do not continue if we are in an error state
	if(isInError())
	{
		return 0;
	}

	// keep writing in pieces until the entire amount has been written
	UInt remainingLength = length;
	while(remainingLength > 0)
	{
		// check if a packet is available for writing
		while(pWorkingPacket == null)
		{
			// get the next acknowledgment packet from the queue
			if(receiveQueue.removeFirst(&pWorkingPacket, timeout))
			{
				forceError();
			}
			workingPacketDataSize = 0;

			// do not continue if we are in an error state
			if(isInError())
			{
				return length - remainingLength;
			}
		}

		// do not continue if we are in an error state
		if(isInError())
		{
			return length - remainingLength;
		}

		// write a piece of data
		const UInt pieceLength = minimum(
			remainingLength,
			pWorkingPacket->getMaximumPacketDataSize() - workingPacketDataSize);
		memoryCopy(
			addToPointer(pWorkingPacket->getPacketData(), workingPacketDataSize),
			pSource,
			pieceLength);
		
		// advance to the next piece
		pSource = addToPointer(pSource, pieceLength);
		workingPacketDataSize += pieceLength;
		remainingLength -= pieceLength;
		
		// check if an entire packet has been written
		if(workingPacketDataSize == pWorkingPacket->getMaximumPacketDataSize())
		{
			// send the data packet
			pWorkingPacket->setSequenceNumber(
				pWorkingPacket->getSequenceNumber() + packetPipelineDepth);
			sendPacket(pWorkingPacket, workingPacketDataSize);

			// no packet available for writing
			pWorkingPacket = null;
		}
	}

	return length;
}

//------------------------------------------------------------------------------------------------
// * CheckWriteChannel::flush
//
// Sends any buffered data.
//------------------------------------------------------------------------------------------------

void CheckWriteChannel::flush()
{
	// check if there is a partial packet to send
	if(pWorkingPacket != null)
	{
		// send the data packet
		pWorkingPacket->setSequenceNumber(
			pWorkingPacket->getSequenceNumber() + packetPipelineDepth);
		sendPacket(pWorkingPacket, workingPacketDataSize);

		// no packet available for writing
		pWorkingPacket = null;
	}
}

//------------------------------------------------------------------------------------------------
// * CheckWriteChannel::reset
//
// Resets the stream and clears errors.
//------------------------------------------------------------------------------------------------

void CheckWriteChannel::reset()
{
	CheckUnidirectionalChannel::reset();

	// initialize pipeline
	for(UInt i = 0; i < packetPipelineDepth; ++i)
	{
		// send an acknowledgment to start the pipeline going
		CheckPacket *pAcknowledgmentPacket = getFreePacket();
		pAcknowledgmentPacket->setChannelId(channelId);
		pAcknowledgmentPacket->setSequenceNumber(i);
		pAcknowledgmentPacket->setDataSize(0);
		pAcknowledgmentPacket->setCrcValue();

		// add the packet into the receive queue
		receiveQueue.addLast(pAcknowledgmentPacket);
	}

	// add the channel to the packet layer
	packetLayer.addChannel(this);
}
