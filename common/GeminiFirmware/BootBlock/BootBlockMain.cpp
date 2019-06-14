#include "../../cPrimitiveTypes.h"
#include "../../multitasking/Task.h"
#include "../../multitasking/TaskScheduler.h"
#include "../../multitasking/sleep.h"
#include "../../MsosMultitasking/arm/RemoteDebuggerAgent.h"
#if defined(__TARGET_CPU_ARM920T)
	#include "../../Mx1Devices/Mx1GpioOutput.h"
	#include "../../Mx1Devices/Mx1UartPort.h"
#endif
#if defined(__TARGET_CPU_SA_1100)
	#include "../../Sa1110Devices/Sa1110GpioOutput.h"
	#include "../../Sa1110Devices/Sa1110UartPort.h"
#endif
#include "../../Communication/CheckPacketLayer.h"
#include "BootTask.h"


//------------------------------------------------------------------------------------------------
// * class HeartbeatTask
//
// Flashes LEDs.
//------------------------------------------------------------------------------------------------

class HeartbeatTask : public Task
{
public:
	// constructor
	HeartbeatTask();

private:
	// task entry point
	void main();
};

//------------------------------------------------------------------------------------------------
// * HeartbeatTask::HeartbeatTask
//
// Constructor.
//------------------------------------------------------------------------------------------------

HeartbeatTask::HeartbeatTask() :
	Task(Task::realtimePriority, 1000)
{
}

//------------------------------------------------------------------------------------------------
// * HeartbeatTask::main
//
// Flashes LEDs.
//------------------------------------------------------------------------------------------------

void HeartbeatTask::main()
{
	const UInt sleepTime = 1000 / 40;
	
	// turn all camera's pins down
	#if defined(__TARGET_CPU_SA_1100)
		
		Sa1110GpioOutput sensorPowerUpPin(11, 1);
		Sa1110GpioOutput fiveVoltEnablePin(12, 1);
		Sa1110GpioOutput targetingLedsOffPin(13, 0);
	
		// LEDs on StrongARM Gemini device
		Sa1110GpioOutput redLed(6);
		Sa1110GpioOutput greenLed(7);
		
	#endif
	
	#if defined(__TARGET_CPU_ARM920T)
	
		Mx1GpioOutput targetingLedsOffPin(Mx1GpioPin::portD, 7, 0);
		Mx1GpioOutput illuminationLevel0Pin(Mx1GpioPin::portD, 8, 1);
		Mx1GpioOutput illuminationLevel1Pin(Mx1GpioPin::portD, 9, 1);
		Mx1GpioOutput illuminationLevel2Pin(Mx1GpioPin::portD, 10, 1);
		Mx1GpioOutput threeVoltDisablePin(Mx1GpioPin::portD, 11, 1);
		Mx1GpioOutput fiveVoltEnablePin(Mx1GpioPin::portD, 12, 1);
		Mx1GpioOutput sensorPowerUpPin(Mx1GpioPin::portD, 14, 1);
		
		// LEDs on DragonBall MX1 Gemini device
		Mx1GpioOutput greenLed(Mx1GpioOutput::portD, 16);
		Mx1GpioOutput redLed(Mx1GpioOutput::portD, 15);
		
	#endif

	// flash LEDs
	while(true)
	{
		greenLed.turnOn();
		sleepForMilliseconds(sleepTime);

		redLed.turnOn();
		sleepForMilliseconds(sleepTime);

		greenLed.turnOff();
		sleepForMilliseconds(sleepTime);

		redLed.turnOff();
		sleepForMilliseconds(sleepTime);
	}
}


//------------------------------------------------------------------------------------------------
// * class MainTask
//
// Task to be debugged.
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
	Task(Task::normalPriority, 1000)
{
}

//------------------------------------------------------------------------------------------------
// * MainTask::main
//
// Do nothing.
//------------------------------------------------------------------------------------------------

void MainTask::main()
{
	const UInt sleepTime = 1000 / 20;

	while(true)
	{
		sleepForMilliseconds(sleepTime);
	}
}


//------------------------------------------------------------------------------------------------
// * main
//
// Entry point into firmware.
//------------------------------------------------------------------------------------------------

Int main()
{
	// create heartbeat task to flash LEDs
	(new HeartbeatTask)->resume();
		
	#if defined(INCLUDE_DEBUGGER)
		// create an initial task to be debugged
		Task *pDebuggedTask = new MainTask;

		// create a remote debugger agent
		#if defined(__TARGET_CPU_ARM920T)
			Mx1UartPort *pSerialPort1 = new Mx1UartPort(Mx1UartPort::port1);
		#endif
		#if defined(__TARGET_CPU_SA_1100)
			Sa1110UartPort *pSerialPort1 = new Sa1110UartPort(Sa1110UartPort::port1);
		#endif
		CheckPacketLayer *pUartPacketLayer = new CheckPacketLayer(
			*pSerialPort1,
			Task::realtimePriority + 100);
		Stream *pDebuggerCommandReceivingChannel = pUartPacketLayer->createChannel(13, 12);
		Stream *pDebuggerCommandTransmittingChannel = pUartPacketLayer->createChannel(15, 14);
		new RemoteDebuggerAgent(
			*pDebuggerCommandReceivingChannel,
			*pDebuggerCommandTransmittingChannel,
			pDebuggedTask);
	#endif

	// create boot task to run firmware if present
	new BootTask;

	// start multitasking
	TaskScheduler::getCurrentTaskScheduler()->start();

	// we will never get here
	return 0;
}
