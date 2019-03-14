#include "ggggc/gc.h"
#include <stdio.h>

void *ggggc_mallocRaw(struct GGGGC_Descriptor **descriptor, /* descriptor to protect, if applicable */
    ggc_size_t size /* size of object to allocate */
    );
void ggggc_collect0(unsigned char gen);
#ifdef CHATTY
void fullDump();
void pointerStackDump();
#endif

struct s3880{
    struct GGGGC_Header header;
    struct s3880 *next;
    ggc_size_t zeros[3878];
};

int main(){
    struct GGGGC_Descriptor *genesis = NULL;
    genesis = ggggc_mallocRaw(NULL, sizeof(struct GGGGC_Descriptor) / sizeof(ggc_size_t));
    genesis->header.descriptor__ptr = genesis;  // self-descriptive
    genesis->size = sizeof(struct GGGGC_Descriptor) / sizeof(ggc_size_t);
    genesis->pointers[0] = 0x3;
    #ifdef CHATTY
    fullDump();
    #endif
    
    struct GGGGC_Descriptor *dd64 = ggggc_malloc(genesis);
    dd64->size = 64;
    dd64->pointers[0] = 0x3;

    struct GGGGC_Descriptor *dd488 = ggggc_malloc(dd64);
    dd488->size = 488;
    dd488->pointers[0] = 0x3;

    struct GGGGC_Descriptor *d3880 = ggggc_malloc(dd488);
    d3880->size = 3880;
    d3880->pointers[0] = 0x3;

    struct s3880 *head = NULL, *newp = NULL;
    GGC_PUSH_2(head, newp);
    for(int i = 0; i < 4096; ++i){
        newp = ggggc_malloc(d3880);
        newp->next = head;
        head = newp;
        newp = NULL;
    }
    head = NULL;
    ggggc_collect0(1);
    for(int i = 0; i < 4096; ++i){
        newp = ggggc_malloc(d3880);
        newp->next = head;
        head = newp;
        newp = NULL;
    }
    GGC_POP();
    return 0;
}