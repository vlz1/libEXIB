#include <stdint.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <EXIB/EXIB.h>
#include <EXIB/Encoder.h>
#include <EXIB/Decoder.h>
#include "Benchmark.h"
#include "EXIB/EncoderArray.h"
#include "Test.h"
#include "Test_DEC.h"

void DumpDatum(EXIB_Header* header, const char* path)
{
    FILE* f = fopen(path, "wb");
    fwrite(header, header->datumSize, 1, f);
    fclose(f);
}

int CheckEncoderContext(EXIB_ENC_Context* ctx)
{
    if (!ctx)
        return 1;

    if (EXIB_ENC_GetLastError(ctx) != EXIB_ENC_ERR_Success)
        return 1;

    return 0;
}



int Test_EXIB_ENC_Encode_NumbersAndObjects()
{
    EXIB_ENC_Options options;
    EXIB_ENC_GetDefaultOptions(&options);
    options.datumName = "Numbers_and_Objects";

    EXIB_ENC_Context* ctx = EXIB_ENC_CreateContext(&options);

    if (CheckEncoderContext(ctx))
        return 1;

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
    if (EXIB_CheckHeader(header))
    {
        puts("TEST: \tERROR: Invalid header!");
        return 1;
    }

    DumpDatum(header, "NumbersAndObjects.exib");

    if (sizeof(DEC_NumbersAndObjects) != header->datumSize ||
        memcmp(header, DEC_NumbersAndObjects, header->datumSize) != 0)
    {
        return 1;
    }

    EXIB_ENC_FreeContext(ctx);
    return 0;
}

int Test_EXIB_ENC_Encode_IntArray()
{
    EXIB_ENC_Context* ctx = EXIB_ENC_CreateContext(NULL);

    if (CheckEncoderContext(ctx))
        return 1;

    // Make an array of some cool numbers.
    EXIB_ENC_Array* array = EXIB_ENC_AddArray(ctx, NULL, "myArray", EXIB_TYPE_INT32);
    if (!array)
    {
        printf("TEST: \tERROR: Failed to create array!\n");
        return 1;
    }
    EXIB_ENC_ArrayAppend(array, (EXIB_Value){ .uint32 = 0xdeadbeef });
    EXIB_ENC_ArrayAppend(array, (EXIB_Value){ .uint32 = 0xdeadc0de });
    EXIB_ENC_ArrayAppend(array, (EXIB_Value){ .uint32 = 0xc001c0de });
    EXIB_ENC_ArrayAppend(array, (EXIB_Value){ .uint32 = 0xbeefb00b });

    EXIB_Header* header = EXIB_ENC_Encode(ctx);
    if (EXIB_CheckHeader(header))
    {
        puts("TEST: \tERROR: Invalid header!");
        return 1;
    }

    DumpDatum(header, "IntArray.exib");

    EXIB_ENC_FreeContext(ctx);
    return 0;
}

int Test_EXIB_ENC_Encode_ObjectArray()
{
    EXIB_ENC_Context* ctx = EXIB_ENC_CreateContext(NULL);

    if (CheckEncoderContext(ctx))
        return 1;

    // Make an array of some cool objects.
    EXIB_ENC_Array* array = EXIB_ENC_AddArray(ctx, NULL, "myArray", EXIB_TYPE_OBJECT);
    if (!array)
    {
        printf("TEST: \tERROR: Failed to create array!\n");
        return 1;
    }

    EXIB_ENC_Object* object1 = EXIB_ENC_ArrayAddObject(ctx, array, "myObject1");
    EXIB_ENC_Object* object2 = EXIB_ENC_ArrayAddObject(ctx, array, "myObject2");
    EXIB_ENC_Object* object3 = EXIB_ENC_ArrayAddObject(ctx, array, "myObject3");

    EXIB_ENC_Field* field = EXIB_ENC_AddField(ctx, object1, "field", EXIB_TYPE_FLOAT);
    EXIB_ENC_SetValue(field, (EXIB_Value){ .float32 = 0.125f });

    field = EXIB_ENC_AddField(ctx, object2, "field", EXIB_TYPE_DOUBLE);
    EXIB_ENC_SetValue(field, (EXIB_Value){ .float64 = 8.125 });

    EXIB_Header* header = EXIB_ENC_Encode(ctx);
    if (EXIB_CheckHeader(header))
    {
        puts("TEST: \tERROR: Invalid header!");
        return 1;
    }

    DumpDatum(header, "ObjectArray.exib");

    EXIB_ENC_FreeContext(ctx);
    return 0;
}

