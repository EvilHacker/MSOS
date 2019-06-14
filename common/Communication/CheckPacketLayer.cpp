#include "CheckPacketLayer.h"
#include "CheckUnidirectionalChannel.h"
#include "CheckChannel.h"
#include "../memoryUtilities.h"
#include "../multitasking/LockedSection.h"
#if defined(_MSC_VER)
	#pragma warning(disable: 4355) // warning C4355: 'this' : used in base member initializer list
#endif

//------------------------------------------------------------------------------------------------
// * CheckPacketLayer::CheckPacketLayer
//
// Constructor.
//------------------------------------------------------------------------------------------------

CheckPacketLayer::CheckPacketLayer(Stream &stream, UInt basePriority) :
	stream(stream),
	freeQueue(maximumNumberOfChannels * packetPipelineDepth * 2 + 2),
	sendQueue(maximumNumberOfChannels * packetPipelineDepth * 2 + 1),
	packetTransmitter(this, basePriority, 20000),
	packetReceiver(this, basePriority + 1, 20000)
{
	connectionNumber = 0;
	#if defined(MSOS_MULTITASKING)
		thisConnectionId = TaskScheduler::getCurrentTaskScheduler()->getTimer()->getTime() | 1;
	#endif
	#if defined(WIN32_MULTITASKING)
		thisConnectionId = GetTickCount() | 1;
	#endif
	otherConnectionId = 1;
	stream.forceError();

	// no channels
	for(UInt i = 0; i < maximumNumberOfChannels; ++i)
	{
		pChannels[i] = null;
	}

	// create working packets
	for(UInt j = 0; j < maximumNumberOfChannels * packetPipelineDepth * 2 + 2; ++j)
	{
		freeQueue.addLast(new CheckPacket);
	}

	// start the transmitter and receiver tasks
	packetTransmitter.resume();
	packetReceiver.resume();
}

//------------------------------------------------------------------------------------------------
// * CheckPacketLayer::~CheckPacketLayer
//
// Destructor.
//------------------------------------------------------------------------------------------------

CheckPacketLayer::~CheckPacketLayer()
{
	// stop the transmitter and receiver tasks
	packetTransmitter.suspend();
	packetReceiver.suspend();

	// delete working packets
	//for(UInt i = 0; i < maximumNumberOfChannels * packetPipelineDepth * 2 + 2; ++i)
	//{
	//	delete freeQueue.removeFirst();
	//}
}

//------------------------------------------------------------------------------------------------
// * CheckPacketLayer::createChannel
//
// Create a channel in the packet layer.
//------------------------------------------------------------------------------------------------

Stream * CheckPacketLayer::createChannel(UInt readChannelId, UInt writeChannelId)
{
	return new CheckChannel(*this, readChannelId, writeChannelId);
}

//------------------------------------------------------------------------------------------------
// * CheckPacketLayer::addChannel
//
// Adds a channel to the packet layer.
//------------------------------------------------------------------------------------------------

void CheckPacketLayer::addChannel(UnidirectionalChannel *pChannel)
{
	// add the channel
	LockedSection channelsLock(channelsMutex);
	pChannels[pChannel->getChannelId()] = (CheckUnidirectionalChannel *)pChannel;
}

//------------------------------------------------------------------------------------------------
// * CheckPacketLayer::removeChannel
//
// Removes a channel from the packet layer.
//------------------------------------------------------------------------------------------------

void CheckPacketLayer::removeChannel(UnidirectionalChannel *pChannel)
{
	// remove the channel
	LockedSection channelsLock(channelsMutex);
	pChannels[pChannel->getChannelId()] = null;
}

//------------------------------------------------------------------------------------------------
// * CheckPacketLayer::transmitPackets
//
// Transmits packets in an infinite loop.
//------------------------------------------------------------------------------------------------

