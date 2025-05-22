// Linked Lists
#ifndef __LL_H__
#define __LL_H__

#include "memory.h"
#include "../root.h"

typedef struct LNode_
{
    struct LNode_* pNext;
    void* pData;
} LNode_;

typedef struct
{
    LNode_* pBase;
    int size;
} LinkedList;

LinkedList* ll_init();
void ll_clear(LinkedList* pLL);

LNode_* create_node(void* data);

void append_node(LinkedList* pLL, LNode_* pNode);
void prepend_node(LinkedList* pLL, LNode_* pNode);

LNode_* pop_front(LinkedList* pLL);
LNode_* pop_back(LinkedList* pLL);

#endif