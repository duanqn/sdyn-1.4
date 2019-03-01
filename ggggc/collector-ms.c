#include "ggggc/gc.h"

int ggggc_yield(){
    // Pretend we are waiting for something
    return 0;
}

/* allocate an object without a descriptor */
void *ggggc_mallocRaw(struct GGGGC_Descriptor **descriptor, /* descriptor to protect, if applicable */
    ggc_size_t size /* size of object to allocate */
    ){
    return NULL;
}

/* allocate an object */
void *ggggc_malloc(struct GGGGC_Descriptor *descriptor)
{
    return NULL;
}

/* run a generation 0 collection */
void ggggc_collect0(unsigned char gen)
{
}
