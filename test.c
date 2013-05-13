#include <stdio.h>
#include "malloc/malloc.h"

typedef long Align;                                     /* for alignment to long boundary */

union header {                                          /* block header */
  struct {
    union header *ptr;                                  /* next block if on free list */
    unsigned size;                                      /* size of this block  - what unit? */ 
  } s;
  Align x;                                              /* force alignment of blocks */
};

typedef union header Header;

typedef struct {
  union header *ptr;
  unsigned size;
} test;

int main() {
    Header *p;
    Header *i;
    long *l;
    p = (Header*)malloc(1);
   
    printf("before p: %p\n", (void*)p);
    
    p = (p-1);
    
    test s;
    
    unsigned size;
    union header *lol;
    
    Header h;
    
    //printf("long: %lu, pointer sizeof: %lu, unsigned size: %lu, header size: %lu, s sizeof: %lu", sizeof(long), sizeof(lol), sizeof(size), sizeof(h), sizeof(s));
    
    printf("ptr: %lu size: %lu total: %lu \n\n", sizeof(s.ptr), sizeof(s.size), sizeof(s));
    
    //printf("sizeof Header %lu. Header: block size: %i", sizeof(Header), p->s.size);
   
    printf("after p: %p\n", (void*)p);
     
    return 0;
}
