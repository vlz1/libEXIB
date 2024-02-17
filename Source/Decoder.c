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
    "Array expected",
    "String expected",
    "Array index out of bounds"
};

static EXIB_DEC_Options s_DefaultOptions = { };

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
    if ((bufferSize < expectedSize) || EXIB_CheckHeader(header, ctx->bufferSize)) // Ensure buffer meets minimum size and validate the header.
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
        if (EXIB_DEC_PartialDecodeAggregate(ctx, rootField, &ctx->rootObject)
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

