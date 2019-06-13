#ifndef _IntertaskQueue_h_
#define _IntertaskQueue_h_

#include "Mutex.h"
#include "Semaphore.h"
#include "LockedSection.h"
#include "TimeValue.h"

//------------------------------------------------------------------------------------------------
// * class IntertaskQueue
//
// This class provides a mechanism for synchronizing tasks.
// This class maintains a fixed sized queue of <Element>s.
// Any task may add or remove elements from such a queue.
// If a task attempts to remove an element when the queue is empty,
// the task will be blocked until an element is added to the queue.
// If a task attempts to add an element when the queue is full,
// the task will be blocked until an element is removed from the queue.
// This is an abstract class, instanciate IntertaskPointerQueue or IntertaskValueQueue instead.
//------------------------------------------------------------------------------------------------

template<class Element>
class IntertaskQueue
{
public:
	// destructor
	virtual ~IntertaskQueue();

	// testing
	inline Bool isFull() const;
	inline Bool isEmpty() const;

	// accessing
	inline UInt getSize() const;
	inline UInt getCapacity() const;
	inline Mutex &getMutex();

	// low-level modifying
	inline Bool reserveElementToAddFirst(TimeValue timeout = infiniteTime);
	inline Bool reserveElementToAddLast(TimeValue timeout = infiniteTime);
	inline Bool reserveElementToRemoveFirst(TimeValue timeout = infiniteTime);
	inline Bool reserveElementToRemoveLast(TimeValue timeout = infiniteTime);

protected:
	// constructor
	IntertaskQueue(UInt capacity);

	// modifying
	Bool basicAddFirst(Element item, TimeValue timeout = infiniteTime);
	Bool basicAddLast(Element item, TimeValue timeout = infiniteTime);
	Bool basicRemoveFirst(Element *pItem, TimeValue timeout = infiniteTime);
	Bool basicRemoveLast(Element *pItem, TimeValue timeout = infiniteTime);

	// low-level modifying
	inline Bool basicReserveElementToAdd(TimeValue timeout = infiniteTime);
	inline Bool basicReserveElementToRemove(TimeValue timeout = infiniteTime);
	void basicAddReservedElementFirst(Element item);
	void basicAddReservedElementLast(Element item);
	Element basicRemoveReservedElementFirst();
	Element basicRemoveReservedElementLast();

private:
	// representation
	UInt capacity;
	Element *pSlots;
	SInt firstIndex;
	SInt lastIndex;
	Mutex slotMutex;
	Semaphore usedSlotSemaphore;
	Semaphore freeSlotSemaphore;
};

//------------------------------------------------------------------------------------------------
// * IntertaskQueue::IntertaskQueue
//
// Constructs an IntertaskQueue capable of holding at most <capacity> elements.
//------------------------------------------------------------------------------------------------

template<class Element>
IntertaskQueue<Element>::IntertaskQueue(UInt capacity) :
	capacity(capacity),
	usedSlotSemaphore(0),
	freeSlotSemaphore(capacity)
{
	pSlots = new Element[capacity];
	firstIndex = 0;
	lastIndex = 0;
}

//------------------------------------------------------------------------------------------------
// * IntertaskQueue::~IntertaskQueue
//
// Destructor.
//------------------------------------------------------------------------------------------------

template<class Element>
IntertaskQueue<Element>::~IntertaskQueue()
{
	delete pSlots;
}

//------------------------------------------------------------------------------------------------
// * IntertaskQueue::isFull
//
// Tests whether there is no free space in the queue.
//------------------------------------------------------------------------------------------------

template<class Element>
inline Bool IntertaskQueue<Element>::isFull() const
{
	return freeSlotSemaphore.getExcessSignalCount() == 0;
}

//------------------------------------------------------------------------------------------------
// * IntertaskQueue::isEmpty
//
// Tests whether there are no elements in the queue.
//------------------------------------------------------------------------------------------------

template<class Element>
inline Bool IntertaskQueue<Element>::isEmpty() const
{
	return usedSlotSemaphore.getExcessSignalCount() == 0;
}

