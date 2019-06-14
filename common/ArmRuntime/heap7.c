/*
 * RCS $Revision: 1.2 $
 * Checkin $Date: 2001/04/16 20:14:47Z $
 * Revising $Author: marks $
 */

/* ***** IMPORTANT ** IMPORTANT ** IMPORTANT ** IMPORTANT ** IMPORTANT *****
 * The #defines which control a large part of this source file are decribed
 * in the header file.
 */

/*
 * NOTES:
 *  Non-implemented (possible) functionality is described under ASSUMPTIONS
 *   and marked with a '!'.
 *  Heap extensions inside the current heap (in a previous heap hole) has not
 *   been tested, but the code is there.
 *  A certain percentage (FRACTION_OF_HEAP_NEEDED_FREE) of the heap is always
 *   kept free, this is a bit wasteful but the number of coalesces and garbage
 *   collections goes down as this percentage rises. It has been found by
 *   experimentation that this fraction should be approximately between 1/8 and
 *   1/4 (currently at 1/6). Large blocks are allocated from the start of the
 *   overflow list ie the low memory addresses and small and medium sized
 *   blocks are allocated from the end of the overflow list. For this reason
 *   the overflow list is a doubly linked list with a head at both ends. A
 *   pointer to the last free block on the heap is also kept so that when the
 *   heap is extended and the old bitmap is returned to the free list (and
 *   merged with any adjacent free block), the last heap block, if it is free,
 *   can be merged with it also.
 * ASSUMPTIONS:
 *  Address units are in bytes.
 *  There are an exact number of address units (bytes) per word.
 *  All target machines are either word aligned or run slower with non word
 *   alignment (so word aligning is a good and right thing to do).
 *  The heap can not grow downwards (all heap extensions must be above the
 *   heap base determined by the first block claimed from OSStorage), and if
 *   two consecutive (in time) blocks are doled out by OSStorage they are only
 *   assumed to be contiguous if the lower limit (arithmetically) of the second
 *   block is equal to the higher limit (arithmetically) of the first block
 *   plus one.
 *  Blocks may be doled out in unspecified address order (but note that
 *   every time a heap extension, which is inside my heap bounds, is given out
 *   the heap has to be scanned in order to find and modify the heap hole in
 *   which the extension has been given.
!*  The range of address units to be found in a single bin can only be the
 *   number of address units in a word, extra code will have to be written to
 *   manage bin ranges other than this size (more trouble than its worth, if
 *   its worth anything at all).
 *  MAXCARD is the largest number representable in a word (ie all bits set).
 * ALLOCATE:
 *  An array of lists of free blocks of similar sizes (bins) is kept so that
 *   when an ALLOCATE of size n is requested the list starting at array entry
 *   n DIV BINRANGE will automatically have as the first element of the list
 *   a block of the correct size (plus the OVERHEADWORDS) or no block at all
 *   (or the block requested may be too big to be in the allocate bins). if
 *   there is no block available in the bin, then bins containing lists of
 *   larger blocks are checked and the block allocated from one of these (if
 *   the bin block is big enough, it is split). if there is still no block
 *   available then the overflow list is checked and if available, the block
 *   is cut from here (the block required is cut from the end of the larger
 *   block if the size required is not large (size < LARGEBLOCK) otherwise it
 *   is taken from the start of the large block). if the remainder of the block
 *   is greater than the largest bin block then it remains in the overflow
 *   list, otherwise it is removed to the correct bin. if the overflow list
 *   does not have a block large enough then the heap is either extended (more
 *   memory claimed from OSStorage), coalesced or garbage collected, depending
 *   on the state of the heap etc and whether garbage collection is enabled.
 *   After coalescing or garbage collection the allocate algorithm is executed
 *   again in order to allocate the block.
 * COALESCE:
 *  if the overflow list does not contain a block large enough and a
 *   reasonable amount of storage has been deallocated since the last coalesce,
 *   (reasonable is difficult to define and is only deducable by
 *   experimentation) then all allocatable blocks (by storage) and all blocks
 *   on the overflow deallocate list are marked free, the heap is scanned and
 *   the blocks scattered into bins and overflow list in increasing address
 *   order.
 * DEALLOCATE:
 *  When a block is DEALLOCATED, if it will fit in a bin then it is put at
 *   the start of the relevant bin list otherwise it is conceptually released
 *   to the overflow deallocate list (there is no need for a list, set the
 *   block's header bits to indicate it is free and it will automatically be
 *   sucked in at the next coalesce).
 * HEAP EXTENSIONS:
 *  Whenever the heap is extended, a certain amount (if available) is allocated
 *   for the garbage collection bit maps (even if garbage collection has not
 *   been enabled.
 */

