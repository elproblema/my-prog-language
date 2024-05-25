add_library(sub ${CMAKE_SOURCE_DIR}/include/llvm/g-sub.h g-sub.cpp)

add_library(g-code 
${CMAKE_SOURCE_DIR}/include/llvm/llvm-shortcuts.h llvm-shortcuts.cpp 
${CMAKE_SOURCE_DIR}/include/llvm/g-commands.h g-commands.cpp )

add_library(translator 
${CMAKE_SOURCE_DIR}/include/llvm/g-adapter.h g-adapter.cpp 
)

target_link_libraries(translator ${LLVM_LIBS})
target_link_libraries(g-code ${LLVM_LIBS})
target_link_libraries(sub ${LLVM_LIBS})
target_link_libraries(sub g-code)
target_link_libraries(translator g-code)
target_link_libraries(translator sub)


# add_library(translator 
# llvm/g-adapter.h g-adapter.cpp 
# llvm/g-commands.h g-commands.cpp 
# llvm/llvm-shortcuts.h llvm-shortcuts.cpp 
# llvm/g-sub.h g-sub.cpp)