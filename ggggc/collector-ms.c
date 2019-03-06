#include "ggggc/gc.h"
#include <errno.h>

struct Pool{
    struct Pool *next;
    struct FreeObjHeader *freelist;
    void * endptr;  // end of used memory space
    unsigned char memSpace[];
};

#define POOL_SIZE ((GGGGC_POOL_BYTES) - 3 * sizeof(void *))

struct FreeObjHeader{
    struct FreeObjHeader *next;
    ggc_size_t size;
};

static struct Pool *poolList = NULL;
static struct Pool *currentPool = NULL;
static struct Pool *lastPool = NULL;

int allocNewPool(void ** pool)
{
    void *ret;
    if ((errno = posix_memalign(pool, GGGGC_POOL_BYTES, GGGGC_POOL_BYTES))) {
        return -1;
    }
    return 0;
}

struct Pool *newPool(){
    struct Pool *ret;
    // then allocate a new one
    if(allocNewPool(&ret)){
        printf("Failed to allocate a new pool.\n");
        return NULL;
    }

    /* set it up */
    // need to fix
    ret->next = NULL;
    ret->freelist = NULL;
    ret->endptr = ret->memSpace;

    return ret;
}

int appendNewPool(){
    struct Pool *p = newPool();
    if(p == NULL){
        printf("newPool() failed.\n");
        return -1;
    }
    if(lastPool){
        lastPool->next = p;
    }
    else{
        // Pool list does not exist
        poolList = p;
        lastPool = p;
    }
    return 0;
}

int ggggc_yield(){
    // Pretend we are waiting for something
    // check heap usage
    // collect if load factor is too large
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
