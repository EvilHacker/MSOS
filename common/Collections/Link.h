#ifndef _Link_h_
#define _Link_h_

//------------------------------------------------------------------------------------------------
// * class Link
//
// A Link keeps track of previous and next Links.
// It is intended to be used by a double-linked-list data structure.
//------------------------------------------------------------------------------------------------

class Link
{
public:
	// accessing
	inline Link *getPrevious() const;
	inline Link *getNext() const;
	inline void setPrevious(Link *pLink);
	inline void setNext(Link *pLink);

private:
	// representation
	Link *pPrevious;
	Link *pNext;
};

//------------------------------------------------------------------------------------------------
// * Link::getPrevious
//
// Accessor.
//------------------------------------------------------------------------------------------------

inline Link *Link::getPrevious() const
{
	return pPrevious;
}

//------------------------------------------------------------------------------------------------
// * Link::getNext
//
// Accessor.
//------------------------------------------------------------------------------------------------

inline Link *Link::getNext() const
{
	return pNext;
}

//------------------------------------------------------------------------------------------------
// * Link::setPrevious
//
// Accessor.
//------------------------------------------------------------------------------------------------

inline void Link::setPrevious(Link *pLink)
{
	pPrevious = pLink;
}

//------------------------------------------------------------------------------------------------
// * Link::setNext
//
// Accessor.
//------------------------------------------------------------------------------------------------

inline void Link::setNext(Link *pLink)
{
	pNext = pLink;
}

#endif // _Link_h_
