set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${cov_flags}")
add_subdirectory(ast)
add_subdirectory(llvm)