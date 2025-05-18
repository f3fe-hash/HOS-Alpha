#ifndef __MEMORY_H__
#define __MEMORY_H__

#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>

#define MEMORY_POOL_SIZE 1048576 // 1MiB
#define ALIGNMENT 8

#define ALIGN(size) (((size) + (ALIGNMENT - 1)) & ~(ALIGNMENT - 1))
#define HEADER_SIZE ALIGN(sizeof(MemoryBlock_))

typedef struct MemoryBlock_
{
    size_t size;
    struct MemoryBlock_* next;
} MemoryBlock_;

typedef struct
{
    union
    {
        char raw[MEMORY_POOL_SIZE];
        max_align_t _align;
    };
    MemoryBlock_* free_list;
} MemoryPool;

void mp_init(MemoryPool* mp);
void* mp_alloc(MemoryPool* mp, size_t size);
void mp_free(MemoryPool* mp, void* ptr);

// Standard memory pool
extern MemoryPool* mpool_;

#endif
