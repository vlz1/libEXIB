add_executable(EXIB_Test Test.c Tests_ENC.c Tests_DEC.c Test.h Test_DEC.h Samples.h
    Benchmark.c Benchmark_ENC.c Benchmark_DEC.c
    Benchmark.h)
target_link_libraries(EXIB_Test PUBLIC EXIB)

add_test(NAME "[Benchmark]"
    COMMAND EXIB_Test Benchmark)
add_test(NAME "[Decode] EXIB_DEC_CreateContext"
    COMMAND EXIB_Test EXIB_DEC_CreateContext)
add_test(NAME "[Decode] EXIB_DEC_CreateBufferedContext (Reject Invalid Datum)"
    COMMAND EXIB_Test EXIB_DEC_CreateBufferedContext_RejectInvalid)
add_test(NAME "[Decode] EXIB_DEC_CreateBufferedContext (Empty Root Object)"
    COMMAND EXIB_Test EXIB_DEC_CreateBufferedContext_EmptyRoot)
add_test(NAME "[Decode] EXIB_DEC_ResetContext (Overflow)"
    COMMAND EXIB_Test EXIB_DEC_ResetContext_Overflow)
add_test(NAME "[Decode] EXIB_DEC_FindField"
    COMMAND EXIB_Test EXIB_DEC_FindField)
add_test(NAME "[Decode] EXIB_DEC_FindField (Numbers And Objects)"
    COMMAND EXIB_Test EXIB_DEC_FindField_NumbersAndObjects)
add_test(NAME "[Encode] EXIB_ENC_CreateContext"
    COMMAND EXIB_Test EXIB_ENC_CreateContext)
add_test(NAME "[Encode] EXIB_ENC_Encode (Numbers)"
    COMMAND EXIB_Test EXIB_ENC_Encode_Numbers)
add_test(NAME "[Encode] EXIB_ENC_Encode (Empty Root)"
    COMMAND EXIB_Test EXIB_ENC_Encode_EmptyRoot)
add_test(NAME "[Encode] EXIB_ENC_Encode (Numbers And Objects)"
    COMMAND EXIB_Test EXIB_ENC_Encode_NumbersAndObjects)
add_test(NAME "[Encode] EXIB_ENC_Encode (Float Array)"
    COMMAND EXIB_Test EXIB_ENC_Encode_Array)
add_test(NAME "[Encode] EXIB_ENC_Encode (Array of Arrays)"
    COMMAND EXIB_Test EXIB_ENC_Encode_ArrayOfArrays)