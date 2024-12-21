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

/* 
 * If you want debugging output, use the following macro.  When you hand
 * in, remove the #define DEBUG line. 
 */
#define DEBUG
#ifdef DEBUG 
# define CHECKHEAP(lineno) printf("%s\n", __func__); mm_checkheap(__LINE__);
#endif


/* Basic constants and macros */
#define WSIZE 4 /* word and header/footer size (bytes)*/
#define DSIZE 8 /* double word size (bytes) */
#define CHUCKSIZE (1<<12) /* extend heap by this amount (bytes) */

#define MAX(x, y) ((x) < (y) ? (y) : (x))
#define MIN(x, y) ((x) < (y) ? (x) : (y))

/* Pack a size and alloctated bit into a word */
#define PACK(size, alloc) ((size) | (alloc))

/* Read and write a word(bytes) at address P */
#define GET(p) (*(unsigned int *)(p))
#define PUT(p, val) ((*(unsigned int *)(p)) = (val))

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
static char *heap_listp;
static char *rover;                                                       // used for next fit rover

/* single word (4) or double word (8) alignment */
#define ALIGNMENT 8

/* rounds up to the nearest multiple of ALIGNMENT */
#define ALIGN(size) (((size) + (ALIGNMENT-1)) & ~0x7)

#define SIZE_T_SIZE (ALIGN(sizeof(size_t)))

/* the prototype of the helper function */
static inline void *extend_heap(size_t words);
static inline void *coalesce(void *bp);
static inline void *find_fit(size_t size);
static inline void *find_next_fit(size_t size);
static inline void *find_best_fit(size_t size);
static inline void place(void *bp, size_t size);

/* 
 * mm_init - initialize the malloc package.
 */
int mm_init(void)
{
    /* request 4 blocks from memory */
    if ((heap_listp = mem_sbrk(4 * WSIZE)) == (void *)-1)
        return -1;
    PUT(heap_listp, 0);
    PUT(heap_listp + WSIZE, PACK(8, 1));
    PUT(heap_listp + 2 * WSIZE, PACK(8, 1));
    PUT(heap_listp + 3 * WSIZE, PACK(0, 1));

    heap_listp += (2 * WSIZE);
    rover = heap_listp;                              // set for next fit
    /* extend heap by CHUCKSIZE/WSIZE blocks at the end of the four blocks */
    if (extend_heap(CHUCKSIZE/WSIZE) == NULL)
        return -1;
    return 0;
}

/*
 * extend heap by bsize blocks at the end of the old heap
 * Use in tw circumstances:
 * 1. when the heap is initialized
 * 2. when mm_malloc is unable to find a suitable fit
 */
static inline void *extend_heap(size_t words)
{
    char * bp;
    size_t asize;
    /* extent words must round up to even numbers to maintain alignment */
    asize = (words % 2) ? (words+1) * WSIZE : words * WSIZE;                    // size is bytes
    /* request words blocks from memory and return bp */
    if ((long)(bp = mem_sbrk(asize)) == -1)
        return NULL;
    /* set current bp's header to (words*WSIZE)/0(new free block header) */
    PUT(HDRP(bp), PACK(asize, 0));
    /* set current bp's footer to (words*WSIZE)/0(new free block footer) */
    PUT(FTRP(bp), PACK(asize, 0));
    /* set the next block's head to 0/1(new epilogue header) */
    PUT(NEXT_BLKP(HDRP(bp)), PACK(0,1));
    /* coalescing bp if the previous block is free */
    return coalesce(bp);
} 