#ifndef _HEAP7_H
 #include "heap7.h"
#endif

#define IGNORE(param) param = param

#define FALSE 0
#define TRUE  1

#ifdef alloc_c

/*
 * FRACTION_OF_HEAP_NEEDED_FREE is used when deciding whether to coalesce, GC
 * or extend the heap. An attempt is made to keep this amount free, if it is
 * not free then the heap is extended. The amount of free space is the total of
 * all free blocks (without overheads). if there is a bitmap at the end of the
 * heap, it is not included in the heap size.
 */
#define FRACTION_OF_HEAP_NEEDED_FREE 6
/* initialisation for blocks on allocation */

/* <Heap7_InternalCoalesce>
 * Coalesce free blocks into larger lumps, reconstructing the free lists
 * 24-06-1997 JSR Addapted to Heap_
 */
static void Heap7_InternalCoalesce
(
        Heap7_Descriptor *h
)
{
        Heap7_Block *block;
        Heap7_Block *previous;
        Heap7_Block *tail;
#ifndef BLOCKS_GUARDED
        Heap7_Block *bin_copy[NBINS+2];
#endif
        size_t size;
        /* where size is used to specify an element of an array it should really be
         * called index, but to generate better code I got rid of the index variable
         */

        h->lookInBins = FALSE;
        h->totalFree = 0;
        /* set bins and overflow lists to empty */
        for (size = 0; size <= NBINS+1; size++)
        {
                h->bin[size] = NULL;
#ifndef BLOCKS_GUARDED
                bin_copy[size] = NULL;
#endif
        }

        block = h->heapLow;

        /* NULL indicates previous doesn't point to start of free block */
        previous = NULL;
        tail = NULL;

        while (block <= h->heapHigh)
        {
                if (INVALID(block))
                {
                        h->broken( h->brokenparam );
                }
                if (FREE(block))
                { /* free block */
                        if (previous == NULL)
                        {
                                previous = block;
                        }
                }
                else if (previous != NULL)
                {
                        size = PTRDIFF(block, previous) - OVERHEAD;
                        /* set flags to Free */
                        h->totalFree += size;
                        previous->size = (size | FREEBIT);
                        if (size <= MAXBINSIZE)
                        {
                                /* return to bin */
                                size /= BINRANGE;
                                if (h->bin[size] == NULL)
                                {
                                        h->bin[size] = previous;
                                }
                                else 
                                {
                                        /* if not BLOCKS_GUARDED use guard word of first block in bin to hold
                                         * a pointer to the last block in the list for this bin otherwise
                                         * use the bin_copy array. This allows me to keep the list in
                                         * ascending address order. Remember to put back the guard words at
                                         * the end of coalescing if BLOCKS_GUARDED.
                                         */
#ifdef BLOCKS_GUARDED
                                        ((Heap7_Block *) h->bin[size]->guard)->next = previous;
#else
                                        (bin_copy[size])->next = previous;
#endif
                                }
#ifdef BLOCKS_GUARDED
                                h->bin[size]->guard = (int) previous;
#else
                                bin_copy[size] = previous;
#endif
                        }
                        else
                        {
                                /* put block on overflow list */
                                if ( h->bin[0] == NULL )
                                {
                                        h->bin[0] = previous;
                                        previous->previous = NULL;
                                }
                                else
                                {
                                        tail->next = previous; 
                                        previous->previous = tail;
                                }
                                tail = previous;
                        }
                        previous = NULL;
                }
                ADDBYTESTO(block, SIZE(block) + OVERHEAD);
        }

        /* replace the guard words at the start of the bins lists */
        for (size = 1; size <= NBINS; size++) 
        {
                if (h->bin[size] != NULL)
                {
                        h->lookInBins = TRUE;
#ifdef BLOCKS_GUARDED
                        ((Heap7_Block *) h->bin[size]->guard)->next = NULL;
                        h->bin[size]->guard = GUARDCONSTANT;
#else
                        (bin_copy[size])->next = NULL;
#endif
                }
        }

        /* do both ends of overflow list */
        if (h->bin[0] != NULL)
        {
                tail->next = NULL;
                h->bin[NBINS+1] = tail;
        }
        else
        {
                h->bin[NBINS+1] = NULL;
        }
        h->lastFreeBlockOnHeap = h->bin[NBINS+1];
}


