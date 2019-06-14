#ifndef _NocheckPacketLayer_h_
#define _NocheckPacketLayer_h_

#include "../../cPrimitiveTypes.h"
#include "../../multitasking/Task.h"
#include "../../multitasking/MemberTask.h"
#include "../../multitasking/IntertaskQueue.h"
#include "../../multitasking/IntertaskEvent.h"
#include "../../multitasking/Mutex.h"
#include "Stream.h"
#include "PacketLayer.h"

class NocheckPacket;
class NocheckUnidirectionalChannel;

//------------------------------------------------------------------------------------------------
// * class NocheckPacketLayer
//
// Multiplexes multiple logical channel over one physical connection.
//------------------------------------------------------------------------------------------------

class NocheckPacketLayer : public PacketLayer
{
public:
	// constructor and destructor
	NocheckPacketLayer(Stream &stream, UInt basePriority = Task::realtimePriority);
	virtual ~NocheckPacketLayer();

	// modifying channels
	void addChannel(UnidirectionalChannel *pChannel);
	void removeChannel(UnidirectionalChannel *pChannel);
	Stream * createChannel(UInt readChannelId, UInt writeChannelId);

	// packet operations
	void sendPacket(NocheckPacket *pPacket);
	
private:
	// representation
	Stream &stream;
	NocheckUnidirectionalChannel *pChannels[PacketLayer::maximumNumberOfChannels];
	
	// packet exchange tasks
	void receivePackets();
	MemberTask(ReceiverTask, NocheckPacketLayer, receivePackets) packetReceiver;

	// error handling
	void handleError();
	void handleNewConnection();
	void handleReestablishedConnection();
};

#endif // _NocheckPacketLayer_h_
