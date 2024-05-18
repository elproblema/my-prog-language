cmake_minimum_required(VERSION 3.22)

find_package(FLEX REQUIRED)
find_package(BISON REQUIRED)
find_package(LLVM REQUIRED CONFIG)
llvm_map_components_to_libnames(LLVM_LIBS core x86asmparser x86codegen)

set(cov_flags "-g -fprofile-arcs -ftest-coverage")
set(CMAKE_EXPORT_COMPILE_COMMANDS 1)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_FLAGS "-stdlib=libc++ -Wno-unused-value ${LLVM_DEFINITIONS}")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fuse-ld=lld -stdlib=libc++ -lc++abi")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin")
set(CMAKE_BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR})

################################
# GTest installation
################################

include(FetchContent)
FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG        v1.14.0
)

FetchContent_Populate(googletest)
add_subdirectory(${googletest_SOURCE_DIR})

################################
# Project settings
################################

project(Compiler)


include_directories(${CMAKE_BINARY_DIR})
include_directories("${CMAKE_SOURCE_DIR}/include")

FLEX_TARGET(scanner flex+bison/lexer.l "${CMAKE_BINARY_DIR}/scanner.cpp" COMPILE_FLAGS --debug DEFINES_FILE "${CMAKE_BINARY_DIR}/scanner.hpp")
BISON_TARGET(parser flex+bison/parser.y "${CMAKE_BINARY_DIR}/parser.cpp" COMPILE_FLAGS "-t --verbose --graph" DEFINES_FILE "${CMAKE_BINARY_DIR}/parser.hpp")
ADD_FLEX_BISON_DEPENDENCY(scanner parser)

add_library(grammar ${FLEX_scanner_OUTPUTS} ${BISON_parser_OUTPUTS})

add_subdirectory(source)
add_subdirectory(tests)
target_link_libraries(tests ast)
target_link_libraries(tests gtest_main)
target_link_libraries(tests gtest)