/* <Heap7_InsertBlockInOverflowList>
 * Insert the given new block of heap into the heap
 * 24-06-1997 JSR Addapted to Heap_
 */
static void Heap7_InsertBlockInOverflowList
(
        Heap7_Descriptor *h,
        Heap7_Block *block
)
{
#if HEAP_ALLOCATED_IN_ASCENDING_ADDRESS_ORDER
        /* OK to add remainder of block to tail of overflow list */
        if (h->bin[0] == NULL)
        {
                h->bin[0] = block;
                block->previous = NULL;
        }
        else
        {
                h->bin[NBINS+1]->next = block;
                block->previous = h->bin[NBINS+1];
        }
        h->bin[NBINS+1] = block;
        block->next = NULL;
#else
        Heap7_Block *previous;
        Heap7_Block *tail;
        if (h->bin[0] == NULL)
        {
                /* OK to add remainder of block to tail of overflow list */
                if (h->bin[0] == NULL) 
                {
                        h->bin[0] = block;
                        block->previous = NULL;
                }
                else 
                {
                        h->bin[NBINS+1]->next = block; 
                        block->previous = h->bin[NBINS+1];
                }
                h->bin[NBINS+1] = block; 
                block->next = NULL;
        }
        else
        {
                /* insert remainder block at right position in overflow list */
                tail = h->bin[0];
                while (tail != NULL && tail < block)
                {
                        if (INVALID(tail))
                        {
                                h->broken( h->brokenparam );
                        }
                        previous = tail; 
                        tail = tail->next;
                }
                if (tail == h->bin[0])
                {
                        block->next = h->bin[0]; 
                        block->previous = NULL;
                        h->bin[0]->previous = block; 
                        h->bin[0] = block;
                }
                else 
                {
                        block->next = previous->next; 
                        block->previous = previous;
                        previous->next = block;
                        if (tail == NULL)
                        {
                                h->bin[NBINS+1] = block;
                        }
                        else
                        {
                                tail->previous = block;
                        }
                }
        }
#endif
}




/* <Heap7_ProvideMemory>
 * Provide some memory to the heap
 * 24-06-1997 JSR Addapted to Heap_
 */
