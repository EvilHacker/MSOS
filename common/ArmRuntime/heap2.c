/*
 * heap2.c
 * Copyright: 1997 Advanced RISC Machines Limited. All rights reserved.
 */

/*
 * RCS $Revision: 1.2 $
 * Checkin $Date: 2001/04/16 20:14:43Z $
 * Revising $Author: marks $
 */

/* Code for a type (2) large block heap

The Heap2 algorithm has in common:
* The heap fragment structure:
        Alloced                 Free
        size                    size
        <user>                  link
                                        <unused>
* Free blocks maintained in an unordered singly linked list.
* Blocks are added to the free list when freed
* Free blocks are merged together when an alloc fails
* The provided blocks are maintained in a structure of their own.
        This means that provided blocks *DO NOT* merge with each other.
* Provided blocks must be:
        A multiple of 4 bytes big
        At least 8 bytes big
        Not starting at address 0, not ending at address 0 and not containing address 0
* Allocation is done with three options:
        first fit
        best fit (default)
        worst fit
* Trimming allocation is trim bottom on big blocks and trim top on little blocks.
        The boundary between these is 512 bytes.

*/
#ifndef _HEAP2_H
#  include "heap2.h"
#endif

#define HEAP2_BIGBLOCKSIZE 512

/* block match types are:
0 - first fit
1 - smallest fit
2 - biggest fit
*/
#define HEAP2_MATCHTYPE 1

#ifndef PARANOID
#  define PARANOID 0
#endif

#ifdef extend_c

/* <Heap2_Initialise>
 * Initialise the type 2 heap
 * 04-06-1997 JSR Written
 */
extern void __Heap2_Initialise(
    Heap2_Descriptor *h,
    int (*full)(void *, size_t), void *fullparam,
    void (*broken)(void *), void *brokenparam)
{
    h->full = full;
    h->fullparam = fullparam;
    h->broken = broken;
    h->brokenparam = brokenparam;
    h->providedchain = 0;
    h->freechain = 0;
}


/* <Heap2_ProvideMemory>
 * Provide memory to the heap
 * 04-06-1997 JSR Written
 */
extern void __Heap2_ProvideMemory(Heap2_Descriptor *h, void *_blk, size_t size)
{
    Heap2_ProvidedBlock *pblk = (Heap2_ProvidedBlock *)_blk;
    Heap2_FreeBlock *blk;

    /* Add provided block to provided block chain */
    pblk->size = size;
    pblk->next = h->providedchain;
    h->providedchain = pblk;

    /* Add spare memory to free chain */
    blk = (Heap2_FreeBlock *)(pblk+1);
    blk->size = (size - sizeof(*pblk)) | Heap2_FreeMark;
    blk->next = h->freechain;
    h->freechain = blk;
}


#endif

/* <Heap2_SizeAdjust>
 * Adjust the block size to be the whole size of of the required allocation block
 * 04-06-1997 JSR Written
 */
static inline size_t Heap2_SizeAdjust(size_t size)
{
    /* Add allocated block overhead and round up to a multiple of 4 bytes */
    size = (size + sizeof( size_t ) + 3) & ~3;
    if (size < sizeof(Heap2_FreeBlock))
    {
        size = sizeof(Heap2_FreeBlock);
    }

    return size;
}


/* <Heap2_TrimBlockTop>
 * Trim the given (optionally free marked) block to the given size by
 * cutting off the block's top
 * 04-06-1997 JSR Written
 */
static inline void Heap2_TrimBlockTop(Heap2_Descriptor *h, Heap2_FreeBlock *blk, size_t size)
{
    Heap2_FreeBlock *fragment;

    if (blk->size - size > sizeof(Heap2_FreeBlock))
    {
        /* Trim off end */
        fragment = (Heap2_FreeBlock *)((char *)blk + size);
        fragment->size = (blk->size - size) | Heap2_FreeMark;
        blk->size = size;
        fragment->next = h->freechain;
        h->freechain = fragment;
    }
}


/* <Heap2_TrimBlockBottom>
 * Trim the given block to the given size by cutting off the block's bottom.
 * Block's new address is returned. Block is assumed to eventually be an allocated block.
 * 04-06-1997 JSR Written
 */
static inline Heap2_FreeBlock *Heap2_TrimBlockBottom(Heap2_Descriptor *h, Heap2_FreeBlock *blk, size_t size)
{
    Heap2_FreeBlock *fragment;

    if (blk->size - size > sizeof(Heap2_FreeBlock))
    {
        /* Trim start off */
        fragment = (Heap2_FreeBlock *)((char *)blk + blk->size - size);
        fragment->size = size;
        blk->size = (blk->size - size) | Heap2_FreeMark;
        blk->next = h->freechain;
        h->freechain = blk;
        return fragment;
    }

    return blk;
}


/* <Heap2_TrimBlock>
 * Trim a block to the given size, trimming top or bottom depending on
 * whether the the block is 'big' or 'small'.
 * 04-06-1997 JSR Written
 */
