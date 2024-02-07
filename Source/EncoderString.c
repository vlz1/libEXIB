#include <stdlib.h>
#include <string.h>
#include <EXIB/EXIB.h>
#include <EXIB/Encoder.h>
#include "EncoderInternal.h"

int EXIB_ENC_InitializeString(EXIB_ENC_Context* ctx,
                              EXIB_ENC_String* string,
                              EXIB_ENC_Object* parent,
                              const char* name,
                              EXIB_Type type)
{
    EXIB_ENC_Field* field = &string->array.object.field;
    EXIB_ENC_InitializeField(ctx, field, parent, name,
                             EXIB_TYPE_ARRAY);

    field->elementType = type;

    if (EXIB_ENC_ArrayReserve(&string->array, 0))
        return 1;

    return 0;
}

EXIB_ENC_String* EXIB_ENC_AddString(EXIB_ENC_Context* ctx,
                                   EXIB_ENC_Object* parent,
                                   const char* name,
                                   EXIB_Type charType,
                                   void* str)
{
    int charSize = EXIB_GetTypeSize(charType);
    EXIB_ENC_String* string;

    // Reject non-integer types.
    if (charType < EXIB_TYPE_INT8 || charType > EXIB_TYPE_UINT64)
        return NULL;

    // Allocate an array and initialize it as a string.
    // Doesn't allocate any memory for the array elements just yet.
    string = EXIB_New(EXIB_ENC_String);
    if (EXIB_ENC_InitializeString(ctx, string, parent, name, charType) != 0)
    {
        EXIB_Free(string);
        return NULL;
    }

    // Get length of string.
    size_t length = 0;
    void* ptr = str;
    while (1)
    {
        int n = 0;
        switch (charSize)
        {
            case 1:
                n = (*(uint8_t*)ptr) == 0;
                break;
            case 2:
                n = (*(uint16_t*)ptr) == 0;
                break;
            case 4:
                n = (*(uint32_t*)ptr) == 0;
                break;
            case 8:
                n = (*(uint64_t*)ptr) == 0;
                break;
            default:
                break;
        }
        ptr += charSize;
        if (n)
            break;
        ++length;
    }

    // Allocate elements and copy string over.
    EXIB_ENC_ArrayResize(&string->array, length + 1);
    memcpy(EXIB_ENC_ArrayGetData(&string->array), str, (length + 1) * charSize);

    // Designate the array as a string.
    string->array.isString = 1;

    return string;
}
