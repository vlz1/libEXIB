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

static int EXIB_DEC_ArrayDecodeNext(EXIB_DEC_Context* ctx, EXIB_DEC_Array* array, EXIB_FieldPrefix* field, EXIB_DEC_FieldValue* value)
{
    value->type = array->object.objectPrefix.arrayType;
    if (value->type == EXIB_TYPE_ARRAY)
    {
        return (EXIB_DEC_ArrayFromField(ctx, field, &value->array) == NULL)
            ? -1 : 0;
    }
    else if (value->type == EXIB_TYPE_OBJECT)
    {
        return (EXIB_DEC_ObjectFromField(ctx, field, &value->object) == NULL)
            ? -1 : 0;
    }

    return -1;
}

static int EXIB_DEC_ArraySpecialNext(EXIB_DEC_Context* ctx, EXIB_DEC_Array* array, EXIB_DEC_FieldValue* value)
{
    // Position of current field prefix within array data.
    EXIB_FieldPrefix* field = value->object.field;
    EXIB_Type type = array->object.objectPrefix.arrayType;
    if (field == NULL)
    {
        // TODO: Safety!
        field = (EXIB_FieldPrefix*)array->data;
        return EXIB_DEC_ArrayDecodeNext(ctx, array, field, value);
    }

    return EXIB_DEC_ArrayDecodeNext(ctx, array, field, value);
}

int EXIB_DEC_ArrayNext(EXIB_DEC_Context* ctx, EXIB_DEC_Array* array, EXIB_DEC_FieldValue* value)
{
    // Use a different function for arrays of arrays/arrays of objects.
    if (array->elementSize == 0)
        return EXIB_DEC_ArraySpecialNext(ctx, array, value);

    // Position of current element within array data.
    void* position = value->value;
    if (position == NULL)
    {
        if (array->elements == 0)
            return -1;

        // Return first element of array.
        value->value = array->data;
        return 0;
    }

    ptrdiff_t elementOffset = (ptrdiff_t)position - (ptrdiff_t)array->data;
    if (elementOffset < 0)
        return -1; // Undershot the array, bad pointer!

    ptrdiff_t currentIndex = (elementOffset != 0) ? (elementOffset / array->elementSize) : 0;
    int nextIndex = (int)(currentIndex + 1);
    if (nextIndex >= array->elements)
        return -1; // Out of bounds or the array ended.

    value->type = array->object.objectPrefix.arrayType;
    value->value = position + EXIB_DEC_ArrayGetStride(array);
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