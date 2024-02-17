#include "Test.h"
#include "Samples.h"

static int CheckDecoderContext(EXIB_DEC_Context* ctx)
{
    if (!ctx)
        return 1;

    if (EXIB_DEC_GetLastError(ctx) != EXIB_DEC_ERR_Success)
    {
        printf("TEST: \tLast error: %s\n", EXIB_DEC_GetLastErrorName(ctx));
        EXIB_DEC_FreeContext(ctx);
        return 1;
    }

    return 0;
}

static int Test_EXIB_DEC_CreateContext()
{
    EXIB_DEC_Context* ctx = EXIB_DEC_CreateContext(NULL);

    if (CheckDecoderContext(ctx))
        return 1;

    EXIB_DEC_FreeContext(ctx);
    return 0;
}

static int Test_EXIB_DEC_CreateBufferedContext_RejectInvalid()
{
    EXIB_DEC_Context* ctx = EXIB_DEC_CreateBufferedContext(Sample_InvalidRoot,
                                                           sizeof(Sample_InvalidRoot),
                                                           NULL);
    // Fail if it didn't get rejected.
    if (!CheckDecoderContext(ctx))
    {
        EXIB_DEC_FreeContext(ctx);
        return 1;
    }

    return 0;
}

static int Test_EXIB_DEC_CreateBufferedContext_EmptyRoot()
{
    EXIB_DEC_Context* ctx = EXIB_DEC_CreateBufferedContext(Sample_EmptyRoot,
                                                           sizeof(Sample_EmptyRoot),
                                                           NULL);

    if (CheckDecoderContext(ctx))
        return 1;

    EXIB_DEC_FreeContext(ctx);
    return 0;
}

void AddDecoderTests()
{
    AddTest("EXIB_DEC_CreateContext",
            Test_EXIB_DEC_CreateContext, NULL, NULL);
    AddTest("EXIB_DEC_CreateBufferedContext_RejectInvalid",
            Test_EXIB_DEC_CreateBufferedContext_RejectInvalid, NULL, NULL);
    AddTest("EXIB_DEC_CreateBufferedContext_EmptyRoot",
            Test_EXIB_DEC_CreateBufferedContext_EmptyRoot, NULL, NULL);
}