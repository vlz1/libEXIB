#ifndef _EXIB_DECODER_INTERNAL_H
#define _EXIB_DECODER_INTERNAL_H

#include <EXIB/EXIB.h>
#include <EXIB/Decoder.h>
#include <stddef.h>

typedef struct _EXIB_DEC_Context
{
    void*  buffer; // Decode buffer.
    size_t bufferSize; // Size of decode buffer in bytes.
    void*  stringTable;

    EXIB_DEC_Object rootObject;
    EXIB_DEC_Object objectCache;

    EXIB_DEC_Options options;
    EXIB_DEC_Error lastError;
} EXIB_DEC_Context;

/**
 * Ensure that an offset is within the bounds of the buffer.
 * @param ctx Decoder context.
 * @param offset Pointer to check against decode buffer.
 * @return 0 if the offset is within bounds, 1 if it's out of bounds.
 */
int EXIB_DEC_CheckBounds(EXIB_DEC_Context* ctx, void* offset);
EXIB_DEC_Error EXIB_DEC_SetError(EXIB_DEC_Context* ctx, EXIB_DEC_Error err);
exib_string_t EXIB_DEC_GetFieldNameOffset(EXIB_DEC_Context* ctx, EXIB_DEC_Field field);

/**
 * Decode the size and data offset of an object. Also check bounds
 * to ensure object is valid.
 * @param ctx Decoder context.
 * @param objectField Field to decode object from.
 * @param objectOut Pointer to structure to receive decoded object.
 * @return Decoder error state.
 */
EXIB_DEC_Error EXIB_DEC_PartialDecodeObject(EXIB_DEC_Context* ctx, EXIB_DEC_Field objectField, EXIB_DEC_Object* objectOut);

/**
 * Get a pointer to a string table entry from a string table offset.
 * @param ctx Decoder context.
 * @param stringOffset String table offset of string.
 * @return Pointer to entry, or EXIB_DEC_INVALID_STRING on error.
 */
EXIB_DEC_TString EXIB_DEC_GetStringFromOffset(EXIB_DEC_Context* ctx, exib_string_t stringOffset);

#endif // _EXIB_ENCODER_INTERNAL_H