//------------------------------------------------------------------------------------------------
// * IntertaskQueue::getSize
//
// Returns the number of items in the queue.
//------------------------------------------------------------------------------------------------

template<class Element>
inline UInt IntertaskQueue<Element>::getSize() const
{
	return usedSlotSemaphore.getExcessSignalCount();
}

//------------------------------------------------------------------------------------------------
// * IntertaskQueue::getCapacity
//
// Returns the maximum number of items that can simultaneously be held in the queue.
//------------------------------------------------------------------------------------------------

template<class Element>
inline UInt IntertaskQueue<Element>::getCapacity() const
{
	return capacity;
}

//------------------------------------------------------------------------------------------------
// * IntertaskQueue::getMutex
//
// Returns a mutex that is used to lock access to this queue.
// This can be usefull if you wish to perform several actions on the queue atomically.
//------------------------------------------------------------------------------------------------

template<class Element>
inline Mutex &IntertaskQueue<Element>::getMutex()
{
	return slotMutex;
}

//------------------------------------------------------------------------------------------------
// * IntertaskQueue::reserveElementToAddFirst
//
// Reserves space for an element in the queue to be added later.
//------------------------------------------------------------------------------------------------

template<class Element>
inline Bool IntertaskQueue<Element>::reserveElementToAddFirst(TimeValue timeout)
{
	return basicReserveElementToAdd(timeout);
}

//------------------------------------------------------------------------------------------------
// * IntertaskQueue::reserveElementToAddLast
//
// Reserves space for an element in the queue to be added later.
//------------------------------------------------------------------------------------------------

template<class Element>
inline Bool IntertaskQueue<Element>::reserveElementToAddLast(TimeValue timeout)
{
	return basicReserveElementToAdd(timeout);
}

//------------------------------------------------------------------------------------------------
// * IntertaskQueue::reserveElementToRemoveFirst
//
// Reserves an element in the queue to be removed later.
//------------------------------------------------------------------------------------------------

template<class Element>
inline Bool IntertaskQueue<Element>::reserveElementToRemoveFirst(TimeValue timeout)
{
	return basicReserveElementToRemove(timeout);
}

//------------------------------------------------------------------------------------------------
// * IntertaskQueue::reserveElementToRemoveLast
//
// Reserves an element in the queue to be removed later.
//------------------------------------------------------------------------------------------------

template<class Element>
inline Bool IntertaskQueue<Element>::reserveElementToRemoveLast(TimeValue timeout)
{
	return basicReserveElementToRemove(timeout);
}

//------------------------------------------------------------------------------------------------
// * IntertaskQueue::basicAddFirst
//
// Adds an element to the queue.
// If the queue is full, the current task will block until another task removes an element.
// Returns true if a timeout occurred, false if the <item> has been added.
//------------------------------------------------------------------------------------------------

template<class Element>
Bool IntertaskQueue<Element>::basicAddFirst(Element item, TimeValue timeout)
{
	// wait for a free slot in the queue
	if(basicReserveElementToAdd(timeout))
	{
		// timeout
		return true;
	}

	// add the item to the queue
	basicAddReservedElementFirst(item);

	// item added
	return false;
}

//------------------------------------------------------------------------------------------------
// * IntertaskQueue::basicAddLast
//
// Adds an element to the queue.
// If the queue is full, the current task will block until another task removes an element.
// Returns true if a timeout occurred, false if the <item> has been added.
//------------------------------------------------------------------------------------------------

template<class Element>
Bool IntertaskQueue<Element>::basicAddLast(Element item, TimeValue timeout)
{
	// wait for a free slot in the queue
	if(basicReserveElementToAdd(timeout))
	{
		// timeout
		return true;
	}

	// add the item to the queue
	basicAddReservedElementLast(item);

	// item added
	return false;
}

