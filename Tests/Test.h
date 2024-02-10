#ifndef _TEST_H
#define _TEST_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include <EXIB/EXIB.h>
#include <EXIB/Encoder.h>
#include <EXIB/Decoder.h>

typedef void*(*test_setup_fn_t)();
typedef void(*test_cleanup_fn_t)(void*);
typedef int(*test_fn_t)(void*);

typedef struct _Test
{
    const char* name;
    test_setup_fn_t setupFn;
    test_cleanup_fn_t cleanupFn;
    test_fn_t fn;
    struct _Test* next;
} Test;

void AddTest(const char* name,
             test_fn_t func,
             test_setup_fn_t funcSetup,
             test_cleanup_fn_t funcCleanup);

void DumpDatum(EXIB_Header* header, const char* path);
int CompareDatum(EXIB_Header* header, const uint8_t* sample, size_t sampleSize);

void AddCommonTests();
void AddEncoderTests();
void AddDecoderTests();

#endif // _TEST_H
