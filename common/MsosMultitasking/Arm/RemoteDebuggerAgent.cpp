#include "RemoteDebuggerAgent.h"
#include "../../memoryUtilities.h"
#include "../../pointerArithmetic.h"
#if defined(__TARGET_CPU_ARM920T)
	#include "../../Mx1Devices/Mx1MemoryCache.h"
	typedef Mx1MemoryCache MemoryCache;
#endif
#if defined(__TARGET_CPU_SA_1100)
	#include "../../Sa1110Devices/Sa1110MemoryCache.h"
	typedef Sa1110MemoryCache MemoryCache;
#endif
#include "exceptionHandlers.h"
#include "../LockedSection.h"

//------------------------------------------------------------------------------------------------
// * RemoteDebuggerAgent::RemoteDebuggerAgent
//
// Constructor.
//------------------------------------------------------------------------------------------------

RemoteDebuggerAgent::RemoteDebuggerAgent(
	Stream &commandReceivingStream,
	Stream &commandTransmittingStream,
	Task *pInitialStoppedTask) :
		commandReceivingStream(commandReceivingStream),
		commandTransmittingStream(commandTransmittingStream),
		commandReceiver(this, Task::debuggerPriority, 20000),
		commandTransmitter(this, Task::debuggerPriority, 20000)
{
	pStoppedTask = pInitialStoppedTask;
	memorySet(pAllTasks, 0, sizeof(pAllTasks));

	// check if there is an initial task to stop
	if(pInitialStoppedTask != null)
	{
		pAllTasks[0].pTask = pInitialStoppedTask;
		pAllTasks[0].stopped = true;
		if(!pInitialStoppedTask->isSuspended())
		{
			pInitialStoppedTask->suspend();
		}
	}

	// start tasks
	commandTransmitter.resume();
	commandReceiver.resume();

	// set singleton
	pCurrentRemoteDebuggerAgent = this;

	// install exception handlers
	oldResetHandler = getExceptionHandler(resetVectorIndex);
	setExceptionHandler(resetVectorIndex, &::handleReset);
	oldUndefinedInstructionHandler = getExceptionHandler(undefinedInstructionVectorIndex);
	setExceptionHandler(undefinedInstructionVectorIndex, &::handleUndefinedInstruction);
	oldInstructionAccessAbortHandler = getExceptionHandler(instructionAccessAbortVectorIndex);
	setExceptionHandler(instructionAccessAbortVectorIndex, &::handleInstructionAccessAbort);
	oldDataAccessAbortHandler = getExceptionHandler(dataAccessAbortVectorIndex);
	setExceptionHandler(dataAccessAbortVectorIndex, &::handleDataAccessAbort);
}

//------------------------------------------------------------------------------------------------
// * RemoteDebuggerAgent::~RemoteDebuggerAgent
//
// Destructor.
//------------------------------------------------------------------------------------------------

RemoteDebuggerAgent::~RemoteDebuggerAgent()
{
	// uninstall exception handlers
	setExceptionHandler(resetVectorIndex, oldResetHandler);
	setExceptionHandler(undefinedInstructionVectorIndex, oldUndefinedInstructionHandler);
	setExceptionHandler(instructionAccessAbortVectorIndex, oldInstructionAccessAbortHandler);
	setExceptionHandler(dataAccessAbortVectorIndex, oldDataAccessAbortHandler);

	// stop tasks
	commandReceiver.suspend();
	commandTransmitter.suspend();

	// singleton destroyed
	pCurrentRemoteDebuggerAgent = null;
}

//------------------------------------------------------------------------------------------------
// * RemoteDebuggerAgent::registerTask
//
// Register a task with the debugger.
//------------------------------------------------------------------------------------------------

void RemoteDebuggerAgent::registerTask(Task *pTask)
{
	LockedSection allTasksLock(allTasksMutex);
	for(UInt i = 0; i < maximumNumberOfTasks; ++i)
	{
		if(pAllTasks[i].pTask == null)
		{
			pAllTasks[i].pTask = pTask;
			pAllTasks[i].stopped = false;
			break;
		}
	}
}

//------------------------------------------------------------------------------------------------
// * RemoteDebuggerAgent::unregisterTask
//
// Unregister a task with the debugger.
//------------------------------------------------------------------------------------------------

void RemoteDebuggerAgent::unregisterTask(Task *pTask)
{
	LockedSection allTasksLock(allTasksMutex);
	for(UInt i = 0; i < maximumNumberOfTasks; ++i)
	{
		if(pAllTasks[i].pTask == pTask)
		{
			pAllTasks[i].pTask = null;
			break;
		}
	}
}

