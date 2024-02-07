#ifndef _EXIB_ENCODER_INTERNAL_H
#define _EXIB_ENCODER_INTERNAL_H

#include <EXIB/EXIB.h>
#include <EXIB/Encoder.h>
#include "AllocatorInternal.h"

/** Encoder string cache entry. */
typedef struct _EXIB_ENC_StringEntry
{
    uint32_t      hash;   // FNV-1 hash of string.
    uint16_t      length; // Length of string.
    exib_string_t offset; // String table relative offset.
    char*         buffer; // Buffer containing string characters.
} EXIB_ENC_StringEntry;

/**
 * Get or create the corresponding string cache entry 
 * for the given string.
 * @param ctx Encoder context.
 * @param str String to get entry for.
 * @return EXIB_ENC_StringEntry* 
 */
EXIB_ENC_StringEntry* EXIB_ENC_GetStringEntry(EXIB_ENC_Context* ctx, const char* str);

struct _EXIB_ENC_Object;
typedef struct _EXIB_ENC_Field
{
    EXIB_Value               value;
    EXIB_Type                type;
    EXIB_Type                elementType; // EXIB_TYPE_NULL for anything but arrays.
    exib_string_t            nameOffset;
    const char*              nameBuffer; // Pointer to corresponding StringEntry's buffer.
    struct _EXIB_ENC_Object* parent;
    struct _EXIB_ENC_Field*  next; // Linked list of parent object's children.
    struct _EXIB_ENC_Field*  prev; // Linked list of parent object's children.
} EXIB_ENC_Field;

typedef struct _EXIB_ENC_Object
{
    EXIB_ENC_Field  field;
    EXIB_ENC_Field* children;
} EXIB_ENC_Object;

typedef struct _EXIB_ENC_Array
{
    EXIB_ENC_Object  object; // An array is basically just an object. We'll reuse the `children` field.
    uint32_t         elementCount; // Number of elements in array.
    uint32_t         elementCapacity; // Size of element buffer.
    int              isString; // 1 if the array is a string.
    EXIB_Value*      valueElements; // Regular values are stored in a vector.
} EXIB_ENC_Array;

typedef struct _EXIB_ENC_String
{
    EXIB_ENC_Array array;
} EXIB_ENC_String;

void EXIB_ENC_InitializeField(EXIB_ENC_Context* ctx,
                              EXIB_ENC_Field* field,
                              EXIB_ENC_Object* parent,
                              const char* name,
                              EXIB_Type type);

typedef struct _EXIB_ENC_Context
{
    uint8_t* encodeBuffer;
    size_t   encodeBufferSize;

    EXIB_ENC_Object rootObject;
    EXIB_MemoryPool fieldPool;

    EXIB_ENC_StringEntry* stringCache;
    uint16_t stringCacheSize;
    uint16_t stringCacheCapacity;
    uint32_t stringOffset;

    EXIB_ENC_Options options;
    EXIB_ENC_Error lastError;
} EXIB_ENC_Context;

#endif // _EXIB_ENCODER_INTERNAL_H
