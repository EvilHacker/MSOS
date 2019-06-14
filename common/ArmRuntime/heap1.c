/*
 * heap1.c
 * Copyright: 1997 Advanced RISC Machines Limited. All rights reserved.
 */

/*
 * RCS $Revision: 1.3 $
 * Checkin $Date: 2001/04/16 20:14:42Z $
 * Revising $Author: marks $
 */

 /* Code for a type (1) large block heaps

The Heap1 algorithms have in common:
* The heap fragment structure:
        Alloced                 Free
        size                    size
        <user>                  link
                                        <unused>
* Free blocks maintained in an address-ordered singly linked list.
* Blocks are merged into the free list when freed (contiguous blocks
        are merged at this stage).
* The provided blocks are not maintained in a structure of their own -
        their memory is added into the heap by being freed.
* Provided blocks must be:
        A multiple of 4 bytes big
        At least 8 bytes big
        Not starting at address 0, not ending at address 0 and not containing address 0
* Allocation is either first fit, next fit, or best fit.

*/

/* block match types are:
0 - first fit
1 - next fit
2 - best fit
*/

#include "heap1.h"

#ifndef HEAP1_MATCHTYPE
#   define HEAP1_MATCHTYPE 0
#endif

#if HEAP1_MATCHTYPE == 1

#define Heap1_LastPrevCheck(h,merged,alternate) \
        if ((h)->lastprev == (merged)) \
        { \
                (h)->lastprev = (alternate); \
        }
#define Heap1_PrevSet(h,newval) \
        (h)->lastprev = (newval);

#else

#define Heap1_LastPrevCheck(h,merged,alternate)
#define Heap1_PrevSet(h,newval)

#endif


/* <Heap1_SizeAdjust>
 * Adjust the block size to be the whole size of of the required allocation block
 * 03-06-1997 JSR Written
 */
static inline size_t Heap1_SizeAdjust(size_t size)
{
    /* Add allocated block overhead and round up to a multiple of 4 bytes */
    size = (size + sizeof(size_t) + 3) & ~3;
    if (size < sizeof(Heap1_FreeBlock))
    {
        size = sizeof(Heap1_FreeBlock);
    }

    return size;
}


#if defined alloc_c || defined SHARED_C_LIBRARY

  /* <Heap1_Initialise>
 * Initialise the heap
 */

extern void __Heap1_Initialise(
    Heap1_Descriptor *h,
    int (*full)(void *, size_t), void *fullparam,
    void (*broken)(void *), void *brokenparam)
{
    h->full = full;
    h->fullparam = fullparam;
    h->broken = broken;
    h->brokenparam = brokenparam;
    h->freechain.size = 0;  /* Guarantees won't merge with following block */
    h->freechain.next = 0;  /* end of list */
    h->lastprev = &h->freechain; /* only for next fit */
}


/* <Heap1_Alloc>
 * Allocate some memory from the heap
 */

#if HEAP1_MATCHTYPE == 1    /* First fit */

extern void *__Heap1_Alloc(Heap1_Descriptor *h, size_t size)
{
    Heap1_FreeBlock *prev;
    Heap1_FreeBlock *rover;
    Heap1_FreeBlock *fragment;

    size = Heap1_SizeAdjust(size);

    /* Loop around until allocated, or full function returns 0 to indicate no
    extra memory released to heap */
    do
    {
        for (prev = &h->freechain, rover = prev->next;
             rover != NULL;
             prev = rover, rover = prev->next)
        {
            if (rover->size >= size)
            {
                /* Block large enough for job */
                if (rover->size >= size + sizeof(Heap1_FreeBlock))
                {
                    /* Block large enough to split */
                    fragment = (Heap1_FreeBlock *)((char *)rover + size);
                    fragment->next = rover->next;
                    fragment->size = rover->size - size;
                    prev->next = fragment;
                    rover->size = size;
                }
                else
                {
                    /* Block too small */
                    prev->next = rover->next;
                }

                return &rover->next;
            }
        }
    } while (h->full != NULL && h->full(h->fullparam, size));

    return NULL;
}

#elif HEAP1_MATCHTYPE == 2  /* looping first fit */

