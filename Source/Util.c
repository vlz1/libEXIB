#include <stdint.h>
#include <EXIB/EXIB.h>

/*
 * FNV-1 hash code adapted from http://isthe.com/chongo/tech/comp/fnv
 */

#define FNV32_PRIME 0x01000193UL

uint32_t EXIB_StringHashAndLength(const char* str, uint32_t* lengthOut)
{
    uint32_t hash = 0;
    uint32_t length = 0;

    while (*str)
    {
        hash *= FNV32_PRIME;
        hash ^= (uint32_t)(*str++);
        ++length;
    }

    *lengthOut = length;
    return hash;
}

int EXIB_CheckHeader(const EXIB_Header* header)
{
    if (header->magic != EXIB_MAGIC)
        return 1;

    if (header->version == 0)
        return 1;

    size_t minimumSize = sizeof(EXIB_Header)
        + header->extendedSize
        + header->stringSize
        + 4;
    if (header->datumSize < minimumSize)
        return 1;

    return 0;
}