#define _GNU_SOURCE

#include "brk.h"
#include <unistd.h>
#include <string.h> 
#include <errno.h> 
#include <sys/mman.h>
#include <stdio.h>

#define NALLOC 1024                                     /* minimum #units to request */

#define STRATEGY_FIRST_FIT      1						/* used to make malloc use strategy first-fit */
#define STRATEGY_BEST_FIT       2						/* used to make malloc use strategy best-fit */
#define STRATEGY_WORST_FIT      3           /* used to make malloc use strategy worst-fit */

#ifndef STRATEGY										
    #define STRATEGY 1									/* set first-fit as default strategy */
#endif

#define ANSI_PAGE_SIZE 1								/* used for replacing 'getpagesize()' */

#ifdef ANSI_PAGE_SIZE									/* use the more portable '_SC_PAGE_SIZE' */
    #ifdef _SC_PAGE_SIZE								/* check which syntax for '_SC_PAGE_SIZE' to use */
        #define GET_PAGE_SIZE() sysconf(_SC_PAGE_SIZE)
    #else
        #define GET_PAGE_SIZE() sysconf(_SC_PAGESIZE)
    #endif
#else
    #define GET_PAGE_SIZE() getpagesize()
#endif

typedef long Align;                                     /* for alignment to long boundary */

union header {                                          /* block header */
  struct {
    union header *ptr;                                  /* next block if on free list */
    unsigned size;                                      /* size of this block  - what unit? */ 
  } s;
  Align x;                                              /* force alignment of blocks */
};

typedef union header Header;

static Header base;                                     /* empty list to get started */
static Header *freep = NULL;                            /* start of free list */

/* free: put block ap in the free list */

void free(void * ap)
{
  Header *bp, *p;

  if(ap == NULL) return;                                /* Nothing to do */

  bp = (Header *) ap - 1;                               /* point to block header */
  for(p = freep; !(bp > p && bp < p->s.ptr); p = p->s.ptr)
    if(p >= p->s.ptr && (bp > p || bp < p->s.ptr))
      break;                                            /* freed block at atrt or end of arena */

  if(bp + bp->s.size == p->s.ptr) {                     /* join to upper nb */
    bp->s.size += p->s.ptr->s.size;
    bp->s.ptr = p->s.ptr->s.ptr;
  }
  else
    bp->s.ptr = p->s.ptr;
  if(p + p->s.size == bp) {                             /* join to lower nbr */
    p->s.size += bp->s.size;
    p->s.ptr = bp->s.ptr;
  } else
    p->s.ptr = bp;
  freep = p;
}

/* morecore: ask system for more memory */

#ifdef MMAP

static void * __endHeap = 0;

void * endHeap(void)
{
  if(__endHeap == 0) __endHeap = sbrk(0);
  return __endHeap;
}
#endif

