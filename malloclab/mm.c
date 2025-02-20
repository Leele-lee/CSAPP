/*
 * Mm-naive.c - The fastest, least memory-efficient malloc package.
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

extern void mm_checkheap(int lineno);
extern void mm_checkfreelist(int lineno);

/* 
 * If you want debugging output, use the following macro.  When you hand
 * in, remove the #define DEBUG line. 
 */
#define DEBUG
#ifdef DEBUG 
//# define CHECKHEAP(lineno) printf("%s\n", __func__); mm_checkheap(__LINE__);
# define CHECKHEAP(lineno) printf("%s\n", __func__);  mm_checkfreelist(__LINE__); mm_checkheap(__LINE__);
#else
# define CHECKHEAP(lineno) 
#endif


/* Basic constants and macros */
#define WSIZE 4 /* word and header/footer size (bytes)*/
#define DSIZE 8 /* double word size (bytes) */
#define CHUCKSIZE (1<<12) /* extend heap by this amount (bytes) */
#define CLASS_SIZE 20

#define MAX(x, y) ((x) < (y) ? (y) : (x))
#define MIN(x, y) ((x) < (y) ? (x) : (y))

/* Pack a size and alloctated bit into a word */
#define PACK(size, alloc) ((size) | (alloc))
#define PACK_ALL(size, prev_alloc, alloc) ((size) | (prev_alloc) | (alloc))

/* Read and write a word(bytes) at address P */
#define GET(p) (*(unsigned int *)(p))
#define PUT(p, val) ((*(unsigned int *)(p)) = (val))

/* For given list number get the start bp address of the correspond free list*/
#define GET_START(num) ((unsigned int *)(long)GET(mem_heap_lo() + num * DSIZE))

/* For given bp, get the address of the previous and next free block bp given bp */
#define GET_PRE(bp) GET(bp)
#define GET_SUC(bp) GET(((unsigned int *)(bp) + 1))
//#define GET_PRE_ADDRS(bp)  ((unsigned int *)(long)(mem_heap_lo() + GET(bp)))
#define GET_PRE_ADDRS(bp)  ((unsigned int *)((char *)mem_heap_lo() + GET(bp)))
//#define GET_SUC_ADDRS(bp) ((unsigned int *)(long)(mem_heap_lo() + GET((unsigned int *)bp + 1)))
#define GET_SUC_ADDRS(bp) ((unsigned int *)((char *)mem_heap_lo() + GET((unsigned int *)bp + 1)))

/* Read and set the size and allocated fileds from address P */
#define GET_SIZE(p) (GET(p) & (~0x7))
#define GET_ALLOC(p) (GET(p) & 0x1)
#define GET_PREV_ALLOC(p) (GET(p) & 0x2)
#define SET_PREV_ALLOC(p) (GET(p) |= 0x2)
#define SET_PREV_FREE(p) (GET(p) &= ~0x2)

//#define SET_PREV_NODE(curr_bp, val) (*(unsigned int *)(curr_bp) = ((unsigned int)(long)((val) - mem_heap_lo())))
//#define SET_NEXT_NODE(curr_bp, val) (*(unsigned int *)((char *)curr_bp + WSIZE) = ((unsigned int)(long)((val) - mem_heap_lo())))

/* Given block ptr bp, computer address of its header and footer */
#define HDRP(bp) ((char *)(bp) - WSIZE)
#define FTRP(bp) ((char *)(bp) + GET_SIZE(HDRP(bp)) - DSIZE)

/* Given block ptr bp, compute address of next and previoud blocks */
// PREV_BLKP only works for the previous block is free block, bc allocated block has no footer
#define NEXT_BLKP(bp) ((char *)(bp) + GET_SIZE(((char *)(bp) - WSIZE)))
#define PREV_BLKP(bp) ((char *)(bp) - GET_SIZE(((char *)(bp) - DSIZE)))

/* Piravate global variables */
static char *heap_listp;
static char *rover;                                                       // used for next fit rover

/* The head pointer array of the free list, each element is a head pointer, 
 * pointing to the first free block of this type of free list
 */
static char** free_lists;

/* single word (4) or double word (8) alignment */
#define ALIGNMENT 8