void CheckPacketLayer::transmitPackets()
{
	// keep sending packets forever
	while(true)
	{
		// get a packet from the send queue
		sendQueue.reserveElementToRemoveFirst();
		LockedSection writeAndResetLock(writeAndResetMutex);
		CheckPacket *pPacket = sendQueue.removeReservedElementFirst();

		// check that this packet belongs to the current connection
		if(pPacket->getConnectionNumber() != connectionNumber)
		{
			// ignore this packet
			freePacket(pPacket);
			continue;
		}

		// write the packet (header and data)
		stream.write(
			pPacket->getPacketHeader(),
			sizeof(CheckPacket::CheckPacketHeader) + pPacket->getDataSize());

		// get the channel this packet belongs to
		LockedSection channelsLock(channelsMutex);
		CheckUnidirectionalChannel *pChannel;
		if(pPacket->getChannelId() < getMaximumNumberOfChannels()
			&& (pChannel = pChannels[pPacket->getChannelId()]) != null)
		{
			// give the packet back to the channel
			// the channel will record the packet in its history queue
			pChannel->recordPacket(pPacket);
		}
		else
		{
			// this packet belongs to an unknown channel, put it back in the free queue
			freePacket(pPacket);
		}
	}
}

//------------------------------------------------------------------------------------------------
// * CheckPacketLayer::receivePackets
//
// Receives packets in an infinite loop.
//------------------------------------------------------------------------------------------------

void CheckPacketLayer::receivePackets()
{
	// keep receiving packets forever
	while(true)
	{
		// get a packet from the free queue
		CheckPacket *pPacket = getFreePacket();
		
		// read the packet
		stream.read(pPacket->getPacketHeader(), sizeof(CheckPacket::CheckPacketHeader));
		if(pPacket->getDataSize() > 0)
		{
			stream.read(pPacket->getPacketData(), pPacket->getDataSize());
		}

		// check for errors
		if(stream.isInError() || !pPacket->isCrcValid())
		{
			// ignore this packet, it may have errors
			freePacket(pPacket);

			// handle the error
			handleError();

			// continue reading packets
			continue;
		}
		
		// dispatch the packet to a channel
		LockedSection channelsLock(channelsMutex);
		CheckUnidirectionalChannel *pChannel;
		if(pPacket->getChannelId() < getMaximumNumberOfChannels()
			&& (pChannel = pChannels[pPacket->getChannelId()]) != null)
		{
			// pass the packet on to the channel
			pChannel->receivePacket(pPacket);
		}
		else
		{
			// this packet belongs to an unknown channel, put it back in the free queue
			freePacket(pPacket);
		}
	}
}

//------------------------------------------------------------------------------------------------
// * CheckPacketLayer::handleError
//
// Handle a communication error.
//------------------------------------------------------------------------------------------------

void CheckPacketLayer::handleError()
{
	LockedSection writeAndResetLock(writeAndResetMutex);

	// check if the stream is in error
	if(!stream.isInError())
	{
		// the error has already been handled
		return;
	}

	// IDs to be exchanged
	UInt32 receiveId;
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
// * CheckPacketLayer::handleNewConnection
//
// Make a new connection.
//------------------------------------------------------------------------------------------------

void CheckPacketLayer::handleNewConnection()
{
	++connectionNumber;

	// propagate error to all channels
	LockedSection channelsLock(channelsMutex);
	for(UInt channelNumber = 0; channelNumber < maximumNumberOfChannels; ++channelNumber)
	{
		UnidirectionalChannel *pChannel = pChannels[channelNumber];
		if(pChannel != null)
		{
			pChannel->forceError();
		}
	}
}

//------------------------------------------------------------------------------------------------
// * CheckPacketLayer::handleReestablishedConnection
//
// Attempt to recover from an error by resending historical packets.
//------------------------------------------------------------------------------------------------

void CheckPacketLayer::handleReestablishedConnection()
{
	LockedSection sendLock(sendQueue.getMutex());

	// resend historical packets of all channels
	LockedSection channelsLock(channelsMutex);
	for(UInt channelNumber = 0; channelNumber < maximumNumberOfChannels; ++channelNumber)
	{
		CheckUnidirectionalChannel *pChannel = pChannels[channelNumber];
		if(pChannel != null)
		{
			// resend packets in the history queue of the channel
			pChannel->resendAllPackets();
		}
	}
}
