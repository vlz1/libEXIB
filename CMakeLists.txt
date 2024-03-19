cmake_minimum_required(VERSION 3.24)
project(EXIB C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_C_FLAGS_RELEASE "-Os -Qn -fomit-frame-pointer -fno-asynchronous-unwind-tables")

option(EXIB_TEXT  "Include support for compiling EXIT (Exi Text) data." ON)
option(EXIB_TOOLS "Build additional tools for working with EXIB files." ON)
option(EXIB_TESTS "Build test suite for EXIB library." ON)

add_library(EXIB STATIC)
add_subdirectory(Source)
target_include_directories(EXIB PUBLIC Include)
target_compile_options(EXIB PRIVATE
        -Wall
        -Werror
        -Wno-unused-variable
        -Wno-unused-parameter
        -Wno-unused-but-set-variable)

if (EXIB_TEXT)
    target_compile_options(EXIB PRIVATE -DEXIB_TEXT)
endif ()

if (EXIB_TOOLS)
    add_subdirectory(Tools)
endif ()

if (EXIB_TESTS)
    enable_testing()
    add_subdirectory(Tests)
endif ()
