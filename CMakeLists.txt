cmake_minimum_required(VERSION 3.20)
project(laba_1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(laba_1 main.cpp)

if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    target_compile_options(laba_1 PRIVATE -Wall -Wconversion -Wextra -Wpedantic)
elseif(MSVC)
    target_compile_options(laba_1 PRIVATE /W4 /utf-8)
endif()