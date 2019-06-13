#ifndef _pointerArithmetic_h_
#define _pointerArithmetic_h_

#include "cPrimitiveTypes.h"

//------------------------------------------------------------------------------------------------
// * addToPointer
//
// Returns <aPointer> + <anInteger> bytes.
//------------------------------------------------------------------------------------------------

template<class TPointer, class TInteger>
inline TPointer *addToPointer(const TPointer *aPointer, TInteger anInteger)
{
	return (TPointer *)((UInt8 *)aPointer + anInteger);
}

//------------------------------------------------------------------------------------------------
// * subtractFromPointer
//
// Returns <aPointer> - <anInteger> bytes.
//------------------------------------------------------------------------------------------------

template<class TPointer, class TInteger>
inline TPointer *subtractFromPointer(const TPointer *aPointer, TInteger anInteger)
{
	return (TPointer *)((UInt8 *)aPointer - anInteger);
}

//------------------------------------------------------------------------------------------------
// * subtractPointers
//
// Returns the the number of bytes between <aPointer> and <anotherPointer>.
//------------------------------------------------------------------------------------------------

inline SInt subtractPointers(const void *aPointer, const void *anotherPointer)
{
	return (SInt)aPointer - (SInt)anotherPointer;
}

//------------------------------------------------------------------------------------------------
// * offsetToMember
//
// Returns the offset in bytes to a member of a class or structure.
//------------------------------------------------------------------------------------------------

#define offsetToMember(Type, member) ((SInt)&(((Type *)null)->member))

#endif // _pointerArithmetic_h_
