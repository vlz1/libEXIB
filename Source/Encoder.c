#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <EXIB/EXIB.h>
#include <EXIB/Encoder.h>
#include "EXIB/EncoderTypes.h"
#include "EncoderInternal.h"

static EXIB_ENC_Options s_DefaultOptions =
    {
        .bufferSize = 65535,
        .stringCacheCapacity = 128,
        .arrayCapacity = 32,
        .datumName = NULL
    };

void EXIB_ENC_GetDefaultOptions(EXIB_ENC_Options* options)
{
    *options = s_DefaultOptions;
}

EXIB_ENC_Context* EXIB_ENC_CreateContext(EXIB_ENC_Options* options)
{
    EXIB_ENC_Context* ctx = EXIB_New(EXIB_ENC_Context);
    if (!options)
        options = &s_DefaultOptions;
    ctx->options = *options;

    // Allocate encode buffer.
    ctx->encodeBuffer = EXIB_Calloc(options->bufferSize, 1);
    ctx->encodeBufferSize = ctx->options.bufferSize;

    // Initialize field pool.
    EXIB_InitializePool(&ctx->fieldPool, sizeof(EXIB_ENC_Field));

    // Allocate string cache.
    ctx->stringCacheCapacity = ctx->options.stringCacheCapacity;
    ctx->stringCache = EXIB_Calloc(ctx->options.stringCacheCapacity,
                      sizeof(EXIB_ENC_StringEntry));

    // Initialize root object.
    if (options->datumName)
    {
        EXIB_ENC_StringEntry* nameEntry = EXIB_ENC_GetStringEntry(ctx, options->datumName);
        ctx->rootObject.field.nameOffset = nameEntry->offset;
    }
    else
        ctx->rootObject.field.nameOffset = EXIB_INVALID_STRING;
    ctx->rootObject.field.type = EXIB_TYPE_OBJECT;

    return ctx;
}

void EXIB_ENC_FreeContext(EXIB_ENC_Context* ctx)
{
    if (ctx->encodeBuffer)
        EXIB_Free(ctx->encodeBuffer);

    EXIB_DestroyPool(&ctx->fieldPool);

    if (ctx->stringCache)
        EXIB_Free(ctx->stringCache);

    EXIB_Free(ctx);
}

EXIB_ENC_Error EXIB_ENC_GetLastError(EXIB_ENC_Context* ctx)
{
    return ctx->lastError;
}

size_t EXIB_ENC_EncodeStringTable(EXIB_ENC_Context* ctx, size_t offset)
{
    for (int i = 0; i < ctx->stringCacheSize; ++i)
    {
        EXIB_ENC_StringEntry* cacheEntry = &ctx->stringCache[i];
        EXIB_StringEntry* e = (EXIB_StringEntry*)&ctx->encodeBuffer[offset + cacheEntry->offset];

        e->length = cacheEntry->length;
        memcpy(e->string, cacheEntry->buffer, cacheEntry->length);
    }

    return ctx->stringOffset;
}

size_t EXIB_ENC_EncodeField(EXIB_ENC_Context* ctx, EXIB_ENC_Field* field, size_t offset)
{
    int typeSize = EXIB_GetTypeSize(field->type);
    int nameSize = (field->nameOffset != EXIB_INVALID_STRING) ? (int)sizeof(exib_string_t) : 0;
    int bytes    = 1 + nameSize + typeSize;
    EXIB_FieldPrefix fieldPrefix = {
        .type    = field->type,
        .named   = nameSize != 0
    };

    if (typeSize > 1)
    {
        int r = ((int)offset + 1 + nameSize) % typeSize;
        int padding = (r > 0) ? (typeSize - r) : 0;
        bytes += padding;
        fieldPrefix.padding = padding;
    }

    // Write field prefix.
    ctx->encodeBuffer[offset++] = fieldPrefix.byte;

    // Write name if one is present.
    if (fieldPrefix.named)
    {
        ctx->encodeBuffer[offset] = field->nameOffset & 0xFF;
        ctx->encodeBuffer[offset + 1] = (field->nameOffset >> 8) & 0xFF;
        offset += 2;
    }

    // TODO: Fix padding for arrays and objects.
    // Add padding bytes if necessary.
    for (int i = 0; i < fieldPrefix.padding; ++i)
        ctx->encodeBuffer[offset++] = 0;

    if (typeSize > 0)
    {
        // Write value.
        *(void**)&ctx->encodeBuffer[offset] = field->value.pointer;
    }

    return bytes;
}

size_t EXIB_ENC_EncodeArray(EXIB_ENC_Context* ctx, EXIB_ENC_Array* array, size_t offset);
size_t EXIB_ENC_EncodeObject(EXIB_ENC_Context* ctx, EXIB_ENC_Object* object, size_t offset);

int EXIB_ENC_EncodeObjectSize(EXIB_ENC_Context* ctx, size_t sizeOffset, size_t innerSize)
{
    if (innerSize < 65536)
    {
        *(uint16_t*)&ctx->encodeBuffer[sizeOffset] = innerSize;
        return 0;
    }

    // Shift data forward 2 bytes to make room for Size32 encoding.
    ctx->encodeBuffer[sizeOffset - 1] |= 0x80; // Set Size flag on object.
    memmove(ctx->encodeBuffer + sizeOffset + 4,
            ctx->encodeBuffer + sizeOffset + 2,
            innerSize);
    *(uint32_t*)&ctx->encodeBuffer[sizeOffset] = innerSize;

    return 2;
}

