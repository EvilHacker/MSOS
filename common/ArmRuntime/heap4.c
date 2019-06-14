/*
 * heap4.c
 * Copyright: 1997 Advanced RISC Machines Limited. All rights reserved.
 */

/*
 * RCS $Revision: 1.2 $
 * Checkin $Date: 2001/04/16 20:14:45Z $
 * Revising $Author: marks $
 */

/* Code for a type (4) bucket heap based on a large block heap.

Small blocks are allocated from buckets. A bucket contains a small
pool of small blocks of a particular size. Within a bucket these
free blocks are held in a 1-way linked list. When allocated the
'size' field (the 4 bytes before the allocated block) points to
the bucket containing it, with bit 1 (unused by any large block
heap) set to distinguish it from a large block.

The buckets are stored in an array of lists of buckets.
The array is indexed by size of block - each entry stores buckets
which supply blocks of that size. Each entry has three lists:
A list of partly used buckets, ordered by amount used.
A list of empty buckets.
The list of empty buckets is held so that when the heap gets full
empty buckets can be released back to the heap. Releasing a bucket
when it becomes empty is to be avoided to prevent continuous bucket
allocation and freeing on small block allocation and freeing.
An ordered list of partly used buckets is held to perform allocation
into the fullest bucket first. This is to give allocation a push towards
greater long-term occupancy.
Full buckets are not held in any list.
*/
#ifndef _HEAP4_H
 #include "heap4.h"
#endif

#ifndef PARANOID
 #define PARANOID 0
#endif

/* These bucket descriptions are calculated as follows:
The block size is 8*(1.5^N) rounded up to the nearest multiple of 4. This means
that the wastage is at most 1/2 of the requested size plus 4 (allocation overhead).
The number of blocks per bucket is calculated to give a bucket size of about 512 bytes,
or 8 blocks per bucket, whichever gives the largest bucketsize.
To alter this table:
blksz must be a multiple of 4
numblk must be 1 or more
The number of entries in this table should match HEAP4_BUCKETCOUNT in heap4.h
The table entries should be in increasing blksize.
*/

/* These generate the biggest 2^N less than or equal to HEAP4_BUCKETCOUNT-1.
This value will be used to binary chop our way towards the right bucket */
#define Heap4_BucketMask1(m) ((m)|(m)>>1)
#define Heap4_BucketMask2(m) (Heap4_BucketMask1(m)|Heap4_BucketMask1(m)>>2)
#define Heap4_BucketMask4(m) (Heap4_BucketMask2(m)|Heap4_BucketMask2(m)>>4)
#define Heap4_BucketMask8(m) (Heap4_BucketMask4(m)|Heap4_BucketMask4(m)>>8)
#define Heap4_BucketMask16(m) (Heap4_BucketMask8(m)|Heap4_BucketMask8(m)>>16)
#define Heap4_BucketMask (Heap4_BucketMask16(HEAP4_BUCKETCOUNT-1)&~(Heap4_BucketMask16(HEAP4_BUCKETCOUNT-1)>>1))

#define Heap4_MaxBucketBlockSize (__Heap4_BucketDescriptions[HEAP4_BUCKETCOUNT-1].blksize)

typedef struct 
{
    size_t blksize;
    size_t freecount;
    size_t bucketsize;
} Heap4_BucketDescription;

const Heap4_BucketDescription __Heap4_BucketDescriptions[];

#ifdef extend_c

#define Heap4_BucketDesc(blksz,numblk) { (blksz), (numblk), sizeof(Heap4_BucketHeader)+((blksz)+sizeof(size_t))*(numblk) }

const Heap4_BucketDescription __Heap4_BucketDescriptions[] =
{
    Heap4_BucketDesc(8,41),
    Heap4_BucketDesc(12,30),
    Heap4_BucketDesc(20,20),
    Heap4_BucketDesc(28,15),
    Heap4_BucketDesc(44,10),
    Heap4_BucketDesc(64,8),
    Heap4_BucketDesc(92,8),
    Heap4_BucketDesc(140,8),
    Heap4_BucketDesc(208,8),
    Heap4_BucketDesc(308,8),
    Heap4_BucketDesc(464,8),
    Heap4_BucketDesc(692,8)
};


/* <Heap4_Full>
 * The base heap is full - lets release our empties and give it another go
 * 10-06-1997 JSR Written
 */
static int Heap4_Full(void *param, size_t size)
{
    Heap4_Descriptor *h = (Heap4_Descriptor *)param;
    int i;
    Heap4_Link *link;
    Heap4_BucketHeader *bkt;
    int doagain = 0;

    for (i = 0; i < HEAP4_BUCKETCOUNT; i++)
    {
        for (link = h->array[i].empty.next;
             link->next != NULL;
             link = h->array[i].empty.next)
        {
            bkt = (Heap4_BucketHeader *)link;

            /* unlink bucket */
            bkt->link.prev->next = bkt->link.next;
            bkt->link.next->prev = bkt->link.prev;

            /* Release bucket to base heap */
            Heap4_BaseFree(&h->base, bkt);

            doagain = 1;
        }
    }

    if (!doagain)
    {
        /* We didn't free any - let's try our creator */
        doagain = h->full( h->fullparam, size );
    }

    return doagain;
}


