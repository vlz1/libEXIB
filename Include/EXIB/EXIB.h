#ifndef _EXIB_H
#define _EXIB_H

#include <stdint.h>
#include <stddef.h>

#define EXIB_VERSION 1

#if __BYTE_ORDER == __LITTLE_ENDIAN
    #define EXIB_MAGIC 0x1BE4
#else
    #define EXIB_MAGIC 0xE41B
#endif

typedef uint32_t exib_offset_t; // Header relative offset.
typedef uint16_t exib_string_t; // String table relative offset.

#define EXIB_INVALID_OFFSET ((exib_offset_t)~0UL)
#define EXIB_INVALID_STRING ((exib_string_t)~0UL)

enum EXIB_HeaderFlag
{
    // Set if an extended header is present.
    EXIB_HEADER_EXT = (1 << 7)
};

typedef struct _EXIB_Header
{
    uint16_t magic;        // E4 1B
    uint8_t  version;
    uint8_t  flags;
    uint32_t datumSize;    // Total size of datum (file) in bytes, header included.
    uint16_t stringSize;   // Size of string table in bytes.
    uint8_t  extendedSize; // Size of extended header if present.
    uint8_t  reserved;     // Reserved for future use.
    uint32_t checksum;     // CRC-32 of entire datum, with this field initialized to 0.
} EXIB_Header;

typedef struct _EXIB_ExtHeader
{
    exib_offset_t blobOffset; // File offset of blob table, 0 if none exists.
} EXIB_ExtHeader;

// Prefix before a field in an object.
typedef union _EXIB_FieldPrefix
{
    struct
    {
        uint8_t type      : 4;
        uint8_t named     : 1; // If 1, then a 16-bit string offset (relative to header.stringOffset) follows the prefix.
        uint8_t padding   : 3; // Number of padding bytes before the actual data.
    };
    uint8_t byte; // Field prefix as a byte.
} EXIB_FieldPrefix;

// Comes after the field prefix for objects.
typedef union _EXIB_ObjectPrefix
{
    struct
    {
        // Type of fields in array (if array = 1).
        uint8_t arrayType   : 4;
        // 1 if the array is a string.
        uint8_t arrayString : 1;
        // Reserved for future use.
        uint8_t reserved    : 2;
        // If 0, followed by 16-bit size. If 1, followed by 32 bit size.
        uint8_t size        : 1;
    };
    uint8_t byte; // Object prefix as a byte.
} EXIB_ObjectPrefix;

// String table entry.
typedef struct _EXIB_StringEntry
{
    uint16_t length;
    char     string[];
} EXIB_StringEntry;

// Blob table entry. Used for storing large chunks of binary data.
typedef struct _EXIB_BlobEntry
{
    uint8_t  flags;
    uint8_t  reserved1;  // Reserved for future use.
    uint16_t reserved2;  // Reserved for future use.
    uint32_t storedSize; // Size of the possibly compressed data in bytes.
    uint32_t realSize;   // Size of the data while uncompressed in bytes.
    uint32_t checksum;   // CRC32 checksum of uncompressed data.
    uint8_t  data[];
} EXIB_BlobEntry;

// Blob directory. Used to quickly find an entry from an index.
typedef struct _EXIB_BlobDirectory
{
    uint32_t entries;   // Total number of blob table entries.
    uint32_t offsets[]; // Blob table relative offset of each entry.
} EXIB_BlobDirectory;

#ifdef __GNUC__
    #define EXIB_PACKED __attribute__((packed))
#else
    #define EXIB_PACKED
#endif

#ifdef _MSC_VER
    #pragma pack(push, 1)
#endif

typedef enum _EXIB_Type
{
    EXIB_TYPE_NULL   = 0,
    EXIB_TYPE_INT8   = 1,
    EXIB_TYPE_UINT8  = 2,
    EXIB_TYPE_INT16  = 3,
    EXIB_TYPE_UINT16 = 4,
    EXIB_TYPE_INT32  = 5,
    EXIB_TYPE_UINT32 = 6,
    EXIB_TYPE_INT64  = 7,
    EXIB_TYPE_UINT64 = 8,
    EXIB_TYPE_FLOAT  = 9,
    EXIB_TYPE_DOUBLE = 10,
    EXIB_TYPE_BLOB   = 13,
    EXIB_TYPE_ARRAY  = 14,
    EXIB_TYPE_OBJECT = 15
} EXIB_Type;

#ifdef _MSC_VER
    #pragma pack(pop)
#endif

static inline int EXIB_GetTypeSize(EXIB_Type type)
{
    static const int TypeSizes[16] = {
        0,
        1, 1, 2, 2,
        4, 4, 8, 8,
        4, 8,
        0, 0, 1, 0, 0
    };
    return TypeSizes[type];
}

typedef struct _EXIB_Value
{
    union
    {
        int8_t      int8;
        int16_t     int16;
        int32_t     int32;
        int64_t     int64;
        uint8_t     uint8;
        uint16_t    uint16;
        uint32_t    uint32;
        uint64_t    uint64;
        float       float32;
        double      float64;
        void*       pointer;
    };
} EXIB_Value;

typedef struct _EXIB_TypedValue
{
    EXIB_Type   type;
    EXIB_Value* value;
} EXIB_TypedValue;

typedef void* (*exib_malloc_t)(size_t n);
typedef void  (*exib_free_t)(void* p);

/**
 * Minimum size of an EXIB datum.
 * Header + 4 bytes for root object prefixes and Size16.
 */
#define EXIB_MINIMUM (sizeof(EXIB_Header) + 4)

#ifdef __cplusplus
extern "C" {
#endif

    /**
     * Calculate the FNV-1 hash of a string and also measure its length.
     * (Not including the null terminator)
     * @param str String to hash and measure.
     * @param lengthOut Pointer to integer that will receive the string length.
     * @return FNV-1 hash of string.
     */
    uint32_t EXIB_StringHashAndLength(const char* str, uint32_t* lengthOut);

    /**
     * Validate an EXIB header.
     * @param header Header to validate.
     * @return 0 on success, 1 on error.
     */
    int EXIB_CheckHeader(const EXIB_Header* header);

    /**
     * Specify the memory allocation functions to be used by the library.
     * WARNING: Invalidates all existing contexts and allocations!
     * @param mallocFn malloc() function.
     * @param freeFn free() function.
     */
    void EXIB_SetAllocator(exib_malloc_t mallocFn, exib_free_t freeFn);

    void* EXIB_Alloc(size_t n);
    void* EXIB_Calloc(size_t objectSize, size_t objectCount);
    void  EXIB_Free(void* ptr);
#ifdef __cplusplus
}
#endif

#endif