/* rounds up to the nearest multiple of ALIGNMENT */
#define ALIGN(p) (((size_t)(p) + (ALIGNMENT-1)) & ~0x7)
#define CHECK_ALIGN(p) ((size_t)(p) == ALIGN(p))

#define SIZE_T_SIZE (ALIGN(sizeof(size_t)))

/* the prototype of the helper function */
static inline void *extend_heap(size_t words);
static inline void *coalesce(void *bp);
static inline void *find_fit(size_t size);
static inline void place(void *bp, size_t size);
static inline void insert_node(void *bp);
static inline void delete_node(void *bp);
static inline size_t find_index(size_t size);


/* 
 * mm_init - initialize the malloc package.
 */
int mm_init(void)
{
  free_lists = mem_heap_lo();
  printf("in init, mem_heap_lo() is %p\n", mem_heap_lo());
    /* request 4 blocks from memory */
    if ((heap_listp = mem_sbrk(CLASS_SIZE * DSIZE+ 4 * WSIZE)) == (void *)-1)
        return -1;
    // init the class part
    for (int i = 0; i < CLASS_SIZE; i++) {
        // class part need 8 bytes space each one so we cann't use PUT
        free_lists[i] = NULL;
    }
    PUT(heap_listp + DSIZE * CLASS_SIZE, 0);
    PUT(heap_listp + DSIZE * CLASS_SIZE + WSIZE, PACK(8, 1));
    PUT(heap_listp + DSIZE * CLASS_SIZE + 2 * WSIZE, PACK(8, 1));
    // now the block before the epilogue block is prologue block, so the prev block bit is 1, 0x11 = 3
    PUT(heap_listp + DSIZE * CLASS_SIZE + 3 * WSIZE, PACK(0, 3)); 


    heap_listp += (DSIZE * CLASS_SIZE + 2 * WSIZE);
    printf("heap_listp at init should point to the middle of prologue is  %p\n", heap_listp);
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
    size_t prev_alloc;
    /* extent words must round up to even numbers to maintain alignment */
    asize = (words % 2) ? (words+1) * WSIZE : words * WSIZE;                    // size is bytes
    /* request words blocks from memory and return bp */
    if ((long)(bp = mem_sbrk(asize)) == -1)
        return NULL;
    prev_alloc = GET_PREV_ALLOC(HDRP(bp)); // first time extend_heap should return 2

    /* set current bp's header to (words*WSIZE)/0(new free block header) */
    PUT(HDRP(bp), PACK_ALL(asize, prev_alloc, 0));
    //printf("in extend_heap, bp header's address is %p\n", HDRP(bp));
    /* set current bp's footer to (words*WSIZE)/0(new free block footer) */
    PUT(FTRP(bp), PACK_ALL(asize, prev_alloc, 0));
    //printf("in extend_heap, bp's footer address is at %p\n", FTRP(bp));
    /* set the next block's head to 0/1(new epilogue header) */
    PUT(HDRP(NEXT_BLKP(bp)), PACK(0,1));
    //printf("extend_heap, bp's next header address at %p\n", NEXT_BLKP(bp));
    /* coalescing bp if the previous block is free */
    return coalesce(bp);
} 


/*
 * colescing adjecent blocks, after colescing set the next block's prev_alloc 
 * statues to 2
 */
