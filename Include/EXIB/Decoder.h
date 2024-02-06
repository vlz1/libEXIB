#ifndef _EXIB_DECODER_H
#define _EXIB_DECODER_H

#include "EXIB.h"

/**
 *
 * EXIB Decoder Implementation
 *
 */

#include <stdint.h>
#include <stddef.h>

typedef enum
{
    EXIB_DEC_ERR_Success        = 0,
    EXIB_DEC_ERR_InvalidHeader  = 1, // Header magic is wrong or fields are invalid.
    EXIB_DEC_ERR_BufferTooSmall = 2, // Decode buffer is too small for operation.
    EXIB_DEC_ERR_BadChecksum    = 3, // Header checksum is wrong.
    EXIB_DEC_ERR_InvalidRoot    = 4, // Root field is not an object.
    EXIB_DEC_ERR_OutOfBounds    = 5, // A size or offset points out of the decode buffer.
    EXIB_DEC_ERR_ObjectExpected = 6, // An object was expected.
    EXIB_DEC_ERR_ArrayExpected  = 7  // An array was expected.
} EXIB_DEC_Error;

/** Opaque decoder context handle. */
typedef struct _EXIB_DEC_Context EXIB_DEC_Context;

/** Pointer to a string table entry within the decode buffer. */
typedef EXIB_StringEntry* EXIB_DEC_TString;

/** Pointer to field's prefix within the decode buffer. */
typedef EXIB_FieldPrefix* EXIB_DEC_Field;

/** Container for a partially decoded object. */
typedef struct _EXIB_DEC_Object
{
    EXIB_DEC_Field field; // Object's field.
    uint32_t size; // Size of object data in bytes.
    uint8_t dataOffset; // Offset from `field` to beginning of object data.
    uint8_t reserved0;
    uint16_t reserved1;
} EXIB_DEC_Object;

#define EXIB_DEC_INVALID_FIELD   ((EXIB_DEC_Field)NULL)
#define EXIB_DEC_INVALID_STRING  ((EXIB_DEC_TString)NULL)

/**
 * Decoder options.
 */
typedef struct _EXIB_DEC_Options
{

} EXIB_DEC_Options;

