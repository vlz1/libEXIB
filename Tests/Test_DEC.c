#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <EXIB/EXIB.h>
#include <EXIB/Decoder.h>
#include "Test.h"
#include "Test_DEC.h"

static int CheckDecoderContext(EXIB_DEC_Context* ctx)
{
    if (!ctx)
        return 1;

    if (EXIB_DEC_GetLastError(ctx) != EXIB_DEC_ERR_Success)
        return 1;

    return 0;
}



/**
 * Make sure that a decoder context can be created from valid data.
 */
int Test_EXIB_DEC_CreateContext()
{
    EXIB_DEC_Context* ctx = EXIB_DEC_CreateBufferedContext(DEC_EmptyRoot, 
        sizeof(DEC_EmptyRoot), 
        NULL);

    if (!ctx)
        return 1;

    if (EXIB_DEC_GetLastError(ctx) != EXIB_DEC_ERR_Success)
    {
        printf("TEST: \tERROR: %s\n", EXIB_DEC_GetLastErrorName(ctx));
        EXIB_DEC_FreeContext(ctx);
        return 1;
    }

    EXIB_DEC_FreeContext(ctx);
    return 0;
}

/**
 * Make sure that a decoder context can't be created from invalid data.
 */
int Test_EXIB_DEC_CreateContext_Invalid()
{
    EXIB_DEC_Context* ctx = EXIB_DEC_CreateBufferedContext(DEC_InvalidRoot, 
        sizeof(DEC_InvalidRoot), 
        NULL);

    if (!ctx)
        return 1;

    if (EXIB_DEC_GetLastError(ctx) == EXIB_DEC_ERR_Success)
    {
        EXIB_DEC_FreeContext(ctx);
        return 1;
    }

    EXIB_DEC_FreeContext(ctx);
    return 0;
}

/**
 * Call ResetContext on a malicious or malformed payload.
 */
int Test_EXIB_DEC_ResetContext_Overflow()
{
    EXIB_DEC_Context* ctx = EXIB_DEC_CreateContext(NULL);
    uint8_t buffer[32] = { 0 };
    EXIB_Header* header = (EXIB_Header*)buffer;
    uint8_t* p = buffer + sizeof(EXIB_Header);

    header->magic     = EXIB_MAGIC;
    header->version   = EXIB_VERSION;
    header->datumSize = 22;
    
    *(p++) = 0x0F; // Unnamed object
    *(p++) = 0x80; // Size32
    *((uint32_t*)p) = 1;

    EXIB_DEC_Error result = EXIB_DEC_ResetContext(ctx, buffer, sizeof(buffer));
    
    if (result == EXIB_DEC_ERR_Success)
        return 1;
    
    printf("TEST: \tProperly rejected with error '%s'\n", EXIB_DEC_GetLastErrorName(ctx));

    EXIB_DEC_FreeContext(ctx);
    return 0;
}

int Test_EXIB_DEC_FindField()
{
    EXIB_DEC_Context* ctx = EXIB_DEC_CreateBufferedContext(DEC_Numbers, 
        sizeof(DEC_Numbers), 
        NULL);

    if (!ctx)
        return 1;

    if (EXIB_DEC_GetLastError(ctx) != EXIB_DEC_ERR_Success)
    {
        printf("TEST: \tCONTEXT ERROR: %s\n", EXIB_DEC_GetLastErrorName(ctx));
        EXIB_DEC_FreeContext(ctx);
        return 1;
    }

    EXIB_DEC_Field c = EXIB_DEC_FindField(ctx, NULL, "c");
    if (c == EXIB_DEC_INVALID_FIELD)
    {
        printf("TEST: \tERROR: %s\n", EXIB_DEC_GetLastErrorName(ctx));
        EXIB_DEC_FreeContext(ctx);
        return 1;
    }

    EXIB_DEC_FreeContext(ctx);
    return 0;
}

int Test_EXIB_DEC_FindField_NumbersAndObjects()
{
    EXIB_DEC_Context* ctx = EXIB_DEC_CreateBufferedContext(DEC_NumbersAndObjects, 
        sizeof(DEC_NumbersAndObjects), 
        NULL);

    if (!ctx)
        return 1;

    if (EXIB_DEC_GetLastError(ctx) != EXIB_DEC_ERR_Success)
    {
        printf("TEST: \tCONTEXT ERROR: %s\n", EXIB_DEC_GetLastErrorName(ctx));
        EXIB_DEC_FreeContext(ctx);
        return 1;
    }

    EXIB_DEC_Object object;
    if (EXIB_DEC_FindObject(ctx, NULL, "object2", &object) != EXIB_DEC_ERR_Success)
    {
        printf("TEST: \tEXIB_DEC_FindObject ERROR: %s\n", EXIB_DEC_GetLastErrorName(ctx));
        EXIB_DEC_FreeContext(ctx);
        return 1;
    }

    EXIB_DEC_Field field = EXIB_DEC_NextField(ctx, &object, NULL);
    while (field != NULL) 
    {
        EXIB_DEC_TString name = EXIB_DEC_FieldGetName(ctx, field);
        printf("%.*s\n", name->length, name->string);
        field = EXIB_DEC_NextField(ctx, &object, field);
    }

    EXIB_DEC_FreeContext(ctx);
    return 0;
}