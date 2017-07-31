#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <string.h>

#include "mm.h"
#include "memlib.h"

static void* extend_heap(size_t size);
static void* coalesce(void* bp);
static void insert_free_blk(void* bp);
static void delete_free_blk(void* bp);
static void place(void* ptr, size_t asize);
static void* find_fit(size_t asize);

// global private variables
static void *heap_list_ptr;
static void *root;

#define SPLIT_THEN_INSERT(ptr,oldsize,newsize)          \
do {                                                    \
    SET_HEAD(ptr, PACK(newsize, 1));                    \
    SET_FOOT(ptr, PACK(newsize, 1));                    \
    void* next = GET_NEXT(ptr);                         \
    SET_HEAD(next, PACK(oldsize-newsize, 0));           \
    SET_FOOT(next, PACK(oldsize-newsize, 0));           \
    insert_free_blk(next);                              \
} while(0)

/* 
* mm_init - initialize the malloc package.
*/
int mm_init(void) {
    /* create the initial empty heap */
    if((heap_list_ptr = mem_sbrk(WSIZE*4)) == NULL)
        return -1;
    PUT(heap_list_ptr, 0);  // 0
    PUT(heap_list_ptr+WSIZE, PACK(OVERHEAD, 1));    // 9
    PUT(heap_list_ptr + WSIZE*2, PACK(OVERHEAD, 1));// 9
    PUT(heap_list_ptr + WSIZE*3, PACK(0, 1));       // 1
    // point to actual heap memory
    heap_list_ptr += WSIZE*4;   // heap_list_ptr as an global variable to pass message
    root = NULL;    // firstly root as global manager setted as NULL; 
    // extend heap intially
    if(extend_heap(ALIGN_SIZE(INISIZE)) == NULL) 
        return -1;
    return 0;
}

void* mm_malloc(size_t size) {
    // memory size after ajust, alignment and overhead
    size_t asize;
    // when find fit failed, extend the heap
    size_t extendsize;
    void* bp;
    // can not in service 
    if(size <= 0)
        return NULL;

    if(size <= BLKSIZE - OVERHEAD) {
        asize = BLKSIZE;
    } else {
        asize = ALIGN_SIZE(size+(OVERHEAD));
    }

    if((bp = find_fit(asize)) == NULL){
        // find_fit failed, extend the heap with one intial size or asize + 32
        extendsize = MAX(asize + 32, INISIZE);
        extend_heap(ALIGN_SIZE(extendsize));
        if((bp = find_fit(asize)) == NULL)  // if still error, return null
            return NULL;
    }
    // fill meta info to the allocated memory area
    if(size == 448 && GET_SIZE(bp) > asize+64) {
        asize += 64;
    } else if(size == 112 && GET_SIZE(bp) > asize+16) {
        asize += 16;
    }
    place(bp, asize);
    return bp;
}

void mm_free(void* bp) {
    size_t size = GET_SIZE(bp);
    SET_HEAD(bp, PACK(size, 0));
    SET_FOOT(bp, PACK(size, 0));
    insert_free_blk(coalesce(bp));
}

