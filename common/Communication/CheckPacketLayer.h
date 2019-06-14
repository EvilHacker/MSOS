#ifndef _CheckPacketLayer_h_
#define _CheckPacketLayer_h_

#include "../cPrimitiveTypes.h"
#include "../multitasking/Task.h"
#include "../multitasking/MemberTask.h"
#include "../multitasking/IntertaskQueue.h"
#include "../multitasking/IntertaskEvent.h"
#include "../multitasking/Mutex.h"
#include "Stream.h"
#include "PacketLayer.h"
class CheckPacket;
class CheckUnidirectionalChannel;

//------------------------------------------------------------------------------------------------
// * class CheckPacketLayer
//
// Multiplexes multiple logical channel over one physical connection.
//------------------------------------------------------------------------------------------------

class CheckPacketLayer : public PacketLayer
{
public:
	// constructor and destructor
	CheckPacketLayer(Stream &stream, UInt basePriority = Task::realtimePriority);
	virtual ~CheckPacketLayer();

	// modifying channels
	void addChannel(UnidirectionalChannel *pChannel);
	void removeChannel(UnidirectionalChannel *pChannel);
	Stream * createChannel(UInt readChannelId, UInt writeChannelId);
	
	// packet operations
	inline CheckPacket *getFreePacket();
	inline void freePacket(CheckPacket *pPacket);
	inline void sendPacket(CheckPacket *pPacket);
	inline void sendPacketFirst(CheckPacket *pPacket);
	
private:
	// querying
	inline UInt getPacketPipelineDepth() const;

	// representation
	Stream &stream;
	UInt connectionNumber;
	enum { packetPipelineDepth = 2 };
	IntertaskPointerQueue<CheckPacket> freeQueue;
	IntertaskPointerQueue<CheckPacket> sendQueue;
	CheckUnidirectionalChannel *pChannels[PacketLayer::maximumNumberOfChannels];

	// packet exchange tasks
	void transmitPackets();
	void receivePackets();
	MemberTask(TransmitterTask, CheckPacketLayer, transmitPackets) packetTransmitter;
	MemberTask(ReceiverTask, CheckPacketLayer, receivePackets) packetReceiver;

	// error handling
	void handleError();
	void handleNewConnection();
	void handleReestablishedConnection();
};

#include "CheckPacket.h"

//------------------------------------------------------------------------------------------------
// * CheckPacketLayer::getPacketPipelineDepth
//
// Returns the packet pipeline depth per channel.
//------------------------------------------------------------------------------------------------

inline UInt CheckPacketLayer::getPacketPipelineDepth() const
{
	return packetPipelineDepth;
}

//------------------------------------------------------------------------------------------------
// * CheckPacketLayer::getFreePacket
//
// Returns a packet from the free queue.
//------------------------------------------------------------------------------------------------

inline CheckPacket *CheckPacketLayer::getFreePacket()
{
	CheckPacket *pPacket = freeQueue.removeFirst();
	pPacket->setConnectionNumber(connectionNumber);
	return pPacket;
}

//------------------------------------------------------------------------------------------------
// * CheckPacketLayer::freePacket
//
// Frees the packet.
//------------------------------------------------------------------------------------------------

inline void CheckPacketLayer::freePacket(CheckPacket *pPacket)
{
	freeQueue.addLast(pPacket);
}

//------------------------------------------------------------------------------------------------
// * CheckPacketLayer::sendPacket
//
// Sends the packet.
//------------------------------------------------------------------------------------------------

inline void CheckPacketLayer::sendPacket(CheckPacket *pPacket)
{
	sendQueue.addLast(pPacket);
}

//------------------------------------------------------------------------------------------------
// * CheckPacketLayer::sendPacketFirst
//
// Sends the packet before all other packets.
//------------------------------------------------------------------------------------------------

inline void CheckPacketLayer::sendPacketFirst(CheckPacket *pPacket)
{
	sendQueue.addFirst(pPacket);
}

#endif // _CheckPacketLayer_h_
