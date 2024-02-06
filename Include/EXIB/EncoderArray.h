#ifndef _EXIB_ENCODER_ARRAY_H
#define _EXIB_ENCODER_ARRAY_H

#include "EXIB.h"
#include "EncoderTypes.h"

#ifdef __cplusplus
extern "C" {
#endif

    /**
     * Add an array to an object.
     * @param ctx Encoder context.
     * @param parent Pointer to parent object. If NULL, uses root as parent.
     * @param name Name of array. If NULL, array is left unnamed.
     * @param elementType Type of elements.
     * @return Pointer to newly-added array, or NULL if an error occurred.
     */
    EXIB_ENC_Array* EXIB_ENC_AddArray(EXIB_ENC_Context* ctx,
                                      EXIB_ENC_Object* parent,
                                      const char* name,
                                      EXIB_Type elementType);

    /**
     * Reserve space in an array for at least `newCapacity` elements.
     * Does nothing for arrays of objects or arrays.
     * @param array Encoder array.
     * @param newCapacity New minimum capacity.
     * @return 0 on success, 1 on failure.
     */
    int EXIB_ENC_ArrayReserve(EXIB_ENC_Array* array, uint32_t newCapacity);

    /** Set the number of elements in an array. Reallocates if necessary. */
    void EXIB_ENC_ArrayResize(EXIB_ENC_Array* array, size_t newSize);

    /** Get the number of elements in an array. */
    size_t EXIB_ENC_ArrayGetSize(EXIB_ENC_Array* array);

    /**
     * Get a pointer to the array's internal buffer.
     * This pointer is invalidated if the array is resized.
     * @param array Encoder array.
     * @return Pointer to array data, or NULL if the array contains objects or other arrays.
     */
    void* EXIB_ENC_ArrayGetData(EXIB_ENC_Array* array);

    /**
     * Set an element in an array to the given value.
     * @param array Encoder array.
     * @param index Index of element.
     * @param value New value.
     * @return Pointer to newly-set value, or NULL if the index was out of bounds.
     */
    void* EXIB_ENC_ArraySet(EXIB_ENC_Array* array, size_t index, EXIB_Value value);

    /**
     * Append a value to the end of the array.
     * @param array Encoder array.
     * @param value New value.
     * @return Pointer to appended value, or NULL if the array failed to expand.
     */
    void* EXIB_ENC_ArrayAppend(EXIB_ENC_Array* array, EXIB_Value value);

    /**
     * Create a new object and add it to the end of the array.
     * @param ctx Encoder context.
     * @param array Encoder array.
     * @param name Name of object, or NULL to leave it unnamed.
     * @return Pointer to newly-added object, or NULL if an error occurred.
     */
    EXIB_ENC_Object* EXIB_ENC_ArrayAddObject(EXIB_ENC_Context* ctx, EXIB_ENC_Array* array, const char* name);
#ifdef __cplusplus
}
#endif

#endif