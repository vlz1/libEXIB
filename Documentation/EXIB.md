# EXIB V1

EXIB is a flexible binary serialization format for storing or transmitting data
over the network. It combines elements of ASN.1, FlatBuffers, and UBJSON.

Each file or packet is known as a *datum*, and is composed of objects and fields.
Fields are typed values that can be optionally named, objects are a special type of
field that can have zero or more child fields. An object **may not** contain multiple
fields with the same name, a decoder error will be raised if this occurs, although an 
object **is allowed** to contain more than one unnamed field.

Together, objects and fields allow for data to be encoded as a recursively-parsable tree
which is directly convertible to/from JSON. The top level field of a datum **must**
be an object, any other type will result in a decoder error.

## Table of Contents

1. [Header](#header)
   1. [Extended Header](#extended-header)
2. [Root Field](#root-field)
3. [Encoding Scheme](#encoding-scheme)
4. [String Table](#string-table)
5. [Blob Table](#blob-table)

## Header

At the beginning of the datum is a 16 byte header with the following structure:

```cpp
struct Header
{
    uint16_t magic;        // E4 1B
    uint8_t  version;
    uint8_t  flags;
    uint32_t datumSize;    // Total size of datum (file) in bytes, header included.
    uint16_t stringSize;   // Size of string table in bytes.
    uint8_t  extendedSize; // Size of extended header if present.
    uint8_t  reserved;     // Reserved for future use.
    uint32_t checksum;     // CRC-32 of entire datum, with this field initialized to 0.
};
```

The first field, `magic`, is a 16-bit value that signifies a valid EXIB header.
On little endian platforms, the signature is 0x1BE4, and on big endian it's 0xE41B.

The `version` field contains the version of the EXIB specification used by the encoder.
A value of zero is invalid and will result in a decoder error.

The `flags` field is a bit mask which indicates the presence of certain optional features.

```cpp
enum HeaderFlag : uint8_t
{
    // Set if an extended header is present.
    EXIB_HEADER_EXT = (1 << 7)
};
```

During encoding, strings are de-duplicated and stored in a table to save space.
16-bit offsets into this string table are used to reference strings.
The size of the string table is provided in `stringSize`, and the table itself is located
immediately after the root object.
The string table may be omitted entirely if no named fields or string values are present.


### Extended Header

If the `EXIB_HEADER_EXT` flag is set in the main header, then it is followed by
an extended header. The size of this extended header varies by version, so the
`extendedSize` field of the main header contains this for forward compatibility
purposes.

As of version 1, the structure of the extended header is as follows:

```cpp
struct ExtendedHeader
{
    uint32_t blobOffset;   // File offset of blob table, 0 if none exists.
    uint16_t blobSize;     // Number of entries in blob table.
};
```


## Root Field

After the headers is a single field definition. For ease of decoding, this field
is **required** to be an object.


## Encoding Scheme

Each field definition, object or otherwise, begins with a byte known as the *field prefix*.
This byte contains information about the field's type and alignment padding, along with
whether it has a name or not.

```cpp
struct FieldPrefix
{
   uint8_t type      : 4;
   uint8_t named     : 1;
   uint8_t padding   : 3;
};
```

The `type` is a 4-bit number describing the type of field.

```cpp
enum class Type : uint8_t
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
};
```

If `named` is set, then a 16-bit offset into the string table follows the field prefix.
The `padding` field indicates how many padding bytes come before the actual field data.
If a name is present, these bytes begin after the 16-bit name offset, otherwise they start after the field prefix.

These padding bytes ensure that values are aligned on their natural boundaries for fast zero-copy deserialization.

Within an object, a field of type INT32 would be encoded like so:
```
    0x35 0x0003      0x00 0xDEADBEEF
[Prefix] [Name] [Padding] [Value]
```

## Objects

Objects use the same field prefix and optional name offset as fields, but they also have
an object prefix and 16-bit or 32-bit size.

```cpp
struct ObjectPrefix
{
    uint8_t arrayType   : 4;
    uint8_t arrayString : 1;
    uint8_t reserved    : 2;
    uint8_t size        : 1;
};
```

For objects, `size` is the only relevant field of the object prefix. If this 
bit is set, then the object uses a 32-bit size, otherwise it uses a 16-bit size. The size of the object in bytes, not including the object definition, is encoded immediately after the object prefix.

An unnamed root object would be encoded like this:
```
    0x0F  0x00   0x0040           ...
[Prefix] [Obj] [Size16] [Object Data]
```

If padding is specified for an object, it is added after the size:
```
    0x4F  0x00   0x0040 0x00 0x00           ...
[Prefix] [Obj] [Size16] [Padding] [Object Data]
```

And finally, a named object may be encoded like this:
```
    0x1F 0x0000  0x00   0x0040           ...
[Prefix] [Name] [Obj] [Size16] [Object Data]
```

## Arrays

Arrays reuse the object prefix instead of requiring a separate array prefix.
The `arrayType` specifies the type of the array's elements. Only one type of
field may be stored within an array. Arrays also make use of the `size` flag, 
and include a 16 or 32 bit size after the object prefix.

Unlike objects, most fields within arrays are encoded without field prefixes. 
This saves space and allows elements to be accessed via pointers into the 
decode buffer.

```
    0x1E 0x0000  0x02   0x0004      0xAA 0xBB 0xCC 0xDD
[Prefix] [Name] [Obj] [Size16] [Array Elements (UINT8)]
```

The exception to this is objects and arrays, they still use complete field 
prefixes when defined in arrays.

## Strings

Strings are a subtype of `EXIB_TYPE_ARRAY`. They make no assumptions about character encoding,
and represent a null-terminated array of any integral type. 
The "character" type is specified by the `arrayType` field of the object prefix.
To encode a string, simply encode an array of integers and set the `arrayString`
of the object prefix.

```
0x1E     0x0000  0x12   0x0006 0x48 0x65 0x6C 0x6C 0x6F 0x00
[Prefix] [Name] [Obj] [Size16]  'H'  'e'  'l'  'l'  'o'  '\0'
```

As you can see, the encoding of a string is nearly identical to an array.
The main purpose of the string type is to disambiguate text data from
other integer arrays, but it also allows strings to be accessed directly as a
`const char*` into the decode buffer because of the added null-terminator.

Strings **MUST** end with a terminator of their respective character type.
For example, a string of `EXIB_TYPE_INT32` must have a 32-bit null-terminator.

## String Table

The string table is made up of variably sized entries that contain strings and 
their length. These entries use the following structure:

```cpp
struct StringEntry
{
    uint16_t length;
    char     string[];
} StringEntry;
```

The main limitation of the string table is that the total size of it must not 
exceed 65536 bytes, since strings are referred to by 16-bit offsets. Since 
TStrings are deduplicated, the only way to hit this limit is to have a single 
object with thousands of fields with long and unique names. Basically, don't do 
that.

## Blob Table

The blob table allows up to 256 arbitrary binary blobs to be embedded within an 
EXIB datum. The offset of the blob table can be found in the extended header if 
one is present.

At the beginning of the table is a structure that provides a lookup table to find 
blobs by index, called the *blob directory*. It has the following structure:

```cpp
struct BlobDirectory
{
    uint32_t entries;   // Total number of blob table entries.
    uint32_t offsets[]; // Blob table relative offset of each entry.
};
```

After the blob directory comes the actual entries. These entries contain the 
optionally-compressed blob data.

```cpp
struct BlobEntry
{
    uint8_t  flags;
    uint8_t  reserved1;  // Reserved for future use.
    uint16_t reserved2;  // Reserved for future use.
    uint32_t storedSize; // Size of the possibly compressed data in bytes.
    uint32_t realSize;   // Size of the data while uncompressed in bytes.
    uint32_t checksum;   // CRC32 checksum of the uncompressed data.
    uint8_t  data[];
} BlobEntry;
```

The blob entry tracks both the compressed and decompressed sizes of the data, 
along with a checksum of the uncompressed data.