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

int CompareDatum(EXIB_Header* header, const uint8_t* sample, size_t sampleSize)
{
    if (sampleSize != header->datumSize)
        return 1;
    if (memcmp(header, sample, header->datumSize) != 0)
        return 1;
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

    AddTest("Benchmark", Test_Benchmark, NULL, NULL);
    AddEncoderTests();

    return RunTestByName(argv[1]);
}
