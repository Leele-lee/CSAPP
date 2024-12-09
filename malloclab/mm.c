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
 * 
 */
static void *extend_heap(size_t words)
{
    char * bp;
    size_t asize;
    /* extent words must round up to even numbers to maintain alignment */
    asize = (words % 2) ? (words+1) * WSIZE : words * WSIZE;                    // size is bytes
    /* request words blocks from memory and return bp */
    if (bp = mem_sbrk(asize) == (char *)(-1))
        return -1;
    /* set current bp's header to (words*WSIZE)/0(new free block header) */
    PUT(HDRP(bp), PACK(asize, 0));
    /* set current bp's footer to (words*WSIZE)/0(new free block footer) */
    PUT(FTRP(bp), PACK(asize, 0));
    /* set the next block's head to 0/1(new epilogue header) */
    PUT(NEXT_BLKP(bp), PACK(0,1));
    /* coalescing bp if the previous block is free */
    return coalesce(bp);
} 

void *coalesce(void *bp)
{
    /* if the prev and next blocks are both allocated  */
    /* direct return */

    /* if only the next block is free */
    /* newsize = current block size + next block size */
    /* change the size of current block header and next block footer */

    /* if only the previous block is free */
    /* newsize = current size + previous size */
    /* change the size of previous header and current footer */
    /* bp point to the previous block's payload */

    /* if the prev and next blocks are both free *?
    /* newsize = previous size + current size + next size */
    /* change the size of previous header and the next footer */
    /* bp point to the previous block's payload */
}

/* 
 * mm_malloc - Allocate a block by incrementing the brk pointer.
 *     Always allocate a block whose size is a multiple of the alignment.
 */
void *mm_malloc(size_t size)
{
    int newsize = ALIGN(size + SIZE_T_SIZE);
    void *p = mem_sbrk(newsize);
    if (p == (void *)-1)
	return NULL;
    else {
        *(size_t *)p = size;
        return (void *)((char *)p + SIZE_T_SIZE);
    }
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