//------------------------------------------------------------------------------------------------
// * IntertaskQueue::basicRemoveFirst
//
// Removes an element from the queue.  <pItem> will point to the element removed.
// If the queue is empty, the current task will block until another task adds an element.
// Returns true if a timeout occurred, false if an item has been removed.
//------------------------------------------------------------------------------------------------

template<class Element>
Bool IntertaskQueue<Element>::basicRemoveFirst(Element *pItem, TimeValue timeout)
{
	// wait for a used slot in the queue
	if(basicReserveElementToRemove(timeout))
	{
		// timeout
		return true;
	}

	// remove an item from the queue
	*pItem = basicRemoveReservedElementFirst();

	// item removed
	return false;
}

//------------------------------------------------------------------------------------------------
// * IntertaskQueue::basicRemoveLast
//
// Removes an element from the queue.  <pItem> will point to the element removed.
// If the queue is empty, the current task will block until another task adds an element.
// Returns true if a timeout occurred, false if an item has been removed.
//------------------------------------------------------------------------------------------------

template<class Element>
Bool IntertaskQueue<Element>::basicRemoveLast(Element *pItem, TimeValue timeout)
{
	// wait for a used slot in the queue
	if(basicReserveElementToRemove(timeout))
	{
		// timeout
		return true;
	}

	// remove an item from the queue
	*pItem = basicRemoveReservedElementLast();

	// item removed
	return false;
}

//------------------------------------------------------------------------------------------------
// * IntertaskQueue::basicReserveElementToAdd
//
// Reserves space for an element in the queue to be added later.
//------------------------------------------------------------------------------------------------

template<class Element>
inline Bool IntertaskQueue<Element>::basicReserveElementToAdd(TimeValue timeout)
{
	return freeSlotSemaphore.wait(timeout);
}

//------------------------------------------------------------------------------------------------
// * IntertaskQueue::basicReserveElementToRemove
//
// Reserves an element in the queue to be removed later.
//------------------------------------------------------------------------------------------------

template<class Element>
inline Bool IntertaskQueue<Element>::basicReserveElementToRemove(TimeValue timeout)
{
	return usedSlotSemaphore.wait(timeout);
}

//------------------------------------------------------------------------------------------------
// * IntertaskQueue::basicAddReservedElementFirst
//
// Adds an element that has previously been reserved to the queue.
//------------------------------------------------------------------------------------------------

template<class Element>
void IntertaskQueue<Element>::basicAddReservedElementFirst(Element item)
{
	// add the item to the queue
	LockedSection slotLock(slotMutex);
	if(--firstIndex < 0)
	{
		firstIndex = capacity - 1;
	}
	pSlots[firstIndex] = item;

	// signal that there is one more item in the queue
	usedSlotSemaphore.signal();
}

//------------------------------------------------------------------------------------------------
// * IntertaskQueue::basicAddReservedElementLast
//
// Adds an element that has previously been reserved to the queue.
//------------------------------------------------------------------------------------------------

template<class Element>
void IntertaskQueue<Element>::basicAddReservedElementLast(Element item)
{
	// add the item to the queue
	LockedSection slotLock(slotMutex);
	pSlots[lastIndex] = item;
	if(++lastIndex >= (SInt)capacity)
	{
		lastIndex = 0;
	}

	// signal that there is one more item in the queue
	usedSlotSemaphore.signal();
}

//------------------------------------------------------------------------------------------------
// * IntertaskQueue::basicRemoveReservedElementFirst
//
// Removes an element that has previously been reserved from the queue.
//------------------------------------------------------------------------------------------------

template<class Element>
Element IntertaskQueue<Element>::basicRemoveReservedElementFirst()
{
	// remove an item from the queue
	LockedSection slotLock(slotMutex);
	Element element = pSlots[firstIndex];
	if(++firstIndex >= (SInt)capacity)
	{
		firstIndex = 0;
	}

	// signal that there is one more free slot in the queue
	freeSlotSemaphore.signal();

	return element;
}

//------------------------------------------------------------------------------------------------
// * IntertaskQueue::basicRemoveReservedElementLast
//
// Removes an element that has previously been reserved from the queue.
//------------------------------------------------------------------------------------------------

