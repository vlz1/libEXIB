#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <EXIB/EXIB.h>
#include <EXIB/Decoder.h>
#include "AllocatorInternal.h"
#include "DecoderInternal.h"

static const char* s_DecoderErrors[] = {
    "Success",
    "Invalid header",
    "Decode buffer too small",
    "Bad checksum",
    "Invalid root object",
    "Out of bounds",
    "Object expected",
    "Array expected"
};

static EXIB_DEC_Options s_DefaultOptions ={
        
};

EXIB_DEC_Context* EXIB_DEC_CreateContext(EXIB_DEC_Options* options)
{
    EXIB_DEC_Context* ctx = EXIB_New(EXIB_DEC_Context);

    ctx->lastError = EXIB_DEC_ERR_Success;
     
    if (!options)
        options = &s_DefaultOptions;
    ctx->options = *options;

    return ctx;
}

EXIB_DEC_Context* EXIB_DEC_CreateBufferedContext(const void* buffer,
                                                 size_t bufferSize,
                                                 EXIB_DEC_Options* options)
{
    EXIB_DEC_Context* ctx = EXIB_DEC_CreateContext(options);
    EXIB_DEC_ResetContext(ctx, buffer, bufferSize);
    return ctx;
}

EXIB_DEC_Error EXIB_DEC_ResetContext(EXIB_DEC_Context* ctx,
                                     const void* buffer,
                                     size_t bufferSize)
{
    size_t expectedSize = EXIB_MINIMUM;

    // Set buffer and size.
    ctx->buffer = (void*)buffer;
    ctx->bufferSize = bufferSize;

    // Clear root object.
    ctx->rootObject.field = EXIB_DEC_INVALID_FIELD;
    ctx->rootObject.size  = 0;
    ctx->rootObject.dataOffset = 0;

    /**
     * The header must be meticulously validated because the decoder
     * effectively passes out raw pointers within the buffer.
     */

    const EXIB_Header* header = ctx->buffer;
    if (bufferSize < expectedSize) // Ensure buffer meets minimum size.
        return EXIB_DEC_SetError(ctx, EXIB_DEC_ERR_InvalidHeader);
    else if (EXIB_CheckHeader(header)) // Validate the header.
        return EXIB_DEC_SetError(ctx, EXIB_DEC_ERR_InvalidHeader);
    else if (bufferSize < header->datumSize) // Make sure we have the whole datum.
        return EXIB_DEC_SetError(ctx, EXIB_DEC_ERR_BufferTooSmall);
    else
    {
        expectedSize += header->extendedSize + header->stringSize;

        // Extended header or string table not accounted for in datumSize, ABORT.
        if (header->datumSize < expectedSize)
            return EXIB_DEC_SetError(ctx, EXIB_DEC_ERR_InvalidHeader);
        
        EXIB_FieldPrefix* rootField = ((EXIB_FieldPrefix*)header) + sizeof(EXIB_Header) + header->extendedSize;
        if (EXIB_DEC_PartialDecodeObject(ctx, rootField, &ctx->rootObject) 
            != EXIB_DEC_ERR_Success)
        {
            return EXIB_DEC_SetError(ctx, EXIB_DEC_ERR_InvalidRoot);
        }

        if (header->stringSize > 0)
        {
            ctx->stringTable = ctx->rootObject.field + 
                ctx->rootObject.dataOffset +
                ctx->rootObject.size;
        }
    }

    return EXIB_DEC_SetError(ctx, EXIB_DEC_ERR_Success);
}

void EXIB_DEC_FreeContext(EXIB_DEC_Context* ctx)
{
    EXIB_Free(ctx);
}

EXIB_DEC_Error EXIB_DEC_GetLastError(EXIB_DEC_Context* ctx)
{
    return ctx->lastError;
}

const char* EXIB_DEC_GetLastErrorName(EXIB_DEC_Context* ctx)
{
    return s_DecoderErrors[ctx->lastError];
}

EXIB_DEC_Object* EXIB_DEC_GetRootObject(EXIB_DEC_Context* ctx)
{
    return &ctx->rootObject;
}

size_t EXIB_DEC_GetFieldSize(EXIB_DEC_Field field)
{
    const EXIB_FieldPrefix* fieldPrefix = (const EXIB_FieldPrefix*)field;
    
    if (fieldPrefix->type == EXIB_TYPE_OBJECT)
    {
        const EXIB_ObjectPrefix* objPrefix = 
                ((const EXIB_ObjectPrefix*)fieldPrefix) + (fieldPrefix->named * 2);
        if (objPrefix->size)
            return *(uint32_t*)(objPrefix + 1);
        return *(uint16_t*)(objPrefix + 1);
    }

    return EXIB_GetTypeSize(fieldPrefix->type);
}

size_t EXIB_DEC_GetFieldOffset(EXIB_DEC_Context* ctx, EXIB_DEC_Field field)
{
    return ((void*)field - ctx->buffer);
}

EXIB_Type EXIB_DEC_GetFieldType(EXIB_DEC_Field field)
{
    return (EXIB_Type)field->type;
}

EXIB_DEC_TString EXIB_DEC_GetFieldName(EXIB_DEC_Context* ctx, EXIB_DEC_Field field)
{
    return EXIB_DEC_GetStringFromOffset(ctx, EXIB_DEC_GetFieldNameOffset(ctx, field));
}

size_t EXIB_DEC_GetFieldDataOffset(EXIB_DEC_Context* ctx, EXIB_DEC_Field field);

EXIB_Type EXIB_DEC_GetFieldValue(EXIB_DEC_Context* ctx, EXIB_DEC_Field field, EXIB_TypedValue* valueOut)
{
    EXIB_Type type = EXIB_DEC_GetFieldType(field);

    // These all have their own special functions for reading.
    if (type == EXIB_TYPE_ARRAY || type == EXIB_TYPE_OBJECT || type == EXIB_TYPE_STRING)
    {
        valueOut->type = EXIB_TYPE_NULL;
        return EXIB_TYPE_NULL;
    }

    // TODO: Safety
    size_t dataOffset = EXIB_DEC_GetFieldDataOffset(ctx, field);
    valueOut->type = type;
    valueOut->value = ctx->buffer + dataOffset;
    return valueOut->type;
}



int EXIB_DEC_CheckBounds(EXIB_DEC_Context* ctx, void* offset)
{
    EXIB_Header* header = ctx->buffer;
    return ((offset - ctx->buffer) >= header->datumSize);
}

EXIB_DEC_Error EXIB_DEC_SetError(EXIB_DEC_Context* ctx, EXIB_DEC_Error err)
{
    ctx->lastError = err;
    return err;
}

exib_string_t EXIB_DEC_GetFieldNameOffset(EXIB_DEC_Context* ctx, EXIB_DEC_Field field)
{
    const EXIB_FieldPrefix* fieldPrefix = (const EXIB_FieldPrefix*)field;
    
    if (!fieldPrefix->named)
        return EXIB_INVALID_STRING;

    return *(exib_string_t*)(fieldPrefix + 1);
}

size_t EXIB_DEC_GetFieldDataOffset(EXIB_DEC_Context* ctx, EXIB_DEC_Field field)
{
    size_t offset = EXIB_DEC_GetFieldOffset(ctx, field);
    
    if (field->named)
        offset += 2;

    return offset + 1 + field->padding;
}