static inline Heap2_FreeBlock *Heap2_TrimBlock(Heap2_Descriptor *h, Heap2_FreeBlock *blk, size_t size)
{
    if (size >= HEAP2_BIGBLOCKSIZE)
    {
        return Heap2_TrimBlockBottom(h, blk, size);
    }
    else
    {
        Heap2_TrimBlockTop(h, blk, size);
        return blk;
    }
}

#ifdef alloc_c

/* <Heap2_FindMatch>
 * Find match in free chain
 * 04-06-1997 JSR Written
 */
static Heap2_FreeBlock *Heap2_FindMatch(Heap2_Descriptor *h, size_t size)
{
#if HEAP2_MATCHTYPE==0
    Heap2_FreeBlock **prev;
    Heap2_FreeBlock *rover;

    /* Hunt for first match */
    for ( prev = &h->freechain, rover = *prev;
          rover != NULL;
          prev = &rover->next, rover = *prev )
    {
        if (rover->size >= size)
        {
            *prev = rover->next;
            rover->size &= ~Heap2_FreeMark;

            return Heap2_TrimBlock(h, rover, size);
        }
    }
#elif HEAP2_MATCHTYPE==1
    Heap2_FreeBlock **prev;
    Heap2_FreeBlock *rover;
    Heap2_FreeBlock **bestprev;
    size_t bestsize = 0;

    /* Hunt for first match */
    bestprev = NULL;
    for ( prev = &h->freechain, rover = *prev;
          rover != NULL;
          prev = &rover->next, rover = *prev )
    {
        if (rover->size >= size)
        {
            /* Block large enough for job */
            if (bestprev == NULL  || rover->size < bestsize)
            {
                bestprev = prev;
                bestsize = rover->size;
            }
        }
    }

    if ( bestprev != NULL )
    {
        rover = *bestprev;
        *bestprev = rover->next;
        rover->size &= ~Heap2_FreeMark;

        return Heap2_TrimBlock( h, rover, size );
    }
#elif HEAP2_MATCHTYPE==2
    Heap2_FreeBlock **prev;
    Heap2_FreeBlock *rover;
    Heap2_FreeBlock **bestprev;
    size_t bestsize = 0;

    /* Hunt for first match */
    bestprev = NULL;
    for ( prev = &h->freechain, rover = *prev;
          rover != NULL;
          prev = &rover->next, rover = *prev )
    {
        if ( rover->size >= size )
        {
            /* Block large enough for job */
            if ( bestprev == NULL || rover->size > bestsize )
            {
                bestprev = prev;
                bestsize = rover->size;
            }
        }
    }

    if ( bestprev != NULL )
    {
        rover = *bestprev;
        *bestprev = rover->next;
        rover->size &= ~Heap2_FreeMark;

        return Heap2_TrimBlock( h, rover, size );
    }
#endif

    return NULL;
}


/* <Heap2_MergeFreeBlocks>
 * Do a merge process on the type 2 heap
 * 04-06-1997 JSR Written
 */
static void Heap2_MergeFreeBlocks(Heap2_Descriptor *h)
{
    Heap2_FreeBlock *prev = NULL;
    Heap2_FreeBlock *prevend;
    Heap2_FreeBlock *rover;
    Heap2_ProvidedBlock *pblk;
    Heap2_FreeBlock *pend;

    h->freechain = NULL;
    for ( pblk = h->providedchain; pblk != NULL; pblk = pblk->next )
    {
        prevend = NULL;
        pend = (Heap2_FreeBlock *)((char *)pblk + pblk->size);
        for ( rover = (Heap2_FreeBlock *)(pblk+1);
              rover != pend;
              rover = (Heap2_FreeBlock *)((char *)rover + (rover->size & ~Heap2_FreeMark)) )
        {
            if ( rover->size & Heap2_FreeMark )
            {
                /* Its a free block */
                if ( rover == prevend )
                {
                    /* Enlarge prev with rover */
                    prev->size += rover->size & ~Heap2_FreeMark;
                }
                else
                {
                    /* New free block */
                    prev = rover;
                    prevend = (Heap2_FreeBlock *)((char *)rover + (rover->size & ~Heap2_FreeMark));
                    rover->next = h->freechain;
                    h->freechain = rover;
                }
            }
            else
            {
                /* Its an allocated block */
                prevend = NULL;
            }
        }
    }
}


/* <Heap2_Alloc>
 * Allocate some memory from the type 2 heap
 * 04-06-1997 JSR Written
 */
