#include <stdlib.h>
#include <string.h>
#include <EXIB/EXIB.h>
#include <EXIB/Encoder.h>
#include "AllocatorInternal.h"
#include "EncoderInternal.h"

void EXIB_ENC_InitializeField(EXIB_ENC_Context* ctx,
                              EXIB_ENC_Field* field,
                              EXIB_ENC_Object* parent,
                              const char* name,
                              EXIB_Type type)
{
    if (parent == NULL)
        parent = &ctx->rootObject;

    if (name != NULL)
    {
        EXIB_ENC_StringEntry* nameEntry = EXIB_ENC_GetStringEntry(ctx, name);

        field->nameOffset = nameEntry->offset;
        field->nameBuffer = nameEntry->buffer;
    }
    else
    {
        field->nameOffset = EXIB_INVALID_STRING;
        field->nameBuffer = NULL;
    }

    field->type = type;
    field->parent = parent;
    field->next = NULL;

    EXIB_ENC_Field* children = parent->children;
    if (!children)
    {
        parent->children = field;
        field->prev = field;
    }
    else
    {
        field->prev = children->prev;
        children->prev->next = field;
        children->prev = field;
    }
}

EXIB_ENC_Object* EXIB_ENC_AddObject(EXIB_ENC_Context* ctx,
                                    EXIB_ENC_Object* parent,
                                    const char* name)
{
    EXIB_ENC_Object* object = EXIB_New(EXIB_ENC_Object);

    EXIB_ENC_InitializeField(ctx, &object->field, parent, name, EXIB_TYPE_OBJECT);

    return object;
}

EXIB_ENC_Field* EXIB_ENC_AddField(EXIB_ENC_Context* ctx,
                                  EXIB_ENC_Object* parent,
                                  const char* name,
                                  EXIB_Type type)
{
    EXIB_ENC_Field* field = EXIB_PoolAlloc(&ctx->fieldPool);

    EXIB_ENC_InitializeField(ctx, field, parent, name, type);

    return field;
}

const char* EXIB_ENC_GetName(EXIB_ENC_Field* field)
{
    return field->nameBuffer;
}

void EXIB_ENC_SetValue(EXIB_ENC_Field* field, EXIB_Value value)
{
    field->value = value;
}
