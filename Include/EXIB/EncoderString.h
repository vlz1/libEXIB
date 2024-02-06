#ifndef _EXIB_ENCODER_STRING_H
#define _EXIB_ENCODER_STRING_H

#include "EXIB.h"
#include "EncoderTypes.h"

#ifdef __cplusplus
extern "C" {
#endif

    /**
     * Add a string to an object.
     * @param ctx Encoder context.
     * @param parent Pointer to parent object. If NULL, uses root as parent.
     * @param name Name of string. If NULL, string is left unnamed.
     * @param charType Type of character (May be any integral type).
     * @param str Data to initialize string with (Can be NULL to leave string empty).
     * @return Pointer to newly-added string, or NULL if an error occurred.
     */
    EXIB_ENC_String* EXIB_ENC_AddString(EXIB_ENC_Context* ctx,
                                        EXIB_ENC_Object* parent,
                                        const char* name,
                                        EXIB_Type charType,
                                        void* str);



#ifdef __cplusplus
}
#endif

#endif