extern void __Heap7_ProvideMemory
(
        Heap7_Descriptor *h,
        void *blk,
        size_t size
)
{
        Heap7_Block *base;
#if !HEAP_ALLOCATED_IN_ASCENDING_ADDRESS_ORDER
        Heap7_Block *tempBlock;
#endif
        Heap7_Block *bitmap;

        base = (Heap7_Block *)blk;
        if (base == ADDBYTES(h->endOfLastExtension, HOLEOVERHEAD))
        {
                base = h->endOfLastExtension;
                size += HOLEOVERHEAD;
        }

        if (size <= HOLEOVERHEAD) 
        {
                size = 0; 
                base = NULL;
        }
        else
        {
                size -= HOLEOVERHEAD;
        }

        bitmap = base;
        if (base == h->endOfLastExtension) 
        {
                /* extension contiguous with last block on heap. */
                if (h->lastFreeBlockOnHeap != NULL &&
            ADDBYTES(h->lastFreeBlockOnHeap, SIZE(h->lastFreeBlockOnHeap)+OVERHEAD) == bitmap) 
                {
                        /* so do the merge of the extension and last block on the heap */
                        h->lastFreeBlockOnHeap->size = SIZE(h->lastFreeBlockOnHeap) + OVERHEAD;
                        h->totalFree -= h->lastFreeBlockOnHeap->size;
                        size += h->lastFreeBlockOnHeap->size;

                        bitmap = h->lastFreeBlockOnHeap;
                        if (h->lastFreeBlockOnHeap == h->bin[NBINS+1]) 
                        {
                                /* remove block from end of overflow list */
                                if (h->lastFreeBlockOnHeap->previous == NULL) 
                                {
                                        h->bin[0] = NULL;
                                }
                                else 
                                {
                                        h->lastFreeBlockOnHeap->previous->next = NULL;
                                }
                                h->bin[NBINS+1] = h->lastFreeBlockOnHeap->previous;
                        } /* else it is not in any list ie waiting for coalesce */
                }
        }

        /* SEE WHAT TO DO WITH NEW BLOCK (IF THERE IS ONE) */
        if (size > MAXBINSIZE+OVERHEAD) 
        {
                /* block is big enough to do something with */
                /* HANDLE BEING DROPPED INTO A HEAP HOLE, AND CREATING THE HEAP HOLE
                   MARKER AT THE END OF THE NEW EXTENSION BLOCK. */
                if (base >= h->heapHigh) 
                {
                        if (h->endOfLastExtension != NULL && base != h->endOfLastExtension) 
                        {
                                /* heap hole, mark it as allocated */
                                h->endOfLastExtension->size =
                                        (PTRDIFF(base, h->endOfLastExtension) - HOLEOVERHEAD) | HEAPHOLEBIT;
                        } 
                        h->endOfLastExtension = ADDBYTES(bitmap, size);
#ifdef BLOCKS_GUARDED
                        h->endOfLastExtension->guard = GUARDCONSTANT;
#endif
                        h->endOfLastExtension->size = 0; /* as an end marker for Coalesce */
                }
#if !HEAP_ALLOCATED_IN_ASCENDING_ADDRESS_ORDER
                else 
                { 
                        /* find the heap hole I've been dropped in and modify it */
                        Heap7_Block *holeStart;
                        Heap7_Block *hole;
                        hole = h->heapLow; holeStart = NULL;
                        while (hole <= base) 
                        {
                                if (HEAPHOLE(hole)) 
                                {
                                        holeStart = hole;
                                }
                                ADDBYTESTO(hole, SIZE(hole)+OVERHEAD);
                        }
                        if (holeStart != base)
                        {
                                /* extension is NOT at start of heap hole */
                                holeStart->size = PTRDIFF(base, holeStart) - HOLEOVERHEAD | HEAPHOLEBIT;
                        }
                        else if (ADDBYTES(holeStart, HOLEOVERHEAD) == base) 
                        {
                                base = holeStart;
                                size += HOLEOVERHEAD;
                        }
                        if (ADDBYTES(base, size+HOLEOVERHEAD) == hole)
                        {
                                size += HOLEOVERHEAD;
                        }
                        else 
                        { 
                                /* create a new hole at the end of the extension */
                                tempBlock = ADDBYTES(base ,size);
#ifdef BLOCKS_GUARDED
                                tempBlock->guard = GUARDCONSTANT;
#endif
                                tempBlock->size = (PTRDIFF(hole, tempBlock) - HOLEOVERHEAD) | HEAPHOLEBIT;
                        }
                }
#endif /* EXTENSIONS_IN_HEAP_HOLES */

                /* INITIALISE HEADER OF NEW BLOCK */
                base = bitmap;
                if (base > h->lastFreeBlockOnHeap)
                {
                        h->lastFreeBlockOnHeap = base;
                }
                size -= OVERHEAD;
#ifdef BLOCKS_GUARDED
                base->guard = GUARDCONSTANT;
#endif
                /* set flags to Free */
                base->size = size | FREEBIT;
                h->totalFree += size;
                Heap7_InsertBlockInOverflowList( h, base );
        } 

        /* endOfLastExtension is the address of the storage after the end of the
           block (used to handle heap holes) */
        if (h->endOfLastExtension > h->heapHigh)
        {
                h->heapHigh = h->endOfLastExtension;
        }
        if (base < h->heapLow && base != NULL)
        {
                h->heapLow = base;
        }
        h->totalHeap = PTRDIFF(h->heapHigh, h->heapLow);
        h->userHeap = h->totalHeap;
}


#ifdef BLOCKS_GUARDED
/* <Heap7_CheckHeap>
 * Check the given heap for validity
 * 24-06-1997 JSR Addapted to Heap_
 */
void Heap7_CheckHeap
(
        Heap7_Descriptor *h
)
{
        Heap7_Block *block;

        if (h->userHeap > 0) 
        {
                for (block = h->heapLow; ; ) 
                {
                        if (block >= h->heapHigh) 
                        {
                                if (block > ADDBYTES(h->heapHigh,OVERHEAD))
                                {
                                        h->broken( h->brokenparam );
                                }
                                else
                                {
                                        return;
                                }
                        }
                        if (INVALID(block))
                        {
                                h->broken( h->brokenparam );
                        }
                        ADDBYTESTO(block, SIZE(block)+OVERHEAD);
                }
        }
}
#endif

