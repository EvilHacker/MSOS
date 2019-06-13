#include "../cPrimitiveTypes.h"
#include "LinkedList.h"

//------------------------------------------------------------------------------------------------
// * LinkedList::addBetween
//
// Adds <pLink> between <pLinkBefore> and <pLinkAfter> which are already in the list.
// If <pLinkBefore> and/or <pLinkAfter> may be null if the insertion point is at the
// beginning and/or at the end of the list.
//------------------------------------------------------------------------------------------------

void LinkedList::addBetween(Link *pLink, Link *pLinkBefore, Link *pLinkAfter)
{
	// insert this link into the list
	pLink->setPrevious(pLinkBefore);
	pLink->setNext(pLinkAfter);
	if(pLinkBefore != null)
	{
		pLinkBefore->setNext(pLink);
	}
	else
	{
		pFirstLink = pLink;
	}
	if(pLinkAfter != null)
	{
		pLinkAfter->setPrevious(pLink);
	}
	else
	{
		pLastLink = pLink;
	}
}

//------------------------------------------------------------------------------------------------
// * LinkedList::addSorted
//
// Adds <pLink> into the list in sorted order.
// The <compare> function returns -, 0, + if the arguments are <, =, > each other.
//------------------------------------------------------------------------------------------------

void LinkedList::addSorted(Link *pLink, SInt (*compare)(const Link *, const Link *))
{
	Link *pLinkBefore = null;
	Link *pLinkAfter = pFirstLink;

	// find the position where this link is to be inserted in the list
	while((pLinkAfter != null) && (compare(pLink, pLinkAfter) > 0))
	{
		pLinkBefore = pLinkAfter;
		pLinkAfter = pLinkAfter->getNext();
	}

	// insert this link into the list
	addBetween(pLink, pLinkBefore, pLinkAfter);
}

//------------------------------------------------------------------------------------------------
// * LinkedList::addSorted
//
// Adds <pLink> into the list in sorted order.
// The <comparator>.compare function returns -, 0, + if the arguments are <, =, > each other.
//------------------------------------------------------------------------------------------------

void LinkedList::addSorted(Link *pLink, Comparator &comparator)
{
	Link *pLinkBefore = null;
	Link *pLinkAfter = pFirstLink;

	// find the position where this link is to be inserted in the list
	while((pLinkAfter != null) && (comparator.compare(pLink, pLinkAfter) > 0))
	{
		pLinkBefore = pLinkAfter;
		pLinkAfter = pLinkAfter->getNext();
	}

	// insert this link into the list
	addBetween(pLink, pLinkBefore, pLinkAfter);
}

//------------------------------------------------------------------------------------------------
// * LinkedList::remove
//
// Removes a link from the list.
// The <pLink> must be a part of this list.
//------------------------------------------------------------------------------------------------

void LinkedList::remove(Link *pLink)
{
	// unlink from the list
	if(pLink->getPrevious() != null)
	{
		pLink->getPrevious()->setNext(pLink->getNext());
	}
	else
	{
		pFirstLink = pLink->getNext();
	}
	if(pLink->getNext() != null)
	{
		pLink->getNext()->setPrevious(pLink->getPrevious());
	}
	else
	{
		pLastLink = pLink->getPrevious();
	}

	// null the unused links just for safety
	//pLink->setPrevious(null);
	//pLink->setNext(null);
}
