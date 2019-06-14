#include "CheckReadChannel.h"
#include "../memoryUtilities.h"
#include "../pointerArithmetic.h"

//------------------------------------------------------------------------------------------------
// * CheckReadChannel::CheckReadChannel
//
// Constructor.
//------------------------------------------------------------------------------------------------

CheckReadChannel::CheckReadChannel(CheckPacketLayer &packetLayer, UInt channelId) :
	CheckUnidirectionalChannel(packetLayer, channelId)
{
	reset();
}

//------------------------------------------------------------------------------------------------
// * CheckReadChannel::~CheckReadChannel
//
// Destructor.
//------------------------------------------------------------------------------------------------

CheckReadChannel::~CheckReadChannel()
{
	// remove the channel from the packet layer
	packetLayer.removeChannel(this);
}

//------------------------------------------------------------------------------------------------
// * CheckReadChannel::read
//
// Read data from the channel.
//------------------------------------------------------------------------------------------------

UInt CheckReadChannel::read(void *pDestination, UInt length, TimeValue timeout)
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
		// check if a packet is available for reading
		while(pWorkingPacket == null)
		{
			// get the next data packet from the queue
			if(receiveQueue.removeFirst(&pWorkingPacket, timeout))
			{
				forceError();
			}

			// do not continue if we are in an error state
			if(isInError())
			{
				return length - remainingLength;
			}

			workingPacketOffset = 0;
			workingPacketDataSize = pWorkingPacket->getDataSize();
		}

		// do not continue if we are in an error state
		if(isInError())
		{
			return length - remainingLength;
		}

		// read a piece of data
		const UInt pieceLength = minimum(remainingLength, workingPacketDataSize);
		memoryCopy(
			pDestination,
			addToPointer(pWorkingPacket->getPacketData(), workingPacketOffset),
			pieceLength);
		
		// advance to the next piece
		pDestination = addToPointer(pDestination, pieceLength);
		workingPacketOffset += pieceLength;
		workingPacketDataSize -= pieceLength;
		remainingLength -= pieceLength;

		// check if an entire packet has been read
		if(workingPacketDataSize == 0)
		{
			// send an acknowledgement back
			sendPacket(pWorkingPacket, 0);

			// no packet available for reading
			pWorkingPacket = null;
		}
	}

	return length;
}

//------------------------------------------------------------------------------------------------
// * CheckReadChannel::reset
//
// Resets the stream and clears errors.
//------------------------------------------------------------------------------------------------

void CheckReadChannel::reset()
{
	CheckUnidirectionalChannel::reset();

	// initialize pipeline
	for(UInt i = 0; i < packetPipelineDepth; ++i)
	{
		// add an empty data packet to the history
		CheckPacket *pDataPacket = getFreePacket();
		pDataPacket->setChannelId(channelId);
		pDataPacket->setSequenceNumber(i);
		pDataPacket->setDataSize(0);
		pDataPacket->setCrcValue();

		// add the packet into the history queue
		historyQueue.addLast(pDataPacket);
	}

	// add the channel to the packet layer
	packetLayer.addChannel(this);
}
