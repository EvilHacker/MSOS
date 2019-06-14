#include "../cPrimitiveTypes.h"
#include "../multitasking/Task.h"
#include "../multitasking/TaskScheduler.h"
#include "../../Sa1110Devices/Sa1110UartPort.h"
#if defined(INCLUDE_DEBUGGER)
	#include "../../Communication/Stream.h"
	#include "../../Communication/CheckPacketLayer.h"
	#include "../../MsosMultitasking/arm/RemoteDebuggerAgent.h"
#endif
#include "JTagGemini.h"

//------------------------------------------------------------------------------------------------
// * class MainTask
//
// Runs BarcodeReaderApplication.
//------------------------------------------------------------------------------------------------

class MainTask : public Task
{
public:
	// constructor
	MainTask();

private:
	// task entry point
	void main();
};

//------------------------------------------------------------------------------------------------
// * MainTask::MainTask
//
// Constructor.
//------------------------------------------------------------------------------------------------

MainTask::MainTask() :
	Task(Task::highPriority + 1000, 10000)
{
}

//------------------------------------------------------------------------------------------------
// * MainTask::main
//
// Start the BarcodeReaderApplication running.
//------------------------------------------------------------------------------------------------

void MainTask::main()
{
	#if defined(INCLUDE_DEBUGGER)
	{
		// create protocol stack for debugger
		Sa1110UartPort *pDebuggerPort = new Sa1110UartPort(Sa1110UartPort::port1);
		CheckPacketLayer *pDebuggerPacketLayer = new CheckPacketLayer(
			*pDebuggerPort,
			Task::realtimePriority + 300);
	
		// create a debugger
		Stream *pDebuggerCommandReceivingChannel = pDebuggerPacketLayer->createChannel(13, 12);
		Stream *pDebuggerCommandTransmittingChannel = pDebuggerPacketLayer->createChannel(15, 14);
		new RemoteDebuggerAgent(
			*pDebuggerCommandReceivingChannel,
			*pDebuggerCommandTransmittingChannel);
	
		// allow the host to synchronize
		sleepForSeconds(10);
	}
	#endif

	(new JTagGemini)->resume();
}

//------------------------------------------------------------------------------------------------
// * main
//
// Entry point into firmware.
//------------------------------------------------------------------------------------------------

Int main()
{
	// create an initial task
	(new MainTask)->resume();

	// start multitasking
	TaskScheduler::getCurrentTaskScheduler()->start();

	// we will never get here
	return 0;
}