template<class Element>
Element IntertaskQueue<Element>::basicRemoveReservedElementLast()
{
	// remove an item from the queue
	LockedSection slotLock(slotMutex);
	if(--lastIndex < 0)
	{
		lastIndex = capacity - 1;
	}

	// signal that there is one more free slot in the queue
	freeSlotSemaphore.signal();

	return pSlots[lastIndex];
}




//------------------------------------------------------------------------------------------------
// * class IntertaskPointerQueue
//
// This class provides the same functionality as it's superclass IntertaskQueue except that
// the <Element> must be pointers of a specific type.
//------------------------------------------------------------------------------------------------

template<class Element>
class IntertaskPointerQueue : public IntertaskQueue<void *>
{
public:
	// constructor
	inline IntertaskPointerQueue(UInt capacity);

	// modifying
	inline Bool addFirst(Element *pItem, TimeValue timeout = infiniteTime);
	inline Bool addLast(Element *pItem, TimeValue timeout = infiniteTime);
	inline Bool removeFirst(Element **ppItem, TimeValue timeout = infiniteTime);
	inline Bool removeLast(Element **ppItem, TimeValue timeout = infiniteTime);
	inline Element *removeFirst();
	inline Element *removeLast();

	// low-level modifying
	inline void addReservedElementFirst(Element *pItem);
	inline void addReservedElementLast(Element *pItem);
	inline Element *removeReservedElementFirst();
	inline Element *removeReservedElementLast();
};

//------------------------------------------------------------------------------------------------
// * IntertaskPointerQueue::IntertaskPointerQueue
//
// Constructs an IntertaskPointerQueue capable of holding at most <capacity> elements.
//------------------------------------------------------------------------------------------------

template<class Element>
inline IntertaskPointerQueue<Element>::IntertaskPointerQueue(UInt capacity) :
	IntertaskQueue<void *>(capacity)
{
}

//------------------------------------------------------------------------------------------------
// * IntertaskPointerQueue::addFirst
//
// Adds an element to the queue, possibly blocking the current task until
// another tasks removes an element from the queue if it is full.
// Returns true if a timeout occurred, false if the <item> has been added.
//------------------------------------------------------------------------------------------------

template<class Element>
inline Bool IntertaskPointerQueue<Element>::addFirst(Element *pItem, TimeValue timeout)
{
	return basicAddFirst((void *)pItem, timeout);
}

//------------------------------------------------------------------------------------------------
// * IntertaskPointerQueue::addLast
//
// Adds an element to the queue, possibly blocking the current task until
// another tasks removes an element from the queue if it is full.
// Returns true if a timeout occurred, false if the <item> has been added.
//------------------------------------------------------------------------------------------------

template<class Element>
inline Bool IntertaskPointerQueue<Element>::addLast(Element *pItem, TimeValue timeout)
{
	return basicAddLast((void *)pItem, timeout);
}

//------------------------------------------------------------------------------------------------
// * IntertaskPointerQueue::removeFirst
//
// Removes an element from the queue, possibly blocking the current task until
// another tasks adds an element to the queue if it is empty.
// Returns true if a timeout occurred, false if an item has been removed.
//------------------------------------------------------------------------------------------------

template<class Element>
inline Bool IntertaskPointerQueue<Element>::removeFirst(Element **ppItem, TimeValue timeout)
{
	return basicRemoveFirst((void **)ppItem, timeout);
}

//------------------------------------------------------------------------------------------------
// * IntertaskPointerQueue::removeLast
//
// Removes an element from the queue, possibly blocking the current task until
// another tasks adds an element to the queue if it is empty.
// Returns true if a timeout occurred, false if an item has been removed.
//------------------------------------------------------------------------------------------------

template<class Element>
inline Bool IntertaskPointerQueue<Element>::removeLast(Element **ppItem, TimeValue timeout)
{
	return basicRemoveLast((void **)ppItem, null, timeout);
}

