#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <EXIB/EXIB.h>
#include <EXIB/Decoder.h>
#include "AllocatorInternal.h"
#include "DecoderInternal.h"

size_t EXIB_DEC_GetFieldOffset(EXIB_DEC_Context* ctx, EXIB_DEC_Field field)
{
    return ((void*)field - ctx->buffer);
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

size_t EXIB_DEC_FieldGetSize(EXIB_DEC_Field field)
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

EXIB_Type EXIB_DEC_FieldGetType(EXIB_DEC_Field field)
{
    return (EXIB_Type)field->type;
}

EXIB_DEC_TString EXIB_DEC_FieldGetName(EXIB_DEC_Context* ctx, EXIB_DEC_Field field)
{
    return EXIB_DEC_GetStringFromOffset(ctx, EXIB_DEC_GetFieldNameOffset(ctx, field));
}

EXIB_Type EXIB_DEC_FieldGet(EXIB_DEC_Context* ctx, EXIB_DEC_Field field, EXIB_TypedValue* valueOut)
{
    EXIB_Type type = EXIB_DEC_FieldGetType(field);

    // These all have their own special functions for reading.
    if (type == EXIB_TYPE_ARRAY || type == EXIB_TYPE_OBJECT)
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