#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <EXIB/EXIB.h>
#include <EXIB/Decoder.h>
#include "Benchmark.h"
#include "Test_DEC.h"

void* SetupDecoder()
{
    return EXIB_DEC_CreateContext(NULL);
}

void CleanupDecoder(void* parameter)
{
    EXIB_DEC_FreeContext(parameter);
}

void Benchmark_DEC_ResetContext(void* parameter)
{
    EXIB_DEC_ResetContext(parameter, DEC_Numbers, sizeof(DEC_Numbers));
}


void* SetupNumbersDecoder()
{
    return EXIB_DEC_CreateBufferedContext(DEC_Numbers, sizeof(DEC_Numbers), NULL);
}

void CleanupNumbersDecoder(void* parameter)
{
    EXIB_DEC_FreeContext(parameter);
}

void Benchmark_DEC_NextField(void* parameter)
{
    EXIB_DEC_Context* ctx = parameter;
    EXIB_DEC_Object* object = EXIB_DEC_GetRootObject(ctx);
    EXIB_DEC_Field field = EXIB_DEC_NextField(ctx, object, NULL);

    while (field != NULL) 
    {
        field = EXIB_DEC_NextField(ctx, object, field);
    }
}

void Benchmark_DEC_FindField(void* parameter)
{
    EXIB_DEC_Context* ctx = parameter;
    EXIB_DEC_Object* object = EXIB_DEC_GetRootObject(ctx);
    EXIB_DEC_Field field = EXIB_DEC_FindField(ctx, NULL, "c");
}

void AddDecoderBenchmarks()
{
    AddBenchmark("DEC_ResetContext",
        Benchmark_DEC_ResetContext,
        SetupDecoder,
        CleanupDecoder,
        4096);
    AddBenchmark("DEC_NextField",
        Benchmark_DEC_NextField,
        SetupNumbersDecoder,
        CleanupNumbersDecoder,
        4096);
    AddBenchmark("DEC_FindField",
        Benchmark_DEC_FindField,
        SetupNumbersDecoder,
        CleanupNumbersDecoder,
        4096);
}