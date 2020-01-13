cmake_minimum_required(VERSION 3.13)
project(DragonLoader)

set(CMAKE_CXX_STANDARD 11)

find_package(LLVM REQUIRED CONFIG)

include_directories(${LLVM_INCLUDE_DIRS})
llvm_map_components_to_libnames(llvm_libs support core irreader)

add_library(DragonLoader library.cpp library.h)
target_link_libraries(DragonLoader ${llvm_libs})