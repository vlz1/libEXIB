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
    EXIB_DEC_ERR_Success           = 0,
    EXIB_DEC_ERR_InvalidHeader     = 1, // Header magic is wrong or fields are invalid.
    EXIB_DEC_ERR_BufferTooSmall    = 2, // Decode buffer is too small for operation.
    EXIB_DEC_ERR_BadChecksum       = 3, // Header checksum is wrong.
    EXIB_DEC_ERR_InvalidRoot       = 4, // Root field is not an object.
    EXIB_DEC_ERR_OutOfBounds       = 5, // A size or offset points out of the decode buffer.
    EXIB_DEC_ERR_AggregateExpected = 6, // An aggregate type was expected.
    EXIB_DEC_ERR_ObjectExpected    = 7, // An object was expected.
    EXIB_DEC_ERR_ArrayExpected     = 8, // An array was expected.
    EXIB_DEC_ERR_StringExpected    = 9, // A string was expected.
    EXIB_DEC_ERR_InvalidArrayIndex = 10, // Array index out of bounds.
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
    EXIB_ObjectPrefix objectPrefix; // Original object prefix.
    uint16_t reserved;
} EXIB_DEC_Object;

/** Container for a partially decoded array. */
typedef struct _EXIB_DEC_Array
{
    EXIB_DEC_Object object;
    EXIB_Value* data;
    uint32_t elements;
    uint8_t elementSize;
} EXIB_DEC_Array;

/** Structure to store the value of a field. */
typedef struct _EXIB_DEC_FieldValue
{
    // Type of the currently-stored value.
    EXIB_Type type;
    union
    {
        // Pointer to primitive value in decode buffer.
        EXIB_Value* value;
        // Decoded object (If type == EXIB_TYPE_OBJECT).
        EXIB_DEC_Object object;
        // Decoded array (If type == EXIB_TYPE_ARRAY).
        EXIB_DEC_Array array;
    };
} EXIB_DEC_FieldValue;

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
    size_t EXIB_DEC_FieldGetSize(EXIB_DEC_Field field);

    /**
     * Get the type of a field.
     * @param field Decoder field.
     * @return Field type.
     */
    EXIB_Type EXIB_DEC_FieldGetType(EXIB_DEC_Field field);

    /**
     * Get the name of a field.
     * @param ctx Decoder context.
     * @param field Decoder field.
     * @return Field name on success, EXIB_DEC_INVALID_STRING if no name is present.
     */
    EXIB_DEC_TString EXIB_DEC_FieldGetName(EXIB_DEC_Context* ctx, EXIB_DEC_Field field);

    /**
     * Get the type and value of a field.
     * @param ctx Decoder context.
     * @param field Decoder field.
     * @param valueOut Pointer to TypedValue struct. Must not be NULL!
     * @return Type of value, or EXIB_TYPE_NULL if no value could be decoded.
     */
    EXIB_Type EXIB_DEC_FieldGet(EXIB_DEC_Context* ctx, EXIB_DEC_Field field, EXIB_DEC_FieldValue* valueOut);

    /**
     * Check if the given field is one of the primitive types.
     * @param field Decoder field.
     * @return 1 if the field is an integer or float, 0 otherwise.
     */
    static inline int EXIB_DEC_FieldIsPrimitive(EXIB_DEC_Field field)
    {
        return (field->type >= EXIB_TYPE_INT8 && field->type <= EXIB_TYPE_DOUBLE);
    }

    /**
     * Check if the given field is one of the aggregate types.
     * @param field Decoder field.
     * @return 1 if the field is an object or array, 0 otherwise.
     */
    static inline int EXIB_DEC_FieldIsAggregate(EXIB_DEC_Field field)
    {
        return (field->type == EXIB_TYPE_ARRAY || field->type == EXIB_TYPE_OBJECT);
    }

    /**
     * Check if the given field is an object.
     * @param field Decoder field.
     * @return 1 if the field is an object, 0 otherwise.
     */
    static inline int EXIB_DEC_FieldIsObject(EXIB_DEC_Field field)
    {
        return field->type == EXIB_TYPE_OBJECT;
    }

    /**
     * Check if the given field is an array.
     * @param field Decoder field.
     * @return 1 if the field is an array, 0 otherwise.
     */
    static inline int EXIB_DEC_FieldIsArray(EXIB_DEC_Field field)
    {
        return field->type == EXIB_TYPE_ARRAY;
    }

    /**
     * Get an object from an object-typed field.
     * @param ctx Decoder context.
     * @param field Object-typed field.
     * @param objectOut Pointer to structure to receive object data.
     * @return Pointer to decoded object on success, NULL on error.
     */
    EXIB_DEC_Object* EXIB_DEC_ObjectFromField(EXIB_DEC_Context* ctx, EXIB_DEC_Field field, EXIB_DEC_Object* objectOut);

    /**
     * Get an array from an array-typed field.
     * @param ctx Decoder context.
     * @param field Array-typed field.
     * @param arrayOut Pointer to structure to receive array data.
     * @return Pointer to decoded array on success, NULL on error.
     */
    EXIB_DEC_Array* EXIB_DEC_ArrayFromField(EXIB_DEC_Context* ctx, EXIB_DEC_Field field, EXIB_DEC_Array* arrayOut);

    /**
     * Get the number of elements in an array.
     * @param array Decoder array.
     * @return Number of elements in array.
     */
    static inline size_t EXIB_DEC_ArrayGetLength(EXIB_DEC_Array* array)
    {
        return array->elements;
    }

    /**
     * Get the element type of an array.
     * @param array Decoder array.
     * @return Element type.
     */
    static inline EXIB_Type EXIB_DEC_ArrayGetType(EXIB_DEC_Array* array)
    {
        return array->object.objectPrefix.arrayType;
    }

    /**
     * Check if an array is a string.
     * @param array Decoder array.
     * @return 1 if the array is a string, 0 otherwise.
     */
    static inline int EXIB_DEC_ArrayIsString(EXIB_DEC_Array* array)
    {
        return array->object.objectPrefix.arrayString;
    }

    /**
     * Get the stride (distance between the beginning of each element) of an array.
     * @param array Decoder array.
     * @return Array stride, or 0 if the array is comprised of aggregate types.
     */
    static inline EXIB_Type EXIB_DEC_ArrayGetStride(EXIB_DEC_Array* array)
    {
        return EXIB_GetTypeSize(array->object.objectPrefix.arrayType);
    }

    /**
     * Get a pointer to the first element of an array and retrieve the element count.
     * @param ctx Decoder context.
     * @param array Decoder array.
     * @param lengthOut Pointer to variable to receive the array length.
     * @return Pointer to the beginning of array[i], or NULL on error.
     */
    EXIB_Value* EXIB_DEC_ArrayBegin(EXIB_DEC_Context* ctx, EXIB_DEC_Array* array, size_t* lengthOut);

    /**
     *
     * @param ctx
     * @param array
     * @param valuePointer
     * @return Index of the current element, or -1 if no elements are left.
     */
    int EXIB_DEC_ArrayNext(EXIB_DEC_Context* ctx, EXIB_DEC_Array* array, EXIB_Value** valuePointer);

    /**
     * Get a pointer to element i of the given array.
     * @param ctx Decoder context.
     * @param array Decoder array.
     * @param i Array index.
     * @return Pointer to element at array[i], or NULL on error.
     */
    EXIB_Value* EXIB_DEC_ArrayLocateElement(EXIB_DEC_Context* ctx, EXIB_DEC_Array* array, size_t i);

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