size_t EXIB_ENC_EncodeSpecialArray(EXIB_ENC_Context* ctx, EXIB_ENC_Array* array, size_t offset)
{
    EXIB_ENC_Field* field = array->object.children;
    size_t origin = offset;
    size_t sizeOffset;
    size_t innerSize = 0;
    EXIB_ObjectPrefix objectPrefix = {
        .arrayType = array->object.field.elementType,
        .size = 0
    };

    // Write object prefix, use Size16 encoding at first.
    ctx->encodeBuffer[offset++] = objectPrefix.byte;

    // Allocate space and save position of size.
    sizeOffset = offset;
    offset += 2;

    EXIB_Type type = objectPrefix.arrayType;

    // Loop through object/array child list.
    while (field != NULL)
    {
        size_t bytes;

        if (field->type != type)
        {
            // TODO: Actually handle the error.
            exit(1);
        }

        if (field->type == EXIB_TYPE_OBJECT)
            bytes = EXIB_ENC_EncodeObject(ctx, (EXIB_ENC_Object*)field, offset);
        else if(field->type == EXIB_TYPE_ARRAY)
            bytes = EXIB_ENC_EncodeArray(ctx, (EXIB_ENC_Array*)field, offset);
        offset += bytes;
        innerSize += bytes;
        field = field->next;
    }

    offset += EXIB_ENC_EncodeObjectSize(ctx, sizeOffset, innerSize);

    return offset - origin;
}

size_t EXIB_ENC_EncodeArray(EXIB_ENC_Context* ctx, EXIB_ENC_Array* array, size_t offset)
{
    EXIB_ENC_Field* field = &array->object.field;
    size_t fieldOffset = offset;

    // Write field prefix and name.
    offset += EXIB_ENC_EncodeField(ctx, field, offset);

    // Deal with arrays of arrays or arrays of objects.
    if (field->elementType >= EXIB_TYPE_ARRAY)
    {
        offset += EXIB_ENC_EncodeSpecialArray(ctx, array, offset);
        return offset - fieldOffset;
    }

    // Calculate size.
    size_t dataSize = EXIB_GetTypeSize(field->elementType) * EXIB_ENC_ArrayGetSize(array);

    // Write object prefix.
    EXIB_ObjectPrefix objectPrefix = {
        .arrayType = field->elementType,
        .size = (dataSize > UINT16_MAX)
    };
    ctx->encodeBuffer[offset++] = objectPrefix.byte;

    // Write size.
    if (objectPrefix.size)
    {
        *(uint32_t*)&ctx->encodeBuffer[offset] = dataSize;
        offset += 4;
    }
    else
    {
        *(uint16_t*)&ctx->encodeBuffer[offset] = dataSize;
        offset += 2;
    }

    // Write data.
    memcpy(&ctx->encodeBuffer[offset], array->valueElements, dataSize);
    offset += dataSize;

    return offset - fieldOffset;
}

size_t EXIB_ENC_EncodeObject(EXIB_ENC_Context* ctx, EXIB_ENC_Object* object, size_t offset)
{
    EXIB_ENC_Field* field = object->children;
    size_t fieldOffset = offset;
    size_t sizeOffset;
    size_t innerSize = 0;
    EXIB_ObjectPrefix objectPrefix = { 0 };

    // Write field prefix and name.
    offset += EXIB_ENC_EncodeField(ctx, &object->field, offset);

    // Write object prefix, use Size16 encoding at first.
    ctx->encodeBuffer[offset++] = objectPrefix.byte;

    // Allocate space and save position of size.
    sizeOffset = offset;
    offset += 2;

    while (field != NULL)
    {
        size_t bytes;

        if (field->type == EXIB_TYPE_OBJECT)
            bytes = EXIB_ENC_EncodeObject(ctx, (EXIB_ENC_Object*)field, offset);
        else if (field->type == EXIB_TYPE_STRING || field->type == EXIB_TYPE_ARRAY)
            bytes = EXIB_ENC_EncodeArray(ctx, (EXIB_ENC_Array*)field, offset);
        else
            bytes = EXIB_ENC_EncodeField(ctx, field, offset);

        offset += bytes;
        innerSize += bytes;
        field = field->next;
    }

    offset += EXIB_ENC_EncodeObjectSize(ctx, sizeOffset, innerSize);

    return offset - fieldOffset;
}

EXIB_Header* EXIB_ENC_Encode(EXIB_ENC_Context* ctx)
{
    EXIB_Header* header = (EXIB_Header*)ctx->encodeBuffer;
    size_t stringTableSize = 0;
    size_t offset = sizeof(EXIB_Header);

    // TODO: Add parameter for enabling the extended header.
    offset += EXIB_ENC_EncodeObject(ctx, &ctx->rootObject, offset);

    stringTableSize = EXIB_ENC_EncodeStringTable(ctx, offset);
    offset += stringTableSize;

    header->magic      = EXIB_MAGIC;
    header->version    = EXIB_VERSION;
    header->datumSize  = offset;
    header->stringSize = stringTableSize;
    return header;
}
