#include "Test.h"
#include "Samples.h"

static int CheckEncoderContext(EXIB_ENC_Context* ctx)
{
    if (!ctx)
        return 1;

    if (EXIB_ENC_GetLastError(ctx) != EXIB_ENC_ERR_Success)
    {
        EXIB_ENC_FreeContext(ctx);
        return 1;
    }

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
    if (EXIB_CheckHeader(header, header->datumSize))
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
    if (EXIB_CheckHeader(header, header->datumSize))
    {
        puts("TEST: \tERROR: Invalid header!");
        return 1;
    }

    DumpDatum(header, "EXIB_ENC_Encode_Numbers.exib");

    if (CompareDatum(header, Sample_Numbers, sizeof(Sample_Numbers)))
        return 1;

    return 0;
}

int Test_EXIB_ENC_Encode_NumbersAndObjects(void* parameter)
{
    EXIB_ENC_Context* ctx = parameter;

    EXIB_ENC_Field* a = EXIB_ENC_AddField(ctx, NULL, "a", EXIB_TYPE_UINT32);
    EXIB_ENC_Field* b = EXIB_ENC_AddField(ctx, NULL, "b", EXIB_TYPE_UINT32);
    EXIB_ENC_Field* c = EXIB_ENC_AddField(ctx, NULL, "c", EXIB_TYPE_INT8);

    EXIB_ENC_SetValue(a, (EXIB_Value){ .uint32 = 0xdeadbeef });
    EXIB_ENC_SetValue(b, (EXIB_Value){ .uint32 = 0xcafebabe });
    EXIB_ENC_SetValue(c, (EXIB_Value){ .int8   = 'd' });

    EXIB_ENC_Object* object1 = EXIB_ENC_AddObject(ctx, NULL, "object1");
    EXIB_ENC_Object* object2 = EXIB_ENC_AddObject(ctx, NULL, "object2");

    EXIB_ENC_Object* nested = EXIB_ENC_AddObject(ctx, object1, "nested");

    EXIB_ENC_Field* x = EXIB_ENC_AddField(ctx, object2, "x", EXIB_TYPE_FLOAT);
    EXIB_ENC_Field* y = EXIB_ENC_AddField(ctx, object2, "y", EXIB_TYPE_FLOAT);
    EXIB_ENC_Field* z = EXIB_ENC_AddField(ctx, object2, "z", EXIB_TYPE_FLOAT);

    EXIB_ENC_SetValue(x, (EXIB_Value){ .float32 = 1.5f });
    EXIB_ENC_SetValue(y, (EXIB_Value){ .float32 = 0.5f });
    EXIB_ENC_SetValue(z, (EXIB_Value){ .float32 = 2.0f });

    EXIB_Header* header = EXIB_ENC_Encode(ctx);
    if (EXIB_CheckHeader(header, header->datumSize))
    {
        puts("TEST: \tERROR: Invalid header!");
        return 1;
    }

    DumpDatum(header, "EXIB_ENC_Encode_NumbersAndObjects.exib");

    if (CompareDatum(header, Sample_NumbersAndObjects, sizeof(Sample_NumbersAndObjects)))
        return 1;

    return 0;
}

int Test_EXIB_ENC_Encode_Array(void* parameter)
{
    EXIB_ENC_Context* ctx = parameter;
    EXIB_ENC_Array* floatArray = EXIB_ENC_AddArray(ctx, NULL, "array", EXIB_TYPE_FLOAT);
    EXIB_ENC_ArrayResize(floatArray, 8);
    for (int i = 0; i < 8; ++i)
    {
        EXIB_Value value = { .float32 = 1.0f * (float)i };
        EXIB_ENC_ArraySet(floatArray, i, value);
    }

    EXIB_Header* header = EXIB_ENC_Encode(ctx);
    if (EXIB_CheckHeader(header, header->datumSize))
    {
        puts("TEST: \tERROR: Invalid header!");
        return 1;
    }

    DumpDatum(header, "EXIB_ENC_Encode_Array.exib");

    if (CompareDatum(header, Sample_Array, sizeof(Sample_Array)))
        return 1;

    return 0;
}

int Test_EXIB_ENC_Encode_ArrayOfArrays(void* parameter)
{
    EXIB_ENC_Context* ctx = parameter;

    EXIB_ENC_Array* arrayArray = EXIB_ENC_AddArray(ctx, NULL, "array", EXIB_TYPE_ARRAY);
    EXIB_ENC_Array* array1 = EXIB_ENC_ArrayAddArray(ctx, arrayArray, EXIB_TYPE_UINT32);
    EXIB_ENC_ArrayAppend(array1, (EXIB_Value){ .uint32 = 0xdeadbeef });
    EXIB_ENC_ArrayAppend(array1, (EXIB_Value){ .uint32 = 0xdeadbeef });
    EXIB_ENC_ArrayAppend(array1, (EXIB_Value){ .uint32 = 0xdeadbeef });
    EXIB_ENC_ArrayAppend(array1, (EXIB_Value){ .uint32 = 0xdeadbeef });

    EXIB_ENC_Array* array2 = EXIB_ENC_ArrayAddArray(ctx, arrayArray, EXIB_TYPE_FLOAT);
    EXIB_ENC_ArrayAppend(array2, (EXIB_Value){ .float32 = 1.0f });
    EXIB_ENC_ArrayAppend(array2, (EXIB_Value){ .float32 = 2.0f });
    EXIB_ENC_ArrayAppend(array2, (EXIB_Value){ .float32 = 3.0f });
    EXIB_ENC_ArrayAppend(array2, (EXIB_Value){ .float32 = 4.0f });

    EXIB_Header* header = EXIB_ENC_Encode(ctx);
    if (EXIB_CheckHeader(header, header->datumSize))
    {
        puts("TEST: \tERROR: Invalid header!");
        return 1;
    }

    DumpDatum(header, "EXIB_ENC_Encode_ArrayOfArrays.exib");

    if (CompareDatum(header, Sample_ArrayOfArrays, sizeof(Sample_ArrayOfArrays)))
        return 1;

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
    AddTest("EXIB_ENC_Encode_NumbersAndObjects", Test_EXIB_ENC_Encode_NumbersAndObjects,
            SetupGenericEncoderContext,
            CleanupGenericEncoderContext);
    AddTest("EXIB_ENC_Encode_Array", Test_EXIB_ENC_Encode_Array,
            SetupGenericEncoderContext,
            CleanupGenericEncoderContext);
    AddTest("EXIB_ENC_Encode_ArrayOfArrays", Test_EXIB_ENC_Encode_ArrayOfArrays,
            SetupGenericEncoderContext,
            CleanupGenericEncoderContext);
}