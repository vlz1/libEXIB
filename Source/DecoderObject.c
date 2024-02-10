#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <EXIB/EXIB.h>
#include <EXIB/Decoder.h>
#include "AllocatorInternal.h"
#include "DecoderInternal.h"

#ifndef EXIB_NO_DECODER_LUT
    #define FIELD_LUT_ROW(o) 1, 2+o, 2+o, 3+o, 3+o, 5+o, 5+o, 9+o, 9+o, 5+o, 9+o, 1+sizeof(exib_string_t)+o, 1, 2+o, 1+o, 1+o
    #define FIELD_LUT(p, n) FIELD_LUT_ROW(p+n)

    static uint8_t s_FieldSizeLUT[] = {
        FIELD_LUT(0, 0), FIELD_LUT(0, 2),
        FIELD_LUT(1, 0), FIELD_LUT(1, 2),
        FIELD_LUT(2, 0), FIELD_LUT(2, 2),
        FIELD_LUT(3, 0), FIELD_LUT(3, 2),
        FIELD_LUT(4, 0), FIELD_LUT(4, 2),
        FIELD_LUT(5, 0), FIELD_LUT(5, 2),
        FIELD_LUT(6, 0), FIELD_LUT(6, 2),
        FIELD_LUT(7, 0), FIELD_LUT(7, 2)
    };
#endif

#define OBJECT_PREFIX(field) (EXIB_ObjectPrefix*)(field + (field->named * 2) + 1);

// Calculate the position of a field relative to its parent object.
static uint32_t EXIB_DEC_ObjectRelativeOffset(EXIB_DEC_Object* object, EXIB_DEC_Field field)
{
    void* start = object->field + object->dataOffset;
    return (void*)field - start;
}

// Decode the prefix, name, and size of an aggregate type (object or array).
EXIB_DEC_Error EXIB_DEC_PartialDecodeAggregate(EXIB_DEC_Context* ctx, EXIB_DEC_Field objectField, EXIB_DEC_Object* objectOut)
{
    EXIB_FieldPrefix* field = (EXIB_FieldPrefix*)objectField;
    EXIB_ObjectPrefix* object = OBJECT_PREFIX(field);
    uint32_t objectSize;
    uint32_t sizeBytes;

    if (!EXIB_DEC_FieldIsAggregate(objectField))
        return EXIB_DEC_SetError(ctx, EXIB_DEC_ERR_AggregateExpected);

    // TODO: More bounds checking for aggregate decoding.

    sizeBytes = 2 + (object->size * 2);
    // Field prefix + Object prefix + Name16 + Size16/32 + padding
    objectOut->dataOffset = 2 + (field->named * 2) + sizeBytes + field->padding;

    if (sizeBytes == 4)
        objectSize = *(uint32_t*)(object + 1);
    else
        objectSize = *(uint16_t*)(object + 1);

    void* end = (field + objectOut->dataOffset + objectSize) - 1;
    if (EXIB_DEC_CheckBounds(ctx, end))
        return EXIB_DEC_SetError(ctx, EXIB_DEC_ERR_OutOfBounds);

    objectOut->field = field;
    objectOut->size = objectSize;
    objectOut->objectPrefix = *object;

    return EXIB_DEC_SetError(ctx, EXIB_DEC_ERR_Success);
}

EXIB_DEC_Object* EXIB_DEC_ObjectFromField(EXIB_DEC_Context* ctx, EXIB_DEC_Field field, EXIB_DEC_Object* objectOut)
{
    EXIB_FieldPrefix* prefix = (EXIB_FieldPrefix*)field;

    if (prefix->type != EXIB_TYPE_OBJECT)
    {
        ctx->lastError = EXIB_DEC_ERR_ObjectExpected;
        return NULL;
    }

    if (ctx->objectCache.field != field)
    {
        if (EXIB_DEC_PartialDecodeAggregate(ctx, field, &ctx->objectCache) != EXIB_DEC_ERR_Success)
            return NULL;
    }
    
    memcpy(objectOut, &ctx->objectCache, sizeof(EXIB_DEC_Object));

    return objectOut;
}

EXIB_DEC_Array* EXIB_DEC_ArrayFromField(EXIB_DEC_Context* ctx, EXIB_DEC_Field field, EXIB_DEC_Array* arrayOut)
{
    EXIB_FieldPrefix* prefix = (EXIB_FieldPrefix*)field;

    if (prefix->type != EXIB_TYPE_ARRAY)
    {
        ctx->lastError = EXIB_DEC_ERR_ArrayExpected;
        return NULL;
    }

    if (EXIB_DEC_PartialDecodeAggregate(ctx, field, &arrayOut->object) != EXIB_DEC_ERR_Success)
        return NULL;

    // Pre-calculate some useful information.
    arrayOut->elementSize = EXIB_GetTypeSize(arrayOut->object.objectPrefix.arrayType);
    arrayOut->elements = arrayOut->object.size ? (arrayOut->object.size / arrayOut->elementSize) : 0;
    arrayOut->data = (void*)(arrayOut->object.field) + arrayOut->object.dataOffset;

    return arrayOut;
}

