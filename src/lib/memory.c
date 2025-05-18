#include "memory.h"

static MemoryPool mpool_instance;
MemoryPool* mpool_ = &mpool_instance;

void mp_init(MemoryPool* mp)
{
    mp->free_list = (MemoryBlock_*)mp->raw;
    mp->free_list->size = MEMORY_POOL_SIZE;
    mp->free_list->next = NULL;
}

void* mp_alloc(MemoryPool* mp, size_t size)
{
    if (!mp || size == 0)
        return NULL;

    size_t total_size = ALIGN(size) + HEADER_SIZE;
    MemoryBlock_ *prev = NULL, *curr = mp->free_list;

    while (curr)
    {
        if (curr->size >= total_size)
        {
            // Can we split the block safely?
            size_t remaining = curr->size - total_size;

            if (remaining >= HEADER_SIZE + ALIGNMENT)
            {
                // Split: allocated part is the front, rest becomes a new block
                MemoryBlock_* next_block = (MemoryBlock_*)((char*)curr + total_size);
                next_block->size = remaining;
                next_block->next = curr->next;

                if (prev)
                    prev->next = next_block;
                else
                    mp->free_list = next_block;

                curr->size = total_size; // shrink current block to allocated size
            }
            else
            {
                // Can't split safely; give whole block
                if (prev)
                    prev->next = curr->next;
                else
                    mp->free_list = curr->next;
            }

            return (void*)((char*)curr + HEADER_SIZE);
        }

        prev = curr;
        curr = curr->next;
    }

    return NULL; // No suitable block
}

void mp_free(MemoryPool* mp, void* ptr)
{
    if (!ptr)
        return;

    MemoryBlock_* block = (MemoryBlock_*)((char*)ptr - HEADER_SIZE);
    block->next = mp->free_list;
    mp->free_list = block;
}