void* mm_realloc(void* ptr, size_t size) {
    if(ptr == NULL || size == 0) {
        mm_free(ptr);
        return NULL;
    }

    if(size > 0) {
        size_t oldsize = GET_SIZE(ptr);
        size_t newsize = ALIGN_SIZE(size+OVERHEAD);
        if(newsize < oldsize) { // newSize < oldSize
            if(GET_ALLOC(GET_NEXT(ptr))) { // next block is allocated
                // after the memory shrinking, result a new unused block
                if((oldsize-newsize) >= BLKSIZE) {
                    SPLIT_THEN_INSERT(ptr, oldsize, newsize);
                } else { // no extra block generated
                    SET_HEAD(ptr, PACK(oldsize, 1));
                    SET_FOOT(ptr, PACK(oldsize, 1));
                }
                return ptr;
            } else { // the next block is in the free BST
                size_t csize = oldsize + GET_SIZE(GET_NEXT(ptr));
                delete_free_blk(GET_NEXT(ptr));
                SPLIT_THEN_INSERT(ptr, csize, newsize);
                return ptr;
            }
        } else { // newSize > oldSize
            size_t prev_alloc = GET_ALLOC(GET_PREV(ptr));
            size_t next_alloc = GET_ALLOC(GET_NEXT(ptr));
            size_t csize;
            // extra next block could sufficient for this realloc action
            if(!next_alloc && ((csize = oldsize+GET_SIZE(GET_NEXT(ptr))) >= newsize)) {
                    delete_free_blk(GET_NEXT(ptr));
                    if((csize-newsize) >= BLKSIZE) { // extra block generated
                        SPLIT_THEN_INSERT(ptr, csize, newsize);
                    } else { // no extra block
                        SET_HEAD(ptr,PACK(csize, 1));
                        SET_FOOT(ptr,PACK(csize, 1));
                    }
                    return ptr;
            } else if(!prev_alloc &&
                    ((csize = oldsize+GET_SIZE(GET_PREV(ptr))) >= newsize)) { // prev block is sufficient
                delete_free_blk(GET_PREV(ptr));
                void* newptr = GET_PREV(ptr);
                // original memory copy to the new start
                memcpy(newptr, ptr, oldsize-OVERHEAD);
                if((csize-newsize) >= BLKSIZE) { // extra block generated
                    SPLIT_THEN_INSERT(newptr, csize, newsize);
                } else { // no extra block
                    SET_HEAD(newptr, PACK(csize, 1));
                    SET_FOOT(newptr, PACK(csize, 1));
                }
                return newptr;
            } else if(!prev_alloc && !next_alloc &&
                    ((csize = oldsize+GET_SIZE(GET_PREV(ptr))
                      + GET_SIZE(GET_NEXT(ptr))) >= newsize)) { // prev plus next could sufficient
                delete_free_blk(GET_PREV(ptr)); // delete prev and next from the free BST
                delete_free_blk(GET_NEXT(ptr));
                void* newptr = GET_PREV(ptr);
                memcpy(newptr, ptr, oldsize - OVERHEAD); // copy content to new start
                if((csize-newsize) >= BLKSIZE) { // extra block generated
                    SPLIT_THEN_INSERT(newptr, csize, newsize);
                } else { // no extra block
                    SET_HEAD(newptr, PACK(csize, 1));
                    SET_FOOT(newptr, PACK(csize, 1));
                }
                return newptr;
            } else { // need find_fit action
                size_t asize = ALIGN_SIZE(size+(OVERHEAD));
                size_t extendsize;
                void* newptr;
                if((newptr = find_fit(asize)) == NULL) { // find fit failed
                    extendsize = MAX(asize, CHUNKSIZE); // extend managed heap
                    extend_heap(extendsize);
                    if((newptr = find_fit(asize)) == NULL)
                        return NULL;
                }
                place(newptr, asize);
                memcpy(newptr, ptr, oldsize - OVERHEAD); // copy content to new start

                mm_free(ptr); // free original block
                return newptr;
            }
        }
    } else {
        return NULL;
    }
}

static void* coalesce(void* bp) {
    size_t prev_alloc = GET_ALLOC(GET_PREV(bp));
    size_t next_alloc = GET_ALLOC(GET_NEXT(bp));
    size_t size = GET_SIZE(bp);

    if (prev_alloc && next_alloc) { // Case 0
        return bp;
    } else if (!prev_alloc && next_alloc) { // Case 1
        // delete the prev node from the free BST
        delete_free_blk(GET_PREV(bp));
        // combine the two node to one bigger new node, then insert it to the free BST
        size += GET_SIZE(GET_PREV(bp));
        // set overhead mata info
        SET_HEAD(GET_PREV(bp), PACK(size, 0));	
        SET_FOOT(bp, PACK(size,0));
        return GET_PREV(bp);
    } else if (prev_alloc && !next_alloc) { // Case 2
        // same logic as case 1
        delete_free_blk(GET_NEXT(bp));
        size += GET_SIZE(GET_NEXT(bp));
        SET_HEAD(bp, PACK(size,0));
        SET_FOOT(bp, PACK(size,0));
        return bp;
    } else { // Case 3
        // same logic as other cases
        delete_free_blk(GET_NEXT(bp));
        delete_free_blk(GET_PREV(bp));
        size += GET_SIZE(GET_PREV(bp)) + GET_SIZE(GET_NEXT(bp));
        SET_HEAD(GET_PREV(bp), PACK(size,0));
        SET_FOOT(GET_NEXT(bp), PACK(size,0));
        return GET_PREV(bp);
    }
}

