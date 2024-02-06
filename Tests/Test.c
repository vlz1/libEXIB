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

void dump(EXIB_Header* header, const char* file)
{
    FILE* f = fopen(file, "wb");
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

int Test_EXIB_ENC_CreateContext()
{
    EXIB_ENC_Context* ctx = EXIB_ENC_CreateContext(NULL);

    if (CheckEncoderContext(ctx))
        return 1;

    EXIB_ENC_FreeContext(ctx);
    return 0;
}

int Test_EXIB_ENC_Encode_EmptyRoot()
{
    EXIB_ENC_Context* ctx = EXIB_ENC_CreateContext(NULL);

    if (CheckEncoderContext(ctx))
        return 1;

    EXIB_Header* header = EXIB_ENC_Encode(ctx);
    if (EXIB_CheckHeader(header))
    {
        puts("TEST: \tERROR: Invalid header!");
        return 1;
    }

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

    EXIB_ENC_FreeContext(ctx);
    return 0;
}

int Test_EXIB_ENC_Encode_Numbers()
{
    EXIB_ENC_Context* ctx = EXIB_ENC_CreateContext(NULL);

    if (CheckEncoderContext(ctx))
        return 1;

    EXIB_ENC_Field* a = EXIB_ENC_AddField(ctx, NULL, "a", EXIB_TYPE_UINT32);
    EXIB_ENC_Field* b = EXIB_ENC_AddField(ctx, NULL, "b", EXIB_TYPE_UINT32);
    EXIB_ENC_Field* c = EXIB_ENC_AddField(ctx, NULL, "c", EXIB_TYPE_INT8);

    EXIB_ENC_SetValue(a, (EXIB_Value){ .uint32 = 0xdeadbeef });
    EXIB_ENC_SetValue(b, (EXIB_Value){ .uint32 = 0xcafebabe });
    EXIB_ENC_SetValue(c, (EXIB_Value){ .int8   = 'd' });

    EXIB_Header* header = EXIB_ENC_Encode(ctx);
    if (EXIB_CheckHeader(header))
    {
        puts("TEST: \tERROR: Invalid header!");
        return 1;
    }

    if (sizeof(DEC_Numbers) != header->datumSize ||
        memcmp(header, DEC_Numbers, header->datumSize) != 0)
    {
        return 1;
    }

    EXIB_ENC_FreeContext(ctx);
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

    dump(header, "NumbersAndObjects.exib");

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
    EXIB_ENC_ArrayAppend(array, (EXIB_Value){ .int32 = 0xdeadbeef });
    EXIB_ENC_ArrayAppend(array, (EXIB_Value){ .int32 = 0xdeadc0de });
    EXIB_ENC_ArrayAppend(array, (EXIB_Value){ .int32 = 0xc001c0de });
    EXIB_ENC_ArrayAppend(array, (EXIB_Value){ .int32 = 0xbeefb00b });

    EXIB_Header* header = EXIB_ENC_Encode(ctx);
    if (EXIB_CheckHeader(header))
    {
        puts("TEST: \tERROR: Invalid header!");
        return 1;
    }

    dump(header, "IntArray.exib");

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

    dump(header, "ObjectArray.exib");

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


    EXIB_Header* header = EXIB_ENC_Encode(ctx);
    if (EXIB_CheckHeader(header))
    {
        puts("TEST: \tERROR: Invalid header!");
        return 1;
    }

    dump(header, "Strings.exib");

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

int run_test(Test* test)
{
    printf("TEST: Running %s.\n", test->name);

    int result = test->fn(rand());
    if (result != 0)
        printf("TEST: \tFAILED WITH RESULT %d!\n", result);
    else
        puts("TEST: \tPASSED");
    return result;
}

int run_test_by_name(const char* name)
{
    Test* test = &tests[0];

    while (test->name != NULL)
    {
        if (strcmp(name, test->name) == 0)
            return run_test(test);
        ++test;
    }

    return 1;
}

void print_tests()
{
    Test* test = &tests[0];
    while (test->name != NULL)
    {
        printf("    %s\n", test->name);
        ++test;
    }
}

void print_help()
{
    puts("Usage: ./Test [Test Name]");
    puts("Available tests:");
    print_tests();
    exit(EXIT_FAILURE);
}

int main(int argc, const char** argv)
{
    srand(time(NULL));

    if (argc < 2)
        print_help();

    if (strcmp(argv[1], "-A") == 0)
    {
        Test* test = tests;
        int result = 0;

        while (test->name != NULL)
        {
            int r = run_test(test);
            if (r != 0)
                result = r;
            ++test;
        }
    }

    return run_test_by_name(argv[1]);
}
