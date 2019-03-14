#include "ggggc/gc.h"
#include <errno.h>
#include <stdio.h>
#include <string.h>

#include "ggggc-internals.h"

void ggggc_collect0(unsigned char gen);
void pointerStackDump();

/*
    This is my own Pool structure.
    next:   a pointer to the next pool
    freelist:   a pointer to the first free object in this pool
    endptr: a pointer to the first available (unallocated) word in this pool
    memSpace:   a pointer to the first word that can be given to the mutator
*/
struct Pool{
    struct Pool *next;
    struct FreeObjHeader *freelist;
    ggc_size_t * endptr;  // end of used memory space
    ggc_size_t memSpace[];
};

// The real pool size (excluding the pool header)
#define POOL_SIZE ((GGGGC_POOL_BYTES) - 3 * sizeof(void *))
// 3 constants for managing GC frequency
#define LOAD_IDEAL 0.4
#define LOAD_COLLECT 0.8
#define LOAD_EXPAND 0.7

/*
    This is the header for free objects, i.e. free memory blocks.
    Note the relationship between FreeObjHeader and GGGGC_Header.
    In GGGGC_Header the first word is a pointer to the object's descriptor,
    while here the first word is a pointer to the next free object.
    Since the first word is used as a pointer in both structures,
    we can use its least significant bits to mark it.
    Bit 0x1 is used to mark the objects during GC.
        Bit 0x1 is set -- object is marked
    Bit 0x2 is used to differentiate a free object from an alive object.
        Bit 0x2 is set -- this object is a free object
    For alive objects, the first word is a valid pointer.

    The second word in FreeObjHeader is the size of this free memory block.
    We need to record the size because its descriptor might have been collected,
    which means we can't use the descriptor to determine its size.
*/
struct FreeObjHeader{
    struct FreeObjHeader *next; // This pointer is always marked (bit 0x2)
    ggc_size_t size;    // size in sizeof(ggc_size_t)
};

// Size of the larger header
#define HEADER_SIZE (sizeof(struct FreeObjHeader) > sizeof(struct GGGGC_Header) ? \
sizeof(struct FreeObjHeader) : sizeof(struct GGGGC_Header))

// Some static global variables
// For maintaining the linked list of pools
static struct Pool *poolList = NULL;
static struct Pool *currentPool = NULL;
static struct Pool *lastPool = NULL;
// For calculating load factor
static ggc_size_t allocated = 0;
static ggc_size_t available = 0;
static double loadFactor = 0;
// This program talks a lot when the CHATTY switch is turned on
#ifdef CHATTY
static int poolCount = 0;
#endif

#ifdef GUARD
// Fail fast if the pointer is not aligned to word boundary
static inline void assertPtrAligned(void *ptr){
    if((ggc_size_t)(ptr) & (ggc_size_t)(sizeof(ggc_size_t) - 1)){
        printf("Error: free space pointer not aligned.\n");
        abort();
    }
}
#endif

// Mask out the flags and cast it to a pointer
// -- for convenience, since we only set flags on pointers
inline ggc_size_t * maskMarks(ggc_size_t val){
    val &= (~0x3);
    return (ggc_size_t *)val;
}

// Mark a word (0x1)
inline void markPointed(ggc_size_t *pos){
    *pos |= 0x1;
}

// Erase the mark on a word (0x1)
inline void unmarkPointed(ggc_size_t *pos){
    *pos &= (~0x1);
}

// Mark a word (0x2)
inline void markFree(ggc_size_t *pos){
    *pos |= 0x2;
}

// Erase the mark on a word (0x2)
inline void unmarkFree(ggc_size_t *pos){
    *pos &= (~0x2);
}

// Test the flag (0x1)
inline int testPointed(ggc_size_t *pos){
    return (*pos) & 0x1;
}

// Test the flag (0x2)
inline int testFree(ggc_size_t *pos){
    return (*pos) & 0x2;
}

#define MAX(a, b) ((a) > (b) ? (a) : (b))