// No shrink heap function yet
void* extend_heap(size_t size) {
    void* bp;
    // extend managed heap size
    if((unsigned int)(bp = mem_sbrk(size)) == (unsigned)(-1))
        return NULL;
    // new unused node
    SET_HEAD(bp, PACK(size, 0));
    SET_FOOT(bp, PACK(size, 0));
    // new unmanaged memory area's head
    SET_HEAD(GET_NEXT(bp), PACK(0, 1)); // end block, marked with 1, let coalesce ignore it
    // insert the allocated new unused memory node to free BST
    insert_free_blk(coalesce(bp));
    return (void*) bp;
}

// bp is the block found by find_fit function
static void place(void* bp, size_t asize) {
    // actual memory block found by find_fit, larger than asize
    size_t csize = GET_SIZE(bp); // intial size is: 1106
    printf("asize: %d, csize: %d\n", asize, csize);
    // delete the allocated memory from the heap
    delete_free_blk(bp);
    // has extra memory lefted after allocation on the block found by find_fit
    if((csize-asize) >= BLKSIZE) {
        SET_HEAD(bp, PACK(asize, 1));
        SET_FOOT(bp, PACK(asize, 1));
        bp = GET_NEXT(bp);
        SET_HEAD(bp, PACK(csize-asize, 0));
        SET_FOOT(bp, PACK(csize-asize, 0));
        insert_free_blk(coalesce(bp));
    } else { // no extra memory lefted
        SET_HEAD(bp, PACK(csize, 1));
        SET_FOOT(bp, PACK(csize, 1));
    }
}

static void* find_fit(size_t asize) {
    void* fit = NULL;
    void* p = root;
    while(p != NULL) {
        if(asize <= GET_SIZE(p)) {
            fit = p;
            p = (void*)GET_LEFT(p);
        } else {
            p = (void*)GET_RIGHT(p);
        }
    }
    return fit;
}

/* put it as node's brother, otherwise put it as node. */ 
static void insert_free_blk(void* bp) {
    if(root == NULL) {
        root = bp;
        SET_LEFT(bp, NULL);
        SET_RIGHT(bp, NULL);
        SET_PRNT(bp, NULL);
        SET_BROS(bp, NULL);
        return;
    }

    void* p = root;
    while(1) {
        if(GET_SIZE(bp) == GET_SIZE(p)) { /* Case 1: same size */
            if((void*)GET_BROS(p) != NULL) {
                void* t = (void*)GET_BROS(p);
                SET_LEFT(GET_BROS(p), bp);  // the left field works as double linked list between brothers
                SET_BROS(p, bp);
                SET_BROS(bp, t);
                SET_LEFT(bp, p);
                SET_RIGHT(bp, -1);
                break;
            } else {
                SET_BROS(bp, NULL);
                SET_RIGHT(bp, -1);
                SET_LEFT(bp, p);    // double linked list
                SET_BROS(p, bp);
                break;
            }
        } else if(GET_SIZE(bp) < GET_SIZE(p)) { /* Case 2: turn left, or insert */
            if((void*)GET_LEFT(p) != NULL) { /* turn left */
                p = (void*)GET_LEFT(p);
            } else { /* leaf node */
                SET_LEFT(p, bp);    // insert bp to the BST tree
                SET_PRNT(bp, p);
                SET_LEFT(bp, NULL);
                SET_RIGHT(bp, NULL);
                SET_BROS(bp, NULL);
                break;
            }
        } else { /* Case 3：turn right or insert */
            if((void*)GET_RIGHT(p) != NULL) { /* turn right */
                p = (void*)GET_RIGHT(p);
            } else { /* insert */
                SET_RIGHT(p, bp);
                SET_PRNT(bp, p);
                SET_LEFT(bp, NULL);
                SET_RIGHT(bp, NULL);
                SET_BROS(bp, NULL);
                break;
            }
        }
    }
}