//------------------------------------------------------------------------------------------------
// * IntertaskPointerQueue::removeFirst
//
// Removes an element from the queue, possibly blocking the current task until
// another tasks adds an element to the queue if it is empty.
//------------------------------------------------------------------------------------------------

template<class Element>
inline Element *IntertaskPointerQueue<Element>::removeFirst()
{
	Element *pItem;
	basicRemoveFirst((void **)&pItem);
	return pItem;
}

//------------------------------------------------------------------------------------------------
// * IntertaskPointerQueue::removeLast
//
// Removes an element from the queue, possibly blocking the current task until
// another tasks adds an element to the queue if it is empty.
//------------------------------------------------------------------------------------------------

template<class Element>
inline Element *IntertaskPointerQueue<Element>::removeLast()
{
	Element *pItem;
	basicRemoveLast((void **)&pItem);
	return pItem;
}

//------------------------------------------------------------------------------------------------
// * IntertaskPointerQueue::addReservedElementFirst
//
// Adds an element that has previously been reserved to the queue.
//------------------------------------------------------------------------------------------------

template<class Element>
inline void IntertaskPointerQueue<Element>::addReservedElementFirst(Element *pItem)
{
	basicAddReservedElementFirst((void *)pItem);
}

//------------------------------------------------------------------------------------------------
// * IntertaskPointerQueue::addReservedElementLast
//
// Adds an element that has previously been reserved to the queue.
//------------------------------------------------------------------------------------------------

template<class Element>
inline void IntertaskPointerQueue<Element>::addReservedElementLast(Element *pItem)
{
	basicAddReservedElementLast((void *)pItem);
}

//------------------------------------------------------------------------------------------------
// * IntertaskPointerQueue::removeReservedElementFirst
//
// Removes an element that has previously been reserved from the queue.
//------------------------------------------------------------------------------------------------

template<class Element>
inline Element *IntertaskPointerQueue<Element>::removeReservedElementFirst()
{
	return (Element *)basicRemoveReservedElementFirst();
}

//------------------------------------------------------------------------------------------------
// * IntertaskPointerQueue::removeReservedElementLast
//
// Removes an element that has previously been reserved from the queue.
//------------------------------------------------------------------------------------------------

template<class Element>
inline Element *IntertaskPointerQueue<Element>::removeReservedElementLast()
{
	return (Element *)basicRemoveReservedElementLast();
}




//------------------------------------------------------------------------------------------------
// * class IntertaskValueQueue
//
// This class provides the same functionality as it's superclass IntertaskQueue except that
// the <Element> must be pointers of a specific type.
//------------------------------------------------------------------------------------------------

template<class Element>
class IntertaskValueQueue : public IntertaskQueue<Element>
{
public:
	// constructor
	inline IntertaskValueQueue(UInt capacity);

	// modifying
	inline Bool addFirst(Element item, TimeValue timeout = infiniteTime);
	inline Bool addLast(Element item, TimeValue timeout = infiniteTime);
	inline Bool removeFirst(Element *pItem, TimeValue timeout = infiniteTime);
	inline Bool removeLast(Element *pItem, TimeValue timeout = infiniteTime);
	inline Element removeFirst();
	inline Element removeLast();

	// low-level modifying
	inline void addReservedElementFirst(Element item);
	inline void addReservedElementLast(Element item);
	inline Element removeReservedElementFirst();
	inline Element removeReservedElementLast();
};

//------------------------------------------------------------------------------------------------
// * IntertaskValueQueue::IntertaskValueQueue
//
// Constructs an IntertaskValueQueue capable of holding at most <capacity> elements.
//------------------------------------------------------------------------------------------------

template<class Element>
inline IntertaskValueQueue<Element>::IntertaskValueQueue(UInt capacity) :
	IntertaskQueue<Element>(capacity)
{
}

//------------------------------------------------------------------------------------------------
// * IntertaskValueQueue::addFirst
//
// Adds an element to the queue, possibly blocking the current task until
// another tasks removes an element from the queue if it is full.
//------------------------------------------------------------------------------------------------