int Test_EXIB_ENC_Encode_Strings()
{
    EXIB_ENC_Context* ctx = EXIB_ENC_CreateContext(NULL);

    if (CheckEncoderContext(ctx))
        return 1;

    EXIB_ENC_String* string1 = EXIB_ENC_AddString(ctx,
                                                  NULL,
                                                  "string1",
                                                  EXIB_TYPE_UINT8,
                                                  "Hello");
    EXIB_ENC_String* string2 = EXIB_ENC_AddString(ctx,
                                                  NULL,
                                                  "string2",
                                                  EXIB_TYPE_UINT16,
                                                  L"Not a fan of UTF-16!");
    EXIB_ENC_Array* array1 = EXIB_ENC_AddArray(ctx,
                                               NULL,
                                               "array1",
                                               EXIB_TYPE_UINT32);

    EXIB_Value value;
    for (int i = 0; i < 8; ++i)
    {
        value.int32 = 32 + (32 * i);
        EXIB_ENC_ArrayAppend(array1, value);
    }

    EXIB_Header* header = EXIB_ENC_Encode(ctx);
    if (EXIB_CheckHeader(header))
    {
        puts("TEST: \tERROR: Invalid header!");
        return 1;
    }

    DumpDatum(header, "Strings.exib");

    EXIB_ENC_FreeContext(ctx);
    return 0;
}

int Test_Benchmark()
{
    RunBenchmarks();
    return 0;
}

extern int Test_EXIB_DEC_CreateContext();
extern int Test_EXIB_DEC_CreateContext_Invalid();
extern int Test_EXIB_DEC_ResetContext_Overflow();
extern int Test_EXIB_DEC_FindField();
extern int Test_EXIB_DEC_FindField_NumbersAndObjects();

/*
Test tests[] = {
    { "Benchmark", Test_Benchmark },
    { "EXIB_ENC_CreateContext", Test_EXIB_ENC_CreateContext },
    { "EXIB_ENC_Encode_Numbers", Test_EXIB_ENC_Encode_Numbers },
    { "EXIB_ENC_Encode_EmptyRoot", Test_EXIB_ENC_Encode_EmptyRoot },
    { "EXIB_ENC_Encode_NumbersAndObjects", Test_EXIB_ENC_Encode_NumbersAndObjects },
    { "EXIB_ENC_Encode_IntArray", Test_EXIB_ENC_Encode_IntArray },
    { "EXIB_ENC_Encode_ObjectArray", Test_EXIB_ENC_Encode_ObjectArray },
    { "EXIB_ENC_Encode_Strings", Test_EXIB_ENC_Encode_Strings },
    { "EXIB_DEC_CreateContext", Test_EXIB_DEC_CreateContext },
    { "EXIB_DEC_CreateContext_Invalid", Test_EXIB_DEC_CreateContext_Invalid },
    { "EXIB_DEC_ResetContext_Overflow", Test_EXIB_DEC_ResetContext_Overflow },
    { "EXIB_DEC_FindField", Test_EXIB_DEC_FindField },
    { "EXIB_DEC_FindField_NumbersAndObjects", Test_EXIB_DEC_FindField_NumbersAndObjects },
    { NULL, NULL }
};
*/

static Test* s_TestList = NULL;

void PrintTests()
{
    Test* test = s_TestList;
    while (test != NULL)
    {
        printf("    %s\n", test->name);
        test = test->next;
    }
}

void PrintHelp()
{
    puts("Usage: ./Test [Test Name]");
    puts("Available tests:");
    PrintTests();
    exit(EXIT_FAILURE);
}

void AddTest(const char* name,
             test_fn_t func,
             test_setup_fn_t funcSetup,
             test_cleanup_fn_t funcCleanup)
{
    Test* test = calloc(sizeof(Test), 1);

    if (!s_TestList)
        s_TestList = test;
    else
    {
        test->next = s_TestList;
        s_TestList = test;
    }

    test->name = name;
    test->fn = func;
    test->setupFn = funcSetup;
    test->cleanupFn = funcCleanup;
}

int RunTest(Test* test)
{
    printf("TEST: Running %s.\n", test->name);

    void* parameter = NULL;
    if (test->setupFn)
        parameter = test->setupFn();

    int result = test->fn(parameter);
    if (result != 0)
        printf("TEST: \tFAILED WITH RESULT %d!\n", result);
    else
        puts("TEST: \tPASSED");

    if (test->cleanupFn)
        test->cleanupFn(parameter);

    return result;
}

int RunTestByName(const char* name)
{
    Test* test = s_TestList;

    while (test != NULL)
    {
        if (strcmp(name, test->name) == 0)
            return RunTest(test);
        test = test->next;
    }

    return 1;
}

int main(int argc, const char** argv)
{
    srand(time(NULL));

    if (argc < 2)
        PrintHelp();

    AddEncoderTests();

    return RunTestByName(argv[1]);
}