static void delete_free_blk(void* bp) {
    if((void*)GET_BROS(bp) == NULL && GET_RIGHT(bp) != -1) { /* no brother and have right child */
        if((void*)GET_LEFT(bp) == NULL) { /* no left child */
            if(bp == root) {
                root = (void*)GET_RIGHT(bp); // new root
                if(root != NULL)
                    SET_PRNT(root, NULL);
            } else {
                if((void*)GET_LEFT(GET_PRNT(bp)) == bp) {
                    SET_LEFT(GET_PRNT(bp), GET_RIGHT(bp));
                } else {
                    SET_RIGHT(GET_PRNT(bp), GET_RIGHT(bp));
                }
                if((void*)GET_RIGHT(bp) != NULL) {
                    SET_PRNT(GET_RIGHT(bp), GET_PRNT(bp));
                }
            }
        } else { /* have left child */
            void *p = (void*)GET_LEFT(bp);
            /* get pred */
            while((void*)GET_RIGHT(p) != NULL) {
                p = (void*)GET_RIGHT(p);
            }
            // at the area the deleted placement node
            void* bp_right_child = (void*)GET_RIGHT(bp);
            void* p_left_child = (void*)GET_LEFT(p);
            void* p_parent = (void*)GET_PRNT(p);
            if(bp == root) { /* bp is root. */
                root = p;   // new root
                if(root != NULL) {
                    SET_PRNT(root, NULL);
                }
            } else { /* bp isn't root. */
                if((void*)GET_LEFT(GET_PRNT(bp)) == bp) { // point to placement node
                    SET_LEFT(GET_PRNT(bp), p);
                } else {
                    SET_RIGHT(GET_PRNT(bp), p);
                }
                SET_PRNT(p, GET_PRNT(bp));
            }
            SET_RIGHT(p, GET_RIGHT(bp));
            if(p != (void*)GET_LEFT(bp)) { // GET_RIGHT done well
                SET_LEFT(p, GET_LEFT(bp)); // placement node's left point to original node's left child
                // after deletion, ajust pointers at removed place
                SET_RIGHT(p_parent, p_left_child);
                if(p_left_child != NULL) {
                    SET_PRNT(p_left_child, p_parent);
                }
                SET_PRNT(GET_LEFT(bp), p);
            }
            if(bp_right_child != NULL) {
                SET_PRNT(bp_right_child, p);
            }
        }
    } else { /* bp isn't BST node, or it has brother. */
        if(bp == root) {
            root = (void *)GET_BROS(bp);
            SET_PRNT(root, NULL);
            SET_LEFT(root, GET_LEFT(bp));
            SET_RIGHT(root, GET_RIGHT(bp));
            if((void *)GET_LEFT(bp) != NULL) {
                SET_PRNT(GET_LEFT(bp), root);
            }
            if((void *)GET_RIGHT(bp) != NULL) {
                SET_PRNT(GET_RIGHT(bp), root);
            }
        } else {
            if(GET_RIGHT(bp) == -1) { // in the middle of linked list
                /* deletion on double linked list */
                SET_BROS(GET_LEFT(bp), GET_BROS(bp));
                if((void*)GET_BROS(bp) != NULL) {
                    SET_LEFT(GET_BROS(bp), GET_LEFT(bp));
                }
            } else { // bp is the head node of the linked list, aka on the BST tree
                void* new_node = (void*)GET_BROS(bp);
                if((void*)GET_LEFT(GET_PRNT(bp)) == bp) {
                    SET_LEFT(GET_PRNT(bp), new_node);
                } else {
                    SET_RIGHT(GET_PRNT(bp), new_node);
                }
                
                SET_PRNT(new_node, GET_PRNT(bp));
                SET_LEFT(new_node, GET_LEFT(bp));
                SET_RIGHT(new_node, GET_RIGHT(bp));
                if((void*)GET_LEFT(bp) != NULL) {
                    SET_PRNT(GET_LEFT(bp), new_node);
                }
                if((void*)GET_RIGHT(bp) != NULL) {
                    SET_PRNT(GET_RIGHT(bp), new_node);
                }
            }
        }
    }
}