//------------------------------------------------------------------------------------------------
// * RemoteDebuggerAgent::processCommand
//
// Processes a single command from the host.
//------------------------------------------------------------------------------------------------

void RemoteDebuggerAgent::processCommand()
{
	// read the command id
	UInt8 commandId;
	commandReceivingStream.read(&commandId, sizeof(commandId));
	if(commandReceivingStream.isInError())
	{
		commandReceivingStream.reset();
		return;
	}

	// switch on the command
	switch(commandId)
	{
		case readMemoryCommand:
		{
			processReadMemoryCommand();
			break;
		}
		case writeMemoryCommand:
		{
			processWriteMemoryCommand();
			break;
		}
		case readRegistersCommand:
		{
			processReadRegistersCommand();
			break;
		}
		case writeCoprocessorRegistersCommand:
		{
			processWriteCoprocessorRegistersCommand();
			break;
		}
		case readCoprocessorRegistersCommand:
		{
			processReadCoprocessorRegistersCommand();
			break;
		}
		case writeRegistersCommand:
		{
			processWriteRegistersCommand();
			break;
		}
		case goCommand:
		{
			processGoCommand();
			break;
		}
		case stopCommand:
		{
			processStopCommand();
			break;
		}
		case getAllTasksCommand:
		{
			processGetAllTasksCommand();
			break;
		}
		case setStoppedTaskCommand:
		{
			processSetStoppedTaskCommand();
			break;
		}
		default:
		{
			commandReceivingStream.forceError();
			return;
		}
	}
}

//------------------------------------------------------------------------------------------------
// * RemoteDebuggerAgent::processReadMemoryCommand
//
// Processes a single command from the host.
//------------------------------------------------------------------------------------------------

void RemoteDebuggerAgent::processReadMemoryCommand()
{
	// get command parameters
	void *pMemory;
	commandReceivingStream.read(&pMemory, sizeof(pMemory));
	UInt length;
	commandReceivingStream.read(&length, sizeof(length));
	UInt8 accessType;
	commandReceivingStream.read(&accessType, sizeof(accessType));
	if(commandReceivingStream.isInError())
	{
		return;
	}

	// read memory in pieces
	const UInt maximumPieceLength = 128;
	while(length != 0)
	{
		UInt8 buffer[maximumPieceLength];
		UInt pieceLength = minimum(length, maximumPieceLength);

		// access memory
		switch(accessType)
		{
			default:
			{
				typedef UInt8 AccessValue;
				arrayCopy(
					(AccessValue *)buffer,
					(AccessValue *)pMemory,
					pieceLength / sizeof(AccessValue));
				break;
			}
			case 1:
			{
				typedef UInt16 AccessValue;
				arrayCopy(
					(AccessValue *)buffer,
					(AccessValue *)pMemory,
					pieceLength / sizeof(AccessValue));
				break;
			}
			case 2:
			{
				typedef UInt32 AccessValue;
				arrayCopy(
					(AccessValue *)buffer,
					(AccessValue *)pMemory,
					pieceLength / sizeof(AccessValue));
				break;
			}
		}

		// write data
		commandReceivingStream.write(buffer, pieceLength);

		// advance to the next piece
		pMemory = addToPointer(pMemory, pieceLength);
		length -= pieceLength;
	}

	commandReceivingStream.flush();
}

//------------------------------------------------------------------------------------------------
// * RemoteDebuggerAgent::processWriteMemoryCommand
//
// Processes a single command from the host.
//------------------------------------------------------------------------------------------------

