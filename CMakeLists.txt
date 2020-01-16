cmake_minimum_required(VERSION 3.13)
project(DragonLoader)

set(CMAKE_CXX_STANDARD 11)

find_package(LLVM REQUIRED CONFIG)

include_directories(${LLVM_INCLUDE_DIRS})
llvm_map_components_to_libnames(llvm_libs support core irreader mcjit executionengine x86info x86asmprinter x86asmparser x86codegen)

include_directories(include)
file(GLOB SRC_FILES lib/*.cpp)
add_library(DragonLoader ${SRC_FILES})
message(${llvm_libs})
target_link_libraries(DragonLoader ${llvm_libs})

add_subdirectory(unittests/googletest)
include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR})

file(GLOB TEST_FILES unittests/*.cpp)
add_executable(unittest ${TEST_FILES} ${SRC_FILES})
target_link_libraries(unittest gtest gtest_main DragonLoader)