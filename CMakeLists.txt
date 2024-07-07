cmake_minimum_required(VERSION 3.25)
project(SPL)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_compile_options(-O0)

file(GLOB_RECURSE TEST_SOURCES tests/*.cpp)
file(GLOB_RECURSE SOURCES src/*.cpp)
set(LEXER_DIR "${CMAKE_CURRENT_BINARY_DIR}")
set(PARSER_DIR "${CMAKE_CURRENT_BINARY_DIR}")
set(LEXER_OUT "${LEXER_DIR}/lexer.cpp")
set(PARSER_OUT "${PARSER_DIR}/parser.cpp")

include_directories(./include)
include_directories(${CMAKE_CURRENT_BINARY_DIR})

find_package(FLEX REQUIRED)
find_package(BISON REQUIRED)

include(FetchContent)
FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)
enable_testing()

flex_target(lexer src/lexer.l "${LEXER_OUT}" DEFINES_FILE ${LEXER_DIR}/lexer.h)
bison_target(parser src/parser.y "${PARSER_OUT}" DEFINES_FILE ${PARSER_DIR}/parser.h)
add_flex_bison_dependency(lexer parser)

add_executable(SPL main.cpp ${SOURCES} ${LEXER_OUT} ${PARSER_OUT})
add_executable(tests ${TEST_SOURCES} ${SOURCES} ${LEXER_OUT} ${PARSER_OUT})
target_link_libraries(SPL)
target_link_libraries(tests GTest::gtest_main GTest::gmock_main)