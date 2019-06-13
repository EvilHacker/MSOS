#ifndef _MemberTask_h_
#define _MemberTask_h_

#include "Task.h"

//------------------------------------------------------------------------------------------------
// * class MemberTask
//
// Used to define a member task within a class which invokes a single member function.
//------------------------------------------------------------------------------------------------

#define MemberTask(TaskName, OuterClass, memberFunction) \
class TaskName; \
friend class TaskName; \
class TaskName : public Task \
{ \
public: \
	inline TaskName(OuterClass &owner, UInt priority, UInt stackSize) : \
		Task(priority, stackSize), \
		pOwner(&owner) \
	{ \
	}; \
	inline TaskName(OuterClass *pOwner, UInt priority, UInt stackSize) : \
		Task(priority, stackSize), \
		pOwner(pOwner) \
	{ \
	}; \
\
private:\
	inline void TaskName::main() \
	{ \
			pOwner->memberFunction(); \
	}; \
\
	OuterClass *pOwner; \
}

//------------------------------------------------------------------------------------------------
// * class MemberTaskWithParameter
//
// Used to define a member task within a class which invokes a single member function.
//------------------------------------------------------------------------------------------------

#define MemberTaskWithParameter(TaskName, OuterClass, memberFunction, ParameterType) \
class TaskName; \
friend class TaskName; \
class TaskName : public Task \
{ \
public: \
	inline TaskName(OuterClass &owner, ParameterType parameter, UInt priority, UInt stackSize) : \
		Task(priority, stackSize), \
		pOwner(&owner), \
		parameter(parameter) \
	{ \
	}; \
	inline TaskName(OuterClass *pOwner, ParameterType parameter, UInt priority, UInt stackSize) : \
		Task(priority, stackSize), \
		pOwner(pOwner), \
		parameter(parameter) \
	{ \
	}; \
\
private:\
	inline void TaskName::main() \
	{ \
			pOwner->memberFunction(parameter); \
	}; \
\
	OuterClass *pOwner; \
	ParameterType parameter; \
}

/*
This templated implementation does not work will all compilers.

//------------------------------------------------------------------------------------------------
// * class MemberTask
//
// Used to define a member task within a class which invokes a single member function.
//------------------------------------------------------------------------------------------------

template<class OuterClass, void (OuterClass::*memberFunction)()>
class MemberTask : public Task
{
public:
	// constructors
	inline MemberTask(OuterClass &owner, UInt priority, UInt stackSize);
	inline MemberTask(OuterClass *pOwner, UInt priority, UInt stackSize);

private:
	// task main function
	inline void main();

	// representation
	OuterClass *pOwner;
};

//------------------------------------------------------------------------------------------------
// * MemberTask::MemberTask
//
// Constructor.
//------------------------------------------------------------------------------------------------

template<class OuterClass, void (OuterClass::*memberFunction)()>
inline MemberTask::MemberTask(OuterClass &owner, UInt priority, UInt stackSize) :
	Task(priority, stackSize),
	pOwner(&owner)
{
}

//------------------------------------------------------------------------------------------------
// * MemberTask::MemberTask
//
// Constructor.
//------------------------------------------------------------------------------------------------

template<class OuterClass, void (OuterClass::*memberFunction)()>
inline MemberTask::MemberTask(OuterClass *pOwner, UInt priority, UInt stackSize) :
	Task(priority, stackSize),
	pOwner(pOwner)
{
}

//------------------------------------------------------------------------------------------------
// * MemberTask::main
//
// Invokes a single member function of the outer class.
//------------------------------------------------------------------------------------------------

template<class OuterClass, void (OuterClass::*memberFunction)()>
inline void MemberTask::main()
{
	(pOwner->*memberFunction)();
}

*/

#endif // _MemberTask_h_