void RemoteDebuggerAgent::processWriteMemoryCommand()
{
	// get command parameters
	void *pMemory;
	commandReceivingStream.read(&pMemory, sizeof(pMemory));
	UInt length;
	commandReceivingStream.read(&length, sizeof(length));
	UInt8 accessType;
	commandReceivingStream.read(&accessType, sizeof(accessType));
	if(commandReceivingStream.isInError())
	{
		return;
	}

	// read memory in pieces
	const UInt maximumPieceLength = 128;
	while(length != 0)
	{
		UInt8 buffer[maximumPieceLength];
		UInt pieceLength = minimum(length, maximumPieceLength);

		// read data
		commandReceivingStream.read(buffer, pieceLength);
		if(commandReceivingStream.isInError())
		{
			return;
		}

		// access memory
		switch(accessType)
		{
			default:
			{
				typedef UInt8 AccessValue;
				arrayCopy(
					(AccessValue *)pMemory,
					(AccessValue *)buffer,
					pieceLength / sizeof(AccessValue));
				break;
			}
			case 1:
			{
				typedef UInt16 AccessValue;
				arrayCopy(
					(AccessValue *)pMemory,
					(AccessValue *)buffer,
					pieceLength / sizeof(AccessValue));
				break;
			}
			case 2:
			{
				typedef UInt32 AccessValue;
				arrayCopy(
					(AccessValue *)pMemory,
					(AccessValue *)buffer,
					pieceLength / sizeof(AccessValue));
				break;
			}
		}

		// ensure coherency between data and instruction caches
		MemoryCache::getCurrentMemoryCache()->flushDataCacheEntries(pMemory, pieceLength);
		MemoryCache::getCurrentMemoryCache()->flushInstructionCache();

		// advance to the next piece
		pMemory = addToPointer(pMemory, pieceLength);
		length -= pieceLength;
	}
}

//------------------------------------------------------------------------------------------------
// * RemoteDebuggerAgent::processReadRegistersCommand
//
// Processes a single command from the host.
//------------------------------------------------------------------------------------------------

void RemoteDebuggerAgent::processReadRegistersCommand()
{
	// get command parameters
	UInt32 registerMask;
	commandReceivingStream.read(&registerMask, sizeof(registerMask));
	if(commandReceivingStream.isInError())
	{
		return;
	}

	UInt registerNumber = 0;
	while(registerMask != 0)
	{
		// check if this register is selected
		if((registerMask & 1) != 0)
		{
			UInt registerValue;

			// access register
			if(pStoppedTask == null || registerNumber > 16)
			{
				// unknown register
				registerValue = 0;
			}
			else
			{
				switch(registerNumber)
				{
					case 16:
					{
						// access cpsr
						registerValue = ((UInt *)pStoppedTask->pStackTop)[0];
						break;
					}
					case 15:
					case 14:
					{
						// access pc(r15) or lr(r14)
						registerValue = ((UInt *)pStoppedTask->pStackTop)[registerNumber];
						break;
					}
					case 13:
					{
						// access sp
						registerValue = (UInt)pStoppedTask->pStackTop + (16 * 4);
						break;
					}
					default:
					{
						// access r0 to r12
						registerValue = ((UInt *)pStoppedTask->pStackTop)[registerNumber + 1];
					}
				}
			}

			// write register
			commandReceivingStream.write(&registerValue, sizeof(registerValue));
		}

		// next register
		++registerNumber;
		registerMask >>= 1;
	}

	commandReceivingStream.flush();
}

//------------------------------------------------------------------------------------------------
// * RemoteDebuggerAgent::processWriteRegistersCommand
//
// Processes a single command from the host.
//------------------------------------------------------------------------------------------------

void RemoteDebuggerAgent::processWriteRegistersCommand()
{
	// get command parameters
	UInt32 registerMask;
	commandReceivingStream.read(&registerMask, sizeof(registerMask));
	if(commandReceivingStream.isInError())
	{
		return;
	}

	UInt registerNumber = 0;
	while(registerMask != 0)
	{
		// check if this register is selected
		if((registerMask & 1) != 0)
		{
			UInt registerValue;

			// read register
			commandReceivingStream.read(&registerValue, sizeof(registerValue));
			if(commandReceivingStream.isInError())
			{
				return;
			}

			// access register
			if(pStoppedTask == null || registerNumber > 16)
			{
				// unknown register
			}
			else
			{
				switch(registerNumber)
				{
					case 16:
					{
						// access cpsr
						((UInt *)pStoppedTask->pStackTop)[0] = registerValue;
						break;
					}
					case 15:
					case 14:
					{
						// access pc(r15) or lr(r14)
						((UInt *)pStoppedTask->pStackTop)[registerNumber] = registerValue;
						break;
					}
					case 13:
					{
						// access sp
						UInt savedRegisters[16];
						memoryCopy(
							savedRegisters,
							pStoppedTask->pStackTop,
							sizeof(savedRegisters));
						pStoppedTask->pStackTop = (void *)(registerValue - (16 * 4));
						memoryCopy(
							pStoppedTask->pStackTop,
							savedRegisters,
							sizeof(savedRegisters));
						break;
					}
					default:
					{
						// access r0 to r12
						((UInt *)pStoppedTask->pStackTop)[registerNumber + 1] = registerValue;
					}
				}
			}
		}

		// next register
		++registerNumber;
		registerMask >>= 1;
	}
}

