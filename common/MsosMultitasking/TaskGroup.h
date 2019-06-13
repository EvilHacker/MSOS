#ifndef _TaskGroup_h_
#define _TaskGroup_h_

#include "../cPrimitiveTypes.h"
#include "../Collections/LinkedList.h"
class Task;

//------------------------------------------------------------------------------------------------
// * class TaskGroup
//
// Keeps a list of task sorted in order of priority.
//------------------------------------------------------------------------------------------------

class TaskGroup : protected LinkedList
{
public:
	// constructors and destructors
	inline TaskGroup();
	virtual ~TaskGroup();

	// testing
	inline Bool isEmpty() const;

	// querying
	inline Task *getFirstTask() const;

	// modifying task priority queue
	inline void addTask(Task *pTask);
	inline void removeTask(Task *pTask);
	inline Task *removeFirstTask();

	// controlling tasks
	virtual void suspendTask(Task *pTask);
	virtual void resumeTask(Task *pTask);
	virtual void yieldTask(Task *pTask);

private:
	// sort function
	static SInt compareTasks(const Link *pTask1, const Link *pTask2);
};

#include "Task.h"

//------------------------------------------------------------------------------------------------
// * TaskGroup::TaskGroup
//
// Constructor.
//------------------------------------------------------------------------------------------------

inline TaskGroup::TaskGroup()
{
}

//------------------------------------------------------------------------------------------------
// * TaskGroup::isEmpty
//
// Tests to see if there are no Tasks in the queue.
//------------------------------------------------------------------------------------------------

inline Bool TaskGroup::isEmpty() const
{
	return LinkedList::isEmpty();
}

//------------------------------------------------------------------------------------------------
// * TaskGroup::getFirstTask
//
// Returns the highest priority task in the queue.
//------------------------------------------------------------------------------------------------

inline Task *TaskGroup::getFirstTask() const
{
	return (Task *)LinkedList::getFirst();
}

//------------------------------------------------------------------------------------------------
// * TaskGroup::addTask
//
// Adds the specified <pTask> to the queue in priority order.
// The <pTask> will be added after any tasks of equal priority.
//------------------------------------------------------------------------------------------------

inline void TaskGroup::addTask(Task *pTask)
{
	LinkedList::addSorted(pTask, &TaskGroup::compareTasks);
}

//------------------------------------------------------------------------------------------------
// * TaskGroup::removeTask
//
// Removes the specified <pTask> from the queue.
//------------------------------------------------------------------------------------------------

inline void TaskGroup::removeTask(Task *pTask)
{
	LinkedList::remove(pTask);
}

//------------------------------------------------------------------------------------------------
// * TaskGroup::removeFirstTask
//
// Removes the highest priority task from the queue.
//------------------------------------------------------------------------------------------------

inline Task *TaskGroup::removeFirstTask()
{
	return (Task *)LinkedList::removeFirst();
}

#endif // _TaskGroup_h_
