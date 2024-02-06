#include <stdlib.h>
#include <stdio.h>
#include <EXIB/Compiler.h>

int main(int argc, const char** argv)
{
    EXIB_ENC_Context* enc = EXIB_ENC_CreateContext(NULL);

    EXIB_CCL_Parse(".name root;{}", 0, enc);

    EXIB_ENC_FreeContext(enc);
    return EXIT_SUCCESS;
}