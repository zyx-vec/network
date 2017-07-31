#ifndef __MM_H
#define __MM_H

#include <stdio.h>

extern int mm_init(void);
extern void* mm_malloc(size_t size);
extern void mm_free(void* ptr);
extern void* mm_realloc(void* ptr, size_t size);

/* basic constants and macros */
#define WSIZE       4           // single word, 4 bytes
#define OVERHEAD    8           // header and footer as overhead for allcated bytes
#define ALIGNMENT   8           // alignment request, 8 bytes
#define BLKSIZE     24          // minimal block size, only meta info for memory manager
#define CHUNKSIZE   (1 << 12)   // initial heap size
#define INISIZE     1016        // intial heap extended size

#define MAX(x, y) ((x) > (y)? (x) : (y))
#define MIN(x, y) ((x) < (y)? (x) : (y))

#define GET(p)	    (*(unsigned int*)(p))
#define PUT(p, val) (*(unsigned int*)(p) = val)

#define SIZE(p)     (GET(p) & ~0x7)     // aligned: 8 bytes
#define ALLOC(p)    (GET(p) & 0x1)      // final bit for allocated or not

/* pack: aligned size or allocated bit */
#define PACK(size, alloc) ((size) | (alloc))

/* meta info offsets, see the table below for details, for memory manager. 4*6 == 24(bytes) */
#define HEAD(p)     ((void*)(p) - WSIZE)
#define LEFT(p)     ((void*)(p))
#define RIGHT(p)    ((void*)(p) + WSIZE)
#define PRNT(p)     ((void*)(p) + WSIZE*2)
#define BROS(p)     ((void*)(p) + WSIZE*3)
#define FOOT(p)     ((void*)(p) + SIZE(HEAD(p)) - WSIZE*2)

//      +--------+
//      |        |  <-------- HEAD, size | alloc, size := usage memory + sizeof meta info
//      +--------+
//      |        |  <-------- LEFT(p)
//      +--------+
//      |        |  <-------- RIGHT
//      +--------+
//      |        |  <-------- PARENT
//      +--------+
//      |        |  <-------- BROTHERS
//      +--------+
//      |  ....  |  <-------- actual memory bytes, allocated block only has 8bytes overhead, head and foot
//      +--------+
//      |        |  <-------- FOOTER, size | alloc
//      +--------+


// read meta info
#define GET_SIZE(bp)    ((GET(HEAD(bp))) & ~0x7)
#define GET_PREV(bp)    ((void*)(bp) - SIZE(((void*)(bp) - WSIZE*2)))
#define GET_NEXT(bp)    ((void*)(bp) + SIZE(HEAD(bp)))
#define GET_ALLOC(bp)   (GET(HEAD(bp)) & 0x1)
#define GET_LEFT(bp)    (GET(LEFT(bp)))
#define GET_RIGHT(bp)   (GET(RIGHT(bp)))
#define GET_PRNT(bp)    (GET(PRNT(bp)))
#define GET_BROS(bp)    (GET(BROS(bp)))
#define GET_FOOT(bp)    (GET(FOOT(bp)))

// write meta info
#define SET_HEAD(bp, val)   (PUT(HEAD(bp), (int)val))
#define SET_FOOT(bp, val)   (PUT(FOOT(bp), (int)val))
#define SET_LEFT(bp, val)   (PUT(LEFT(bp), (int)val))
#define SET_RIGHT(bp, val)  (PUT(RIGHT(bp), (int)val))
#define SET_PRNT(bp, val)   (PUT(PRNT(bp), (int)val))
#define SET_BROS(bp, val)   (PUT(BROS(bp), (int)val))

// alignment
#define ALIGN_SIZE(size)    (((size) + (ALIGNMENT-1)) & ~0x7)


#endif

