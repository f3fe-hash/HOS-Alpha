#include "ll.h"

LinkedList* ll_init()
{
    LinkedList* pLL = mp_alloc(mpool_, sizeof(LinkedList));
    pLL->size = 0;
    pLL->pBase = NULL;
    return pLL;
}

void ll_clear(LinkedList* pLL)
{
    pLL->pBase = NULL;
    pLL->size = 0;
    mp_free(mpool_, pLL);
}

LNode_* create_node(void* data)
{
    LNode_* pNode = mp_alloc(mpool_, sizeof(LNode_));
    pNode->pData = data;
    pNode->pNext = NULL;
    return pNode;
}

void append_node(LinkedList* pLL, LNode_* pNode)
{
    if (pLL->size == 0)
    {
        pLL->pBase = pNode;
        pNode->pNext = NULL; // safe
    }
    else
    {
        pNode->pNext = pLL->pBase;
        pLL->pBase = pNode;
    }
    pLL->size++;
}

void prepend_node(LinkedList* pLL, LNode_* pNode)
{
    pNode->pNext = NULL;
    if (pLL->size == 0)
    {
        pLL->pBase = pNode;
    }
    else
    {
        LNode_* pCurr = pLL->pBase;
        while (pCurr->pNext != NULL)
            pCurr = pCurr->pNext;

        pCurr->pNext = pNode;
    }
    pLL->size++;
}

LNode_* pop_front(LinkedList* pLL)
{
    if (pLL->size == 0)
        return NULL;

    LNode_* pFront = pLL->pBase;
    pLL->pBase = pFront->pNext;
    pLL->size--;

    return pFront;
}

LNode_* pop_back(LinkedList* pLL)
{
    if (pLL->size == 0)
        return NULL;

    LNode_* pCurr = pLL->pBase;

    if (pCurr->pNext == NULL)
    {
        pLL->pBase = NULL;
        pLL->size--;
        return pCurr;
    }

    while (pCurr->pNext->pNext != NULL)
        pCurr = pCurr->pNext;

    LNode_* pLast = pCurr->pNext;
    pCurr->pNext = NULL;
    pLL->size--;

    return pLast;
}
