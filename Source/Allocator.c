#include <stdlib.h>
#include <string.h>
#include <EXIB/EXIB.h>
#include "AllocatorInternal.h"

#ifdef EXIB_NO_LIBC_MALLOC
static exib_malloc_t EXIB_AllocFn  = NULL;
static exib_free_t   EXIB_FreeFn   = NULL;
#else
static exib_malloc_t EXIB_MallocFn = malloc;
static exib_free_t   EXIB_FreeFn   = free;
#endif

// Expand the pool by one block.
EXIB_MemoryBlock* EXIB_PoolExpand(EXIB_MemoryPool* pool)
{
    EXIB_MemoryBlock* block = EXIB_Alloc(sizeof(EXIB_MemoryBlock));
    size_t blockSize = pool->objectSize * OBJECTS_PER_BLOCK;

    if (!block)
        return NULL;

    memset(block->freeMap, 0xFF, sizeof(block->freeMap));
    block->block = EXIB_Alloc(blockSize);

    if (!block->block)
    {
        EXIB_Free(block);
        return NULL;
    }

    memset(block->block, 0, blockSize);
    block->totalObjects = OBJECTS_PER_BLOCK;
    block->freeObjects  = OBJECTS_PER_BLOCK;

    if (pool->blockList)
        block->next = pool->blockList;
    else
        block->next = NULL;

    pool->blockList = block;
    return block;
}

int EXIB_InitializePool(EXIB_MemoryPool* pool, size_t objectSize)
{
    pool->objectSize = objectSize;
    pool->blockList  = NULL;
    return EXIB_PoolExpand(pool) != NULL;
}

void EXIB_DestroyPool(EXIB_MemoryPool* pool)
{
    EXIB_MemoryBlock* block = pool->blockList;

    while (block != NULL)
    {
        EXIB_MemoryBlock* next = block->next;
        EXIB_Free(block->block);
        EXIB_Free(block);
        block = next;
    }
}

int EXIB_BlockAllocIndex(EXIB_MemoryBlock* block)
{
    // TODO: Make block search faster.
    for (int i = 0; i < BITMAP_SIZE; ++i)
    {
        bitmap_t e = block->freeMap[i];
#if SIZE_MAX == UINT32_MAX
        int bit = __builtin_ffsll((int)e) - 1;
#else
        int bit = __builtin_ffsll((long long)e) - 1;
#endif
        if (bit >= 0)
        {
            --block->freeObjects;
#if SIZE_MAX == UINT32_MAX
            block->freeMap[i] ^= (1ULL << bit);
#else
            block->freeMap[i] ^= (1UL << bit);
#endif
            return (i * BITMAP_ELEMENT_BITS) + bit;
        }
    }

    return -1;
}

void* EXIB_PoolAlloc(EXIB_MemoryPool* pool)
{
    EXIB_MemoryBlock* block = pool->blockList;
    int index;

    while (block != NULL)
    {
        if (block->freeObjects > 0)
            break;
        block = block->next;
    }

    if (block == NULL)
    {
        block = EXIB_PoolExpand(pool);
        if (!block)
            return NULL;
    }

    index = EXIB_BlockAllocIndex(block);
    return block->block + (pool->objectSize * index);
}

void EXIB_PoolFree(EXIB_MemoryPool* pool, void* ptr)
{
    // TODO: Implement EXIB_PoolFree
}

void* EXIB_Alloc(size_t n)
{
    if (!EXIB_MallocFn)
        return NULL;
    return EXIB_MallocFn(n);
}

void EXIB_Free(void* ptr)
{
    if (!EXIB_FreeFn)
        return;
    EXIB_FreeFn(ptr);
}

void* EXIB_Calloc(size_t objectSize, size_t objectCount)
{
    size_t size = objectSize * objectCount;
    void* p = EXIB_Alloc(size);
    return memset(p, 0, size);
}

char* EXIB_Strdup(const char* str)
{
    size_t len = strlen(str);
    char* buf = EXIB_Alloc(len + 1);

    // Copy whole string, including NULL terminator.
    memcpy(buf, str, len + 1);

    return buf;
}