EXIB_DEC_Field EXIB_DEC_FirstField(EXIB_DEC_Context* ctx, EXIB_DEC_Object* object)
{
    // An empty object has no fields!
    if (object->size == 0)
        return EXIB_DEC_INVALID_FIELD;
    
    if (EXIB_DEC_CheckBounds(ctx, object->field + object->dataOffset))
    {
        ctx->lastError = EXIB_DEC_ERR_OutOfBounds;
        return EXIB_DEC_INVALID_FIELD;
    }

    return object->field + object->dataOffset;
}

EXIB_DEC_Field EXIB_DEC_NextField(EXIB_DEC_Context* ctx, EXIB_DEC_Object* object, EXIB_DEC_Field field)
{
    EXIB_FieldPrefix* prefix = (EXIB_FieldPrefix*)field;
    EXIB_DEC_Field next = EXIB_DEC_INVALID_FIELD;

    ctx->lastError = EXIB_DEC_ERR_Success;

    if (field == EXIB_DEC_INVALID_FIELD)
    {
        // Get first field in object.
        field = EXIB_DEC_FirstField(ctx, object);
        if (!field)
            return EXIB_DEC_INVALID_FIELD;
        return field;
    }

    #ifndef EXIB_NO_DECODER_LUT
        int stride = s_FieldSizeLUT[prefix->byte];
    #else
        int stride = 1 // Field prefix
                + (prefix->named * 2) // Name16 
                + prefix->padding
                + EXIB_GetTypeSize(prefix->type);
    #endif

    if (EXIB_DEC_FieldIsAggregate(field))
    {
        EXIB_ObjectPrefix* objectPrefix = (EXIB_ObjectPrefix*)(prefix + stride);

        // Partially decode object into object cache.
        // Speeds up a potential GetObject call.
        if (EXIB_DEC_PartialDecodeAggregate(ctx, field, &ctx->objectCache) != EXIB_DEC_ERR_Success)
            return EXIB_DEC_INVALID_FIELD;

        stride += 1 // Object prefix
                  + (2 + (objectPrefix->size * 2)) // Object size
                  + ctx->objectCache.size; // Object data
    }

    next = field + stride;
    if (EXIB_DEC_ObjectRelativeOffset(object, next) >= object->size)
    {
        // End of the object, no more fields.
        return EXIB_DEC_INVALID_FIELD;
    }
    else if (EXIB_DEC_CheckBounds(ctx, next))
    {
        ctx->lastError = EXIB_DEC_ERR_OutOfBounds;
        return EXIB_DEC_INVALID_FIELD;
    }

    return next;
}

EXIB_DEC_Field EXIB_DEC_FindField(EXIB_DEC_Context* ctx, EXIB_DEC_Object* parent, const char* name)
{
    int nameLength = strlen(name);

    if (parent == NULL)
        parent = &ctx->rootObject;

    // Iterate through the object's fields.
    EXIB_DEC_Field field = EXIB_DEC_NextField(ctx, parent, EXIB_DEC_INVALID_FIELD);
    while (field != EXIB_DEC_INVALID_FIELD)
    {
        exib_string_t nameOffset = EXIB_DEC_GetFieldNameOffset(ctx, field);
        EXIB_DEC_TString nameString = EXIB_DEC_GetStringFromOffset(ctx, nameOffset);

        if (nameString != EXIB_DEC_INVALID_STRING)
        {
            // Return field if the names match.
            if ((nameLength == nameString->length) &&
                !strncmp(name, nameString->string, nameString->length))
                return field;
        }

        field = EXIB_DEC_NextField(ctx, parent, field);
    }
    
    return EXIB_DEC_INVALID_FIELD;
}

EXIB_DEC_Error EXIB_DEC_FindObject(EXIB_DEC_Context* ctx, 
                                   EXIB_DEC_Object* parent,
                                   const char* name,
                                   EXIB_DEC_Object* objectOut)
{
    EXIB_DEC_Field field = EXIB_DEC_FindField(ctx, parent, name);

    if (field == EXIB_DEC_INVALID_FIELD)
        return EXIB_DEC_GetLastError(ctx);

    if (!EXIB_DEC_ObjectFromField(ctx, field, objectOut))
        return EXIB_DEC_GetLastError(ctx);
    
    return EXIB_DEC_SetError(ctx, EXIB_DEC_ERR_Success);
}
