#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <EXIB/EXIB.h>
#include <EXIB/Decoder.h>
#include "AllocatorInternal.h"
#include "DecoderInternal.h"

EXIB_DEC_TString EXIB_DEC_GetStringFromOffset(EXIB_DEC_Context* ctx, exib_string_t stringOffset)
{
    EXIB_Header* header = ctx->buffer;

    if (stringOffset == EXIB_INVALID_STRING)
        return EXIB_DEC_INVALID_STRING;
    else if ((stringOffset + sizeof(EXIB_StringEntry)) >= header->stringSize)
        return EXIB_DEC_INVALID_STRING;

    return ctx->stringTable + stringOffset;
}