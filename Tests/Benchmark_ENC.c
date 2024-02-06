#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <EXIB/EXIB.h>
#include <EXIB/Encoder.h>
#include "Benchmark.h"

void* SetupEncoder()
{
    return EXIB_ENC_CreateContext(NULL);
}

void CleanupEncoder(void* parameter)
{
    EXIB_ENC_FreeContext(parameter);
}

void Benchmark_ENC_AddField_Duplicate(void* parameter)
{
    EXIB_ENC_Context* ctx = parameter;
    EXIB_ENC_AddField(ctx, NULL, "field", EXIB_TYPE_INT32);
}

void Benchmark_ENC_AddField_Random(void* parameter)
{
    EXIB_ENC_Context* ctx = parameter;
    char name[4] = "!!!";
    int r = rand();

    name[0] += r % 0x70;
    name[1] += (r % (0x70 * 0x70)) % 0x70;
    name[2] += r % 25;

    EXIB_ENC_AddField(ctx, NULL, name, EXIB_TYPE_INT32);
}

void AddEncoderBenchmarks()
{
    AddBenchmark("ENC_AddField_Duplicate",
        Benchmark_ENC_AddField_Duplicate,
        SetupEncoder,
        CleanupEncoder,
        4096);
    AddBenchmark("ENC_AddField_Random",
        Benchmark_ENC_AddField_Random,
        SetupEncoder,
        CleanupEncoder,
        4096);
}