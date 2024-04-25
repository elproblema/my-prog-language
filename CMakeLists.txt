cmake_minimum_required(VERSION 3.22.1)
project(Compiler)

find_package(FLEX REQUIRED)
find_package(BISON REQUIRED)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin")

include_directories(${CMAKE_BINARY_DIR})
include_directories("${CMAKE_SOURCE_DIR}/include")

FLEX_TARGET(scanner flex+bison/lexer.l "${CMAKE_BINARY_DIR}/scanner.cpp" COMPILE_FLAGS --debug DEFINES_FILE scanner.hpp)
BISON_TARGET(parser flex+bison/parser.y "${CMAKE_BINARY_DIR}/parser.cpp" COMPILE_FLAGS "-t --verbose" DEFINES_FILE parser.hpp)
ADD_FLEX_BISON_DEPENDENCY(scanner parser)

add_library(grammar ${FLEX_scanner_OUTPUTS} ${BISON_parser_OUTPUTS})
add_library(ast include/ast.h source/bound.cpp)

################################
# GTest
################################

find_package(GTest REQUIRED)
enable_testing()
include_directories(${GTEST_INCLUDE_DIR})

################################
# Unit Tests
################################

add_executable(tests tests/ast.cpp)
target_link_libraries(tests GTest::gtest_main)
target_link_libraries(tests grammar)
target_link_libraries(tests ast)

include(GoogleTest)
gtest_discover_tests(tests)