static inline void *coalesce(void *bp)
{
    size_t prev_alloc = GET_PREV_ALLOC(HDRP(bp));
    size_t next_alloc = GET_ALLOC(HDRP(NEXT_BLKP(bp)));
    size_t size = GET_SIZE(HDRP(bp)); 
    
    /* if the prev and next blocks are both allocated  */
    if (prev_alloc && next_alloc) {
        SET_PREV_FREE(HDRP(NEXT_BLKP(bp)));
    }
    /* if only the next block is free */
    else if (prev_alloc && !next_alloc) {
        // delete the old free block node
        delete_node(NEXT_BLKP(bp));
        /* newsize = current block size + next block size */
        size += GET_SIZE(HDRP(NEXT_BLKP(bp)));
        /* change the size of current block header and new block footer */
        PUT(HDRP(bp), PACK_ALL(size, 2, 0));                               // before and after coalescing, bp is not changed
        PUT(FTRP(bp), PACK_ALL(size, 2, 0));                               // only the header size had been changed, so the footer is the new address
    }

    /* if only the previous block is free */
    else if (next_alloc && !prev_alloc) {
        delete_node(PREV_BLKP(bp));
	SET_PREV_FREE(HDRP(NEXT_BLKP(bp)));
        /* newsize = current size + previous size */
        size += GET_SIZE(FTRP(PREV_BLKP(bp)));
        /* change the size of previous header and current footer */
        size_t new_prev_block = GET_PREV_ALLOC(HDRP(PREV_BLKP(bp)));
        PUT(FTRP(bp), PACK_ALL(size, new_prev_block, 0));
        PUT(HDRP(PREV_BLKP(bp)), PACK_ALL(size, new_prev_block, 0));
        /* bp point to the previous block's payload */
        bp = PREV_BLKP(bp);
    }

    /* if the prev and next blocks are both free */
    else {
        delete_node(PREV_BLKP(bp));
        delete_node(NEXT_BLKP(bp));
        /* newsize = previous size + current size + next size */
        size += GET_SIZE(FTRP(PREV_BLKP(bp))) + GET_SIZE(HDRP(NEXT_BLKP(bp)));
        size_t new_pre_block = GET_PREV_ALLOC(HDRP(PREV_BLKP(bp)));
        /* change the size of previous header and the next footer */
        PUT(HDRP(PREV_BLKP(bp)), PACK_ALL(size, new_pre_block, 0));
        PUT(FTRP(NEXT_BLKP(bp)), PACK_ALL(size, new_pre_block, 0));
        /* bp point to the previous block's payload */
        bp = PREV_BLKP(bp);
    }
    //CHECKHEAP(__LINE__);
    // and insert new block node to suitable free list
    insert_node(bp);
    return bp;
}

/* put relative address of prev node to current bp */
static inline void set_prev_node(char *curr_bp, void *val) {
    if (val == NULL) {
        *curr_bp = NULL;
    } else {
        *(unsigned int *)(curr_bp) = (long)((char *)(val) - (char *)mem_heap_lo());
    }
}

/* Put relative address of next node to current bp */
static inline void set_next_node(char *curr_bp, void *val) {
    if (val == NULL) {
        *(unsigned int *)((char *)curr_bp + 4) = NULL;
    } else {
        *(unsigned int *)((char *)curr_bp + 4) = (unsigned int)(long)((val) - mem_heap_lo());
    }
}

/*
 * insert a free block to a suitable free list 
 * using LIFO policy, put newly free block at the head of the free list
 */
static inline void insert_node(void *bp) {
    // according size determine suitable free list number
    size_t size = GET_SIZE(HDRP(bp));
    size_t index = find_index(size);
    // save the head block of the suitable free list, if not NULL change the prev pointer points to bp 
    char *old_head = free_lists[index];
    // change free_list[index] point to bp
    free_lists[index] = bp;
    if (old_head != NULL) {
        //set_prev_node(old_head, bp);
        PUT(old_head, (long)((char *)bp - (char *)mem_heap_lo()));
        PUT((unsigned int *)bp + 1, (long)((char *)old_head - (char*)mem_heap_lo()));
    } else {
        PUT((unsigned int *)bp + 1, NULL);
    }
    // change bp's prev pointer points to NULL, next pointer points to the old head block
    //set_prev_node(bp, NULL);
    //set_next_node(bp, old_head);
    PUT(bp, NULL);
    //PUT((unsigned int *)bp + 1, NULL);
}