#ifdef __cplusplus
extern "C" {
#endif

    /**
     * Create a decoder context without a buffer associated.
     * Use EXIB_DEC_ResetContext to assign a decode buffer.
     * @param options Pointer to structure containing decoder parameters. (Can be NULL)
     * @return Pointer to decoder context, or NULL on error.
     */
    EXIB_DEC_Context* EXIB_DEC_CreateContext(EXIB_DEC_Options* options);

    /**
     * Create a decoder context and assign a buffer to it.
     * @param buffer Decoder buffer.
     * @param bufferSize Size of buffer in bytes.
     * @param options Pointer to structure containing decoder parameters. (Can be NULL)
     * @return Pointer to decoder context, or NULL on error.
     */
    EXIB_DEC_Context* EXIB_DEC_CreateBufferedContext(const void* buffer,
                                                     size_t bufferSize,
                                                     EXIB_DEC_Options* options);

    /**
     * Reset a decoder context and set a new decode buffer.
     * @param ctx Decoder context to reset.
     * @param buffer New decode buffer.
     * @param bufferSize Size of buffer in bytes.
     * @return Last decoder error, or EXIB_DEC_ERR_Success.
     */
    EXIB_DEC_Error EXIB_DEC_ResetContext(EXIB_DEC_Context* ctx,
                                         const void* buffer,
                                         size_t bufferSize);

    /**
     * Free and de-initialize an decoder context.
     * @param ctx Pointer to decoder context to free.
     */
    void EXIB_DEC_FreeContext(EXIB_DEC_Context* ctx);

    /**
     * Get the last reported decoder error.
     * @param ctx Decoder context.
     * @return Last error.
     */
    EXIB_DEC_Error EXIB_DEC_GetLastError(EXIB_DEC_Context* ctx);

    /**
     * Get the name of the last reported decoder error.
     * @param ctx Decoder context.
     * @return Name of last error.
     */
    const char* EXIB_DEC_GetLastErrorName(EXIB_DEC_Context* ctx);

    /**
     * Get the root object of a decoder context.
     * @param ctx Decoder context.
     * @return Pointer to root object.
     */
    EXIB_DEC_Object* EXIB_DEC_GetRootObject(EXIB_DEC_Context* ctx);

    /**
     * Get the size of a field, object, or array's data in bytes.
     * @param field Decoder field.
     * @return Size of data in bytes (not including padding)
     */
    size_t EXIB_DEC_GetFieldSize(EXIB_DEC_Field field);

    /**
     * Get the offset of a field relative to the beginning of the datum.
     * @param ctx Decoder context.
     * @param field Decoder field.
     * @return Offset of field prefix.
     */
    size_t EXIB_DEC_GetFieldOffset(EXIB_DEC_Context* ctx, EXIB_DEC_Field field);

    /**
     * Get the type of a field.
     * @param field Decoder field.
     * @return Field type.
     */
    EXIB_Type EXIB_DEC_GetFieldType(EXIB_DEC_Field field);

    /**
     * Get the name of a field.
     * @param ctx Decoder context.
     * @param field Decoder field.
     * @return Field name on success, EXIB_DEC_INVALID_STRING if no name is present.
     */
    EXIB_DEC_TString EXIB_DEC_GetFieldName(EXIB_DEC_Context* ctx, EXIB_DEC_Field field);

    /**
     * Get the type and value of a field.
     * @param ctx Decoder context.
     * @param field Decoder field.
     * @param valueOut Pointer to TypedValue struct. Must not be NULL!
     * @return Type of value, or EXIB_TYPE_NULL if no value could be decoded.
     */
    EXIB_Type EXIB_DEC_GetFieldValue(EXIB_DEC_Context* ctx, EXIB_DEC_Field field, EXIB_TypedValue* valueOut);

    /**
     * Get an object from an object-typed field.
     * @param ctx Decoder context.
     * @param field Object-typed field.
     * @param objectOut Pointer to structure to receive object data.
     * @return `objectOut` on success, NULL on error.
     */
    EXIB_DEC_Object* EXIB_DEC_GetObject(EXIB_DEC_Context* ctx, EXIB_DEC_Field field, EXIB_DEC_Object* objectOut);

    /**
     * Get the next field within an object.
     * Useful for iterating through all of an object's fields.
     * @param ctx Decoder context.
     * @param object Object.
     * @param field Last result from NextField, or NULL to get the first field.
     * @return Next field, or EXIB_DEC_INVALID_FIELD when there are no more fields.
     */
    EXIB_DEC_Field EXIB_DEC_NextField(EXIB_DEC_Context* ctx, EXIB_DEC_Object* object, EXIB_DEC_Field field);

    /**
     * Find a field by name.
     * @param ctx Decoder context.
     * @param parent Parent object, or NULL to use root object.
     * @param name String containing name of field.
     * @return Field, or EXIB_DEC_INVALID_FIELD if none was found.
     */
    EXIB_DEC_Field EXIB_DEC_FindField(EXIB_DEC_Context* ctx, EXIB_DEC_Object* parent, const char* name);

    /**
     * Find a field by name and attempt to decode it as an object.
     * @param ctx Decoder context.
     * @param parent Parent object, or NULL to use root object.
     * @param name String containing name of field.
     * @param objectOut Pointer to struct that will receive the decoded object.
     * @return EXIB_DEC_ERR_Success or decoder error if one is encountered.
     */
    EXIB_DEC_Error EXIB_DEC_FindObject(EXIB_DEC_Context* ctx, 
                                       EXIB_DEC_Object* parent,
                                       const char* name,
                                       EXIB_DEC_Object* objectOut);
#ifdef __cplusplus
}
#endif

#endif