set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fprofile-arcs")
################################
# Defining tests
################################

add_executable(tests ast_test.cpp SC_test.cpp)

################################
# Unit Tests
################################

enable_testing()
include("${CMAKE_SOURCE_DIR}/cmake/CodeCoverage.cmake")
create_coverage_target(tests_cov)

add_library(test_helper help.cpp)
add_library(printer printer.cpp)

target_link_libraries(tests grammar)
target_link_libraries(tests test_helper)
target_link_libraries(tests printer)

add_test_target(tests_cov tests)