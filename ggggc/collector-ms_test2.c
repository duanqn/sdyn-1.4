#ifndef GGGGC_DEBUG_MEMORY_CORRUPTION
#define GGGGC_DEBUG_MEMORY_CORRUPTION
#endif
#include "ggggc/gc.h"
#include <stdio.h>

void *ggggc_mallocRaw(struct GGGGC_Descriptor **descriptor, /* descriptor to protect, if applicable */
    ggc_size_t size /* size of object to allocate */
    );
void fullDump();
void ggggc_collect0(unsigned char gen);
void pointerStackDump();

struct s1760{
    struct GGGGC_Header header;
    struct s1760 *next;
    ggc_size_t zeros[1757];
};

int main(){
    struct GGGGC_Descriptor *genesis = NULL;
    genesis = ggggc_mallocRaw(NULL, sizeof(struct GGGGC_Descriptor) / sizeof(ggc_size_t));
    genesis->header.descriptor__ptr = genesis;  // self-descriptive
    genesis->size = sizeof(struct GGGGC_Descriptor) / sizeof(ggc_size_t);
    genesis->pointers[0] = 0x5;
    fullDump();
    
    struct GGGGC_Descriptor *dd8 = ggggc_malloc(genesis);
    dd8->size = 8;
    dd8->pointers[0] = 0x5;

    struct GGGGC_Descriptor *dd32 = ggggc_malloc(dd8);
    dd32->size = 32;
    dd32->pointers[0] = 0x5;

    struct GGGGC_Descriptor *dd224 = ggggc_malloc(dd32);
    dd224->size = 224;
    dd224->pointers[0] = 0x5;

    struct GGGGC_Descriptor *d1760 = ggggc_malloc(dd224);
    d1760->size = 1760;
    d1760->pointers[0] = 0x5;

    struct s1760 *head = NULL, *newp = NULL;
    GGC_PUSH_2(head, newp);
    for(int i = 0; i < 4096; ++i){
        newp = ggggc_malloc(d1760);
        newp->next = head;
        head = newp;
        newp = NULL;
    }
    head = NULL;
    ggggc_collect0(1);
    for(int i = 0; i < 4096; ++i){
        newp = ggggc_malloc(d1760);
        newp->next = head;
        head = newp;
        newp = NULL;
    }
    GGC_POP();
    return 0;
}