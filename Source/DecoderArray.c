#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <EXIB/EXIB.h>
#include <EXIB/Decoder.h>
#include "AllocatorInternal.h"
#include "DecoderInternal.h"

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
    return (void*)(array->object.field) + array->object.dataOffset;
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