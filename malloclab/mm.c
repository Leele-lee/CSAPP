/*
 * mm-naive.c - The fastest, least memory-efficient malloc package.
 * 
 * In this naive approach, a block is allocated by simply incrementing
 * the brk pointer.  A block is pure payload. There are no headers or
 * footers.  Blocks are never coalesced or reused. Realloc is
 * implemented directly using mm_malloc and mm_free.
 *
 * NOTE TO STUDENTS: Replace this header comment with your own header
 * comment that gives a high level description of your solution.
 */
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <string.h>

#include "mm.h"
#include "memlib.h"

/*********************************************************
 * NOTE TO STUDENTS: Before you do anything else, please
 * provide your team information in the following struct.
 ********************************************************/
team_t team = {
    /* Team name */
    "ateam",
    /* First member's full name */
    "Harry Bovik",
    /* First member's email address */
    "bovik@cs.cmu.edu",
    /* Second member's full name (leave blank if none) */
    "",
    /* Second member's email address (leave blank if none) */
    ""
};

/* Basic constants and macros */
#define WSIZE 4 /* word and header/footer size (bytes)*/
#define DSIZE 8 /* double word size (bytes) */
#define CHUCKSIZE (1<<12) /* extend heap by this amount (bytes) */

#define MAX(x, y) ((x) < (y) ? (y) : (x))

/* Pack a size and alloctated bit into a word */
#define PACK(size, alloc) ((size) | (alloc))

/* Read and write a word(bytes) at address P */
#define GET(p) (*(unsigned int *)(p))
#define PUT(p, val) ((*(unsigned int *)(p)) = val)

/* Read the size and allocated fileds from address P */
#define GET_SIZE(p) (GET(p) & (~0x7))
#define GET_ALLOC(p) (GET(p) & 0x1)

/* Given block ptr bp, computer address of its header and footer */
#define HDRP(bp) ((char *)(bp) - WSIZE)
#define FTRP(bp) ((char *)(bp) + GET_SIZE(HDRP(bp)) - DSIZE)

/* Given block ptr bp, compute address of next and previoud blocks */
#define NEXT_BLKP(bp) ((char *)(bp) + GET_SIZE(((char *)(bp) - WSIZE)))
#define PREV_BLKP(bp) ((char *)(bp) - GET_SIZE(((char *)(bp) - DSIZE)))

/* Piravate global variables */
static char * heap_listp;


/* single word (4) or double word (8) alignment */
#define ALIGNMENT 8

/* rounds up to the nearest multiple of ALIGNMENT */
#define ALIGN(size) (((size) + (ALIGNMENT-1)) & ~0x7)


#define SIZE_T_SIZE (ALIGN(sizeof(size_t)))

/* 
 * mm_init - initialize the malloc package.
 */
int mm_init(void)
{
    /* request 4 blocks from memory */
    if (heap_listp = mem_sbrk(4 * WSIZE) == (void *)-1)
        return -1;
    PUT(heap_listp, 0);
    PUT(heap_listp + WSIZE, PACK(8, 1));
    PUT(heap_listp + 2 * WSIZE, PACK(8, 1));
    PUT(heap_listp + 3 * WSIZE, PACK(0, 1));

    heap_listp += DSIZE;

    /* extend heap by CHUCKSIZE/WSIZE blocks at the end of the four blocks */
    extend_heap(CHUCKSIZE/WSIZE);
}

/*
 * extend heap by bsize blocks at the end of the old heap
 * Use in tw circumstances:
 * 1. when the heap is initialized
 * 2. when mm_malloc is unable to find a suitable fit
 */
static void *extend_heap(size_t words)
{
    char * bp;
    size_t asize;
    /* extent words must round up to even numbers to maintain alignment */
    asize = (words % 2) ? (words+1) * WSIZE : words * WSIZE;                    // size is bytes
    /* request words blocks from memory and return bp */
    if (bp = mem_sbrk(asize) == (char *)(-1))
        return NULL;
    /* set current bp's header to (words*WSIZE)/0(new free block header) */
    PUT(HDRP(bp), PACK(asize, 0));
    /* set current bp's footer to (words*WSIZE)/0(new free block footer) */
    PUT(FTRP(bp), PACK(asize, 0));
    /* set the next block's head to 0/1(new epilogue header) */
    PUT(NEXT_BLKP(bp), PACK(0,1));
    /* coalescing bp if the previous block is free */
    return coalesce(bp);
} 

