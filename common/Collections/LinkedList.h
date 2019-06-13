#ifndef _LinkedList_h_
#define _LinkedList_h_

#include "../cPrimitiveTypes.h"
#include "Link.h"

//------------------------------------------------------------------------------------------------
// * class LinkedList
//
// Double linked list data type.
//------------------------------------------------------------------------------------------------

class LinkedList
{
public:
	// types
	class Comparator
	{
	public:
		virtual SInt compare(const Link *, const Link *) = 0;
	};

	// constructor
	inline LinkedList();

	// testing
	inline Bool isEmpty() const;

	// accessing
	inline Link *getFirst() const;
	inline Link *getLast() const;

	// modifying
	inline void add(Link *pLink);
	inline void addFirst(Link *pLink);
	inline void addLast(Link *pLink);
	inline void addBefore(Link *pLink, Link *pLinkAfter);
	inline void addAfter(Link *pLink, Link *pLinkBefore);
	void addSorted(Link *pLink, SInt (*compare)(const Link *, const Link *));
	void addSorted(Link *pLink, Comparator &comparator);
	void remove(Link *pLink);
	inline Link *removeFirst();
	inline Link *removeLast();

private:
	// modifying
	void addBetween(Link *pLink, Link *pLinkBefore, Link *pLinkAfter);

private:
	// representation
	Link *pFirstLink;
	Link *pLastLink;
};

//------------------------------------------------------------------------------------------------
// * LinkedList::LinkedList
//
// Constructs an empty list.
//------------------------------------------------------------------------------------------------

inline LinkedList::LinkedList()
{
	pFirstLink = pLastLink = null;
}

//------------------------------------------------------------------------------------------------
// * LinkedList::isEmpty
//
// Tests to see if there are no elements in the list.
//------------------------------------------------------------------------------------------------

inline Bool LinkedList::isEmpty() const
{
	return pFirstLink == null;
}

//------------------------------------------------------------------------------------------------
// * LinkedList::getFirst
//
// Returns the first link in the list or null if the list is empty.
//------------------------------------------------------------------------------------------------

inline Link *LinkedList::getFirst() const
{
	return pFirstLink;
}

//------------------------------------------------------------------------------------------------
// * LinkedList::getFirst
//
// Returns the last link in the list or null if the list is empty.
//------------------------------------------------------------------------------------------------

inline Link *LinkedList::getLast() const
{
	return pLastLink;
}

//------------------------------------------------------------------------------------------------
// * LinkedList::add
//
// Adds <pLink> to the end of the list.
//------------------------------------------------------------------------------------------------

inline void LinkedList::add(Link *pLink)
{
	addLast(pLink);
}

//------------------------------------------------------------------------------------------------
// * LinkedList::addFirst
//
// Adds <pLink> to the beginning of the list.
//------------------------------------------------------------------------------------------------

inline void LinkedList::addFirst(Link *pLink)
{
	addBefore(pLink, pFirstLink);
}

//------------------------------------------------------------------------------------------------
// * LinkedList::addLast
//
// Adds <pLink> to the end of the list.
//------------------------------------------------------------------------------------------------

inline void LinkedList::addLast(Link *pLink)
{
	addAfter(pLink, pLastLink);
}

//------------------------------------------------------------------------------------------------
// * LinkedList::addBefore
//
// Adds <pLink> before <pLinkAfter> which is already in the list.
// If <pLinkAfter> is null then <pLink> is added to the beginning of the list.
//------------------------------------------------------------------------------------------------

inline void LinkedList::addBefore(Link *pLink, Link *pLinkAfter)
{
	addBetween(pLink, pLinkAfter == null ? pFirstLink : pLinkAfter->getPrevious(), pLinkAfter);
}

//------------------------------------------------------------------------------------------------
// * LinkedList::addAfter
//
// Adds <pLink> after <pLinkBefore> which is already in the list.
// If <pLinkBefore> is null then <pLink> is added to the end of the list.
//------------------------------------------------------------------------------------------------

inline void LinkedList::addAfter(Link *pLink, Link *pLinkBefore)
{
	addBetween(pLink, pLinkBefore, pLinkBefore == null ? pLastLink : pLinkBefore->getNext());
}

//------------------------------------------------------------------------------------------------
// * LinkedList::removeFirst
//
// Removes the first link in the list and returns it.
//------------------------------------------------------------------------------------------------

inline Link *LinkedList::removeFirst()
{
	Link *pLink = getFirst();
	remove(pLink);
	return pLink;
}

//------------------------------------------------------------------------------------------------
// * LinkedList::removeLast
//
// Removes the first link in the list and returns it.
//------------------------------------------------------------------------------------------------

inline Link *LinkedList::removeLast()
{
	Link *pLink = getLast();
	remove(pLink);
	return pLink;
}

#endif // _LinkedList_h_
