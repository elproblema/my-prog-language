add_library(ast ast.cpp SC.cpp lambda.cpp)
target_include_directories(ast PUBLIC ${CMAKE_BINARY_DIR})
target_include_directories(ast PUBLIC ${LLVM_INCLUDE_DIRS})