//------------------------------------------------------------------------------------------------
// * RemoteDebuggerAgent::processReadCoprocessorRegistersCommand
//
// Processes a single command from the host.
//------------------------------------------------------------------------------------------------

void RemoteDebuggerAgent::processReadCoprocessorRegistersCommand()
{
	// get command parameters
	UInt8 coprocessorNumber;
	UInt16 registerMask;
	commandReceivingStream.read(&coprocessorNumber, sizeof(coprocessorNumber));
	commandReceivingStream.read(&registerMask, sizeof(registerMask));
	if(commandReceivingStream.isInError())
	{
		return;
	}

	coprocessorNumber &= 0xF;
	UInt registerNumber = 0;
	while(registerMask != 0)
	{
		// check if this register is selected
		if((registerMask & 1) != 0)
		{
			// construct the following code sequence
			// 	mrc p<coprocessorNumber>, 0, a1, c<registerNumber>, c0, 0
			// 	mov pc, lr
			UInt code[2];
			code[0] = 0xEE100010
				| ((UInt)coprocessorNumber << 8)
				| ((UInt)registerNumber << 16);
			code[1] = 0xE1A0F00E;

			// ensure coherency between data and instruction caches
			MemoryCache::getCurrentMemoryCache()->flushDataCacheEntries(code, sizeof(code));
			MemoryCache::getCurrentMemoryCache()->flushInstructionCache();
			asm
			{
				nop
				nop
				nop
			}

			// access the register
			UInt registerValue = ((UInt (*)())code)();

			// write register
			commandReceivingStream.write(&registerValue, sizeof(registerValue));
		}

		// next register
		++registerNumber;
		registerMask >>= 1;
	}

	commandReceivingStream.flush();
}

//------------------------------------------------------------------------------------------------
// * RemoteDebuggerAgent::processWriteCoprocessorRegistersCommand
//
// Processes a single command from the host.
//------------------------------------------------------------------------------------------------

void RemoteDebuggerAgent::processWriteCoprocessorRegistersCommand()
{
	// get command parameters
	UInt8 coprocessorNumber;
	UInt16 registerMask;
	commandReceivingStream.read(&coprocessorNumber, sizeof(coprocessorNumber));
	commandReceivingStream.read(&registerMask, sizeof(registerMask));
	if(commandReceivingStream.isInError())
	{
		return;
	}

	UInt registerNumber = 0;
	while(registerMask != 0)
	{
		// check if this register is selected
		if((registerMask & 1) != 0)
		{
			UInt registerValue;

			// read register
			commandReceivingStream.read(&registerValue, sizeof(registerValue));
			if(commandReceivingStream.isInError())
			{
				return;
			}

			// construct the following code sequence
			// 	mcr p<coprocessorNumber>, 0, a1, c<registerNumber>, c0, 0
			// 	mov pc, lr
			UInt code[2];
			code[0] = 0xEE000010
				| ((UInt)coprocessorNumber << 8)
				| ((UInt)registerNumber << 16);
			code[1] = 0xE1A0F00E;

			// ensure coherency between data and instruction caches
			MemoryCache::getCurrentMemoryCache()->flushDataCacheEntries(code, sizeof(code));
			MemoryCache::getCurrentMemoryCache()->flushInstructionCache();
			asm
			{
				nop
				nop
				nop
			}

			// access the register
			((void (*)(UInt registerValue))code)(registerValue);
		}

		// next register
		++registerNumber;
		registerMask >>= 1;
	}

	commandReceivingStream.flush();
}

//------------------------------------------------------------------------------------------------
// * RemoteDebuggerAgent::processGoCommand
//
// Processes a single command from the host.
//------------------------------------------------------------------------------------------------

void RemoteDebuggerAgent::processGoCommand()
{
	LockedSection allTasksLock(allTasksMutex);

	const Task *pSavedStoppedTask = pStoppedTask;

	// resume the stopped task first
	if(pStoppedTask != null)
	{
		pStoppedTask->resume();
		pStoppedTask = null;
	}

	// resume all other stopped tasks
	for(UInt i = 0; i < maximumNumberOfTasks; ++i)
	{
		if(pAllTasks[i].pTask != null
			&& pAllTasks[i].stopped)
		{
			if(pAllTasks[i].pTask != pSavedStoppedTask)
			{
				pAllTasks[i].pTask->resume();
			}
			pAllTasks[i].stopped = false;
		}
	}
}