/* <Heap4_Initialise>
 * Initialise the type 4 heap
 * 10-06-1997 JSR Written
 */
extern void __Heap4_Initialise(
    Heap4_Descriptor *h,
    int (*full)( void *, size_t ), void *fullparam,
    void (*broken)( void * ), void *brokenparam)
{
    int i;

    Heap4_BaseInitialise( &h->base, Heap4_Full, h, broken, brokenparam );

    h->full = full;
    h->fullparam = fullparam;
    h->broken = broken;
    h->brokenparam = brokenparam;

    for (i = 0; i < HEAP4_BUCKETCOUNT; i++)
    {
        h->array[i].used.next = (Heap4_Link *)&h->array[i].used.marker;
        h->array[i].used.marker = 0;
        h->array[i].used.prev = (Heap4_Link *)&h->array[i].used.next;

        h->array[i].empty.next = (Heap4_Link *)&h->array[i].empty.marker;
        h->array[i].empty.marker = 0;
        h->array[i].empty.prev = (Heap4_Link *)&h->array[i].empty.next;
    }
}


/* <Heap4_ProvideMemory>
 * Provide memory to the heap
 * 10-06-1997 JSR Written
 */
extern void __Heap4_ProvideMemory(Heap4_Descriptor *h, void *_blk, size_t size)
{
    /* Just give the memory to our base heap */
    Heap4_BaseProvideMemory( &h->base, _blk, size );
}

#endif

#ifdef alloc_c

/* <Heap4_Alloc>
 * Allocate some memory from the type 4 heap
 * 10-06-1997 JSR Written
 */
extern void *__Heap4_Alloc(Heap4_Descriptor *h, size_t size)
{
    int bucketid;
    int bchop;
    Heap4_BucketChain *ch;
    Heap4_BucketHeader *bkt;
    Heap4_FreeBlock *blk;
    Heap4_FreeBlock *rover;
    size_t i;
    size_t blksize;

    /* Check if larger than biggest bucket block */
    if (size > Heap4_MaxBucketBlockSize)
    {
        return Heap4_BaseAlloc(&h->base, size);
    }

    /* Binary chop to find which bucket we want */
    bucketid = 0;
    for (bchop = Heap4_BucketMask; bchop != 0; bchop >>= 1)
    {
        if (bucketid + bchop >= HEAP4_BUCKETCOUNT)
        {
            continue;
        }

        if (__Heap4_BucketDescriptions[bucketid + bchop - 1].blksize < size)
        {
            bucketid += bchop;
        }
    }

    /* bucketid is the bucket containing blocks of the required size */
    ch = &h->array[bucketid];

    /* Check used list */
    if (ch->used.next->next != NULL)
    {
        /* used list contains bucket - use that */
        bkt = (Heap4_BucketHeader *)ch->used.next;

        /* Pick block from free list */
        blk = bkt->free;
        bkt->free = blk->next;
        bkt->usedcount += 1;

        /* Check if no more free blocks in bucket */
        if (bkt->free == NULL)
        {
            /* Remove from used list */
            bkt->link.prev->next = bkt->link.next;
            bkt->link.next->prev = bkt->link.prev;
        }
        return &blk->next;
    }

    /* Check empties list */
    if (ch->empty.next->next != NULL)
    {
        /* empty list contains bucket - use that */
        bkt = (Heap4_BucketHeader *)ch->empty.next;

        /* Pick block from free list */
        blk = bkt->free;
        bkt->free = blk->next;
#if PARANOID
        if (bkt->free == NULL)
        {
            h->broken(h->brokenparam);
        }
#endif
        bkt->usedcount += 1;

        /* Move bucket to used list, which we know is empty */
        bkt->link.prev->next = bkt->link.next;
        bkt->link.next->prev = bkt->link.prev;
        bkt->link.prev = (Heap4_Link *)&ch->used.next;
        bkt->link.next = (Heap4_Link *)&ch->used.marker;
        ch->used.prev = ch->used.next = &bkt->link;

        return &blk->next;
    }

    /* allocate new bucket */
    bkt = (Heap4_BucketHeader *)Heap4_BaseAlloc(&h->base, __Heap4_BucketDescriptions[bucketid].bucketsize);
    if (bkt != NULL)
    {
        bkt->usedcount = 1;
        bkt->ch = ch;

        blk = (Heap4_FreeBlock *)(bkt+1);

        /* Initialise the free chain inside the bucket */
        blk->size = (size_t)bkt | Heap4_Marker;
        blksize = __Heap4_BucketDescriptions[bucketid].blksize + sizeof(size_t);
        rover = (Heap4_FreeBlock *)((char *)blk + blksize);
        rover->size = (size_t)bkt | Heap4_Marker;
        rover->next = 0;
        for (i = 2; i < __Heap4_BucketDescriptions[bucketid].freecount; i++)
        {
            rover = (Heap4_FreeBlock *)((char *)rover + blksize);
            rover->size = (size_t)bkt | Heap4_Marker;
            rover->next = (Heap4_FreeBlock *)((char *)rover - blksize);
        }
        bkt->free = rover;

        /* Put bucket onto used list, which we know is empty */
        bkt->link.prev = (Heap4_Link *)&ch->used.next;
        bkt->link.next = (Heap4_Link *)&ch->used.marker;
        ch->used.prev = ch->used.next = &bkt->link;

        return &blk->next;
    }

    /* (option here: try using a bucket for larger blocks) */

    /* Allocate big block */
    return Heap4_BaseAlloc(&h->base, size);
}