#define COALESCED     (1U<<31)
#define FORCECOALESCE (1U<<30)


/* <Heap7_Alloc>
 * Allocate some memory from a type 7 heap
 * 24-06-1997 JSR Addapted for Heap_
 */
extern void *__Heap7_Alloc
(
        Heap7_Descriptor *h,
        size_t size
)
{ 
        Heap7_Block *block;
        size_t actualSize;
        register int index;
        int fromHighMemory;
        unsigned status = 0;

        /* round to whole words */
        size = (size + 3) & ~3;
        if (size >= MAXBYTES)
        {
                return 0;
        }
        else if (size == 0)
        {
                return 0;
        }

        index = 0;
        fromHighMemory = ((size <= LARGEBLOCK) && h->sys_heap_top);
        for (;;) 
        {
                if (size <= MAXBINSIZE && h->lookInBins) 
                { 
                        /* get from bin (if not empty) */
                        index = size / BINRANGE;
                        do
                        {
                                block = h->bin[index];
                                if (block != NULL) 
                                { 
                                        /* got a block */
                                        if (INVALID(block))
                                        {
                                                h->broken( h->brokenparam );
                                        }
                                        h->bin[index] = block->next;
                                        actualSize = SIZE(block);
                                        goto got_block;
                                } /* else try other bins */
                        } while (++index <= NBINS);
                }

                /* block bigger than largest bin / bin is empty, check overflow list */
                /* if large block required, take it from high memory otherwise from low */
get_from_overflow:
                if (fromHighMemory) 
                {
                        block = h->bin[NBINS+1]; 
                }
                else 
                {
                        block = h->bin[0]; 
                }

                while (block != NULL) 
                {
                        if (INVALID(block))
                        {
                                h->broken( h->brokenparam );
                        }
                        actualSize = SIZE(block);
                        if (actualSize >= size) 
                        {
                                /* got a block big enough, now see if it needs splitting */
                                if (actualSize-size <= MAXBINSIZE+OVERHEAD) 
                                {
                                        /* remove all of block from overflow list */
                                        if (block == h->lastFreeBlockOnHeap) 
                                        {
                                                h->lastFreeBlockOnHeap = NULL;
                                        }
          
                                        if (block->previous == NULL)
                                        {
                                                h->bin[0] = block->next;
                                        }
                                        else 
                                        {
                                                block->previous->next = block->next;
                                        }

                                        if (block->next == NULL)
                                        {
                                                h->bin[NBINS+1] = block->previous;
                                        }
                                        else
                                        {
                                                block->next->previous = block->previous;
                                        }
                                        goto got_block;
                                }
                                else
                                {
                                        /* split and leave unwanted part of the block in list */
                                        goto split_block;
                                }
                        }
                        else
                        {
                                if (fromHighMemory) 
                                {
                                        block = block->previous; 
                                }
                                else 
                                {
                                        block = block->next; 
                                }
                        }
                }

                /* no block in bin or overflow list, try coalesce if desirable */
                if (!(COALESCED & status) &&
                         ((h->totalFree > (size + 4096) &&
                           h->totalFree > h->userHeap/FRACTION_OF_HEAP_NEEDED_FREE)
                         || FORCECOALESCE & status))
                {
                        Heap7_InternalCoalesce( h );
                        status |= COALESCED;
                        continue; /* try the allocation again */
                }
                else
                {
                        /* no block available in Storage, must go to OSStorage to get one */
                        /* now we have to get more heap */
                        if ( h->full( h->fullparam, size ))
                        {
                                goto get_from_overflow;
                        }
                        else
                        {
                                if (!h->enoughMemoryForGC) 
                                {
                                        if (FORCECOALESCE & status)
                                        {
                                                return 0;
                                        }
                                        else
                                        {
                                                status |= FORCECOALESCE;
                                        }
                                }
                                h->enoughMemoryForGC = FALSE;
                        }
                }
        }

got_block:
        if (fromHighMemory && (actualSize > size+MINBLOCKSIZE))
        {
                /* split and put unwanted part of block into a bin or on overflow list*/
split_block:
                { 
                        Heap7_Block *tempBlock = block;
                        h->totalFree -= OVERHEAD;
                        /* large block taken from bottom of this block */
                        /* medium and small blocks (and bitmaps) taken off top of this block */
                        if ((size > LARGEBLOCK) || (!h->sys_heap_top)) 
                        {
                                ADDBYTESTO(tempBlock, size+OVERHEAD);
                        }
                        else 
                        {
                                ADDBYTESTO(block, actualSize-size);
                        }
                        block->size = size;
                        /* set flags on block to Free */
                        size = actualSize - (size + OVERHEAD);
                        tempBlock->size = size | FREEBIT;

                        if (!fromHighMemory) 
                        {
                                /* The block has been cut from the start of the overflow block.
                                   This means that the large block that was in the overflow list
                                   has to be replaced with new one (tempBlock).
                                */
                                tempBlock->previous = block->previous;
                                tempBlock->next = block->next;
                                if (tempBlock->previous == NULL)
                                {
                                        h->bin[0] = tempBlock;
                                }
                                else
                                {
                                        tempBlock->previous->next = tempBlock;
                                }

                                if (tempBlock->next == NULL)
                                {
                                        h->bin[NBINS+1] = tempBlock;
                                }
                                else
                                {
                                        tempBlock->next->previous = tempBlock;
                                }
                        }
#ifdef BLOCKS_GUARDED
                        tempBlock->guard = GUARDCONSTANT;
#endif

                        if (size <= MAXBINSIZE) 
                        {
                                /* work out the bin number */
                                h->lookInBins = TRUE;
                                index = size / BINRANGE;
                                tempBlock->next = h->bin[index]; h->bin[index] = tempBlock;
                        }
                }
        }

        size = SIZE(block);
        /* set flags to not Free, and gcbits */
        block->size = size;
#ifdef BLOCKS_GUARDED
        block->guard = GUARDCONSTANT;
#endif
        h->totalFree -= size;
        if (h->bin[NBINS+1] > h->lastFreeBlockOnHeap) 
        {
                h->lastFreeBlockOnHeap = h->bin[NBINS+1];
        }
        ADDBYTESTO(block, OVERHEAD);
        return block;
}

