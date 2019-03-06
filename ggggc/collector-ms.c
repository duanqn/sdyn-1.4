#include "ggggc/gc.h"
#include <errno.h>
#include <stdio.h>
#include <string.h>

void ggggc_collect0(unsigned char gen);

struct Pool{
    struct Pool *next;
    struct FreeObjHeader *freelist;
    ggc_size_t * endptr;  // end of used memory space
    ggc_size_t memSpace[];
};

#define POOL_SIZE ((GGGGC_POOL_BYTES) - 3 * sizeof(void *))
#define LOAD_IDEAL 0.4
#define LOAD_COLLECT 0.8
#define LOAD_EXPAND 0.7

struct FreeObjHeader{
    struct FreeObjHeader *next;
    ggc_size_t size;    // size in sizeof(ggc_size_t)
};

#define HEADER_SIZE (sizeof(struct FreeObjHeader) > sizeof(struct GGGGC_Header) ? \
sizeof(struct FreeObjHeader) : sizeof(struct GGGGC_Header))

static struct Pool *poolList = NULL;
static struct Pool *currentPool = NULL;
static struct Pool *lastPool = NULL;
static ggc_size_t allocated = 0;
static ggc_size_t available = 0;
static double loadFactor = 0;

void poolDump(struct Pool *p){
    printf("===Pool dump===\n");
    printf("Next pool pointer: %p\n", p->next);
    printf("Free list pointer: %p\n", p->freelist);
    printf("Occupied space: [%p, %p)\n", p, (unsigned char *)p + GGGGC_POOL_BYTES);
    printf("Visible space: [%p, %p)\n", p->memSpace, p->memSpace + POOL_SIZE);
    printf("Allocated space: [%p, %p)\n", p->memSpace, p->endptr);
    for(unsigned char *ptr = (unsigned char *)(p->memSpace); ptr < (unsigned char *)(p->endptr);){
        printf("%p:", ptr);
        for(int j = 0; j < 4; ++j){
            printf(" ");
            for(int i = 0; i < sizeof(ggc_size_t) && ptr < (unsigned char *)(p->endptr); ++i){
                printf("%02hhX", *ptr);
                ++ptr;
            }
        }
        printf("\n");
    }
}

void fullDump(){
    printf("=====Full heap dump=====\n");
    printf("Global load factor: %lf\n", loadFactor);
    for(struct Pool *p = poolList; p; p = p->next){
        poolDump(p);
    }
}

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
    if(allocNewPool((void **)&ret)){
        printf("Failed to allocate a new pool.\n");
        return NULL;
    }

    /* set it up */
    // need to fix
    ret->next = NULL;
    ret->freelist = NULL;
    ret->endptr = ret->memSpace;

    available += POOL_SIZE / sizeof(ggc_size_t);
    loadFactor = allocated / (double)available;

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
    printf("Raw malloc %u bytes\n", size * sizeof(size));
    if(size * sizeof(size) > POOL_SIZE){
        printf("Requested space cannot fit in a pool.\n");
        return NULL;
    }
    int err = 0;
    struct GGGGC_Header *mem = NULL;
    int foundFreeSpace = 0;
    int GC_ed = 0;
    int expanded = 0;
    if(currentPool == NULL){
        if(poolList == NULL){
            err = appendNewPool();
            if(err){
                printf("Cannot initialize pool list.\n");
                return NULL;
            }
        }
        currentPool = poolList;
    }
    // must have enough room for the header
    if(size * sizeof(ggc_size_t) < HEADER_SIZE){
        size = HEADER_SIZE / sizeof(ggc_size_t);    // must ensure HEADER_SIZE is multiple of sizeof(ggc_size_t)
    }
    CHECK:
    // check if there is enough space in current pool
    if(currentPool->memSpace + POOL_SIZE >= currentPool->endptr + size){
        // bump pointer
        mem = (struct GGGGC_Header *)currentPool->endptr;
        currentPool->endptr += size;    // size * sizeof(ggc_size_t) bytes
        mem->descriptor__ptr = NULL;
#ifdef GGGGC_DEBUG_MEMORY_CORRUPTION
        /* set its canary */
        mem->ggggc_memoryCorruptionCheck = GGGGC_MEMORY_CORRUPTION_VAL;
#endif
        // Clear memory
        memset((void *)mem + sizeof(struct GGGGC_Header), 0xAA, size * sizeof(ggc_size_t) - sizeof(struct GGGGC_Header));
    }
    else{
        // go through free list
        // strategy: first
        for(struct FreeObjHeader *prev = NULL, *p = currentPool->freelist; p; prev = p, p = p->next){
            if(p->size == size){
                // perfect
                mem = (struct GGGGC_Header *)p;
                // remove p from free list
                if(prev != NULL){
                    prev->next = p->next;
                }
                else{
                    currentPool->freelist = p->next;
                }
                foundFreeSpace = 1;
                break;
            }
            else if(p->size >= size + sizeof(struct FreeObjHeader) / sizeof(ggc_size_t)){
                // split
                mem = (struct GGGGC_Header *)p;
                struct FreeObjHeader *newEntry = (struct FreeObjHeader *)(p + size);
                newEntry->next = p->next;
                newEntry->size = p->size - size;
                if(prev != NULL){
                    prev->next = newEntry;
                }
                else{
                    currentPool->freelist = newEntry;
                }
                foundFreeSpace = 1;
                break;
            }
        }
        if(!foundFreeSpace){
            // Go to the next pool if possible
            if(currentPool->next){
                currentPool = currentPool->next;
                goto CHECK;
            }
            // Full GC
            // Don't recycle the descriptor
            if(!GC_ed){
                GGC_PUSH_1(*descriptor);
                ggggc_collect0(0);
                GGC_POP();
                GC_ed = 1;
                if(loadFactor > LOAD_EXPAND){
                    err = 0;
                    while(err == 0 && loadFactor > LOAD_IDEAL){
                        err = appendNewPool();
                    }
                    expanded = 1;
                }
                goto CHECK;
            }
            else{
                if(!expanded){
                    // allocate new pools
                    err = 0;
                    while(err == 0 && loadFactor > LOAD_IDEAL){
                        err = appendNewPool();
                    }
                    expanded = 1;
                    goto CHECK;
                }
                else{
                    return NULL;
                }
            }
        }
        else{
            // must set this pointer AFTER maintaining linked list
            mem->descriptor__ptr = NULL;
            memset((void *)mem + sizeof(struct GGGGC_Header), 0xAA, size * sizeof(ggc_size_t) - sizeof(struct GGGGC_Header));
        }
    }

    // maintain load factor
    allocated += size;
    loadFactor = allocated / (double)available;
    return (void *)mem;
}

/* allocate an object */
void *ggggc_malloc(struct GGGGC_Descriptor *descriptor)
{
    struct GGGGC_Header *ret = (struct GGGGC_Header *) ggggc_mallocRaw(&descriptor, descriptor->size);
    ret->descriptor__ptr = descriptor;
    return ret;
}

/* run a generation 0 collection */
void ggggc_collect0(unsigned char gen)
{
}
