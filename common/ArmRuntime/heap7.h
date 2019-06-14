/*
 * RCS $Revision: 1.2 $
 * Checkin $Date: 2001/04/16 20:14:47Z $
 * Revising $Author: marks $
 */

#ifdef _HEAP7_H
 #error "Heap7 has been included more than once"
#else
#define _HEAP7_H

#include "heapCommon.h"

#define BLOCKS_GUARDED */
/* #define GC */
/* #define MEMDUMP gives a memory dump to screen and $.memdump when corrupt */
/*
 * if the OSStorage can not be depended upon to allocate areas of the
 *   heap in consistently increasing address order then the following
 *   constant must be set to FALSE.
 */
#define HEAP_ALLOCATED_IN_ASCENDING_ADDRESS_ORDER 1

typedef struct BlockStruct {
#ifdef BLOCKS_GUARDED
  unsigned int guard; /* guard word should contain GuardConstant if all ok */
#endif
  size_t size; /* block size (not including header) in address units. */
               /* The top 5 bits of size hold the flags declared above */
  struct BlockStruct *next; /* next and previous are used internaly in */
  struct BlockStruct *previous; /* coalescing and managing free blocks. */
                                /* next is the first word of the user block */
} Heap7_Block;

#define SIZEMASK      0xfffffffc /* all except bottom two bits, when applied */
                                 /* to block.size yields no of address units */
                                 /* of user space in this block. */
#define FREEBIT       (1U<<0) /* if set, indicates that the block is free */
#define HEAPHOLEBIT   (1U<<1) /* block used for marking start of heap hole */
#define GUARDCONSTANT 0x3E694C3C /* not a legal word pointer */
#define BYTESPERWORD  sizeof(int)
/* Block.next offset from Block (in words) */
#define FIRSTUSERWORD ((sizeof(Heap7_Block)-sizeof(Heap7_Block *)*2) / BYTESPERWORD)

#define BITSIZE(bytes) ((bytes)<<3)
#define BITSPERWORD  BITSIZE(sizeof(int))
#define BITSPERBYTE  (BITSPERWORD/BYTESPERWORD)
/*
 * The following constants are all in address units
 */
/* MAXBYTES should be something outrageously big */
#define MAXBYTES     0x01000000
#define OVERHEAD     ((int)(FIRSTUSERWORD * BYTESPERWORD))
#define HOLEOVERHEAD OVERHEAD
#define MINBLOCKSIZE (OVERHEAD + BYTESPERWORD)

/* the following constants are tunable */
/* multiple of required block size needing to be free before coalesce done */
#define BINRANGE     (BYTESPERWORD * 1) /* see assumptions */
#define NBINS        16
#define MAXBINSIZE   (BINRANGE*(NBINS)-1)
#define LARGEBLOCK   512

/*
 * Code macros.
 */
#define SIZE(block) ((size_t)((block)->size & SIZEMASK))
#define BITSTOWORDS(bits) ((bits+(BITSPERWORD-1))/BITSPERWORD)
#define BYTESTOWORDS(bytes) ((bytes+(BYTESPERWORD-1))/BYTESPERWORD)
#define ADDBYTES(bp, bytes) (Heap7_Block *)((char *)bp + (bytes))
#define ADDBYTESTO(bp, bytes) bp = (Heap7_Block *)((char *)bp + (bytes))
#define PTRDIFF(hi, lo) ((char *)hi - (char *)lo)
#define FREE(block) (FREEBIT & ((Heap7_Block *)block)->size)
#define HEAPHOLE(block) (HEAPHOLEBIT & block->size)

#ifdef BLOCKS_GUARDED
#define INVALID(block) (((Heap7_Block *)block)->guard != GUARDCONSTANT)
#else
#define INVALID(block) (0)
#endif
#define BADUSERBLOCK(block) (INVALID(ADDBYTES(block,-OVERHEAD)) \
                            || FREE(ADDBYTES(block,-OVERHEAD)))



typedef struct Heap7_Descriptor
{
        int (*full)( void *, size_t );
        void *fullparam;
        void (*broken)( void * );
        void *brokenparam;

        Heap7_Block *heapLow;  /* address of the base of the heap */
        Heap7_Block *heapHigh; /* address of heap hole guard at the top of heap */
        Heap7_Block *sys_heap_top; /* address of top of system heap, should = heapLow
                               after _init_user_alloc is called          */

        /*
         * amount of heap that user can actually write to, does not include bitmaps
         * and block overheads
         */
        size_t totalFree;
        size_t userHeap;  /* size of heap (bytes) excluding gc bitmaps */
        size_t totalHeap; /* size of heap (bytes) including gc bitmaps */
        /*
         * The overflow list is a chain of large blocks ready for use, the chain is a
         * doubly linked list of blocks in increasing address order.
         * bin[0] is the start of the overflow list.
         * bin[NBINS+1] is end of the overflow list.
         *
         * bin is an array of pointers to lists of free small blocks ( <= MAXBINSIZE)
         * of the same size. Last deallocated block is at the start of the list.
         */
        Heap7_Block *bin[NBINS+2];

        Heap7_Block *endOfLastExtension;

        int lookInBins;

        Heap7_Block *lastFreeBlockOnHeap;
        int enoughMemoryForGC;
        char *mapForExistingHeap;
        char *mapForNewHeap;
        Heap7_Block *endOfExistingHeap;
        Heap7_Block *startOfNewHeap;
} Heap7_Descriptor;

#define inline __inline

extern void __Heap7_Initialise( Heap7_Descriptor *h, int (*full)( void *, size_t ), void *fullparam, void (*broken)( void * ), void *brokenparam );
extern void __Heap7_ProvideMemory( Heap7_Descriptor *h, void *blk, size_t size );
extern void *__Heap7_Alloc( Heap7_Descriptor *h, size_t size );
extern void __Heap7_Free( Heap7_Descriptor *h, void *blk );
extern void *__Heap7_Realloc( Heap7_Descriptor *h, void *_blk, size_t size );
extern void *__Heap7_Stats( int (*pr)( char const *format, ... ), Heap7_Descriptor *h );
#define __Heap7_TraceAlloc(h,s,f,l) __Heap7_Alloc(h,s)

#endif