static void *coalesce(void *bp)
{
    size_t prev_alloc = GET_ALLOC(FTRP(PREV_BLKP(bp)));
    size_t next_alloc = GET_ALLOC(HDRP(NEXT_BLKP(bp)));
    size_t size = GET_SIZE(HDRP(bp));

    /* if the prev and next blocks are both allocated  */
    if (prev_alloc && next_alloc) {
    /* direct return */
        return bp;
    }
    /* if only the next block is free */
    else if (prev_alloc) {
        /* newsize = current block size + next block size */
        size += GET_SIZE(HDRP(NEXT_BLKP(bp)));
        /* change the size of current block header and new block footer */
        PUT(HDRP(bp), size);                               // before and after coalescing, bp is not changed
        PUT(FTRP(bp), size);                               // only the header size had been changed, so the footer is the new address          
    }

    /* if only the previous block is free */
    else if (next_alloc) {
        /* newsize = current size + previous size */
        size += GET_SIZE(FTRP(PREV_BLKP(bp)));
        /* change the size of previous header and current footer */
        PUT(FTRP(bp), size);
        PUT(HDRP(PREV_BLKP(bp)), size);
        /* bp point to the previous block's payload */
        bp = PREV_BLKP(bp);
    }

    /* if the prev and next blocks are both free */
    else {
        /* newsize = previous size + current size + next size */
        size += GET_SIZE(FTRP(PREV_BLKP(bp))) + GET_SIZE(HDRP(NEXT_BLKP(bp)));
        /* change the size of previous header and the next footer */
        PUT(HDRP(PREV_BLKP(bp)), size);
        PUT(FTRP(NEXT_BLKP(bp)), size);
        /* bp point to the previous block's payload */
        bp = PREV_BLKP(bp);
    }
    return bp;
}

/* 
 * mm_malloc - Allocate a block by incrementing the brk pointer.
 *     Always allocate a block whose size is a multiple of the alignment.
 */
void *mm_malloc(size_t size)
{
    size_t asize;
    size_t extendsize;
    char *bp;

    if (size == 0)
        return NULL;
    /* adjust the size to the multiple of 8 bytes asize */
    /* if size < 8, asize = 16 */
    if (size <= DSIZE) {
        asize = 2 * DSIZE;
    } else {
        asize = 2 * DSIZE + ((size + (DSIZE - 1)) / DSIZE) * DSIZE;      // make sure we get correct answer
    }

    /* find if the free list has a free block can hold asize(find_fit) */
    if (bp = find_fit(asize) != NULL) {
        place(bp, asize);     
        return bp;
    }
    /* if not find call extend_heap, put this request block to the new free block */
    extendsize = MAX(asize, CHUCKSIZE);
    if (bp = extend_heap(extendsize) != NULL) {
        /*  put this request block to the fit free block and splitting the block 
        if rest block is satisfy the minimum request (16 bytes?) */
        place(bp, asize);
    } else {
        return NULL;
    }
    return bp;
}

/* 
 * Search the free list for a suitable free block.
 * first try --- first fit
 * Search list from beginning, choose first free block that fits.
 */
static void *find_fit(size_t size) {
    char *bp;
    
    /* from heap_listp to the end check evey block size */
    for (bp = heap_listp; GET_SIZE(HDRP(bp)) > 0; bp = NEXT_BLKP(bp)) {
        /* if block size >= size and not allocated, return current pointer */
        if (GET_ALLOC(HDRP(bp)) && (HDRP(bp)) >= size) {
            return bp;
        }
    }
    /* if not find, return NULL */
    return NULL;
}

/* 
 * best fit, Search the list, choose the best free block: fits, with fewest bytes left over
 */
 static void *find_bestfit(size_t size) {
    /* set minsizebp = NULL */
    /* from the beginning of the list to the end */
        /* if current block is free and current block size >= size */
            /* if minisizebp == NULL set minsizebp = bp(aka current block size) */
            /* if != NULL, bp is the smallest block size's bp */
    /* return bp */

 }

/*
 * Place the request block ans optionally splits the excess if rest block is satisfy 
 * the minimum request (16 bytes?) then returns the address of the newly allocated block. 
 */
static void *place(void *bp, size_t size) {
    return;
}

/*
 * mm_free - Freeing a block does nothing.
 */
void mm_free(void *ptr)
{
}

/*
 * mm_realloc - Implemented simply in terms of mm_malloc and mm_free
 */
void *mm_realloc(void *ptr, size_t size)
{
    void *oldptr = ptr;
    void *newptr;
    size_t copySize;
    
    newptr = mm_malloc(size);
    if (newptr == NULL)
      return NULL;
    copySize = *(size_t *)((char *)oldptr - SIZE_T_SIZE);
    if (size < copySize)
      copySize = size;
    memcpy(newptr, oldptr, copySize);
    mm_free(oldptr);
    return newptr;
}














