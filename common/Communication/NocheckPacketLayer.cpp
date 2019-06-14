#include "NocheckPacketLayer.h"
#include "NocheckUnidirectionalChannel.h"
#include "NocheckChannel.h"
#include "../../memoryUtilities.h"
#include "../../multitasking/LockedSection.h"

//------------------------------------------------------------------------------------------------
// * NocheckPacketLayer::NocheckPacketLayer
//
// Constructor.
//------------------------------------------------------------------------------------------------

NocheckPacketLayer::NocheckPacketLayer(Stream &stream, UInt basePriority) :
	stream(stream),
	packetReceiver(this, basePriority + 1, 20000)
{
	thisConnectionId = TaskScheduler::getCurrentTaskScheduler()->getTimer()->getTime() | 1;
	otherConnectionId = 1;
	stream.forceError();

	// no channels
	for(UInt i = 0; i < maximumNumberOfChannels; ++i)
	{
		pChannels[i] = null;
	}

	// start the receiver tasks
	packetReceiver.resume();
}

//------------------------------------------------------------------------------------------------
// * NocheckPacketLayer::~NocheckPacketLayer
//
// Destructor.
//------------------------------------------------------------------------------------------------

NocheckPacketLayer::~NocheckPacketLayer()
{
	// stop the transmitter and receiver tasks
	packetReceiver.suspend();
}

//------------------------------------------------------------------------------------------------
// * NocheckPacketLayer::createChannel
//
// Create a channel in the packet layer.
//------------------------------------------------------------------------------------------------

Stream * NocheckPacketLayer::createChannel(UInt readChannelId, UInt writeChannelId)
{
	return new NocheckChannel(*this, readChannelId, writeChannelId);
}

//------------------------------------------------------------------------------------------------
// * NocheckPacketLayer::addChannel
//
// Adds a channel to the packet layer.
//------------------------------------------------------------------------------------------------

void NocheckPacketLayer::addChannel(UnidirectionalChannel *pChannel)
{
	// add the channel
	LockedSection channelsLock(channelsMutex);
	pChannels[pChannel->getChannelId()] = (NocheckUnidirectionalChannel *)pChannel;
}

//------------------------------------------------------------------------------------------------
// * NocheckPacketLayer::removeChannel
//
// Removes a channel from the packet layer.
//------------------------------------------------------------------------------------------------

void NocheckPacketLayer::removeChannel(UnidirectionalChannel *pChannel)
{
	// remove the channel
	LockedSection channelsLock(channelsMutex);
	pChannels[pChannel->getChannelId()] = null;
}

//------------------------------------------------------------------------------------------------
// * NocheckPacketLayer::sendPacket
//
// Sends the packet.
//------------------------------------------------------------------------------------------------

void NocheckPacketLayer::sendPacket(NocheckPacket *pPacket)
{
	LockedSection writeAndResetLock(writeAndResetMutex);
	stream.write(pPacket->getPacketHeader(), sizeof(NocheckPacket::NocheckPacketHeader));
	stream.write(pPacket->getPacketData(), pPacket->getDataSize());
}

//------------------------------------------------------------------------------------------------
// * NocheckPacketLayer::receivePackets
//
// Receives packets in an infinite loop.
//------------------------------------------------------------------------------------------------

void NocheckPacketLayer::receivePackets()
{
	// keep receiving packets forever
	while(true)
	{
		// read the packet header
		NocheckPacket packet;
		stream.read(packet.getPacketHeader(), sizeof(NocheckPacket::NocheckPacketHeader));

		// check for errors
		if(stream.isInError())
		{
			// handle the error
			handleError();

			// continue reading packets
			continue;
		}
		
		if (packet.getDataSize() == 0)
		{
			continue;
		}
		
		NocheckUnidirectionalChannel *pChannel = null;
		if (packet.getChannelId() < getMaximumNumberOfChannels() &&
			(pChannel = pChannels[packet.getChannelId()]) != null)
		{

			while (true)
			{
				NocheckPacket * pPacket = pChannel->waitPacket();
				if (pChannel->isInError())
				{
					break;
				}

				if (pPacket != null)
				{
					if (packet.getDataSize() <= pPacket->getDataSize())
					{
						// read the packet data
						stream.read(pPacket->getPacketData(), packet.getDataSize());
						if (stream.isInError())
						{
							break;
						}

						pPacket->setDataSize(packet.getDataSize());
						pChannel->releasePacket(pPacket);
					}

					break;
				}
			}
		}
		else
		{
			// data for a non-existent channel
			stream.forceError();
		}
	}
}

//------------------------------------------------------------------------------------------------
// * NocheckPacketLayer::handleError
//
// Handle a communication error.
//------------------------------------------------------------------------------------------------

void NocheckPacketLayer::handleError()
{
	LockedSection writeAndResetLock(writeAndResetMutex);

	// check if the stream is in error
	if(!stream.isInError())
	{
		// the error has already been handled
		return;
	}

	// IDs to be exchanged
	UInt32 receiveId = 0;
	const UInt32 sendId = thisConnectionId;

	// attempt resynchronization several times
	for(UInt retryCount = 0; retryCount < 16; ++retryCount)
	{
		// reset the stream
		stream.reset();
	
		// read synchronization bytes
		UInt8 syncByte;
		do
		{
			syncByte = (UInt8)~0;
			stream.read(&syncByte, sizeof(syncByte));
		}
		while(syncByte == 0);

		// read a connection ID from other end
		stream.read(&receiveId, sizeof(receiveId));

		// check if we were successfull
		if(!stream.isInError())
		{
			break;
		}
	}

	// check if synchronization was successfull
	if(stream.isInError())
	{
		// could not reset the stream,
		// establish a new connection next time
		thisConnectionId |= 1;
		handleNewConnection();
		return;
	}

	// check the IDs to determine if a new connection has been made
	if((thisConnectionId & 1) != 0
		|| otherConnectionId != receiveId)
	{
		// a new connection has been made, reset all channels
		otherConnectionId = receiveId & ~(UInt32)1;
		thisConnectionId &= ~(UInt32)1;
		handleNewConnection();
	}
	else
	{
		// reestablish an existing connection
		handleReestablishedConnection();
	}

	// send synchronization bytes,
	// this sequence works with both UART and USB ports
	UInt8 syncByte;
	syncByte = 0;
	stream.write(&syncByte, sizeof(syncByte));
	stream.write(&syncByte, sizeof(syncByte));
	syncByte = (UInt8)~0;
	stream.write(&syncByte, sizeof(syncByte));

	// write this connection ID
	stream.write(&sendId, sizeof(sendId));
}

//------------------------------------------------------------------------------------------------
// * NocheckPacketLayer::handleNewConnection
//
// Make a new connection.
//------------------------------------------------------------------------------------------------

void NocheckPacketLayer::handleNewConnection()
{
	// propagate error to all channels
	LockedSection channelsLock(channelsMutex);
	for(UInt channelNumber = 0; channelNumber < maximumNumberOfChannels; ++channelNumber)
	{
		NocheckUnidirectionalChannel *pChannel = pChannels[channelNumber];
		if(pChannel != null)
		{
			pChannel->forceError();
		}
	}
}

//------------------------------------------------------------------------------------------------
// * NocheckPacketLayer::handleReestablishedConnection
//
// Attempt to recover from an error by resending historical packets.
//------------------------------------------------------------------------------------------------

void NocheckPacketLayer::handleReestablishedConnection()
{
}
