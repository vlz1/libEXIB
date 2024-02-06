#ifndef _EXIB_COMPILER_H
#define _EXIB_COMPILER_H

#include "EXIB.h"
#include "Encoder.h"

/**
 * EXIT compiler, loads data in a JSON dialect into an EXIB encoder.
 * Not meant to be super fast, we're not using SIMD or anything.
 * Also only supports ASCII text at the moment.
 */

typedef enum
{
    EXIB_CCL_ERR_Success        = 0,
    EXIB_CCL_ERR_OpenDirective  = 1,
    EXIB_CCL_ERR_OpenObject     = 2,
    EXIB_CCL_ERR_MissingRoot    = 3
} EXIB_CCL_Error;

// Infer types from field values instead of raising an error.
#define EXIB_CCL_INFERTYPES (1 << 0)

#ifdef __cplusplus
extern "C" {
#endif

    /**
    * Parse EXIT data into an EXIB encoder context.
    * @param exiText EXIT formatted data.
    * @param cclFlags Compiler flags.
    * @param ctx Encoder context.
    * @return EXIB_CCL_Error 
    */
    EXIB_CCL_Error EXIB_CCL_Parse(const char* exiText, uint32_t cclFlags, EXIB_ENC_Context* ctx);

#ifdef __cplusplus
}
#endif

#endif