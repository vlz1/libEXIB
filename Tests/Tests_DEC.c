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

static int Test_EXIB_DEC_FindField_Numbers()
{
    EXIB_DEC_Context* ctx = EXIB_DEC_CreateBufferedContext(Sample_Numbers,
                                                           sizeof(Sample_Numbers),
                                                           NULL);

    if (CheckDecoderContext(ctx))
        return 1;

    EXIB_DEC_Object* rootObject = EXIB_DEC_GetRootObject(ctx);
    EXIB_DEC_Field a = EXIB_DEC_FindField(ctx, rootObject, "a");
    EXIB_DEC_Field b = EXIB_DEC_FindField(ctx, rootObject, "b");
    EXIB_DEC_Field c = EXIB_DEC_FindField(ctx, rootObject, "c");

    if (a == EXIB_DEC_INVALID_FIELD
    || b == EXIB_DEC_INVALID_FIELD
    || c == EXIB_DEC_INVALID_FIELD)
        return 1;

    EXIB_DEC_FreeContext(ctx);
    return 0;
}

static int Test_EXIB_DEC_FindField_NumbersAndObjects()
{
    EXIB_DEC_Context* ctx = EXIB_DEC_CreateBufferedContext(Sample_NumbersAndObjects,
                                                           sizeof(Sample_NumbersAndObjects),
                                                           NULL);

    if (CheckDecoderContext(ctx))
        return 1;

    EXIB_DEC_Object* rootObject = EXIB_DEC_GetRootObject(ctx);
    EXIB_DEC_Field a = EXIB_DEC_FindField(ctx, rootObject, "a");
    EXIB_DEC_Field b = EXIB_DEC_FindField(ctx, rootObject, "b");
    EXIB_DEC_Field c = EXIB_DEC_FindField(ctx, rootObject, "c");

    if (a == EXIB_DEC_INVALID_FIELD
        || b == EXIB_DEC_INVALID_FIELD
        || c == EXIB_DEC_INVALID_FIELD)
        return 1;

    EXIB_DEC_Field object1 = EXIB_DEC_FindField(ctx, rootObject, "object1");
    EXIB_DEC_Field object2 = EXIB_DEC_FindField(ctx, rootObject, "object2");
    if (object1 == EXIB_DEC_INVALID_FIELD || object2 == EXIB_DEC_INVALID_FIELD)
        return 1;

    EXIB_DEC_FreeContext(ctx);
    return 0;
}

static int Test_EXIB_DEC_FindObject_NumbersAndObjects()
{
    EXIB_DEC_Context* ctx = EXIB_DEC_CreateBufferedContext(Sample_NumbersAndObjects,
                                                           sizeof(Sample_NumbersAndObjects),
                                                           NULL);

    if (CheckDecoderContext(ctx))
        return 1;

    EXIB_DEC_Object* rootObject = EXIB_DEC_GetRootObject(ctx);


    EXIB_DEC_Object object1;
    EXIB_DEC_Object object2;

    if (EXIB_DEC_FindObject(ctx, rootObject, "object1", &object1)
        != EXIB_DEC_ERR_Success)
        return 1;

    if (EXIB_DEC_FindObject(ctx, rootObject, "object2", &object2)
        != EXIB_DEC_ERR_Success)
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
    AddTest("EXIB_DEC_FindField_Numbers",
            Test_EXIB_DEC_FindField_Numbers, NULL, NULL);
    AddTest("EXIB_DEC_FindField_NumbersAndObjects",
            Test_EXIB_DEC_FindField_NumbersAndObjects, NULL, NULL);
    AddTest("EXIB_DEC_FindObject_NumbersAndObjects",
            Test_EXIB_DEC_FindObject_NumbersAndObjects, NULL, NULL);
}