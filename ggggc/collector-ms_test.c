#include "ggggc/gc.h"
#include <stdio.h>

void *ggggc_mallocRaw(struct GGGGC_Descriptor **descriptor, /* descriptor to protect, if applicable */
    ggc_size_t size /* size of object to allocate */
    );
void fullDump();

struct ListNode{
    struct GGGGC_Header header;
    ggc_size_t value;
    struct ListNode *next;
};

struct ListNode * insert(ggc_size_t x, struct ListNode *pos, struct GGGGC_Descriptor *nodeDescriptor){
    struct ListNode *p = ggggc_malloc(nodeDescriptor);
    p->value = x;
    if(pos){
        p->next = pos->next;
        pos->next = p;
    }
    else{
        p->next = NULL;
    }
    return p;
}

struct ListNode *findTail(struct ListNode *head){
    struct ListNode *prev = NULL;
    for(struct ListNode *p = head; p; prev = p, p = p->next);
    return prev;
}

struct ListNode * append(ggc_size_t x, struct ListNode *head, struct GGGGC_Descriptor *nodeDescriptor){
    return insert(x, findTail(head), nodeDescriptor);
}

int main(){
    struct GGGGC_Descriptor *genesis = NULL;
    fullDump();
    genesis = ggggc_mallocRaw(NULL, sizeof(struct GGGGC_Descriptor) / sizeof(ggc_size_t));
    genesis->header.descriptor__ptr = genesis;  // self-descriptive
    genesis->size = sizeof(struct GGGGC_Descriptor) / sizeof(ggc_size_t);
    genesis->pointers[0] = 3;
    fullDump();
    
    struct GGGGC_Descriptor *ListNode_Descriptor = ggggc_malloc(genesis);
    ListNode_Descriptor->size = sizeof(struct ListNode) / sizeof(ggc_size_t);
    #ifndef GGGGC_DEBUG_MEMORY_CORRUPTION
    ListNode_Descriptor->pointers[0] = 0x5;
    #else
    ListNode_Descriptor->pointers[0] = 0x9;
    #endif

    struct ListNode *head = NULL;
    head = append(0, head, ListNode_Descriptor);
    GGC_PUSH_1(head);
    for(ggc_size_t i = 1; i < 100; ++i){
        printf("Append node %u\n", i);
        append(i, head, ListNode_Descriptor);
    }
    fullDump();
    GGC_POP();
    return 0;
}