#include <stdlib.h>
#include <string.h>
#include <EXIB/EXIB.h>
#include <EXIB/Encoder.h>
#include "EncoderInternal.h"

static EXIB_ENC_StringEntry* EXIB_ENC_AddTString(EXIB_ENC_Context* ctx, const char* str, uint32_t hash, uint32_t length);
static EXIB_ENC_StringEntry* EXIB_ENC_FindTString(EXIB_ENC_Context* ctx, uint32_t hash);
static int EXIB_ENC_StringInsertionIndex(EXIB_ENC_Context* ctx, uint32_t hash);
static void EXIB_ENC_ExpandStringCache(EXIB_ENC_Context* ctx, int newCapacity);

EXIB_ENC_StringEntry* EXIB_ENC_GetStringEntry(EXIB_ENC_Context* ctx, const char* str)
{
    uint32_t length;
    uint32_t hash = EXIB_StringHashAndLength(str, &length);

    EXIB_ENC_StringEntry* entry = EXIB_ENC_FindTString(ctx, hash);
    if (!entry)
    {
        entry = EXIB_ENC_AddTString(ctx, str, hash, length);
        if (!entry)
            return NULL;
    }

    return entry;
}

static EXIB_ENC_StringEntry* EXIB_ENC_AddTString(EXIB_ENC_Context* ctx, const char* str, uint32_t hash, uint32_t length)
{
    EXIB_ENC_StringEntry* entry;
    size_t entrySize = sizeof(EXIB_StringEntry) + length;
    int i;

    if (ctx->stringOffset >= (EXIB_INVALID_STRING - entrySize))
    {
        ctx->lastError = EXIB_ENC_ERR_StringTableFull;
        return NULL;
    }

    i = EXIB_ENC_StringInsertionIndex(ctx, hash);

    if ((ctx->stringCacheSize + 1) >= ctx->stringCacheCapacity)
        EXIB_ENC_ExpandStringCache(ctx, ctx->stringCacheCapacity + 64);

    if (i == ctx->stringCacheSize)
    {
        // Insert at the end of the list, no movement necessary.
        entry = &ctx->stringCache[ctx->stringCacheSize++];
    }
    else
    {
        // Insert within the list, move the higher half up a slot.
        memmove(&ctx->stringCache[i + 1], &ctx->stringCache[i],
                sizeof(EXIB_ENC_StringEntry) * ((ctx->stringCacheSize++) - i));
        entry = &ctx->stringCache[i];
    }

    entry->hash = hash;
    entry->length = length;
    entry->offset = ctx->stringOffset;
    entry->buffer = strdup(str);

    ctx->stringOffset += entrySize;
    ctx->lastError = EXIB_ENC_ERR_Success;
    return entry;
}

static EXIB_ENC_StringEntry* EXIB_ENC_FindTString(EXIB_ENC_Context* ctx, uint32_t hash)
{
    int left = 0;
    int right = ctx->stringCacheSize - 1;

    while (left <= right)
    {
        int mid = (left + right) / 2;
        EXIB_ENC_StringEntry* entry = &ctx->stringCache[mid];

        if (hash == entry->hash)
            return entry;
        else if (hash < entry->hash)
            right = mid - 1;
        else
            left = mid + 1;
    }

    return NULL;
}

static int EXIB_ENC_StringInsertionIndex(EXIB_ENC_Context* ctx, uint32_t hash)
{
    int left = 0;
    int right = ctx->stringCacheSize - 1;

    while (left <= right)
    {
        int mid = (left + right) / 2;
        EXIB_ENC_StringEntry* entry = &ctx->stringCache[mid];

        if (hash < entry->hash)
            right = mid - 1;
        else
            left = mid + 1;
    }

    return left;
}

static void EXIB_ENC_ExpandStringCache(EXIB_ENC_Context* ctx, int newCapacity)
{
    EXIB_ENC_StringEntry* newCache = calloc(newCapacity, sizeof(EXIB_ENC_StringEntry));

    // Copy data into new cache.
    memcpy(newCache, ctx->stringCache, ctx->stringCacheSize * sizeof(EXIB_ENC_StringEntry));

    // Free old cache and update context.
    free(ctx->stringCache);
    ctx->stringCache = newCache;
    ctx->stringCacheCapacity = newCapacity;
}