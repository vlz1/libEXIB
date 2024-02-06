#ifndef _EXIB_ALLOCATOR_INTERNAL_H
#define _EXIB_ALLOCATOR_INTERNAL_H

#include <stdint.h>
#include <stddef.h>

typedef size_t bitmap_t;

#define OBJECTS_PER_BLOCK 768
#define BITMAP_ELEMENT_BITS ((int)sizeof(bitmap_t) * 8)
#define BITMAP_SIZE (OBJECTS_PER_BLOCK / BITMAP_ELEMENT_BITS)

typedef struct _EXIB_MemoryBlock
{
    void* block;
    size_t totalObjects;
    size_t freeObjects;
    bitmap_t freeMap[BITMAP_SIZE];
    struct _EXIB_MemoryBlock* next;
} EXIB_MemoryBlock;

typedef struct _EXIB_MemoryPool
{
    size_t objectSize; // Size of objects that will be allocated.
    EXIB_MemoryBlock* blockList; // Forward list of memory blocks.
} EXIB_MemoryPool;

int   EXIB_InitializePool(EXIB_MemoryPool* pool, size_t objectSize);
void  EXIB_DestroyPool(EXIB_MemoryPool* pool);
void* EXIB_PoolAlloc(EXIB_MemoryPool* pool);
void  EXIB_PoolFree(EXIB_MemoryPool* pool, void* ptr);

void* EXIB_Calloc(size_t objectSize, size_t objectCount);

char* EXIB_Strdup(const char* str);

#define EXIB_New(type) EXIB_Calloc(sizeof(type), 1)

#endif //_EXIB_ALLOCATOR_INTERNAL_H