static inline void delete_node(void *bp) {
    size_t size = GET_SIZE(HDRP(bp));
    size_t index = find_index(size);
    char *prev_block_bp;
    char *next_block_bp;
    char *old_bp = bp;

    printf("in delete node, delete node bp at %p, size is %d\n", bp, size);
    printf("in delete node, bp is %p, GET_PRE(bp) is %p, actual is %p\n", bp, GET_PRE(bp), *(unsigned int *)bp);
    printf("in delete node, bp is %p, GET_SUC(bp) is %p, actual is %p\n", bp, GET_SUC(bp), *((unsigned int *)bp + 1));
    if (GET_PRE(bp) == NULL) {
        // if bp is only node in the free list, only change the head of list to NULL
        if (GET_SUC(bp) == NULL) {
            free_lists[index] = NULL;
        } else {
            // if bp has no prev but has success, set new head to next block and 
            // change the prev pointer of the next block to NULL
            free_lists[index] = GET_SUC_ADDRS(bp);
	    printf("in delete node, free_lists[%d] = GET_SUC_ADDRS(bp); is equal %p\n", index, GET_SUC_ADDRS(bp));
            next_block_bp = GET_SUC_ADDRS(bp);
            //set_prev_node(next_block_bp, NULL);
	    PUT(next_block_bp, NULL);
	    printf("delete node's bp at %p\n", bp);
	    //printf("delete nodebp's prev bp is at %p\n", prev_block_bp + mem_heap_lo());
  	    printf("delete nodebp's next bp is at %p\n", next_block_bp);
        }    
    } else if (GET_SUC(bp) == NULL) {
        // if bp has no success block, change the previous block's succs pointer to NULL
        prev_block_bp = GET_PRE_ADDRS(bp);
        printf("[%d] check the correctness of prev block bp's size in delete node: %d\n", __LINE__, GET_SIZE(HDRP(prev_block_bp)));
        //set_next_node(prev_block_bp, NULL);
        //bp = prev_block_bp;
        PUT((char *)prev_block_bp + 1, NULL);
    } else {
        // if bp in the middle of the free list, change the prev block of bp's next pointer points to bp's next pointer
        // and change the next block's prev pointer points to bp's prev pointer
        //prev_block_bp = GET_PRE_ADDRS(bp);
        //next_block_bp = GET_SUC_ADDRS(bp);
        //set_next_node(prev_block_bp, next_block_bp);
        //set_prev_node(next_block_bp, prev_block_bp);
        prev_block_bp = GET_PRE(bp);
        next_block_bp = GET_SUC(bp);
        bp = NEXT_BLKP(bp);
        PUT(bp, (long)((char *)prev_block_bp - (char *)mem_heap_lo()));
        bp = PREV_BLKP(bp);
        PUT((char *)bp + 1, (long)((char *)next_block_bp - (char *)mem_heap_lo()));
    }
    PUT(old_bp, NULL);
    PUT((char *)old_bp + 1, NULL);
}

/* According size find suitable free list number 
 * 0-16， 17-31. 32-47， 48-95，
 */
static inline size_t find_index(size_t size) {
    int i;
    for(i = 4; i <=22; i++){
        if(size <= (1 << i))
            return i-4;
    }
    return i-4;
}

/* 
 * mm_malloc - Allocate a block by incrementing the brk pointer.
 *     Always allocate a block whose size is a multiple of the alignment.
 */
void *mm_malloc(size_t size)
{
    unsigned int head, foot;
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
        // make sure add extra WSIZE to play as head
        //asize = WSIZE + ((size + (DSIZE - 1)) / DSIZE) * DSIZE;
        asize = DSIZE * ((size + (WSIZE)+(DSIZE - 1)) / DSIZE);
        printf("malloc size: %zu\n", size);
        printf("malloc asize: %zu\n", asize);
    }
    /* find if the free list has a free block can hold asize(find_fit) */
    if ((bp = find_fit(asize)) != NULL) {
        place(bp, asize);
        printf("malloc after find, bp at %p\n", bp);
        head = GET(HDRP(bp));
        CHECKHEAP(__LINE__);
        return bp;
    }
    /* if not find call extend_heap, put this request block to the new free block */
    extendsize = MAX(asize, CHUCKSIZE);
    if ((bp = extend_heap(extendsize/WSIZE)) != NULL) {
        head = GET(HDRP(bp));
        foot = GET(FTRP(bp));
        printf("malloc after extend_heap bp at %p\n", bp);
        CHECKHEAP(__LINE__);
        /*  put this request block to the fit free block and splitting the block 
        if rest block is satisfy the minimum request (16 bytes?) */
        place(bp, asize);
    } else {
        return NULL;
    }
    CHECKHEAP(__LINE__);
    return bp;
}