//------------------------------------------------------------------------------------------------
// * RemoteDebuggerAgent::processStopCommand
//
// Processes a single command from the host.
//------------------------------------------------------------------------------------------------

void RemoteDebuggerAgent::processStopCommand()
{
	// check if a task has already been stopped
	if(pStoppedTask != null)
	{
		// stop the same task for the same reason
		stopTask(pStoppedTask, stopReason);
		return;
	}

	Task *pTask = TaskScheduler::getCurrentTaskScheduler()->getCurrentTask();

	// search for a running low priority task
	{
		UninterruptableSection criticalSection;
		while((pTask = pTask->getNextTask()) != null)
		{
			// check if this is a low priority task
			if(pTask->getPriority() > 0 && pTask->getPriority() < Task::debuggerPriority)
			{
				break;
			}
		}
	}

	// check if a running low priority task was not found
	if(pTask == null)
	{
		// search for any other low priority task
		LockedSection allTasksLock(allTasksMutex);
		for(UInt i = 0; i < maximumNumberOfTasks; ++i)
		{
			if(pAllTasks[i].pTask != null
				&& pAllTasks[i].pTask->getPriority() > 0
				&& pAllTasks[i].pTask->getPriority() < Task::debuggerPriority)
			{
				pTask = pAllTasks[i].pTask;
				break;
			}
		}
	}

	// task found, stop it
	stopTask(pTask, userInterrupted);
}

//------------------------------------------------------------------------------------------------
// * RemoteDebuggerAgent::processGetAllTasksCommand
//
// Processes a single command from the host.
//------------------------------------------------------------------------------------------------

void RemoteDebuggerAgent::processGetAllTasksCommand()
{
	// write the stopped task
	const UInt stoppedTaskHandle = (UInt)pStoppedTask | 1;
	commandReceivingStream.write(&stoppedTaskHandle, sizeof(stoppedTaskHandle));

	// iterate over all tasks
	for(UInt i = 0; i < maximumNumberOfTasks; ++i)
	{
		if(pAllTasks[i].pTask != null)
		{
			// convert the task pointer to a handle
			UInt taskHandle = (UInt)pAllTasks[i].pTask;
			if(pAllTasks[i].stopped)
			{
				// lowest bit indicates that the task is stopped
				taskHandle |= 1;
			}

			// write the task handle
			commandReceivingStream.write(&taskHandle, sizeof(taskHandle));
		}
	}

	// write 0 to indicate the end of the list
	const UInt zero = 0;
	commandReceivingStream.write(&zero, sizeof(zero));
}

//------------------------------------------------------------------------------------------------
// * RemoteDebuggerAgent::processSetStoppedTaskCommand
//
// Processes a single command from the host.
//------------------------------------------------------------------------------------------------

void RemoteDebuggerAgent::processSetStoppedTaskCommand()
{
	// read a task handle
	UInt taskHandle;
	commandReceivingStream.read(&taskHandle, sizeof(taskHandle));
	if(commandReceivingStream.isInError())
	{
		return;
	}

	// set the stopped task
	pStoppedTask = (Task *)(taskHandle & ~1u);
}

//------------------------------------------------------------------------------------------------
// * RemoteDebuggerAgent::stopTask
//
// Stops the specified task.
//------------------------------------------------------------------------------------------------

void RemoteDebuggerAgent::stopTask(Task *pTask, StopReason stopReason)
{
	UninterruptableSection criticalSection;

	if(pTask != null && pTask != pStoppedTask)
	{
		// suspend the task so that it will not run
		pTask->suspend();
	}

	// remember this task and the reason why it was stopped
	pStoppedTask = pTask;
	this->stopReason = stopReason;

	// signal that a task has been stopped
	stopEvent.signal();
}

//------------------------------------------------------------------------------------------------
// * RemoteDebuggerAgent::handleStop
//
// Stops the specified task.
//------------------------------------------------------------------------------------------------

