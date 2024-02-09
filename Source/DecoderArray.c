#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <EXIB/EXIB.h>
#include <EXIB/Decoder.h>
#include "AllocatorInternal.h"
#include "DecoderInternal.h"

/**
 * Make sure an element pointer is within the bounds
 * of the given array.
 * @param ctx Decoder context.
 * @param array Decoder array.
 * @param pointer Pointer to array element.
 * @return 0 if the pointer is valid, 1 otherwise.
 */
static inline int EXIB_DEC_CheckArrayPointer(EXIB_DEC_Context* ctx, EXIB_DEC_Array* array, EXIB_Value* pointer)
{
    uintptr_t dataStart = (uintptr_t)array->data;
    uintptr_t dataEnd   = dataStart + (array->elements * EXIB_GetTypeSize(array->object.objectPrefix.arrayType));
    uintptr_t ptr = (uintptr_t)pointer;
    // TODO: Should we check alignment too?
    return (ptr < dataStart || ptr >= dataEnd);
}

EXIB_Value* EXIB_DEC_ArrayBegin(EXIB_DEC_Context* ctx, EXIB_DEC_Array* array, size_t* lengthOut)
{
    if (array->elements == 0)
    {
        ctx->lastError = EXIB_DEC_ERR_InvalidArrayIndex;
        return NULL;
    }

    if (lengthOut != NULL)
        *lengthOut = array->elements;

    ctx->lastError = EXIB_DEC_ERR_Success;
    return array->data;
}

int EXIB_DEC_ArrayNext(EXIB_DEC_Context* ctx, EXIB_DEC_Array* array, EXIB_Value** value)
{
    void* val = *value;
    ptrdiff_t pointerDiff;
    intptr_t currentIndex;
    int nextIndex;

    *value = NULL;
    if (val == NULL)
    {
        if (array->elements == 0)
            return -1;

        *value = array->data;
        return 0;
    }

    // TODO: Handle arrays of objects/arrays of arrays.
    assert(array->elementSize != 0);

    pointerDiff = (ptrdiff_t)val - (ptrdiff_t)array->data;
    currentIndex = (pointerDiff != 0) ? (pointerDiff / array->elementSize) : 0;
    nextIndex = (int)currentIndex + 1;
    if (currentIndex < 0 || nextIndex >= array->elements)
        return -1;

    *value = val + EXIB_DEC_ArrayGetStride(array);
    return nextIndex;
}

EXIB_Value* EXIB_DEC_ArrayLocateElement(EXIB_DEC_Context* ctx, EXIB_DEC_Array* array, size_t i)
{
    if (i >= array->elements)
    {
        ctx->lastError = EXIB_DEC_ERR_InvalidArrayIndex;
        return NULL;
    }

    int elementSize = EXIB_GetTypeSize(array->object.objectPrefix.arrayType);
    ctx->lastError = EXIB_DEC_ERR_Success;
    return (void*)(array->object.field) + array->object.dataOffset + (i * elementSize);
}