/* 
 * Search the free list for a suitable free block.
 * first try --- first fit
 * Search list from beginning, choose first free block that fits.
 */
static inline void *find_fit(size_t size) {
    size_t index = find_index(size);
    char *list_head;
    char *bp;
    size_t curr_size;
    for (; index < CLASS_SIZE; index++) {
        bp = free_lists[index];
        while (bp != NULL) {
            curr_size = GET_SIZE(HDRP(bp));
            if (curr_size >= size)
                return bp;
            if (GET_SUC(bp) != NULL) {
                bp = GET_SUC_ADDRS(bp);
            } else {
                // if suc pointer points to NULL, go for next free list
                break;
            }
        }
    }
    return NULL;
}

/*
 * Place the request block and optionally splits the excess if rest block is satisfy 
 * the minimum request (16 bytes?) then returns the address of the newly allocated block. 
 * delete new placed block and insert new free block if exists
 */
static inline void place(void *bp, size_t size) {
    /* set minisizefree = 2 * DSIZE */
    size_t splitsize = 2 * DSIZE;
    size_t currsize = GET_SIZE(HDRP(bp));
    size_t difsize = currsize - size;
    //CHECKHEAP(__LINE__);
    /* if bp block size - size >= minisizefree split the free block, allocated size block 
    : 1. change the allocated block's head 2. delete it from the free list
    the rest free block: 
     1. change the next block's prev alloc to allocted 2. change head and foot 3. insert node in a suitable free list */
    delete_node(bp);
    if (difsize >= splitsize) {
        // change the allocated block's head
        PUT(HDRP(bp), PACK_ALL(size, GET_PREV_ALLOC(HDRP(bp)), 1));
        printf("in place function, bp header at %p\n", HDRP(bp));
        printf("in place function, bp size is %d\n", GET_SIZE(HDRP(bp)));

        // change free head and foot
        PUT(HDRP(NEXT_BLKP(bp)), PACK_ALL(difsize, 2, 0));
        PUT(FTRP(NEXT_BLKP(bp)), PACK_ALL(difsize, 2, 0));

        // insert free node in a suitable free list
        insert_node(NEXT_BLKP(bp));
        printf("in place function, bp next head at %p\n", HDRP(NEXT_BLKP(bp)));
        printf("in place function, bp next block size is %d\n", GET_SIZE(HDRP(NEXT_BLKP(bp))));
        printf("in place function, bp next footer at %p\n", FTRP(NEXT_BLKP(bp)));
    } else {
        /* if block size - size < minisizefree not split, 1. just change bp's header, 2. delete node from free list 
        , 3. change the next block's prev alloc to allocted*/
        PUT(HDRP(bp), PACK_ALL(currsize, GET_PREV_ALLOC(HDRP(bp)), 1));
        // change the next block's prev alloc to allocted
        SET_PREV_ALLOC(HDRP(NEXT_BLKP(bp)));
        // if next block is free block, set footer prev alloc either(maybe no need?)
        if (!GET_ALLOC(HDRP(NEXT_BLKP(bp)))) {
            SET_PREV_ALLOC(FTRP(NEXT_BLKP(bp)));
        }
        //printf("in place function, bp header at %p\n", HDRP(bp));
        //printf("in place function, bp footer at %p\n", FTRP(bp));
    }
    //CHECKHEAP(__LINE__);
}

/*
 * mm_free - Freeing a block does nothing.
 * 1. change head, add foot
 * 2. coalescing
 * 3. insert new free block
 * 4. set next block's head prev alloc to 0, if next block's size > 0 and is free block set foot prev
 *  alloc to 0 either 
 */
void mm_free(void *ptr)
{
    size_t size;
    CHECKHEAP(__LINE__);
    printf("free bp %p\n, free size %u\n", ptr, GET_SIZE(HDRP(ptr)));
    if (ptr == NULL)
        return;
    size = GET_SIZE(HDRP(ptr));
    /* change the allocated bit of bp's header and footer */
    PUT(HDRP(ptr), PACK_ALL(size, GET_PREV_ALLOC(HDRP(ptr)), 0));
    PUT(FTRP(ptr), PACK_ALL(size, GET_PREV_ALLOC(HDRP(ptr)), 0));
    /* coalescing bp */
    char * after_coales_bp = coalesce(ptr);
    CHECKHEAP(__LINE__);
    //int new_size = GET_SIZE(HDRP(after_coales_bp));
    //size_t num = find_index(new_size);
    //if (free_lists[num] != after_coales_bp) {
    //  fprintf("Free list Error: after free free_lists[%d] = %p is not the correct bp: %p after coalescing\n", num, free_lists[num], after_coales_bp);
    //}
}

