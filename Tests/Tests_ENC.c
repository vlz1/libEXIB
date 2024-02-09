#include "Test.h"
#include "Samples.h"

static int CheckEncoderContext(EXIB_ENC_Context* ctx)
{
    if (!ctx)
        return 1;

    if (EXIB_ENC_GetLastError(ctx) != EXIB_ENC_ERR_Success)
        return 1;

    return 0;
}

static void* SetupGenericEncoderContext()
{
    EXIB_ENC_Context* ctx = EXIB_ENC_CreateContext(NULL);

    if (CheckEncoderContext(ctx))
        return NULL;

    return ctx;
}

static void CleanupGenericEncoderContext(void* ctx)
{
    EXIB_ENC_FreeContext(ctx);
}

static int Test_EXIB_ENC_CreateContext()
{
    EXIB_ENC_Context* ctx = EXIB_ENC_CreateContext(NULL);

    if (CheckEncoderContext(ctx))
        return 1;

    EXIB_ENC_FreeContext(ctx);
    return 0;
}

static int Test_EXIB_ENC_Encode_EmptyRoot(void* parameter)
{
    EXIB_ENC_Context* ctx = parameter;
    EXIB_Header* header = EXIB_ENC_Encode(ctx);
    if (EXIB_CheckHeader(header))
    {
        puts("TEST: \tERROR: Invalid header!");
        return 1;
    }

    DumpDatum(header, "EXIB_ENC_Encode_EmptyRoot.exib");

    uint8_t* bytes = (uint8_t*)(header + 1);
    if (bytes[0]    != 0x0F  // Field Prefix
        || bytes[1] != 0x00  // Object Prefix
        || bytes[2] != 0x00  // Size16
        || bytes[3] != 0x00) // Size16
    {
        puts("TEST: ERROR: Wrong root object definition.");
        printf("       Got %02x %02x %02x %02x, expected %02x %02x %02x %02x.\n",
               bytes[0], bytes[1], bytes[2], bytes[3],
               0x0F, 0, 0, 0);
        return 1;
    }

    return 0;
}

static int Test_EXIB_ENC_Encode_Numbers(void* parameter)
{
    EXIB_ENC_Context* ctx = parameter;
    EXIB_ENC_Field* a = EXIB_ENC_AddField(ctx, NULL, "a", EXIB_TYPE_UINT32);
    EXIB_ENC_Field* b = EXIB_ENC_AddField(ctx, NULL, "b", EXIB_TYPE_UINT32);
    EXIB_ENC_Field* c = EXIB_ENC_AddField(ctx, NULL, "c", EXIB_TYPE_UINT16);

    EXIB_ENC_SetValue(a, (EXIB_Value){ .uint32 = 0xdeadbeef });
    EXIB_ENC_SetValue(b, (EXIB_Value){ .uint32 = 0xcafebabe });
    EXIB_ENC_SetValue(c, (EXIB_Value){ .uint16 = 0xc001 });

    EXIB_Header* header = EXIB_ENC_Encode(ctx);
    if (EXIB_CheckHeader(header))
    {
        puts("TEST: \tERROR: Invalid header!");
        return 1;
    }

    DumpDatum(header, "EXIB_ENC_Encode_Numbers.exib");

    if (sizeof(Sample_Numbers) != header->datumSize ||
        memcmp(header, Sample_Numbers, header->datumSize) != 0)
    {
        return 1;
    }

    return 0;
}

void AddEncoderTests()
{
    AddTest("EXIB_ENC_CreateContext", Test_EXIB_ENC_CreateContext, NULL, NULL);
    AddTest("EXIB_ENC_Encode_EmptyRoot", Test_EXIB_ENC_Encode_EmptyRoot,
            SetupGenericEncoderContext,
            CleanupGenericEncoderContext);
    AddTest("EXIB_ENC_Encode_Numbers", Test_EXIB_ENC_Encode_Numbers,
            SetupGenericEncoderContext,
            CleanupGenericEncoderContext);
}