#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <EXIB/EXIB.h>
#include <EXIB/Decoder.h>
#include "Benchmark.h"
#include "Samples.h"

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
    EXIB_DEC_ResetContext(parameter, Sample_Numbers, sizeof(Sample_Numbers));
}


void* SetupNumbersDecoder()
{
    return EXIB_DEC_CreateBufferedContext(Sample_Numbers, sizeof(Sample_Numbers), NULL);
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

void Benchmark_DEC_FindField_ResetAndRead(void* parameter)
{
    EXIB_DEC_Context* ctx = parameter;
    EXIB_DEC_ResetContext(ctx, Sample_Numbers, sizeof(Sample_Numbers));

    EXIB_DEC_Field a = EXIB_DEC_FindField(ctx, NULL, "a");
    EXIB_DEC_Field b = EXIB_DEC_FindField(ctx, NULL, "b");
    EXIB_DEC_Field c = EXIB_DEC_FindField(ctx, NULL, "c");

    EXIB_DEC_FieldValue aValue;
    EXIB_DEC_FieldValue bValue;
    EXIB_DEC_FieldValue cValue;
    EXIB_DEC_FieldGet(ctx, a, &aValue);
    EXIB_DEC_FieldGet(ctx, b, &bValue);
    EXIB_DEC_FieldGet(ctx, c, &cValue);
}


typedef struct
{
    EXIB_DEC_Context* ctx;
    EXIB_DEC_Object* root;
    EXIB_DEC_Array string1;
    EXIB_DEC_Array string2;
} StringsBenchmarkData;

void* SetupStringsDecoder()
{
    StringsBenchmarkData* data = EXIB_Calloc(1, sizeof(StringsBenchmarkData));
    //data->ctx = EXIB_DEC_CreateBufferedContext(DEC_Strings, sizeof(DEC_Strings), NULL);
    data->root = EXIB_DEC_GetRootObject(data->ctx);

    EXIB_DEC_Field string1 = EXIB_DEC_FindField(data->ctx, data->root, "string1");
    EXIB_DEC_Field string2 = EXIB_DEC_FindField(data->ctx, data->root, "string2");
    EXIB_DEC_ArrayFromField(data->ctx, string1, &data->string1);
    EXIB_DEC_ArrayFromField(data->ctx, string2, &data->string2);

    return data;
}

void CleanupStringsDecoder(void* parameter)
{
    StringsBenchmarkData* data = parameter;
    EXIB_DEC_FreeContext(data->ctx);
    EXIB_Free(data);
}

void Benchmark_DEC_ArrayNext(void* parameter)
{
    StringsBenchmarkData* data = parameter;
    EXIB_DEC_FieldValue value = { };

    while (EXIB_DEC_ArrayNext(data->ctx, &data->string2, &value) >= 0)
        ;
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
    AddBenchmark("DEC_FindField (Reset & Read)",
        Benchmark_DEC_FindField_ResetAndRead,
        SetupDecoder,
        CleanupDecoder,
        4096);
    /*
    AddBenchmark("DEC_ArrayNext",
        Benchmark_DEC_ArrayNext,
        SetupStringsDecoder,
        CleanupStringsDecoder,
        4096);
        */
}