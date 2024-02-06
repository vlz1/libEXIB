#include "EXIB/Decoder.h"
#include "EXIB/EXIB.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <inttypes.h>
#include <EXIB/Decompiler.h>

// TODO: Optimize decompiler.

#define EXIB_DCL_BUFFER_INCREMENT   128

static const char* s_TypeNames[16] = {
    NULL,
    ":i8",
    ":u8",
    ":i16",
    ":u16",
    ":i32",
    ":u32",
    ":i64",
    ":u64",
    ":f32",
    ":f64",
    NULL,
    ":string",
    ":blob",
    NULL,
    NULL
};

typedef struct _EXIB_DCL_Context
{
    EXIB_DEC_Context* decoder;
    char* textBuffer;
    size_t textBufferPos;
    size_t textBufferSize;
    int intsAsHex;
} EXIB_DCL_Context;

void EXIB_DCL_ResizeBuffer(EXIB_DCL_Context* dclCtx, size_t newSize)
{
    char* oldBuffer = dclCtx->textBuffer;
    char* newBuffer = EXIB_Calloc(newSize, 1);

    memcpy(newBuffer, oldBuffer, dclCtx->textBufferPos);

    dclCtx->textBuffer = newBuffer;
    dclCtx->textBufferSize = newSize;
    free(oldBuffer);
}

static inline void EXIB_DCL_Emit(EXIB_DCL_Context* dclCtx, const char* str)
{
    size_t length = strlen(str);
    if (dclCtx->textBufferPos + length >= dclCtx->textBufferSize)
    {
        EXIB_DCL_ResizeBuffer(dclCtx, dclCtx->textBufferPos + length + EXIB_DCL_BUFFER_INCREMENT);
    }

    strcpy(dclCtx->textBuffer + dclCtx->textBufferPos, str);
    dclCtx->textBufferPos += length;
}

static inline void EXIB_DCL_EmitN(EXIB_DCL_Context* dclCtx, const char* str, int n)
{
    if (dclCtx->textBufferPos + n >= dclCtx->textBufferSize)
    {
        EXIB_DCL_ResizeBuffer(dclCtx, dclCtx->textBufferPos + n + EXIB_DCL_BUFFER_INCREMENT);
    }

    strncpy(dclCtx->textBuffer + dclCtx->textBufferPos, str, n);
    dclCtx->textBufferPos += n;
}

static inline void EXIB_DCL_EmitIndentation(EXIB_DCL_Context* dclCtx, int count)
{
    if (dclCtx->textBufferPos + count >= dclCtx->textBufferSize)
    {
        EXIB_DCL_ResizeBuffer(dclCtx, dclCtx->textBufferPos + count + EXIB_DCL_BUFFER_INCREMENT);
    }

    for (int i = 0; i < count; ++i)
        dclCtx->textBuffer[dclCtx->textBufferPos++] = ' ';
}

void EXIB_DCL_EmitNameDirective(EXIB_DCL_Context* dclCtx, EXIB_DEC_TString rootName)
{
    if (rootName->length >= dclCtx->textBufferSize)
    {
        EXIB_DCL_ResizeBuffer(dclCtx, dclCtx->textBufferSize + rootName->length);
    }

    EXIB_DCL_Emit(dclCtx, "%name(\"");
    EXIB_DCL_EmitN(dclCtx, rootName->string, rootName->length);
    EXIB_DCL_Emit(dclCtx, "\")\n");
}

void EXIB_DCL_EmitInt(EXIB_DCL_Context* dclCtx, int64_t i)
{
    char valueBuffer[128];
    if (dclCtx->intsAsHex)
        snprintf(valueBuffer, sizeof(valueBuffer), "0x%" PRIx64, i);
    else
        snprintf(valueBuffer, sizeof(valueBuffer), "%" PRIi64, i);
    EXIB_DCL_Emit(dclCtx, valueBuffer);
}

void EXIB_DCL_EmitUInt(EXIB_DCL_Context* dclCtx, uint64_t i)
{
    char valueBuffer[128];
    if (dclCtx->intsAsHex)
        snprintf(valueBuffer, sizeof(valueBuffer), "0x%" PRIx64, i);
    else
        snprintf(valueBuffer, sizeof(valueBuffer), "%" PRIu64, i);
    EXIB_DCL_Emit(dclCtx, valueBuffer);
}

void EXIB_DCL_EmitDouble(EXIB_DCL_Context* dclCtx, double d)
{
    char valueBuffer[128];
    snprintf(valueBuffer, sizeof(valueBuffer), "%f", d);
    EXIB_DCL_Emit(dclCtx, valueBuffer);
}