/*
 * mm_realloc - Implemented simply in terms of mm_malloc and mm_free
 * if ptr is NULL, call malloc(size) 
 * if size == 0, call free(ptr) 
 * if ptr is NULL, call malloc(size)request size bytes memory from heap,
 * if pre is not NULL, using memcpy copy min(size, ptr's size) and free ptr return newptr
 */
void *mm_realloc(void *ptr, size_t size) 
{
    void *newptr;
    size_t copysize;
    printf("in realloc where prt is %p, size is %d\n", ptr, size);
    if ((newptr = mm_malloc(size)) == NULL)
        return NULL;
    //printf("checkcheck\n");
    if (ptr == NULL) 
        return newptr;
    copysize = MIN(GET_SIZE(HDRP(ptr)), size);
    memcpy(newptr, ptr, copysize);
    mm_free(ptr);
    CHECKHEAP(__LINE__);
    return newptr;
}

/* check the correct of the heap */
void mm_checkheap(int lineno)
{
    // get the beginning
    char *bp = mem_heap_lo() + DSIZE * CLASS_SIZE;
    //check the start 
    if (GET(bp) != 0) {
        printf("[%d] Prologue Error: word before prolofue incorrect at %p\n", lineno, bp);
        exit(1);
    }

    // check prologue's header and footer
    if (GET(bp + WSIZE) != PACK(DSIZE, 1)) {
        printf("[%d] Prologue Error: prologue's header incorrect at %p\n", lineno, bp);
        exit(1);
    }
    if (GET(bp + DSIZE) != PACK(DSIZE, 1)) {
        printf("[%d] Prologue Error: prologue's footer incorrect at %p\n", lineno, bp);
        exit(1);
    }

    // move bp point to the effective part of the blocks that next to the prologue block
    bp += 2 * DSIZE;

    size_t is_prev_free = 0;
    size_t is_prev_alloc = 1;
    size_t curr_alloc;
    size_t prev_alloc;

    while ((void *)bp < mem_heap_hi()) {
        curr_alloc = GET_ALLOC(HDRP(bp));
        prev_alloc = GET_PREV_ALLOC(HDRP(bp));
        // check the block's alignment
        if (!CHECK_ALIGN(bp)) {
            printf("[%d] Align Error: block not align at %p\n", lineno, bp);
            exit(1);
        }

        //for free block check block, head and footer information of the block are the same
        if (curr_alloc == 0 && GET(HDRP(bp)) != GET(FTRP(bp))) {
            printf("[%d] Block Error: block header and footer are not matched at %p\n", lineno, bp);
            printf("-- the dismatch header and footer at %p is %zu and %zu\n", bp, GET(HDRP(bp)), GET(FTRP(bp)));
            exit(1);
        }
        
        // check the previous block is right settle
        if (is_prev_alloc != 1 && prev_alloc != is_prev_alloc) {
            printf("[%d] Block Error: next block's previous block status should be %d, but now is %d, current bp is %p\n", lineno, is_prev_alloc, prev_alloc, bp);
            exit(1);
        }
        //except for the epilogue block, the block size is not allowed to be 0
        if (GET_SIZE(HDRP(bp)) == 0) {
            printf("[%d] Block Error: block size is illegal at %p\n", lineno, bp);
            exit(1);
        }

        //check the allocated block
        if (curr_alloc == 1) {
            is_prev_alloc = 2;
            is_prev_free = 0;
        }

        // check the unallocated block 
        if (curr_alloc == 0) {
            // make sure no continuous free blocks by checking the next block's allocated state
            if (is_prev_free) {
                printf("[%d] List Error: contiguous free block is not allowed at %p\n", lineno, bp);
            }
            is_prev_alloc = 0;
            is_prev_free = 1;
        }

        bp = NEXT_BLKP(bp);
    }
    //check the imformation about epilogue block
    // check the epilougue block size equal 0
    if (GET_SIZE(HDRP(bp)) != 0) {
        printf("[%d] Epilogue Error: the size of epilogue block is not 0 at %p\n", lineno, bp);
    }
    // check epilogue allocate status is 1
    if (GET_ALLOC(HDRP(bp)) != 1) {
        printf("[%d] Epilogue Error: the allocated of epilogue block is wrong at %p\n", lineno, bp);
    }
    // check the epilogue block's previous allocated status is correctly set
    if (GET_PREV_ALLOC(HDRP(bp)) != is_prev_alloc) {
        printf("[%d] Epilogue Error: the allocated of epilogue block is wrong at %p\n", lineno, bp);
    }

    // make sure the curent bp is not beyond the boudary of heap
    if ((void *)bp > mem_heap_hi() + 1) {
        printf("[%d] Heap Error: point beyond the heap boudary at %p\n", lineno, bp);
        exit(1);
    }

    // check the alignment again 
    if (!CHECK_ALIGN(bp)) {
        printf("[%d] Align Error: block not align at %p\n", lineno, bp);
        exit(1);
    }

    for (int i = 0; i < 20; i++) {
      char *p = free_lists[i];
      if (p != NULL && GET_PRE(p) != NULL) {
        printf("[%d]Free List Error: free list num: %d has non-NULL prev head, actual was: %p, the head at %p\n", lineno, i, GET_PRE(p), p);
      }
    }
    mm_checkfreelist(lineno);
}

