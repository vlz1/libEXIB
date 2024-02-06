#include <EXIB/Compiler.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include "AllocatorInternal.h"
#include "EXIB/EncoderTypes.h"

EXIB_CCL_Error EXIB_CCL_ParseDirective(char* directive, uint32_t cclFlags, EXIB_ENC_Context* ctx)
{
    printf("dx: %s\n", directive);
    return EXIB_CCL_ERR_Success;
}

EXIB_ENC_Object* EXIB_CCL_ParseObject(char* start, char* end, uint32_t cclFlags, EXIB_ENC_Context* ctx)
{
    size_t i = 0;
    char* p = start;

    char* objectStart = NULL;
    char* objectEnd = NULL;

    while (p != end)
    {
        char* lp = p++;
        char c = *lp;

        if (isspace(c))
            continue;

        if (c == '{')
        {
            
        }
    }
    return NULL;
}

EXIB_CCL_Error EXIB_CCL_Parse(const char* exiText, uint32_t cclFlags, EXIB_ENC_Context* ctx)
{
    EXIB_CCL_Error error = EXIB_CCL_ERR_Success;
    EXIB_ENC_Object* rootObject = NULL;
    size_t length = strlen(exiText);
    char* buffer = EXIB_Strdup(exiText);
    char* directive = NULL;
    
    for (size_t i = 0; i < length; ++i)
    {
        char* p = buffer + i;
        char c = *p;

        if (c == '.') // Directive start
        {
            if (directive != NULL)
            {
                error = EXIB_CCL_ERR_OpenDirective;
                break;
            }
            directive = p;
        }
        else if (c == ';') // Directive end
        {
            *p = '\0';
            EXIB_CCL_ParseDirective(directive, cclFlags, ctx);
            directive = NULL;
        }
        else if (c == '{') // Object
        {
            if (directive != NULL)
            {
                error = EXIB_CCL_ERR_OpenDirective;
                break;
            }

            EXIB_CCL_ParseObject(p + 1, buffer + length - 1, cclFlags, ctx);
        }
    }

    if (rootObject == NULL)
    {
        error = EXIB_CCL_ERR_MissingRoot;
    }

    EXIB_Free(buffer);
    return error;
}