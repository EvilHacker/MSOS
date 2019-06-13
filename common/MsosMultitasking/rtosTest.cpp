#include "Task.h"
#include "Mutex.h"
#include "IntertaskQueue.h"
#include "LockedSection.h"
#if defined(_MSC_VER) && defined(_M_IX86) || defined(__GNUC__)
	#define PRINT
#endif
#if defined(PRINT)
	#if defined(__ARMCC_VERSION) && !defined(std)
		#define std
	#endif
	#include <iostream>
	#include <stdlib.h>
#endif

//------------------------------------------------------------------------------------------------
// * class NumberedTask
//------------------------------------------------------------------------------------------------

class NumberedTask : public Task
{
protected:
	// constructor
	NumberedTask(UInt priority, UInt stackSize);

public:
	// representation
	UInt taskNumber;

private:
	// representation
	static UInt taskCount;
	static Mutex taskCountMutex;
};

NumberedTask::NumberedTask(UInt priority, UInt stackSize):
	Task(priority, stackSize)
{
	LockedSection lockedSection(taskCountMutex); 
	taskNumber = ++taskCount;
}

UInt NumberedTask::taskCount = 0;
Mutex NumberedTask::taskCountMutex; 


//------------------------------------------------------------------------------------------------
// * class Task1
//------------------------------------------------------------------------------------------------

class Task1 : public NumberedTask
{
public:
	// constructor
	Task1(UInt priority = defaultPriority);

protected:
	// main entry point
	void main();
};

Task1::Task1(UInt priority) :
	NumberedTask(priority, 10000)
{
}

void Task1::main()
{
	while(true)
	{
		UInt8 *pBuffer = new UInt8[taskNumber];

		#if defined(PRINT)
			std::cout << "Task" << taskNumber << '\n';
		#endif

		yield();

		delete pBuffer;
	}
}


//------------------------------------------------------------------------------------------------
// * class Producer
//------------------------------------------------------------------------------------------------

class Producer : public NumberedTask
{
public:
	// constructor
	Producer(
		Int product, Int numberOfProducts, IntertaskValueQueue<Int> &queue,
		UInt priority = defaultPriority);

protected:
	// main entry point
	void main();

private:
	// representation
	Int product;
	Int numberOfProducts;
	IntertaskValueQueue<Int> &queue;
};

Producer::Producer(Int product, Int numberOfProducts, IntertaskValueQueue<Int> &queue, UInt priority) :
	NumberedTask(priority, 10000),
	queue(queue)
{
	this->product = product;
	this->numberOfProducts = numberOfProducts;
}

void Producer::main()
{
	while(numberOfProducts-- > 0)
	{
		queue.addLast(product);

		#if defined(PRINT)
			std::cout << "Task" << taskNumber << ": Producing " << product << '\n';
		#endif
	}
}


//------------------------------------------------------------------------------------------------
// * class Consumer
//------------------------------------------------------------------------------------------------

class Consumer : public NumberedTask
{
public:
	// constructor
	Consumer(
		Int numberOfProducts, IntertaskValueQueue<Int> &queue,
		UInt priority = defaultPriority);

protected:
	// main entry point
	void main();

private:
	// representation
	Int numberOfProducts;
	IntertaskValueQueue<Int> &queue;
	static UInt consumerCount;
	static Mutex consumerCountMutex; 
};

Consumer::Consumer(Int numberOfProducts, IntertaskValueQueue<Int> &queue, UInt priority) :
	NumberedTask(priority, 10000),
	queue(queue)
{
	this->numberOfProducts = numberOfProducts;

	LockedSection lockedSection(consumerCountMutex); 
	++consumerCount;
}

void Consumer::main()
{
	while(numberOfProducts-- > 0)
	{
		Int product = queue.removeFirst();

		#if defined(PRINT)
			std::cout << "Task" << taskNumber << ": Consuming " << product << '\n';
		#else
			++product;
		#endif
	}

	#if defined(PRINT)
		std::cout << "Task" << taskNumber << ": Exitting" "\n";
	#endif

	LockedSection lockedSection(consumerCountMutex); 
	if(--consumerCount == 0)
	{
		#if defined(PRINT)
			exit(0);
		#endif
	}
}

UInt Consumer::consumerCount = 0;
Mutex Consumer::consumerCountMutex; 


//------------------------------------------------------------------------------------------------
// * rtosTest
//------------------------------------------------------------------------------------------------

void rtosTest()
{
	const UInt numberOfTask1s = 5;
	const UInt numberOfProducers = 10;
	const UInt numberOfConsumers = 3;
	const UInt productQueueCapacity = 4;

	// create a bunch of tasks
	{
		for(UInt taskNumber = 0; taskNumber < numberOfTask1s; ++taskNumber)
		{
			(new Task1())->resume();
		}
	}

	IntertaskValueQueue<Int> *pProductQueue = new IntertaskValueQueue<Int>(productQueueCapacity);
	UInt numberOfProducts = 0;

	// create a bunch of producers
	{
		for(UInt taskNumber = 0; taskNumber < numberOfProducers; ++taskNumber)
		{
			numberOfProducts += taskNumber + 1;
			(new Producer(taskNumber + 1, taskNumber + 1, *pProductQueue))->resume();
		}
	}

	// create a bunch of consumers
	{
		UInt numberOfProductsPerConsumer = numberOfProducts / numberOfConsumers;
		for(UInt taskNumber = 0; taskNumber < numberOfConsumers - 1; ++taskNumber)
		{
			numberOfProducts -= numberOfProductsPerConsumer;
			(new Consumer(numberOfProductsPerConsumer, *pProductQueue))->resume();
		}
		(new Consumer(numberOfProducts, *pProductQueue))->resume();
	}

	// start the RTOS
	TaskScheduler::getCurrentTaskScheduler()->start();

	// we will never get here
}
