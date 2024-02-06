#ifndef _BENCHMARK_H
#define _BENCHMARK_H

#include <stddef.h>

typedef void* (*benchmark_fn_setup_t)();
typedef void (*benchmark_fn_cleanup_t)(void* parameter);
typedef void (*benchmark_fn_t)(void* parameter);

typedef struct _Benchmark 
{
    const char* name;

    benchmark_fn_setup_t   funcSetup;
    benchmark_fn_cleanup_t funcCleanup;
    benchmark_fn_t         func;
    
    size_t iterations;

    struct _Benchmark* next;
} Benchmark;

void AddBenchmark(const char* name, 
                  benchmark_fn_t func,
                  benchmark_fn_setup_t funcSetup,
                  benchmark_fn_cleanup_t funcCleanup,
                  size_t iterations);
void RunBenchmarks();

#endif // _BENCHMARK_H