#ifdef CHATTY
// Debug function; Prints the allocated part of a pool
void poolDump(struct Pool *p){
    printf("===Pool dump===\n");
    printf("Next pool pointer: %p\n", p->next);
    printf("Free list pointer: %p\n", p->freelist);
    printf("Occupied space: [%p, %p)\n", p, (unsigned char *)p + GGGGC_POOL_BYTES);
    printf("Visible space: [%p, %p)\n", p->memSpace, (unsigned char *)(p->memSpace) + POOL_SIZE);
    printf("Allocated space: [%p, %p)\n", p->memSpace, p->endptr);
    for(unsigned char *ptr = (unsigned char *)(p->memSpace); ptr < (unsigned char *)(p->endptr);){
        printf("%p:", ptr);
        for(int j = 0; j < 4; ++j){
            printf(" ");
            if(*(ggc_size_t *)ptr == 0){
                for(int i = sizeof(ggc_size_t) - 1; i >= 0 && ptr < (unsigned char *)(p->endptr); --i){
                    printf("  ");
                    ++ptr;
                }
            }
            else{
                int nonzero = 0;
                for(int i = sizeof(ggc_size_t) - 1; i >= 0 && ptr < (unsigned char *)(p->endptr); --i){
                    if(!nonzero && *(ptr+i) == 0){
                        printf("  ");
                    }
                    else{
                        nonzero = 1;
                        printf("%02hhX", *(ptr+i));
                    }
                    
                }
                ptr += sizeof(ggc_size_t);
            }
        }
        printf("\n");
    }
}

// Debug function; Prints the memory layout of an object
void objDump(struct GGGGC_Header *header){
    ggc_size_t val = (ggc_size_t)header->descriptor__ptr;
    val &= ~1;
    printf("Object descriptor: %p\n", val);
    struct GGGGC_Descriptor *descriptor = (struct GGGGC_Descriptor *)val;
    val = descriptor->size;
    printf("Object size: %lu\n", val);
    for(unsigned i = 0; i < val; ++i){
        printf("Offset %08x: %lx\n", i * sizeof(ggc_size_t), *((ggc_size_t *)header+i));
    }
}

// Debug function; Dumps all pools and root pointers
void fullDump(){
    printf("=====Full heap dump=====\n");
    printf("Global load factor: %lf\n", loadFactor);
    for(struct Pool *p = poolList; p; p = p->next){
        poolDump(p);
    }
    printf("===Root pointers===\n");
    pointerStackDump();
    printf("=====End=====\n");
}

// Debug function; Prints all free list entries in a pool
void freeListDump(struct Pool *pool){
    struct FreeObjHeader *header = pool->freelist;
    printf("*** Freelist of pool %p:\n", pool);
    while(header){
        if(!testFree(header)){
            printf("Object on free list not marked as free\n");
            abort();
        }
        printf("%p\n", header);
        header = maskMarks((ggc_size_t)(header->next));
    }
    printf("***End\n");
}
#endif

#ifdef GUARD
// Fail fast if this pointer is pointing to an address outside of any known pool
static inline void assertHeapPointer(void * ptr){
    if(!ptr){
        return;
    }
    struct Pool * poolptr = GGGGC_POOL_OF(ptr);
    for(struct Pool *p = poolList; p; p = p->next){
        if(p == poolptr){
            return;
        }
    }
    abort();
}

// Debug function; Walks through all pools to check the parsability
// As slow as GC
static void assertParsableHeap(){
    for(struct Pool *pool = poolList; pool; pool = pool->next){
        ggc_size_t *objPointer = pool->memSpace;
        while(objPointer != pool->endptr){
            if(testPointed(objPointer)){
                printf("Pointer should not be marked\n");
                abort();
            }
            if(objPointer > pool->endptr){
                printf("Heap parsability lost\n");
                abort();
            }
            if(testFree(objPointer)){
                objPointer += ((struct FreeObjHeader *)objPointer)->size;
            }
            else{
                assertHeapPointer(*objPointer);
                objPointer += MAX(((struct GGGGC_Descriptor *)(*objPointer))->size, HEADER_SIZE / sizeof(ggc_size_t));
            }
        }
    }
}
#endif

