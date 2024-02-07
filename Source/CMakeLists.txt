target_sources(EXIB PRIVATE Util.c AllocatorInternal.h Allocator.c
    EncoderInternal.h Encoder.c EncoderTString.c EncoderString.c EncoderObject.c EncoderArray.c
    DecoderInternal.h Decoder.c DecoderTString.c DecoderArray.c DecoderObject.c DecoderField.c

        )

if (EXIB_TEXT)
    target_sources(EXIB PRIVATE
            Compiler.c ../Include/EXIB/Compiler.h
            Decompiler.c ../Include/EXIB/Decompiler.h)
endif ()

target_sources(EXIB PUBLIC ../Include/EXIB/EXIB.h
    ../Include/EXIB/Encoder.h
    ../Include/EXIB/EncoderTypes.h
    ../Include/EXIB/EncoderArray.h
    ../Include/EXIB/EncoderString.h
    ../Include/EXIB/Decoder.h)