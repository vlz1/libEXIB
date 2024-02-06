# EXIT V1

EXIT is a modified dialect of JSON for describing an EXIB datum in a human-readable format.

## Table of Contents

1. [Syntax](#syntax)
2. [Directives](#directives)
   1. [%name](#name)

## Syntax

The syntax of EXIT is essentially the same as JSON, aside from the addition of 
[directives](#directives) and [types](#types).

The following notation specifies a simple RGBA color, using 8 bits per channel:

```json
%name("color")
{
  "r:u8": 32,
  "g:u8": 32,
  "b:u8": 64
  "a:u8": 255
}
```



## Types





## Directives

Directives are short statements beginning with a `%` that provide information or instructions
to the EXIT compiler.

### %name

The `name` directive is used to specify the name of the root object.

```json
%name("my_object")
{
  "a": 12,
  "b": 1.5
}
```

The above notation converts into 