// Ask the OS to give us a new pool
int allocNewPool(void ** pool)
{
    void *ret;
    if ((errno = posix_memalign(pool, GGGGC_POOL_BYTES, GGGGC_POOL_BYTES))) {
        return -1;
    }
    #ifdef CHATTY
    ++poolCount;
    #endif
    return 0;
}

// Allocate a new pool and initialize it
struct Pool *newPool(){
    struct Pool *ret;
    if(allocNewPool((void **)&ret)){
        printf("Failed to allocate a new pool.\n");
        return NULL;
    }

    /* set it up */
    ret->next = NULL;
    ret->freelist = NULL;
    ret->endptr = ret->memSpace;
    #ifdef GUARD
    assertPtrAligned(ret->endptr);
    #endif

    // Update the load factor
    available += POOL_SIZE / sizeof(ggc_size_t);
    loadFactor = allocated / (double)available;

    return ret;
}

// Allocate a new pool, initialize it, and append it to the linked list
int appendNewPool(){
    struct Pool *p = newPool();
    if(p == NULL){
        printf("newPool() failed.\n");
        return -1;
    }
    if(lastPool){
        lastPool->next = p;
        lastPool = p;
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
    int err = 0;
    if(loadFactor > LOAD_COLLECT){
        ggggc_collect0(0);
        if(loadFactor > LOAD_EXPAND){
            err = 0;
            // at least allocate 1 new pool
            do{
                #ifdef CHATTY
                printf("*** 1 new pool appended ***\n");
                #endif
                err = appendNewPool();
            }while(err == 0 && loadFactor > LOAD_IDEAL);
        }
    }
    return 0;
}

/* allocate an object without a descriptor */
void *ggggc_mallocRaw(struct GGGGC_Descriptor **descriptor, /* descriptor to protect, if applicable */
    ggc_size_t size /* size of object to allocate */
    ){
    #ifdef CHATTY
    printf("Raw malloc %lu bytes\n", size * sizeof(size));
    #endif
    // size too large?
    if(size > POOL_SIZE / sizeof(ggc_size_t)){
        printf("Requested space cannot fit in a pool.\n");
        return NULL;
    }
    int err = 0;
    struct GGGGC_Header *mem = NULL;
    int foundFreeSpace = 0;
    int GC_ed = 0;
    int expanded = 0;
    // Allocate a pool if there is none
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
    #ifdef GUARD
    assertPtrAligned(currentPool->endptr);
    #endif
    // must have enough room for the header
    if(size * sizeof(ggc_size_t) < HEADER_SIZE){
        size = HEADER_SIZE / sizeof(ggc_size_t);    // must ensure HEADER_SIZE is multiple of sizeof(ggc_size_t)
    }
    CHECK:
    #ifdef GUARD
    assertPtrAligned(currentPool->endptr);
    #endif
    // check if there is enough space in current pool
    if((unsigned char *)(currentPool->memSpace) + POOL_SIZE >= (unsigned char *)(currentPool->endptr + size)){
        // bump pointer
        #ifdef CHATTY
        printf("*** Bump pointer ***\n");
        #endif
        mem = (struct GGGGC_Header *)currentPool->endptr;
        currentPool->endptr += size;    // size * sizeof(ggc_size_t) bytes
        mem->descriptor__ptr = NULL;
#ifdef GGGGC_DEBUG_MEMORY_CORRUPTION
        /* set its canary */
        mem->ggggc_memoryCorruptionCheck = GGGGC_MEMORY_CORRUPTION_VAL;
#endif
        // Clear memory
        memset((void *)((unsigned char *)mem + sizeof(struct GGGGC_Header)), 0, size * sizeof(ggc_size_t) - sizeof(struct GGGGC_Header));
    }
    else{
        // go through free list
        // strategy: first
        #ifdef CHATTY
        printf("*** Search free list ***\n");
        #endif
        for(struct FreeObjHeader *prev = NULL, *p = currentPool->freelist; p;){
            #ifdef GUARD
            if(!testFree((ggc_size_t *)p)){
                printf("Object not marked as free\n");
                assertParsableHeap();
                abort();
            }
            #endif
            if(p->size == size){
                // perfect
                mem = (struct GGGGC_Header *)p;
                // remove p from free list
                if(prev != NULL){
                    #ifdef GUARD
                    if(!testFree(p) || !testFree(prev)){
                        printf("p and prev should both be marked.\n");
                        abort();
                    }
                    #endif
                    prev->next = p->next;
                }
                else{
                    // First entry in the free list
                    currentPool->freelist = (struct FreeObjHeader *)maskMarks((ggc_size_t)(p->next));
                    #ifdef GUARD
                    if(currentPool->freelist!= NULL && !testFree(currentPool->freelist)){
                        printf("p->next should both be marked.\n");
                        abort();
                    }
                    #endif
                }
                foundFreeSpace = 1;
                break;
            }
            else if(p->size >= size + sizeof(struct FreeObjHeader) / sizeof(ggc_size_t)){
                // split
                mem = (ggc_size_t *)p + (p->size - size);
                p->size -= size;
                foundFreeSpace = 1;
                break;
            }
            prev = p;
            p = (struct FreeObjHeader *)maskMarks((ggc_size_t)(p->next));
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
                #ifdef CHATTY
                printf("*** GC during mallocRaw ***\n");
                #endif
                ggggc_collect0(0);
                GGC_POP();
                GC_ed = 1;
                // Still too full?
                if(loadFactor > LOAD_EXPAND){
                    err = 0;
                    do{
                        err = appendNewPool();
                    }while(err == 0 && loadFactor > LOAD_IDEAL);
                    expanded = 1;
                }
                goto CHECK;
            }
            else{
                if(!expanded){
                    #ifdef CHATTY
                    printf("*** Allocate new pool ***\n");
                    #endif
                    // allocate new pools
                    err = 0;
                    // at least allocate 1 new pool
                    do{
                        #ifdef CHATTY
                        printf("*** 1 new pool appended ***\n");
                        #endif
                        err = appendNewPool();
                    }while(err == 0 && loadFactor > LOAD_IDEAL);
                    expanded = 1;
                    goto CHECK;
                }
                else{
                    // Should not hit here
                    return NULL;
                }
            }
        }
        else{
            // must set this pointer AFTER maintaining linked list
            mem->descriptor__ptr = NULL;
            #ifdef GGGGC_DEBUG_MEMORY_CORRUPTION
            /* set its canary */
            mem->ggggc_memoryCorruptionCheck = GGGGC_MEMORY_CORRUPTION_VAL;
            #endif
            memset((void *)mem + sizeof(struct GGGGC_Header), 0, size * sizeof(ggc_size_t) - sizeof(struct GGGGC_Header));
        }
    }
    #ifdef GUARD
    assertPtrAligned(mem);  // Unaligned pointers must not leave our allocator
    #endif

    // Update load factor
    allocated += size;
    loadFactor = allocated / (double)available;
    return (void *)mem;
}