extern void *__Heap2_Alloc(Heap2_Descriptor *h, size_t size)
{
    Heap2_FreeBlock *rover;

    size = Heap2_SizeAdjust(size);

    /* Loop around until allocated, or full function returns 0 to indicate no
    extra memory released to heap */
    do
    {
        /* Hunt for match */
        rover = Heap2_FindMatch(h, size);
        if (rover != NULL)
        {
            return &rover->next;
        }

        /* First match failed - do a merge sequence */
        Heap2_MergeFreeBlocks(h);

        /* Hunt for match again */
        rover = Heap2_FindMatch(h, size);
        if ( rover != NULL )
        {
            return &rover->next;
        }
    } while (h->full != NULL && h->full(h->fullparam, size + sizeof(Heap2_ProvidedBlock)));

    return 0;
}

#endif

#ifdef free_c

/* <Heap2_Free>
 * Free some memory back to the heap - block is simply added
 * to the free list's beginning and marked free
 * 04-06-1997 JSR Written
 */
extern void __Heap2_Free(Heap2_Descriptor *h, void *_blk)
{
    Heap2_FreeBlock *blk = (Heap2_FreeBlock *)((char *)_blk - sizeof( size_t ));

    blk->size |= Heap2_FreeMark;
    blk->next = h->freechain;
    h->freechain = blk;
}

#endif

#ifdef realloc_c

/* <Heap2_Realloc>
 * Re-allocate the given block, ensuring the newly allocated block has the same
 * min(newsize,oldsize) bytes at its start
 * 03-06-1997 JSR Written
 */
extern void *__Heap2_Realloc(Heap2_Descriptor *h, void *_blk, size_t size)
{
    Heap2_FreeBlock *blk = (Heap2_FreeBlock *)((char *)_blk - sizeof( size_t ));
    Heap2_FreeBlock **prev;
    Heap2_FreeBlock *rover;
    size_t requirement;
    Heap2_FreeBlock *fragment;
    size_t fragmentsize;
    size_t originalsize;
    Heap2_ProvidedBlock *pblk;
    Heap2_FreeBlock *pblkend = NULL;

    size = Heap2_SizeAdjust(size);

    if (size <= blk->size)
    {
        /* No change or shrink */
        Heap2_TrimBlockTop( h, blk, size );

        return &blk->next;
    }

    /* Enlarge */

    /* Find which provided block blk is in */
    for (pblk = h->providedchain; pblk != NULL; pblk = pblk->next)
    {
        pblkend = (Heap2_FreeBlock *)((char *)pblk + pblk->size);
        if ((char *)pblk <= (char *)blk && blk < pblkend)
        {
            break;
        }
    }

#if PARANOID
    /* Check if realloc block is inside a provided block */
    if (pblk == NULL)
    {
        h->broken(h->brokenparam);
        return 0;
    }
#endif

    /* Check if there's sufficient free blocks after blk to enlarge it */
    originalsize = blk->size;
    requirement = size - originalsize;
    for ( fragment = (Heap2_FreeBlock *)((char *)blk + (blk->size & ~Heap2_FreeMark));
          fragment != pblkend && ((fragmentsize = fragment->size) & Heap2_FreeMark);
          fragment = (Heap2_FreeBlock *)((char *)fragment + fragmentsize) )
    {
        /* Add fragment to end of blk... */
        for ( prev = &h->freechain, rover = *prev;
              rover != NULL && rover != fragment;
              prev = &rover->next, rover = *prev )
        {
        }

#if PARANOID
        /* Check for inconsistent case */
        if (rover == NULL)
        {
            h->broken(h->brokenparam);
            return 0;
        }
#endif

        *prev = fragment->next;
        fragmentsize &= ~Heap2_FreeMark;
        blk->size += fragmentsize;

        if (requirement <= fragmentsize)
        {
            Heap2_TrimBlockTop(h, blk, size);

            /* All done */
            return &blk->next;
        }

        requirement -= fragmentsize;
    }

    /* Failed to stretch to full length, so trim off any added bits */
    Heap2_TrimBlockTop(h, blk, originalsize);

    /* No free block of sufficient size follows blk, so do an alloc,copy,free sequence */
    rover = (Heap2_FreeBlock *)__Heap2_Alloc(h, size - sizeof(size_t));
    if (rover != NULL)
    {
        memcpy(rover, &blk->next, blk->size - sizeof(size_t));
        __Heap2_Free(h, &blk->next);
    }

    return rover;
}

#endif

#ifdef stats_c

/* <Heap2_Stats>
 * Print stats on the given heap
 * 04-06-1997 JSR Written
 */
extern void *__Heap2_Stats(int (*dprint)(char const *format, ...), Heap2_Descriptor *h)
{
    Heap2_FreeBlock *rover;
    size_t totsize;
    int numblocks;
    int sizecount[32];
    int i;
    size_t size;
    char *hwm;

    hwm = 0;
    totsize = 0;
    numblocks = 0;

    for (i = 0; i < 32; i++)
    {
        sizecount[i] = 0;
    }

    for (rover = h->freechain; rover != NULL; rover = rover->next)
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

    dprint("%d bytes in %d free blocks (avge size %d)\n",
           totsize, numblocks, totsize/(numblocks==0?1:numblocks) );
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

/* End of file heap2.c */
