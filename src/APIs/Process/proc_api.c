#include "proc_api.h"

LinkedList* proc_list = NULL;
LinkedList* thread_list = NULL;

pthread_mutex_t proc_mutex = PTHREAD_MUTEX_INITIALIZER;

void* thread_task(void* arg);

pthread_t* newThread()
{
    pthread_t* pNewThread = mp_alloc(mpool_, sizeof(pthread_t));
    if (pthread_create(pNewThread, NULL, thread_task, NULL) != 0)
    {
        perror("pthread_create failed");
        exit(1);
    }
    return pNewThread;
}

void PROCAPI_init(int num_threads)
{
    proc_list = ll_init();
    thread_list = ll_init();
    pthread_mutex_init(&proc_mutex, NULL);

    for (int i = 0; i < num_threads; i++)
    {
        pthread_t* thread = newThread();
        append_node(thread_list, create_node(thread));
    }
}

HOSProcess* PROCAPI_spawn(void(*entry)(void*), void* args)
{
    HOSProcess* pNewProc = mp_alloc(mpool_, sizeof(HOSProcess));
    pNewProc->entry = entry;
    pNewProc->args = args;
    pNewProc->running = false;
    pNewProc->finished = false;
    pthread_mutex_init(&pNewProc->lock, NULL);
    pthread_cond_init(&pNewProc->cond, NULL);

    pthread_mutex_lock(&proc_mutex);
    prepend_node(proc_list, create_node((void*)pNewProc));
    pthread_mutex_unlock(&proc_mutex);

    return pNewProc;
}

void PROCAPI_kill(HOSProcess* pProc)
{
    pthread_mutex_lock(&proc_mutex);

    LNode_* pPrev = NULL;
    LNode_* pCurr = proc_list->pBase;

    while (pCurr)
    {
        if ((HOSProcess*)pCurr->pData == pProc)
        {
            if (pPrev)
                pPrev->pNext = pCurr->pNext;
            else
                proc_list->pBase = pCurr->pNext;

            proc_list->size--;

            pthread_mutex_unlock(&proc_mutex);

            pthread_mutex_destroy(&pProc->lock);
            pthread_cond_destroy(&pProc->cond);
            mp_free(mpool_, pCurr);
            mp_free(mpool_, pProc);
            return;
        }

        pPrev = pCurr;
        pCurr = pCurr->pNext;
    }

    pthread_mutex_unlock(&proc_mutex);

    // If the process is already running, we can't kill it
    printf("Warning: Cannot kill process %p because it's already running\n", (void*)pProc);
}

void PROCAPI_wait(HOSProcess* pProc)
{
    pthread_mutex_lock(&pProc->lock);
    while (!pProc->finished)
        pthread_cond_wait(&pProc->cond, &pProc->lock);
    pthread_mutex_unlock(&pProc->lock);
}

void* thread_task(void* arg)
{
    while (true)
    {
        pthread_mutex_lock(&proc_mutex);
        if (proc_list->size > 0)
        {
            LNode_* pNode = pop_front(proc_list);
            pthread_mutex_unlock(&proc_mutex);
            if (pNode)
            {
                HOSProcess* proc = (HOSProcess*)pNode->pData;

                pthread_mutex_lock(&proc->lock);
                proc->running = true;
                pthread_mutex_unlock(&proc->lock);

                proc->entry(proc->args);

                pthread_mutex_lock(&proc->lock);
                proc->finished = true;
                pthread_cond_broadcast(&proc->cond);
                pthread_mutex_unlock(&proc->lock);
            }
        }
        else
        {
            pthread_mutex_unlock(&proc_mutex);
            usleep(1000);
        }
    }

    return NULL; // Required by pthread function signature
}