/* allocate an object */
void *ggggc_malloc(struct GGGGC_Descriptor *descriptor)
{
    #ifdef CHATTY
    //printf("malloc with descriptor at %p\n", descriptor);
    //objDump((struct GGGGC_Header *)descriptor);
    #endif
    struct GGGGC_Header *ret = (struct GGGGC_Header *) ggggc_mallocRaw(&descriptor, descriptor->size);
    #ifdef GUARD
    assertPtrAligned(ret);
    #endif
    ret->descriptor__ptr = descriptor;
    return ret;
}

// Copied from gembc
/* list of pointers to search and associated macros */
#define TOSEARCH_SZ 1024
struct ToSearch {
    struct ToSearch *prev, *next;
    unsigned short used;
    void **buf;
};
static struct ToSearch toSearchList;

#define TOSEARCH_INIT(toSearch) do { \
    if (toSearchList.buf == NULL) { \
        toSearchList.buf = (void **) malloc(TOSEARCH_SZ * sizeof(void *)); \
        if (toSearchList.buf == NULL) { \
            /* FIXME: handle somehow? */ \
            perror("malloc"); \
            abort(); \
        } \
    } \
    toSearch = &toSearchList; \
    toSearch->used = 0; \
} while(0)
#define TOSEARCH_NEXT(toSearch) do { \
    if (!(toSearch)->next) { \
        struct ToSearch *tsn = (struct ToSearch *) malloc(sizeof(struct ToSearch)); \
        (toSearch)->next = tsn; \
        tsn->prev = (toSearch); \
        tsn->next = NULL; \
        tsn->buf = (void **) malloc(TOSEARCH_SZ * sizeof(void *)); \
        if (tsn->buf == NULL) { \
            perror("malloc"); \
            abort(); \
        } \
    } \
    (toSearch) = (toSearch)->next; \
    (toSearch)->used = 0; \
} while(0)
#define TOSEARCH_ADD(toSearch, ptr) do { \
    if (toSearch->used >= TOSEARCH_SZ) TOSEARCH_NEXT(toSearch); \
    toSearch->buf[toSearch->used++] = (ptr); \
} while(0)
#define TOSEARCH_POP(toSearch, type, into) do { \
    into = (type) toSearch->buf[--toSearch->used]; \
    if (toSearch->used == 0 && toSearch->prev) \
        toSearch = toSearch->prev; \
} while(0)

