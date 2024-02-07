#include <stdlib.h>
#include <string.h>
#include <EXIB/EXIB.h>
#include <EXIB/Encoder.h>
#include "AllocatorInternal.h"
#include "EXIB/EncoderTypes.h"
#include "EncoderInternal.h"

int EXIB_ENC_InitializeArray(EXIB_ENC_Context* ctx,
                             EXIB_ENC_Array* array,
                             EXIB_ENC_Object* parent,
                             const char* name,
                             EXIB_Type type,
                             int reserve)
{
    EXIB_ENC_Field* field = &array->object.field;
    EXIB_ENC_InitializeField(ctx, field, parent, name,
                           EXIB_TYPE_ARRAY);
    
    field->elementType = type;

    if (reserve < 0)
        reserve = 32;

    // Reserve space if this is an array of values.
    if (EXIB_ENC_ArrayReserve(array, reserve))
        return 1;

    return 0;
}

EXIB_ENC_Array* EXIB_ENC_AddArray(EXIB_ENC_Context* ctx,
                                  EXIB_ENC_Object* parent,
                                  const char* name,
                                  EXIB_Type elementType)
{
    EXIB_ENC_Array* array = EXIB_New(EXIB_ENC_Array);

    if (EXIB_ENC_InitializeArray(ctx, array, parent, name, elementType, -1) != 0)
    {
        EXIB_Free(array);
        return NULL;
    }

    return array;
}

int EXIB_ENC_ArrayReserve(EXIB_ENC_Array* array, uint32_t newCapacity)
{
    EXIB_Type type = array->object.field.elementType;
    int elementSize = EXIB_GetTypeSize(type);

    // Objects and arrays use a linked list instead.
    if (type == EXIB_TYPE_OBJECT || type == EXIB_TYPE_ARRAY)
        return 0;

    // Don't try to shrink the array.
    if (!newCapacity || newCapacity < array->elementCapacity)
        return 0;

    if (!array->valueElements)
    {
        // Allocate array from scratch.
        array->elementCapacity = newCapacity;
        array->valueElements = EXIB_Calloc(elementSize, 
                                           array->elementCapacity);
        if (!array->valueElements)
            return 1;
        return 0;
    }

    void* elements = EXIB_Calloc(elementSize, newCapacity);
    if (!elements)
        return 1;

    // Copy data into new buffer, then free old one.
    memcpy(elements, array->valueElements, elementSize * array->elementCapacity);
    EXIB_Free(array->valueElements);
    array->valueElements = elements;
    array->elementCapacity = newCapacity;

    return 0;
}

void EXIB_ENC_ArrayResize(EXIB_ENC_Array* array, size_t newSize)
{
    uint32_t capacity = array->elementCapacity;

    if (capacity < newSize)
        EXIB_ENC_ArrayReserve(array, capacity + (newSize - capacity));

    array->elementCount = newSize;
}

size_t EXIB_ENC_ArrayGetSize(EXIB_ENC_Array* array)
{
    return array->elementCount;
}

void* EXIB_ENC_ArrayGetData(EXIB_ENC_Array* array)
{
    return array->valueElements;
}

void* EXIB_ENC_ArraySet(EXIB_ENC_Array* array, size_t index, EXIB_Value value)
{
    int elementSize = EXIB_GetTypeSize(array->object.field.elementType);
    
    if (index >= array->elementCount)
        return NULL;

    void* p = ((void*)array->valueElements) + (index * elementSize);

    if (elementSize == 1)
        *(uint8_t*)p = value.uint8;
    else if (elementSize == 2)
        *(uint16_t*)p = value.uint16;
    else if (elementSize == 4)
        *(uint32_t*)p = value.uint32;
    else if (elementSize == 8)
        *(uint64_t*)p = value.uint64;

    return p;
}

void* EXIB_ENC_ArrayAppend(EXIB_ENC_Array* array, EXIB_Value value)
{
    if (array->elementCount == array->elementCapacity)
    {
        if (!EXIB_ENC_ArrayReserve(array, array->elementCapacity + 32))
            return NULL;
    }

    return EXIB_ENC_ArraySet(array, array->elementCount++, value);
}

EXIB_ENC_Object* EXIB_ENC_ArrayAddObject(EXIB_ENC_Context* ctx, EXIB_ENC_Array* array, const char* name)
{
    return EXIB_ENC_AddObject(ctx, &array->object, name);
}