static Header *morecore(unsigned nu)
{
  void *cp;
  Header *up;
#ifdef MMAP
  unsigned noPages;
  if(__endHeap == 0) __endHeap = sbrk(0);
#endif

  if(nu < NALLOC)
    nu = NALLOC;
#ifdef MMAP
  noPages = ((nu*sizeof(Header))-1)/GET_PAGE_SIZE() + 1;
  cp = mmap(__endHeap, noPages*GET_PAGE_SIZE(), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
  nu = (noPages*GET_PAGE_SIZE())/sizeof(Header);
  __endHeap += noPages*GET_PAGE_SIZE();
#else
  cp = sbrk(nu*sizeof(Header));
#endif
  if(cp == (void *) -1){                                 /* no space at all */
    perror("failed to get more memory");
    return NULL;
  }
  up = (Header *) cp;
  up->s.size = nu;
  free((void *)(up+1));
  return freep;
}

void * malloc(size_t nbytes)
{
  Header *p, *prevp;
  Header * morecore(unsigned);
  unsigned nunits;

  if(nbytes == 0) return NULL;

  nunits = (nbytes+sizeof(Header)-1)/sizeof(Header) +1;

  if((prevp = freep) == NULL) {
    base.s.ptr = freep = prevp = &base;
    base.s.size = 0;
  }
  
  if(STRATEGY == STRATEGY_FIRST_FIT) {					  	/* use malloc strategy first-fit */
    for(p = prevp->s.ptr;  ; prevp = p, p = p->s.ptr) {
      if(p->s.size >= nunits) {                           	/* big enough */
        if (p->s.size == nunits)                         	/* exactly */
	        prevp->s.ptr = p->s.ptr;
        else {                                            	/* allocate tail end */
	        p->s.size -= nunits;
	        p += p->s.size;
	        p->s.size = nunits;
        }
        freep = prevp;
        return (void *)(p+1);
      }
      if(p == freep)                                      	/* wrapped around free list */
        if((p = morecore(nunits)) == NULL)
	    return NULL;                                        /* none left */
    }
  } else if(STRATEGY == STRATEGY_BEST_FIT) {				/* use malloc strategy best-fit */
    Header *best = NULL;									/* header to the current block with best fit */
    Header *bestprev = NULL;								/* previous header to header 'best' */
    
    for(p = prevp->s.ptr;  ; prevp = p, p = p->s.ptr) {
      if (p->s.size == nunits) {                        	/* exactly */
        prevp->s.ptr = p->s.ptr;						
        best = p;											/* set the best header */
        freep = prevp;
        break;												/* no need to continue, better match not possible */
      } else if(p->s.size > nunits) {						/* big enough but not exactly */
        if(best == NULL) {									/* first possible match is of course the current best match */
          best = p;
          bestprev = prevp;
        } else {											/* a match has been found earlier */
          if(best->s.size > p->s.size) {					/* only change best match if better */
            best = p;
            bestprev = prevp;
          }
        }
      }
      
      if(p == freep) {										/* entire free list has been checked */
        if(best == NULL) {									/* no match found */
          if((p = morecore(nunits)) == NULL)				/* try to get more memory, return null if not possible */
	        return NULL;
        } else {											/* a best match was found that was not a exact match */
			best->s.size -= nunits;							/* remove needed space from the free block */
			best += best->s.size;							/* make 'best' a header over the removed space */
	        best->s.size = nunits;							/* set the size to be equal to the removed space*/
	        freep = bestprev;								
	        break;
        }
	    }
    }
    
    return (void *)(best+1);								/* return the block with the best match */ 
  } else if(STRATEGY == STRATEGY_WORST_FIT) {				/* use malloc strategy best-fit */
    Header *worst = NULL;									/* header to the current block with best fit */
    Header *worstprev = NULL;								/* previous header to header 'best' */
    
    for(p = prevp->s.ptr;  ; prevp = p, p = p->s.ptr) {
      if(p->s.size >= nunits) {						/* big enough but not exactly */
        if(worst == NULL) {									/* first possible match is of course the current best match */
          worst = p;
          worstprev = prevp;
        } else {											/* a match has been found earlier */
          if(worst->s.size < p->s.size) {					/* only change best match if better */
            worst = p;
            worstprev = prevp;
          }
        }
      }
      
      if(p == freep) {										/* entire free list has been checked */
        if(worst == NULL) {									/* no match found */
          if((p = morecore(nunits)) == NULL)				/* try to get more memory, return null if not possible */
	        return NULL;
        } else if(worst->s.size > nunits) {											/* a best match was found that was not a exact match */
			worst->s.size -= nunits;							/* remove needed space from the free block */
			worst += worst->s.size;							/* make 'best' a header over the removed space */
	        worst->s.size = nunits;							/* set the size to be equal to the removed space*/
	        freep = worstprev;								
	        break;
        } else {
          worstprev->s.ptr = worst->s.ptr;
          freep = worstprev;
          break;
        }
	    }
    }
  
    return (void *)(worst+1);								/* return the block with the best match */
  }
}

void *realloc(void *ptr, size_t size) {
  void *newptr = malloc(size);
  
  if(ptr != NULL) {
    
    if(newptr != NULL) {
      Header *header = (Header*)(ptr - sizeof(Header));
  
      unsigned ptrsize = header->s.size*sizeof(Header) - sizeof(Header);
      unsigned copysize = ptrsize;
      
      if(ptrsize > size) {
        copysize = size;
      }
      
      memcpy(newptr, ptr, copysize);
    }
  
    free(ptr);
  }
  
  return newptr;
}