#define TOSEARCH_EMPTY(toSearch) (toSearch->used == 0)

#define TOSEARCH_FREE do { \
    while(toSearchList->next){ \
        if(toSearchList->buf){ \
            free(toSearchList->buf); \
        } \
        toSearchList = toSearchList->next; \
    } \
    if(toSearchList->buf){ \
        free(toSearchList->buf); \
    } \
} while(0)

#ifdef CHATTY
// Debug function; Print all root pointers
void pointerStackDump(){
    struct GGGGC_PointerStackList pointerStackNode, *pslCur;
    struct GGGGC_JITPointerStackList jitPointerStackNode, *jpslCur;
    struct GGGGC_PointerStack *psCur;
    void **jpsCur;
    ggc_size_t i = 0;
    pointerStackNode.pointerStack = ggggc_pointerStack;
    pointerStackNode.next = ggggc_blockedThreadPointerStacks;
    ggggc_rootPointerStackList = &pointerStackNode;
    jitPointerStackNode.cur = ggc_jitPointerStack;
    jitPointerStackNode.top = ggc_jitPointerStackTop;
    jitPointerStackNode.next = ggggc_blockedThreadJITPointerStacks;
    ggggc_rootJITPointerStackList = &jitPointerStackNode;

    for (pslCur = ggggc_rootPointerStackList; pslCur; pslCur = pslCur->next) {
        printf("Stack list at %p\n", pslCur);
        for (psCur = pslCur->pointerStack; psCur; psCur = psCur->next) {
            printf("Stack at %p\n", psCur);
            printf("%lu pointers in this stack\n", psCur->size);
            for (i = 0; i < psCur->size; i++) {
                printf("Found root pointer %p\n", *(void **)psCur->pointers[i]);
                if(*(void **)psCur->pointers[i]){
                    objDump((struct GGGGC_Header *)(*(void **)psCur->pointers[i]));
                }
            }
        }
    }
    for (jpslCur = ggggc_rootJITPointerStackList; jpslCur; jpslCur = jpslCur->next) {
        for (jpsCur = jpslCur->cur; jpsCur < jpslCur->top; jpsCur++) {
            printf("Found JIT root pointer %p\n", *(void **)jpsCur);
        }
    }
}
#endif