extern void *__Heap1_Alloc(Heap1_Descriptor *h, size_t size)
{
    /* Add allocated block overhead and round up to a multiple of 4 bytes */
    size = Heap1_SizeAdjust(size);

    /* Loop around until allocated, or full function returns 0 to indicate no
    extra memory released to heap */
    do
    {
        Heap1_FreeBlock *prev = h->lastprev;
        Heap1_FreeBlock *rover = prev->next;
        do
        {
            if (rover != NULL)
            {
                if (rover->size >= size)
                {
                    /* Block large enough for job */

                    /* Check if split, or use as-is */
                    if (rover->size >= size + sizeof(Heap1_FreeBlock))
                    {
                        /* Block large enough to split */
                        Heap1_FreeBlock *fragment = (Heap1_FreeBlock *)((char *)rover + size);
                        fragment->size = rover->size - size;
                        fragment->next = rover->next;
                        prev->next = fragment;
                        rover->size = size;
                    }
                    else
                    {
                        /* Block too small */
                        prev->next = rover->next;
                    }
                    h->lastprev = prev;

                    return &rover->next;
                }

                prev = rover;
            }
            else
            {
                prev = &h->freechain;
            }
            rover = prev->next;
        } while (prev != h->lastprev);
    } while (h->full != NULL && h->full(h->fullparam, size));

    return 0;
}

#else   /* best fit */

extern void *__Heap1_Alloc(Heap1_Descriptor *h, size_t size)
{
    Heap1_FreeBlock *prev;
    Heap1_FreeBlock *rover;
    Heap1_FreeBlock *fragment;
    Heap1_FreeBlock *bestprev;
    size_t bestsize = 0;

    size = Heap1_SizeAdjust(size);

    /* Loop around until allocated, or full function returns 0 to indicate no
    extra memory released to heap */
    bestprev = NULL;
    do
    {
        for (prev = &h->freechain, rover = prev->next;
             rover != NULL;
             prev = rover, rover = prev->next)
        {
            if (rover->size >= size)
            {
                /* Block large enough for job */
                if (bestprev == NULL || rover->size < bestsize)
                {
                    bestprev = prev;
                    bestsize = rover->size;
                }
            }
        }

        if (bestprev != NULL)
        {
            /* Found a good match */
            rover = bestprev->next;

            /* Block large enough for job */
            if (rover->size >= size + sizeof(Heap1_FreeBlock))
            {
                /* Block large enough to split */
                fragment = (Heap1_FreeBlock *)((char *)rover + size);
                fragment->size = rover->size - size;
                fragment->next = rover->next;
                bestprev->next = fragment;
                rover->size = size;
            }
            else
            {
                /* Block too small */
                bestprev->next = rover->next;
            }

            return &rover->next;
        }
    } while (h->full != NULL && h->full(h->fullparam, size));

    return 0;
}

#endif

#endif


#if defined extend_c || defined SHARED_C_LIBRARY

/* <Heap1_ProvideMemory>
 * Provide memory to the heap
 * 02-06-1997 JSR Written
 */
extern void __Heap1_InitMemory(Heap1_Descriptor *h, void *_blk, size_t size)
{
    Heap1_FreeBlock *blk = (Heap1_FreeBlock *)_blk;

    /* Turn block into a Heap1 block and free it */
    blk->size = size;
    h->freechain.next = blk;
    blk->next = NULL;
}


/* <Heap1_ProvideMemory>
 * Provide memory to the heap
 * 02-06-1997 JSR Written
 */
extern void __Heap1_ProvideMemory(Heap1_Descriptor *h, void *_blk, size_t size)
{
    Heap1_FreeBlock *blk = (Heap1_FreeBlock *)_blk;

    /* Turn block into a Heap1 block and free it */
    blk->size = size;
    __Heap1_Free(h, &blk->next);
}

#endif


#if defined free_c || defined SHARED_C_LIBRARY
  
/* <Heap1_Free>
 * Free some memory back to the heap
 * 03-06-1997 JSR Written
 */
