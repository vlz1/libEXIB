#ifndef _EXIB_ENCODER_H
#define _EXIB_ENCODER_H

#include "EXIB.h"

/**
 *
 * EXIB Encoder Implementation
 *
 */

#include "EncoderTypes.h"
#include "EncoderArray.h"
#include "EncoderString.h"

#ifdef __cplusplus
extern "C" {
#endif

    /**
     * Get the default encoder options.
     * @param options Pointer to struct that will receive the default options.
     */
    void EXIB_ENC_GetDefaultOptions(EXIB_ENC_Options* options);

    /**
     * Create an encoder context.
     * @param options Pointer to structure containing encoder parameters. (Can be NULL)
     * @return Pointer to encoder context, or NULL on error.
     */
    EXIB_ENC_Context* EXIB_ENC_CreateContext(EXIB_ENC_Options* options);

    /**
     * Free and de-initialize an encoder context.
     * @param ctx Pointer to encoder context to free.
     */
    void EXIB_ENC_FreeContext(EXIB_ENC_Context* ctx);

    /**
     * Get the last reported encoder error.
     * @param ctx Encoder context.
     * @return Last error.
     */
    EXIB_ENC_Error EXIB_ENC_GetLastError(EXIB_ENC_Context* ctx);

    /**
     * Encode the datum and return a pointer the beginning within the
     * internal encoder buffer.
     * @param ctx
     * @return Pointer to EXIB header within encoder buffer.
     */
    EXIB_Header* EXIB_ENC_Encode(EXIB_ENC_Context* ctx);

    /**
     * Add a child object to another object.
     * @param ctx Encoder context.
     * @param parent Pointer to parent object. If NULL, uses root as parent.
     * @param name Name of object. If NULL, object is left unnamed.
     * @return Pointer to newly-added object, or NULL if an error occurred.
     */
    EXIB_ENC_Object* EXIB_ENC_AddObject(EXIB_ENC_Context* ctx,
                                        EXIB_ENC_Object* parent,
                                        const char* name);

    /**
     * Add a field to an object.
     * @param ctx Encoder context.
     * @param parent Pointer to parent object. If NULL, uses root as parent.
     * @param name Name of field. If NULL, field is left unnamed.
     * @param type Type of field.
     * @return Pointer to newly-added field, or NULL if an error occurred.
     */
    EXIB_ENC_Field* EXIB_ENC_AddField(EXIB_ENC_Context* ctx,
                                      EXIB_ENC_Object* parent,
                                      const char* name,
                                      EXIB_Type type);

    /**
     * Get the name of a field.
     * @param field Encoder field.
     * @return Pointer to field name, or NULL if the field is unnamed.
     */
    const char* EXIB_ENC_GetName(EXIB_ENC_Field* field);

    /**
     * Set the value of a field.
     * @param field Encoder field.
     * @param value New field value.
     */
    void EXIB_ENC_SetValue(EXIB_ENC_Field* field, EXIB_Value value);

#ifdef __cplusplus
}
#endif

#endif