#endif

#ifdef free_c

/* <Heap4_Free>
 * Free some memory back to the heap
 * 10-06-1997 JSR Written
 */
extern void __Heap4_Free(Heap4_Descriptor *h, void *_blk)
{
    Heap4_FreeBlock *blk = (Heap4_FreeBlock *)((char *)_blk - sizeof(size_t));
    Heap4_BucketHeader *bkt;
    Heap4_BucketChain *ch;

    if ( !(blk->size & Heap4_Marker) )
    {
        Heap4_BaseFree(&h->base, _blk);
        return;
    }

    bkt = (Heap4_BucketHeader *)(blk->size & ~Heap4_Marker);
    ch = bkt->ch;
    bkt->usedcount -= 1;
    if (bkt->free == NULL)
    {
        /* was full - add to used list tail */
        bkt->link.prev = ch->used.prev;
        bkt->link.next = (Heap4_Link *)&ch->used.marker;
        bkt->link.prev->next = ch->used.prev = &bkt->link;
    }
    else if (bkt->usedcount == NULL)
    {
        /* Now empty - move from used list to empty list tail */
        bkt->link.prev->next = bkt->link.next;
        bkt->link.next->prev = bkt->link.prev;
        bkt->link.prev = ch->empty.prev;
        bkt->link.next = (Heap4_Link *)&ch->empty.marker;
        bkt->link.prev->next = ch->empty.prev = &bkt->link;
    }
    else
    {
        /* Neither empty nor full - bubble bucket down used list */
        while (bkt->link.next->next != NULL
               && ((Heap4_BucketHeader *)bkt->link.next)->usedcount > bkt->usedcount)
        {
            bkt->link.prev->next = bkt->link.next;
            bkt->link.next->prev = bkt->link.prev;
            bkt->link.prev = bkt->link.next;
            bkt->link.next = bkt->link.prev->next;
            bkt->link.prev->next = bkt->link.next->prev = &bkt->link;
        }
    }
    blk->next = bkt->free;
    bkt->free = blk;
}

#endif

#ifdef realloc_c

/* <Heap4_Realloc>
 * Re-allocate the given block, ensuring the newly allocated block has the same
 * min(newsize,oldsize) bytes at its start
 * 10-06-1997 JSR Written
 */
extern void *__Heap4_Realloc(Heap4_Descriptor *h, void *_blk, size_t size)
{
    Heap4_FreeBlock *blk = (Heap4_FreeBlock *)((char *)_blk - sizeof(size_t));
    void *rover;
    Heap4_BucketHeader *bkt;
    Heap4_BucketChain *ch;
    size_t oldsize;

    /* realloc base blocks using the base allocator */
    if (!(blk->size & Heap4_Marker))
    {
        return Heap4_BaseRealloc(&h->base, _blk, size);
    }

    bkt = (Heap4_BucketHeader *)(blk->size & ~Heap4_Marker);
    ch = bkt->ch;
    oldsize = __Heap4_BucketDescriptions[ch - h->array].blksize;

    if (size <= oldsize)
    {
        return _blk;
    }

    /* Do an alloc,copy,free sequence as bucket blocks can't be stretched */
    rover = __Heap4_Alloc(h, size);
    if ( rover )
    {
        memcpy(rover, _blk, oldsize < size ? oldsize: size);
        __Heap4_Free(h, _blk);
    }

    return rover;
}

#endif

#ifdef stats_c

/* <Heap4_Stats>
 * Print stats on the given heap
 * 10-06-1997 JSR Written
 */
extern void *__Heap4_Stats(int (*dprint)(char const *format, ...), Heap4_Descriptor *h)
{
    return Heap4_BaseStats(dprint, &h->base);
}

#endif

/* End of file heap4.c */