/* Check the correctness of free list */
void mm_checkfreelist(int lineno) {
  char * bp;
  char * next_bp;
  char * prev_bp;
  size_t free_list_blocks_num = 0;
  size_t free_blocks_num = 0;

  for (int i = 0; i < 20; i++) {
    bp = free_lists[i];
    while (bp != NULL) {
        size_t size = GET_SIZE(HDRP(bp));
        // for every blocks in `free_lists[index]`, size is satisfy this `index`
        if (find_index(size) != i) {
            printf("[%d] Free List Error: block size not match index at %p\n", lineno, bp);
            printf("-----block size is: %zu, index should be: %d, actual was: %d\n", size, find_index(size), i);
            //exit(1);
        }
        // every block must not be allocted
        if (GET_ALLOC(HDRP(bp)) == 1) {
            printf("[%d] Free List Error: block should not be allocated at %p\n", lineno, bp);
            //exit(1);
        }
        if (GET_SUC(bp) == NULL) {
            ++free_list_blocks_num;
            break;
        } else {
            next_bp = GET_SUC_ADDRS(bp);
            // if next block != `NULL`, next block's prev block must be current `bp`
            if (bp != GET_PRE_ADDRS(next_bp)) {
                printf("[%d] Free List Error: next block bp's prev block bp is not match current bp, current bp at %p\n", lineno, bp);
                printf("--------this dismatch happen at index: %d, next_bp at: %p, next bp's prev_bp at: %p\n", find_index(size), next_bp, GET_PRE_ADDRS(next_bp));
                printf("--------this wrong prev block get from GET_PRE(bp): %p\n", GET_PRE(next_bp));
                //exit(1);
            }
        }
        bp = next_bp;
        // count the number of all blocks
        ++free_list_blocks_num;
    }
  }

    // count all free blocks and check the numebr is the same as the number of free lists
    bp = mem_heap_lo() + CLASS_SIZE * DSIZE + 2 * WSIZE;
    for (; GET_SIZE(HDRP(bp)) != 0; bp = NEXT_BLKP(bp)) {
        if (GET_ALLOC(HDRP(bp)) == 0) {
            ++free_blocks_num;
            }
    }
    
    if (free_list_blocks_num != free_blocks_num) {
        printf("[%d] Free List Error: not all free block in free lists %p\n", lineno, bp);
        printf("----all free  blocks in free lists are: %d, all free blocks in heap are: %d\n", free_list_blocks_num, free_blocks_num);
        exit(1);
    }
}