/* run a generation 0 collection */
void ggggc_collect0(unsigned char gen)
{
    // I have no idea about the meaning of these variables
    // I just copied them from gembc
    struct GGGGC_PointerStackList pointerStackNode, *pslCur;
    struct GGGGC_JITPointerStackList jitPointerStackNode, *jpslCur;
    struct GGGGC_PointerStack *psCur;
    void **jpsCur;
    struct ToSearch *currentBlock;
    ggc_size_t wordval;
    ggc_size_t * pointer;
    struct FreeObjHeader *freeListPointer, *secondLastFreeListPointer;

    /* initialize our roots */
    pointerStackNode.pointerStack = ggggc_pointerStack;
    pointerStackNode.next = ggggc_blockedThreadPointerStacks;
    ggggc_rootPointerStackList = &pointerStackNode;
    jitPointerStackNode.cur = ggc_jitPointerStack;
    jitPointerStackNode.top = ggc_jitPointerStackTop;
    jitPointerStackNode.next = ggggc_blockedThreadJITPointerStacks;
    ggggc_rootJITPointerStackList = &jitPointerStackNode;

    #ifdef GUARD
    assertParsableHeap();
    #endif
    TOSEARCH_INIT(currentBlock);

    /* add our roots to the to-search list */
    for (pslCur = ggggc_rootPointerStackList; pslCur; pslCur = pslCur->next) {
        for (psCur = pslCur->pointerStack; psCur; psCur = psCur->next) {
            #ifdef CHATTY
            printf("%lu pointers in this stack\n", psCur->size);
            #endif
            for (wordval = 0; wordval < psCur->size; wordval++) {
                #ifdef CHATTY
                printf("Adding root pointer %p\n", *(void **)psCur->pointers[wordval]);
                printf("Current block: %p\n", currentBlock);
                #endif
                #ifdef GUARD
                assertHeapPointer(*(void **)psCur->pointers[wordval]);
                #endif
                TOSEARCH_ADD(currentBlock, *(void **)psCur->pointers[wordval]);
            }
        }
    }
    for (jpslCur = ggggc_rootJITPointerStackList; jpslCur; jpslCur = jpslCur->next) {
        for (jpsCur = jpslCur->cur; jpsCur < jpslCur->top; jpsCur++) {
            #ifdef CHATTY
            printf("Adding JIT root pointer %p\n", *(void **)jpsCur);
            #endif
            #ifdef GUARD
            assertHeapPointer(*(void **)jpsCur);
            #endif
            TOSEARCH_ADD(currentBlock, *(void **)jpsCur);
        }
    }

    // Mark
    #ifdef CHATTY
    printf("GC / Mark phase\n");
    #endif
    while(!TOSEARCH_EMPTY(currentBlock)){
        TOSEARCH_POP(currentBlock, ggc_size_t *, pointer);
        #ifdef GUARD
        assertHeapPointer(pointer);
        #endif
        // pointer --> obj header[ descriptor_ptr  --> descripor
        //                         DEADBEEF
        //                         value ... ]
        #ifdef CHATTY
        printf("Processing pointer %p\n", pointer);
        #endif
        if(pointer == NULL){
            continue;
        }
        #ifdef CHATTY
        printf("Obj descriptor at %p\n", ((struct GGGGC_Header *)pointer)->descriptor__ptr);
        #endif
#ifdef GGGGC_DEBUG_MEMORY_CORRUPTION
        /* check for pre-corruption */
        if (((struct GGGGC_Header *)pointer)->ggggc_memoryCorruptionCheck != GGGGC_MEMORY_CORRUPTION_VAL) {
            fprintf(stderr, "GGGGC: Canary corrupted!\n");
            fprintf(stderr, "Canary address: %p\n", &(((struct GGGGC_Header *)pointer)->ggggc_memoryCorruptionCheck));
            fprintf(stderr, "Got: %lu\tExpected: %u\n", ((struct GGGGC_Header *)pointer)->ggggc_memoryCorruptionCheck, GGGGC_MEMORY_CORRUPTION_VAL);
            abort();
        }
#endif
        if(testPointed(pointer)){    // already marked
            #ifdef CHATTY
            printf("Object already marked.\n");
            #endif
            continue;
        }

        // The first word is the descriptor pointer in the GGGGC Header
        struct GGGGC_Descriptor *descriptor = (struct GGGGC_Descriptor *)(*pointer);
        #ifdef CHATTY
        printf("Adding pointer %p\n", (void *)(*pointer));
        #endif
        #ifdef GUARD
        assertHeapPointer((void *)*pointer);
        #endif
        TOSEARCH_ADD(currentBlock, (void *)*pointer);  // The descriptor pointer should always be alive
        markPointed(pointer);
        #ifdef CHATTY
        for(int i = 0; i < descriptor->size; ++i){
            printf("Offset %08x: %lx\n", i * sizeof(ggc_size_t), *(pointer+i));
        }
        #endif
        if(descriptor->pointers[0] & 1 != 0){
            // TODO: optimize
            for(wordval = 1; wordval < descriptor->size; ++wordval){
                int word = wordval / GGGGC_BITS_PER_WORD;
                unsigned int pos = wordval % GGGGC_BITS_PER_WORD;
                #ifdef CHATTY
                printf("bitmap %lu\tpos %d\n", descriptor->pointers[word], pos);
                #endif
                if((descriptor->pointers[word] & (1 << pos)) != 0){
                    #ifdef GUARD
                    assertHeapPointer((void *)*(pointer + wordval));
                    #endif
                    TOSEARCH_ADD(currentBlock, (void *)*(pointer + wordval));
                    #ifdef CHATTY
                    printf("Adding pointer %p\n", (void *)(*(pointer + wordval)));
                    #endif
                }
            }
        }
    }

    // Sweep
    // wordval used to keep the next step length
    allocated = 0;
    for(currentPool = poolList; currentPool; currentPool = currentPool->next){
        freeListPointer = currentPool->freelist = NULL;
        secondLastFreeListPointer = NULL;
        for(pointer = currentPool->memSpace; pointer < currentPool->endptr;){
            if(testPointed(pointer)){
                #ifdef GUARD
                if(testFree(pointer)){
                    printf("Object marked as free and pointed\n");
                    abort();
                }
                #endif
                // marked
                unmarkPointed(pointer); // unmark
                wordval = ((struct GGGGC_Header *)pointer)->descriptor__ptr->size;  // record this now
                // If this size is too small, we must have overallocated
                if(wordval < HEADER_SIZE / sizeof(ggc_size_t)){
                    wordval = HEADER_SIZE / sizeof(ggc_size_t);
                }
                allocated += wordval;
                #ifdef CHATTY
                printf("Obj at %p is marked\n", pointer);
                #endif
            }
            else if(testFree(pointer)){
                // Free object from last collection
                wordval = ((struct FreeObjHeader *)pointer)->size;
            }
            else{
                // Unmarked, construct free object header
                // Note:
                // the descriptor of this object might have been collected
                // need to ensure that the 'size' field of a collected descriptor is not overwritten
                // The 'size' field is the 3rd word
                #ifdef CHATTY
                printf("Obj at %p is not marked\n", pointer);
                #endif
                wordval = ((struct GGGGC_Header *)pointer)->descriptor__ptr->size;  // record this now
                // If this size is too small, we must have overallocated
                if(wordval < HEADER_SIZE / sizeof(ggc_size_t)){
                    wordval = HEADER_SIZE / sizeof(ggc_size_t);
                }
                ((struct FreeObjHeader *)pointer)->size = ((struct GGGGC_Header *)pointer)->descriptor__ptr->size;  // overwrites the 2nd word
                // descriptor__ptr is valid because this object has not been marked
                ((struct FreeObjHeader *)pointer)->next = NULL; // overwrites the 1st word
                markFree(pointer);
                // Do not zero the memory!

                // Now maintain the free list
                if(freeListPointer){
                    // TODO: merge consecutive blocks of empty memory
                    freeListPointer->next = (struct FreeObjHeader *)pointer;
                    markFree((ggc_size_t *)freeListPointer);
                    secondLastFreeListPointer = (struct FreeObjHeader *)maskMarks((ggc_size_t)freeListPointer);
                    freeListPointer = (struct FreeObjHeader *)maskMarks((ggc_size_t)(freeListPointer->next));
                }
                else{
                    // The first free object in this pool
                    freeListPointer = currentPool->freelist = (struct FreeObjHeader *)pointer;  // pointer itself is never marked
                }
            }
            #ifdef CHATTY
            printf("Increase pointer by %lu\n", wordval);
            #endif
            pointer += wordval;
        }
    }
    loadFactor = allocated / (double)available; // Update load factor
    currentPool = poolList; // reset to the first pool
    #ifdef GUARD
    assertParsableHeap();
    #endif
}
