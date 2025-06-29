#ifndef __MEMORY_H__
#define __MEMORY_H__

#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>

#define KiB 1024        // 1 KiB
#define MiB 1024 * KiB  // 1 MiB
#define GiB 1024 * MiB  // 1 GiB

#define MEMORY_POOL_SIZE 2097152  // 2MiB
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

void  mp_init(MemoryPool* mp);
void* mp_alloc(MemoryPool* mp, size_t size);
void* mp_realloc(MemoryPool* mp, void* ptr, size_t new_size);
void  mp_free(MemoryPool* mp, void* ptr);

// Standard memory pool
extern MemoryPool* mpool_;

#endif