template<class Element>
inline Bool IntertaskValueQueue<Element>::addFirst(Element item, TimeValue timeout)
{
	return basicAddFirst(item, timeout);
}

//------------------------------------------------------------------------------------------------
// * IntertaskValueQueue::addLast
//
// Adds an element to the queue, possibly blocking the current task until
// another tasks removes an element from the queue if it is full.
//------------------------------------------------------------------------------------------------

template<class Element>
inline Bool IntertaskValueQueue<Element>::addLast(Element item, TimeValue timeout)
{
	return this->basicAddLast(item, timeout);
}

//------------------------------------------------------------------------------------------------
// * IntertaskValueQueue::removeFirst
//
// Removes an element from the queue, possibly blocking the current task until
// another tasks adds an element to the queue if it is empty.
// Returns true if a timeout occurred, false if an item has been removed.
//------------------------------------------------------------------------------------------------

template<class Element>
inline Bool IntertaskValueQueue<Element>::removeFirst(Element *pItem, TimeValue timeout)
{
	return basicRemoveFirst(pItem, timeout);
}

//------------------------------------------------------------------------------------------------
// * IntertaskValueQueue::removeLast
//
// Removes an element from the queue, possibly blocking the current task until
// another tasks adds an element to the queue if it is empty.
// Returns true if a timeout occurred, false if an item has been removed.
//------------------------------------------------------------------------------------------------

template<class Element>
inline Bool IntertaskValueQueue<Element>::removeLast(Element *pItem, TimeValue timeout)
{
	return basicRemoveLast(pItem, timeout);
}

//------------------------------------------------------------------------------------------------
// * IntertaskValueQueue::removeFirst
//
// Removes an element from the queue, possibly blocking the current task until
// another tasks adds an element to the queue if it is empty.
//------------------------------------------------------------------------------------------------

template<class Element>
inline Element IntertaskValueQueue<Element>::removeFirst()
{
	Element item;
	this->basicRemoveFirst(&item);
	return item;
}

//------------------------------------------------------------------------------------------------
// * IntertaskValueQueue::removeLast
//
// Removes an element from the queue, possibly blocking the current task until
// another tasks adds an element to the queue if it is empty.
//------------------------------------------------------------------------------------------------

template<class Element>
inline Element IntertaskValueQueue<Element>::removeLast()
{
	Element item;
	basicRemoveLast(&item);
	return item;
}

//------------------------------------------------------------------------------------------------
// * IntertaskValueQueue::addReservedElementFirst
//
// Adds an element that has previously been reserved to the queue.
//------------------------------------------------------------------------------------------------

template<class Element>
inline void IntertaskValueQueue<Element>::addReservedElementFirst(Element item)
{
	basicAddReservedElementFirst(item);
}

//------------------------------------------------------------------------------------------------
// * IntertaskValueQueue::addReservedElementLast
//
// Adds an element that has previously been reserved to the queue.
//------------------------------------------------------------------------------------------------

template<class Element>
inline void IntertaskValueQueue<Element>::addReservedElementLast(Element item)
{
	basicAddReservedElementLast(item);
}

//------------------------------------------------------------------------------------------------
// * IntertaskValueQueue::removeReservedElementFirst
//
// Removes an element that has previously been reserved from the queue.
//------------------------------------------------------------------------------------------------

template<class Element>
inline Element IntertaskValueQueue<Element>::removeReservedElementFirst()
{
	return IntertaskQueue<Element>::basicRemoveReservedElementFirst();
}

//------------------------------------------------------------------------------------------------
// * IntertaskValueQueue::removeReservedElementLast
//
// Removes an element that has previously been reserved from the queue.
//------------------------------------------------------------------------------------------------

template<class Element>
inline Element IntertaskValueQueue<Element>::removeReservedElementLast()
{
	return IntertaskQueue<Element>::basicRemoveReservedElementLast();
}

#endif // _IntertaskQueue_h_