void RemoteDebuggerAgent::handleStop()
{
	// wait for a stopped task
	stopEvent.wait();

	{
		LockedSection allTasksLock(allTasksMutex);
		while(true)
		{
			Bool stoppedTaskFound = (pStoppedTask == null);

			// stop all low priority tasks
			for(UInt i = 0; i < maximumNumberOfTasks; ++i)
			{
				if(pAllTasks[i].pTask != null)
				{
					if(pAllTasks[i].pTask == pStoppedTask)
					{
						pAllTasks[i].stopped = true;
						stoppedTaskFound = true;
					}
					else if(pAllTasks[i].pTask->getPriority() > 0
						&& pAllTasks[i].pTask->getPriority() < Task::debuggerPriority
						&& !pAllTasks[i].stopped)
					{
						pAllTasks[i].stopped = true;
						pAllTasks[i].pTask->suspend();
					}
				}
			}

			// check if we found the stopped task
			if(stoppedTaskFound)
			{
				// stopped task found
				break;
			}
			else
			{
				// stopped task not found, register it and try again
				registerTask(pStoppedTask);
			}
		}
	}

	// check for communication error
	if(commandTransmittingStream.isInError())
	{
		// communication error
		commandTransmittingStream.reset();
	}

	// notify host
	const UInt8 stopReasonByte = stopReason;
	commandTransmittingStream.write(&stopReasonByte, sizeof(stopReasonByte));

	// handshake with host
	UInt8 ack;
	commandTransmittingStream.read(&ack, sizeof(ack));
	commandTransmittingStream.write(&ack, sizeof(ack));
	commandTransmittingStream.flush();
}

//------------------------------------------------------------------------------------------------
// * RemoteDebuggerAgent::handleReset
//
// Handles a reset exception.
//------------------------------------------------------------------------------------------------

void RemoteDebuggerAgent::handleReset(UInt **ppInstruction)
{
	// unused argument
	ppInstruction = ppInstruction;

	// stop the task which executed this instruction
	stopTask(TaskScheduler::getCurrentTaskScheduler()->getCurrentTask(), branchThroughZero);
}

//------------------------------------------------------------------------------------------------
// * RemoteDebuggerAgent::handleUndefinedInstruction
//
// Handles an undefined instruction exception.
//------------------------------------------------------------------------------------------------

void RemoteDebuggerAgent::handleUndefinedInstruction(UInt **ppInstruction)
{
	// check if the instruction can be emulated
	if(false)
	{
		// emulate
		// ...

		// advance to the next instruction
		++(*ppInstruction);
		return;
	}

	StopReason stopReason;

	// check if this is a breakpoint
	if(**ppInstruction == breakpointInstruction)
	{
		// breakpoint
		stopReason = breakpointReached;
	}
	else if(**ppInstruction == hardcodedBreakpointInstruction)
	{
		// hardcoded breakpoint
		stopReason = userInterrupted;
		++(*ppInstruction);
	}
	else
	{
		// undefined instruction
		stopReason = undefinedInstruction;
	}

	// stop the task which executed this instruction
	stopTask(TaskScheduler::getCurrentTaskScheduler()->getCurrentTask(), stopReason);
}

//------------------------------------------------------------------------------------------------
// * RemoteDebuggerAgent::handleInstructionAccessAbort
//
// Handles an instruction access abort exception.
//------------------------------------------------------------------------------------------------

void RemoteDebuggerAgent::handleInstructionAccessAbort(UInt **ppInstruction)
{
	// unused argument
	ppInstruction = ppInstruction;

	// stop the task which executed this instruction
	stopTask(TaskScheduler::getCurrentTaskScheduler()->getCurrentTask(), instructionAccessAbort);
}

//------------------------------------------------------------------------------------------------
// * RemoteDebuggerAgent::handleDataAccessAbort
//
// Handles a data access abort exception.
//------------------------------------------------------------------------------------------------

void RemoteDebuggerAgent::handleDataAccessAbort(UInt **ppInstruction)
{
	// unused argument
	ppInstruction = ppInstruction;

	// stop the task which executed this instruction
	stopTask(TaskScheduler::getCurrentTaskScheduler()->getCurrentTask(), dataAccessAbort);
}

//------------------------------------------------------------------------------------------------
// * RemoteDebuggerAgent::receiveCommands
//
// Processes commands in an infinite loop.
//------------------------------------------------------------------------------------------------

void RemoteDebuggerAgent::receiveCommands()
{
	// process commands forever
	while(true)
	{
		processCommand();
	}
}

//------------------------------------------------------------------------------------------------
// * RemoteDebuggerAgent::transmitCommands
//
// Transmit commands in an infinite loop.
//------------------------------------------------------------------------------------------------

void RemoteDebuggerAgent::transmitCommands()
{
	// handle stopped tasks forever
	while(true)
	{
		handleStop();
	}
}

//------------------------------------------------------------------------------------------------
// * RemoteDebuggerAgent static variables
//------------------------------------------------------------------------------------------------

RemoteDebuggerAgent *RemoteDebuggerAgent::pCurrentRemoteDebuggerAgent = null;
