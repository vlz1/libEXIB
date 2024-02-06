#ifndef _EXIB_DECOMPILER_H
#define _EXIB_DECOMPILER_H

#include "EXIB.h"
#include "Decoder.h"

#ifdef __cplusplus
extern "C" {
#endif

    /**
    * Parse EXIT data into an EXIB encoder context.
    * @param data EXIB data to decompile.
    * @param size Size of `data` in bytes.
    * @param dclFlags Decompiler flags.
    * @param errorOut Pointer to variable that will receive any decoder errors.
    * @return Pointer to buffer containing EXIT representation of data.
    */
    char* EXIB_DCL_Decompile(EXIB_Header* data, size_t size, EXIB_DEC_Error* errorOut);

#ifdef __cplusplus
}
#endif

#endif