extern void __Heap1_Free(Heap1_Descriptor *h, void *_blk)
{
    Heap1_FreeBlock *blk = (Heap1_FreeBlock *)((char *)_blk - sizeof(size_t));
    Heap1_FreeBlock *prev;
    Heap1_FreeBlock *rover;

    /* Find the blocks before and after blk */
    for (prev = &h->freechain, rover = prev->next;
         rover != NULL && rover < blk;
         prev = rover, rover = prev->next)
        continue;

    /* Check for merging with the block before blk */
    if ((char *)prev + prev->size == (char *)blk)
    {   /* Merge blk into prev */
        prev->size += blk->size;
        blk = prev;
    }
    else
    {   /* Point prev at blk */
        prev->next = blk;
    }

    /* Check for merging with the block after blk */
    if ((char *)blk + blk->size == (char *)rover)
    {   /* Merge rover into blk */

        /* Check if front of lastprev is being merged into rover */
        Heap1_LastPrevCheck(h, rover, blk)

        blk->next = rover->next;
        blk->size += rover->size;
    }
    else
    {
        /* Chain rover after blk */
        blk->next = rover;
    }
}

#endif

#if defined realloc_c || defined SHARED_C_LIBRARY

/* <Heap1_Realloc>
 * Re-allocate the given block, ensuring the newly allocated block has the same
 * min(newsize,oldsize) bytes at its start
 * 03-06-1997 JSR Written
 */
void *__Heap1_Realloc(Heap1_Descriptor *h, void *_blk, size_t size)
{
    Heap1_FreeBlock *blk = (Heap1_FreeBlock *)((char *)_blk - sizeof(size_t));
    Heap1_FreeBlock *prev;
    Heap1_FreeBlock *rover;
    Heap1_FreeBlock *fragment;

    size = Heap1_SizeAdjust(size);

    if (size <= blk->size)
    {
        /* No change or shrink */
        if (blk->size - size >= sizeof(Heap1_FreeBlock))
        {
            /* Split */
            fragment = (Heap1_FreeBlock *)((char *)blk + size);
            fragment->size = blk->size - size;
            blk->size = size;
            __Heap1_Free(h, &fragment->next);
        }

        return &blk->next;
    }

    /* Enlarge */

    /* Find the blocks before and after blk */
    for (prev = &h->freechain, rover = prev->next;
         rover != NULL && rover < blk;
         prev = rover, rover = prev->next)
        continue;

    if ((char *)blk + blk->size == (char *)rover && blk->size + rover->size >= size)
    {
        /* A free block of sufficient size follows blk */

        if (blk->size + rover->size - size >= sizeof(Heap1_FreeBlock))
        {
            /* Split */
            fragment = (Heap1_FreeBlock *)((char *)blk + size);
            fragment->next = rover->next;
            fragment->size = blk->size + rover->size - size;
            blk->size = size;
            prev->next = fragment;
        }
        else
        {
            /* Engulf */
            blk->size += rover->size;
            prev->next = rover->next;
        }
        Heap1_PrevSet(h,prev)

        return &blk->next;
    }

    /* No free block of sufficient size follows blk, so do an alloc,copy,free sequence */
    rover = (Heap1_FreeBlock *)__Heap1_Alloc(h, size - sizeof(size_t));
    if (rover != NULL)
    {
        memcpy(rover, &blk->next, blk->size - sizeof(size_t));
        __Heap1_Free(h, &blk->next);
    }

    return rover;
}

#endif

#if defined stats_c || defined SHARED_C_LIBRARY
  
/* <Heap1_Stats>
 * Print stats on the given heap
 * 03-06-1997 JSR Written
 */
void *__Heap1_Stats(int (*dprint)(char const *format, ...), Heap1_Descriptor *h)
{
    Heap1_FreeBlock *rover;
    int sizecount[32];
    int i;
    size_t size;
    char *hwm = NULL;
    size_t totsize = 0;
    int numblocks = 0;

    for (i = 0; i < 32; i++)
    {
        sizecount[i] = 0;
    }

    for (rover = h->freechain.next; rover != NULL; rover = rover->next)
    {
        if ((char *)rover > hwm)
        {
            hwm = (char *)rover;
        }
        numblocks += 1;
        size = rover->size;
        totsize += size;
        for (i = 0; i < 32; i++)
        {
            size >>= 1;
            if (size == 0)
            {
                break;
            }
        }
        sizecount[i]++;
    }

    dprint("%d bytes in %d free blocks (avge size %d)\n", totsize, numblocks, totsize/(numblocks==0?1:numblocks));
    for (i = 0; i < 32; i++)
    {
        if (sizecount[i] != 0)
        {
            dprint("%d blocks 2^%d+1 to 2^%d\n", sizecount[i], i-1, i);
        }
    }

    return hwm;
}

#endif

/* End of file heap1.c */