void EXIB_DCL_EmitValue(EXIB_DCL_Context* dclCtx, EXIB_TypedValue* typedValue)
{
    EXIB_Value* value = typedValue->value;
    EXIB_Type printType = EXIB_TYPE_NULL;
    uint64_t ival = 0;
    float    fval = 0.0f;
    double   dval = 0.0;

    switch (typedValue->type)
    {
        case EXIB_TYPE_INT8:
            ival = value->int8;
            printType = EXIB_TYPE_INT64;
            break;
        case EXIB_TYPE_INT16:
            ival = value->int16;
            printType = EXIB_TYPE_INT64;
            break;
        case EXIB_TYPE_INT32:
            ival = value->int32;
            printType = EXIB_TYPE_INT64;
            break;
        case EXIB_TYPE_INT64:
            ival = value->int64;
            printType = EXIB_TYPE_INT64;
            break;
        
        case EXIB_TYPE_BLOB:    
        case EXIB_TYPE_UINT8:
            ival = value->uint8;
            printType = EXIB_TYPE_UINT64;
        case EXIB_TYPE_UINT16:
            ival = value->uint16;
            printType = EXIB_TYPE_UINT64;
        case EXIB_TYPE_UINT32:
            ival = value->uint32;
            printType = EXIB_TYPE_UINT64;
            break;
        case EXIB_TYPE_UINT64:
            ival = value->uint64;
            printType = EXIB_TYPE_UINT64;
            break;

        case EXIB_TYPE_FLOAT:
            fval = value->float32;
            EXIB_DCL_EmitDouble(dclCtx, fval);
            return;
        case EXIB_TYPE_DOUBLE:
            dval = value->float64;
            EXIB_DCL_EmitDouble(dclCtx, fval);
            return;

        case EXIB_TYPE_NULL:
        case EXIB_TYPE_STRING:
        case EXIB_TYPE_ARRAY:
        case EXIB_TYPE_OBJECT:
            return;
    }

    if (printType == EXIB_TYPE_INT64)
        EXIB_DCL_EmitInt(dclCtx, ival);
    else if (printType == EXIB_TYPE_UINT64)
        EXIB_DCL_EmitUInt(dclCtx, ival);
}

void EXIB_DCL_EmitObject(EXIB_DCL_Context* dclCtx, EXIB_DEC_Object* object, int indent, int depth);

void EXIB_DCL_EmitField(EXIB_DCL_Context* dclCtx, EXIB_DEC_Object* parentObject, EXIB_DEC_Field field, int indent, int depth)
{
    int innerIndent = (depth + 1) * indent;
    EXIB_DEC_TString name = EXIB_DEC_GetFieldName(dclCtx->decoder, field);
    const char* typeName = s_TypeNames[field->type];

    EXIB_DCL_EmitIndentation(dclCtx, innerIndent);
    EXIB_DCL_Emit(dclCtx, "\"");
    EXIB_DCL_EmitN(dclCtx, name->string, name->length);

    // TODO: Handle type suffixes for arrays and strings
    if (typeName != NULL)
    {
        EXIB_DCL_Emit(dclCtx, typeName);
    }

    EXIB_DCL_Emit(dclCtx, "\": ");

    if (field->type == EXIB_TYPE_OBJECT)
    {
        EXIB_DCL_Emit(dclCtx, "{\n");
        
        EXIB_DEC_Object childObject = { };
        EXIB_DEC_GetObject(dclCtx->decoder, field, &childObject);
        EXIB_DCL_EmitObject(dclCtx, &childObject, indent, depth + 1);

        EXIB_DCL_EmitIndentation(dclCtx, innerIndent);
        EXIB_DCL_Emit(dclCtx, "}");
    }
    else
    {
        EXIB_TypedValue value;
        if (EXIB_DEC_GetFieldValue(dclCtx->decoder, field, &value) == EXIB_TYPE_NULL)
        {
            EXIB_DCL_Emit(dclCtx, "NULL");
            return;
        }

        EXIB_DCL_EmitValue(dclCtx, &value);
    }
}

void EXIB_DCL_EmitObject(EXIB_DCL_Context* dclCtx, EXIB_DEC_Object* object, int indent, int depth)
{
    int outerIndent = depth * indent;

    EXIB_DEC_Field field = EXIB_DEC_NextField(dclCtx->decoder, object, NULL);
    while (field != EXIB_DEC_INVALID_FIELD) 
    {
        EXIB_DCL_EmitField(dclCtx, object, field, indent, depth);
        field = EXIB_DEC_NextField(dclCtx->decoder, object, field);
        if (field != EXIB_DEC_INVALID_FIELD)
            EXIB_DCL_Emit(dclCtx, ",\n");
        else
            EXIB_DCL_Emit(dclCtx, "\n");
    }
}

char* EXIB_DCL_Decompile(EXIB_Header* data, size_t size, EXIB_DEC_Error* errorOut)
{
    EXIB_DCL_Context dclCtx = { };
    dclCtx.decoder = EXIB_DEC_CreateBufferedContext(data, size, NULL);

    *errorOut = EXIB_DEC_GetLastError(dclCtx.decoder);
    if (*errorOut != EXIB_DEC_ERR_Success)
        return NULL;

    EXIB_DEC_Object* root = EXIB_DEC_GetRootObject(dclCtx.decoder);
    EXIB_DEC_TString rootName = EXIB_DEC_GetFieldName(dclCtx.decoder, root->field);
    dclCtx.textBufferSize = EXIB_DCL_BUFFER_INCREMENT;
    dclCtx.textBuffer = EXIB_Calloc(dclCtx.textBufferSize, 1);

    // Write name of root object.
    if (rootName != EXIB_DEC_INVALID_STRING)
        EXIB_DCL_EmitNameDirective(&dclCtx, rootName);
    
    // Write root object.
    EXIB_DCL_Emit(&dclCtx, "{\n");
    EXIB_DCL_EmitObject(&dclCtx, root, 2, 0);
    EXIB_DCL_Emit(&dclCtx, "}\n");

    dclCtx.textBuffer[dclCtx.textBufferPos] = 0;
    return dclCtx.textBuffer;
}