#endif

#ifdef realloc_c

/* <Heap7_Realloc>
 * Reallocate a block in a Heap7 heap
 * 24-06-1997 JSR Addapted to Heap_
 */
extern void *__Heap7_Realloc
(
        Heap7_Descriptor *h,
        void *p,
        size_t size
)
{
        size_t oldsize;
        Heap7_Block *block;
        void *newb = NULL;

        /* round to whole words */
        size = (size + 3) & ~3;
        if (BADUSERBLOCK(p))
        {
                h->broken( h->brokenparam );
        }

        block = (Heap7_Block *)p;
        ADDBYTESTO(block, -OVERHEAD);

        oldsize = SIZE(block);
        if (oldsize < size)
        {
                newb = __Heap7_Alloc( h, size );
                if (newb == NULL)
                {
                        return NULL;
                }
                memcpy(newb, p, oldsize);   /* copies 0 words for bad p! */
        }

        if ( oldsize < size || size == 0 || oldsize > size+MINBLOCKSIZE+BYTESPERWORD )
        {
                if ( oldsize > size+MINBLOCKSIZE+BYTESPERWORD && size != 0 )
                {
                        Heap7_Block *b = ADDBYTES(p, -OVERHEAD);
                        b->size = size+BYTESPERWORD | (b->size&(!SIZEMASK));
                        newb = p;
                        ADDBYTESTO(b, size+BYTESPERWORD+OVERHEAD);
#ifdef BLOCKS_GUARDED
                        b->guard = GUARDCONSTANT;
#endif
                        b->size = (oldsize-OVERHEAD-BYTESPERWORD-size);
                        p = ADDBYTES(b, OVERHEAD);
                }
                __Heap7_Free( h, p );
                return newb;
        }
        else
        {
                return p;
        }
}

#endif

#ifdef free_c

/* <Heap7_Free>
 * Release a block back to the type 7 heap
 * 24-06-1997 JSR Addapted to Heap_
 */
