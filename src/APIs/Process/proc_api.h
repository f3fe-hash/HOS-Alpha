#ifndef __PROCESS_API_H__
#define __PROCESS_API_H__

#include <pthread.h>

#include "../../lib/ll.h"
#include "../../root.h"

typedef struct
{
    int pid;
    void(*entry)(void*);
    void* args;

    bool running;
    bool finished;

    pthread_mutex_t lock;
    pthread_cond_t cond;
} HOSProcess;

void PROCAPI_init(int num_threads);

HOSProcess* PROCAPI_spawn(void(*entry)(void*), void* args);

void PROCAPI_kill(HOSProcess* pProc);
void PROCAPI_wait(HOSProcess* pProc);

void* thread_task(void* arg);

pthread_t* newThread();

extern LinkedList* proc_list;
extern LinkedList* thread_list;

#endif