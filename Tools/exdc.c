#include <stdlib.h>
#include <stdio.h>
#include <EXIB/Decompiler.h>
#include "../Tests/Samples.h"

int main(int argc, const char** argv)
{
    EXIB_DEC_Error decError;
    //char* buffer = EXIB_DCL_Decompile((EXIB_Header*)data, size, &decError);
    //char* buffer = EXIB_DCL_Decompile((EXIB_Header*)Sample_NumbersAndObjects, sizeof(Sample_NumbersAndObjects), &decError);
    char* buffer = EXIB_DCL_Decompile((EXIB_Header*)Sample_Array, sizeof(Sample_Array), &decError);
    printf("%s", buffer);
    EXIB_Free(buffer);
    return EXIT_SUCCESS;
}