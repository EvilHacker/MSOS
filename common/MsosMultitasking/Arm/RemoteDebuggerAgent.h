#ifndef _RemoteDebuggerAgent_h_
#define _RemoteDebuggerAgent_h_

#include "../../cPrimitiveTypes.h"
#include "../../Communication/Stream.h"
#include "../Task.h"
#include "../MemberTask.h"
#include "../Mutex.h"
#include "../IntertaskEvent.h"

//------------------------------------------------------------------------------------------------
// * class RemoteDebuggerAgent
//
// Assists a host-side debugger by executing primitive commands.
//------------------------------------------------------------------------------------------------

class RemoteDebuggerAgent
{
public:
	// constructor and destructor
	RemoteDebuggerAgent(
		Stream &commandReceivingStream,
		Stream &commandTransmittingStream,
		Task *pInitialStoppedTask = null);
	~RemoteDebuggerAgent();

	// accessing
	static inline RemoteDebuggerAgent *getCurrentRemoteDebuggerAgent();

	// task registration
	void registerTask(Task *pTask);
	void unregisterTask(Task *pTask);

private:
	// types
	enum CommandId
	{
		readMemoryCommand,
		writeMemoryCommand,
		readRegistersCommand,
		writeRegistersCommand,
		readCoprocessorRegistersCommand,
		writeCoprocessorRegistersCommand,
		goCommand,
		stopCommand,
		getAllTasksCommand,
		setStoppedTaskCommand
	};
	enum StopReason
	{
		userInterrupted,
		breakpointReached,
		branchThroughZero,
		undefinedInstruction,
		instructionAccessAbort,
		dataAccessAbort
	};
	struct TaskInfo
	{
		Task *pTask;
		Bool stopped;
	};
	static const UInt breakpointInstruction = 0xE7FDDEFE;
	static const UInt hardcodedBreakpointInstruction = 0xE7FDDEEE;

	// host command processing
	void processCommand();
	void processReadMemoryCommand();
	void processWriteMemoryCommand();
	void processReadRegistersCommand();
	void processWriteRegistersCommand();
	void processReadCoprocessorRegistersCommand();
	void processWriteCoprocessorRegistersCommand();
	void processGoCommand();
	void processStopCommand();
	void processGetAllTasksCommand();
	void processSetStoppedTaskCommand();

	// task stop handling
	void handleStop();

	// exception handling
	void handleReset(UInt **ppInstruction);
	void handleUndefinedInstruction(UInt **ppInstruction);
	void handleInstructionAccessAbort(UInt **ppInstruction);
	void handleDataAccessAbort(UInt **ppInstruction);

	// controlling execution
	void stopTask(Task *pTask, StopReason stopReason);

	// communication
	Stream &commandReceivingStream;
	Stream &commandTransmittingStream;

	// currently debugged task
	Task *pStoppedTask;
	StopReason stopReason;
	IntertaskEvent stopEvent;

	// task registry
	static const UInt maximumNumberOfTasks = 64;
	TaskInfo pAllTasks[maximumNumberOfTasks];
	Mutex allTasksMutex;

	// tasks
	void receiveCommands();
	void transmitCommands();
	MemberTask(ReceiverTask, RemoteDebuggerAgent, receiveCommands) commandReceiver;
	MemberTask(TransmitterTask, RemoteDebuggerAgent, transmitCommands) commandTransmitter;

	// singleton
	static RemoteDebuggerAgent *pCurrentRemoteDebuggerAgent;
};

//------------------------------------------------------------------------------------------------
// * RemoteDebuggerAgent::getCurrentRemoteDebuggerAgent
//
// Returns the singleton instance.
//------------------------------------------------------------------------------------------------

inline RemoteDebuggerAgent *RemoteDebuggerAgent::getCurrentRemoteDebuggerAgent()
{
	return pCurrentRemoteDebuggerAgent;
}

#endif // _RemoteDebuggerAgent_h_
