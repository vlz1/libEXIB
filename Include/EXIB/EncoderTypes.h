#ifndef _EXIB_ENCODER_TYPES_H
#define _EXIB_ENCODER_TYPES_H

#include <stdint.h>
#include <stddef.h>

typedef enum
{
    EXIB_ENC_ERR_Success = 0,
    EXIB_ENC_ERR_StringTableFull = 1,
    EXIB_ENC_ERR_OutOfBounds = 2
} EXIB_ENC_Error;

typedef struct _EXIB_ENC_Context EXIB_ENC_Context;
typedef struct _EXIB_ENC_Object  EXIB_ENC_Object;
typedef struct _EXIB_ENC_Array   EXIB_ENC_Array;
typedef struct _EXIB_ENC_Field   EXIB_ENC_Field;
typedef struct _EXIB_ENC_String  EXIB_ENC_String;

/*
 * Encoder options.
 */
typedef struct _EXIB_ENC_Options
{
    int bufferSize; // Initial size of encode buffer in bytes. (Default: 65536)
    int stringCacheCapacity; // Initial capacity of string cache. (Default: 128)
    int arrayCapacity; // Initial array allocation size. (Default: 32)
    const char* datumName; // Name of the datum/root object. (Unnamed by default)
} EXIB_ENC_Options;

#endif