extern void __Heap7_Free
(
        Heap7_Descriptor *h,
        void *_block
)
{
        Heap7_Block *block = (Heap7_Block *)_block;
        int size;

        if ((block <= h->heapLow) || (block >= h->heapHigh)) 
        {
                h->broken( h->brokenparam );
        }
        ADDBYTESTO(block, -OVERHEAD);

#ifdef BLOCKS_GUARDED
        if (INVALID(block))
        {
                h->broken( h->brokenparam );
        }
#endif
        size = block->size;
        if (FREEBIT & size)
        {
                h->broken( h->brokenparam );
        }
        /* set flags to Free */
        size &= SIZEMASK;
        block->size = size | FREEBIT;
        h->totalFree += size;

        if (size <= MAXBINSIZE) 
        { 
                /* return to bin */
                h->lookInBins = TRUE; 
                size /= BINRANGE;
                block->next = h->bin[size]; 
                h->bin[size] = block;
        } 
        else 
        {
                /* put block on deallocate overflow list, for reuse after coalesce */
                if (block > h->lastFreeBlockOnHeap)
                {
                        h->lastFreeBlockOnHeap = block;
                }
        }
}


#endif

#ifdef extend_c

extern void _terminate_user_alloc
(
        Heap7_Descriptor *h
)
{
        h->heapLow = h->sys_heap_top;
}

extern void _init_user_alloc
(
        Heap7_Descriptor *h
)
{
        h->sys_heap_top = h->heapLow;
        h->heapLow = h->bin[0];
        h->totalHeap = PTRDIFF(h->heapHigh, h->heapLow);
        h->userHeap = h->totalHeap;
}

extern void __Heap7_Initialise
(
        Heap7_Descriptor *h,
        int (*full)( void *, size_t ),
        void *fullparam, 
        void (*broken)( void * ), 
        void *brokenparam
)
{
        int j;

        h->full = full;
        h->fullparam = fullparam;
        h->broken = broken;
        h->brokenparam = brokenparam;

        h->lastFreeBlockOnHeap = NULL;
        h->mapForExistingHeap = NULL;
        h->enoughMemoryForGC = TRUE;
        /* to get rid of warnings */
        h->mapForExistingHeap = NULL;
        h->mapForNewHeap = NULL;
        h->endOfExistingHeap = NULL;
        h->startOfNewHeap = NULL;
        h->lookInBins = FALSE;
        h->totalFree = 0;
        h->endOfLastExtension = NULL;
        /* set allocate bins and overflow lists to empty */
        for (j=0; j <= NBINS+1; ++j)
        {
                h->bin[j] = NULL;
        }
        h->totalHeap = 0;
        h->userHeap = 0;
        h->sys_heap_top = 0;
        h->heapHigh = 0;
        h->heapLow = (Heap7_Block *) 0x7fffffff;
}


#endif

#ifdef stats_c

/* <Heap7_Stats>
 * Print stats on the given heap
 * 24-06-1997 JSR Written
 */
void *__Heap7_Stats
(
        int (*dprint)( char const *format, ... ),
        Heap7_Descriptor *h
)
{
        Heap7_Block *rover;
        size_t totsize;
        int numblocks;
        int sizecount[32];
        int i;
        size_t size;
        char *hwm;
        int bin;

        hwm = 0;
        totsize = 0;
        numblocks = 0;

        for ( i = 0; i < 32; i++ )
        {
                sizecount[i] = 0;
        }

        /* Start at bin[0] to cover the overflow list */
        for ( bin = 0; bin <= NBINS; bin++ )
        {
                for ( rover = h->bin[bin]; rover; rover = rover->next )
                {
                        if ( (char *)rover > hwm )
                        {
                                hwm = (char *)rover;
                        }
                        numblocks += 1;
                        size = rover->size;
                        totsize += size;
                        for ( i = 0; i < 32; i++ )
                        {
                                size >>= 1;
                                if ( !size )
                                {
                                        break;
                                }
                        }
                        sizecount[i]++;
                }
        }

        dprint( "%d bytes in %d free blocks (avge size %d)\n", totsize, numblocks, totsize/(numblocks==0?1:numblocks) );
        for ( i = 0; i < 32; i++ )
        {
                if ( sizecount[i] )
                {
                        dprint( "%d blocks 2^%d+1 to 2^%d\n", sizecount[i], i-1, i );
                }
        }

        return hwm;
}

#endif

/* end of heap7.c */