static inline void *coalesce(void *bp)
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
    else if (prev_alloc && !next_alloc) {
        /* newsize = current block size + next block size */
        size += GET_SIZE(HDRP(NEXT_BLKP(bp)));
        /* change the size of current block header and new block footer */
        PUT(HDRP(bp), PACK(size, 0));                               // before and after coalescing, bp is not changed
        PUT(FTRP(bp), PACK(size, 0));                               // only the header size had been changed, so the footer is the new address          
    }

    /* if only the previous block is free */
    else if (next_alloc && !prev_alloc) {
        /* newsize = current size + previous size */
        size += GET_SIZE(FTRP(PREV_BLKP(bp)));
        /* change the size of previous header and current footer */
        PUT(FTRP(bp), PACK(size, 0));
        PUT(HDRP(PREV_BLKP(bp)), PACK(size, 0));
        /* bp point to the previous block's payload */
        bp = PREV_BLKP(bp);
    }

    /* if the prev and next blocks are both free */
    else {
        /* newsize = previous size + current size + next size */
        size += GET_SIZE(FTRP(PREV_BLKP(bp))) + GET_SIZE(HDRP(NEXT_BLKP(bp)));
        /* change the size of previous header and the next footer */
        PUT(HDRP(PREV_BLKP(bp)), PACK(size, 0));
        PUT(FTRP(NEXT_BLKP(bp)), PACK(size, 0));
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
        asize = DSIZE + ((size + (DSIZE - 1)) / DSIZE) * DSIZE;      // make sure we get correct answer
        //asize = DSIZE * ((size + (DSIZE) + (DSIZE-1)) / DSIZE);
    }

    /* find if the free list has a free block can hold asize(find_fit) */
    if ((bp = find_fit(asize)) != NULL) {
        place(bp, asize);
        return bp;
    }
    /* if not find call extend_heap, put this request block to the new free block */
    extendsize = MAX(asize, CHUCKSIZE);
    if ((bp = extend_heap(extendsize)) != NULL) {
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
static inline void *find_fit(size_t size) {
    char *bp;
    /* from heap_listp to the end check evey block size */
    for (bp = heap_listp; GET_SIZE(HDRP(bp)) > 0; bp = NEXT_BLKP(bp)) {
        /* if block size >= size and not allocated, return current pointer */
        if ((!GET_ALLOC(HDRP(bp))) && (GET_SIZE(HDRP(bp)) >= size)) {
            return bp;
        }
    }
    /* if not found, return NULL */
    return NULL;
}

 /*
  * next fit - like first fit but search list starting where previous search finished
  *
  */
static inline void *find_next_fit(size_t size) {
    char *bp;
    /* from roverbp to the end */
    for (bp = rover; GET_SIZE(HDRP(bp)) > 0; bp = NEXT_BLKP(bp)) {
        /* if not allocated and block size >= size, set rover to the next bp and return current pointer */
        if ((!GET_ALLOC(HDRP(bp))) && GET_SIZE(HDRP(bp)) >= size) {
            rover = bp;
            return bp;
        }
    }
    /* if not find, return NULL */
    return NULL;
}

/* 
 * best fit, Search the list, choose the best free block: fits, with fewest bytes left over
 */
 static inline void *find_best_fit(size_t size) {
    /* set minsizebp = NULL */
    char *minsizebp = NULL;
    char *bp;

    /* from the beginning of the list to the end */
    for (bp = heap_listp; GET_SIZE(HDRP(bp)) > 0; bp = NEXT_BLKP(bp)) {
        /* if current block is free and current block size >= size */
        if ((!GET_ALLOC(HDRP(bp))) && GET_SIZE(HDRP(bp)) >= size) {
            /* if minisizebp == NULL set minsizebp = bp(aka current block size) */
            if (minsizebp == NULL) {
                minsizebp = bp;
            } else {
                /* if != NULL, minisizebp is the smallest block size's bp */
                if (GET_SIZE(HDRP(minsizebp)) >= GET_SIZE(HDRP(bp))) {
                    minsizebp = bp;
                }
            }
        }
    }
    /* return bp */
    return minsizebp;
 }

/*
 * Place the request block and optionally splits the excess if rest block is satisfy 
 * the minimum request (16 bytes?) then returns the address of the newly allocated block. 
 */
static inline void place(void *bp, size_t size) {
    /* set minisizefree = 2 * DSIZE */
    size_t splitsize = 2 * DSIZE;
    size_t difsize = GET_SIZE(HDRP(bp)) - size;
    /* if bp block size - size >= minisizefree split, change bp's header and footer's 
    allocated bit and size, next block's header and footer's size(block size - size). */
    if (difsize >= splitsize) {
        PUT(HDRP(bp), PACK(size, 1));
        PUT(FTRP(bp), PACK(size, 1));

        PUT(HDRP(NEXT_BLKP(bp)), PACK(difsize, 0));
        PUT(FTRP(NEXT_BLKP(bp)), PACK(difsize, 0));
    } else {
        /* if block size - size < minisizefree not split, just change bp's header and footer's allocate bit and size */
        PUT(HDRP(bp), PACK(size, 1));
        PUT(FTRP(bp), PACK(size, 1));
    }
}

/*
 * mm_free - Freeing a block does nothing.
 */
void mm_free(void *ptr)
{
    size_t size;
    if (ptr == 0)
        return;
    size = GET_SIZE(HDRP(ptr));
    /* change the allocated bit of bp's header and footer */
    PUT(HDRP(ptr), PACK(size, 0));
    PUT(FTRP(ptr), PACK(size, 0));
    /* coalescing bp */
    coalesce(ptr);
}

void mm_nextfit_free(void *ptr) {
    char *bp;
    size_t size;
    if (ptr == NULL)
        return;
    size = GET_SIZE(HDRP(ptr));
    /* change allocated bit of ptr's header and footer */
    PUT(HDRP(ptr), PACK(size, 0));
    PUT(FTRP(ptr), PACK(size, 0));

    /* bp = coalescing ptr */
    bp = coalesce(ptr);
    /* if rover == (char *)ptr and bp is point to the previous block, 
    set rover point to the previous block  */
    if ((rover == (char *)ptr) && (bp == PREV_BLKP(ptr)))
        rover = bp;
}

/*
 * mm_realloc - Implemented simply in terms of mm_malloc and mm_free
 * if ptr is NULL, call malloc(size) 
 * if size == 0, call free(ptr) 
 * if ptr is NULL, call malloc(size)request size bytes memory from heap, 
 * memcpy min(size, ptr's size) and free ptr return newptr
 */
void *mm_realloc(void *ptr, size_t size) 
{
    void *newptr;
    size_t copysize;
    if ((newptr = mm_malloc(size)) == NULL)
        return NULL;
    if (ptr == NULL) 
        return newptr;
    copysize = MIN(GET_SIZE(HDRP(ptr)), size);
    memcpy(newptr, ptr, copysize);
    mm_free(ptr);
    return newptr;
}

void *mm_realloc1(void *ptr, size_t size)
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

/* check the correct of the heap */
void mm_checkheap(int lineno)
{
    // get the beginning
    char *bp = mem_heap_lo();
    //check the start 
    if (GET(bp) != 0) {
        printf("[%d] Prologue Error: word before prolofue incorrect at %p\n", lineno, bp);
    }

    // check prologue's header and footer
    if (GET(bp + WSIZE) != PACK(DSIZE, 1)) {
        printf("[%d] Prologue Error: prologue's header incorrect at %p/n", lineno, bp);
    }
    if (GET(bp + DSIZE) != PACK(DSIZE, 1)) {
        printf("[%d] Prologue Error: prologue's footer incorrect at %p/n", lineno, bp);
    }
}









