#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include "Benchmark.h"

#ifdef WIN32
    #define WIN32_LEAN_AND_MEAN
    #include <windows.h>
#endif

#ifdef __x86_64
    #define MFENCE() __asm__ volatile("mfence" ::: "memory")
#else
    #define MFENCE()
#endif

static Benchmark* s_BenchmarkList = NULL;

uint64_t GetNanoTime()
{
#ifdef __linux__
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (ts.tv_sec * 1000000000ULL) + ts.tv_nsec;
#elif defined WIN32
    // The precision of this is atrocious,
    // but at least it's portable (on win32).
    LARGE_INTEGER frequency;
    LARGE_INTEGER count;
    QueryPerformanceFrequency(&frequency);
    QueryPerformanceCounter(&count);
    return (1000000000ULL / frequency.QuadPart) * count.QuadPart;
#endif
}

void AddBenchmark(const char* name, 
                  benchmark_fn_t func,
                  benchmark_fn_setup_t funcSetup,
                  benchmark_fn_cleanup_t funcCleanup,
                  size_t iterations)
{
    Benchmark* benchmark = calloc(sizeof(Benchmark), 1);

    if (!s_BenchmarkList)
        s_BenchmarkList = benchmark;
    else
    {
        benchmark->next = s_BenchmarkList;
        s_BenchmarkList = benchmark;
    }

    benchmark->name = name;
    benchmark->func = func;
    benchmark->funcSetup = funcSetup;
    benchmark->funcCleanup = funcCleanup;
    benchmark->iterations = iterations;
}

void RunBenchmark(Benchmark* benchmark)
{
    uint64_t startNanos;
    uint64_t endNanos;
    uint64_t averageNanos;
    void* parameter = benchmark->funcSetup 
        ? benchmark->funcSetup() 
        : NULL;

    printf("TEST: \tBENCHMARK: Running %s.\n", benchmark->name);

    {
        MFENCE();
        startNanos = GetNanoTime();
        for (size_t i = 0; i < benchmark->iterations; ++i)
            benchmark->func(parameter);
        
        MFENCE();
        endNanos = GetNanoTime();
    }

    if (benchmark->funcCleanup)
        benchmark->funcCleanup(parameter);

    averageNanos = (endNanos - startNanos) / benchmark->iterations;

    printf("TEST: \tBENCHMARK: \t%lu ns/it\n", averageNanos);
}

extern void AddEncoderBenchmarks();
extern void AddDecoderBenchmarks();

void RunBenchmarks()
{
    AddEncoderBenchmarks();
    AddDecoderBenchmarks();
    
    Benchmark* benchmark = s_BenchmarkList;
    while (benchmark != NULL)
    {
        RunBenchmark(benchmark);
        benchmark = benchmark->next;
    }
}
