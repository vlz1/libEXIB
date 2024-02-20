#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <EXIB/EXIB.h>
#include "Benchmark.h"
#include "Test.h"

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
    AddDecoderTests();

    return RunTestByName(argv[1]);
}
