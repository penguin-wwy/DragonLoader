cmake_minimum_required(VERSION 3.13)
project(DragonLoader)

set(PROJECT_ROOT_DIR "${CMAKE_CURRENT_SOURCE_DIR}")

add_subdirectory(example)

set(CMAKE_CXX_STANDARD 11)

find_package(Clang REQUIRED)
find_package(LLVM REQUIRED CONFIG)

include_directories(${CLANG_INCLUDE_DIRS})
link_directories(${CLANG_INSTALL_PREFIX}/lib)
message(${CLANG_INSTALL_PREFIX}/lib)

include_directories(${LLVM_INCLUDE_DIRS})
llvm_map_components_to_libnames(llvm_libs x86disassembler x86asmparser x86codegen globalisel
        selectiondag asmprinter codegen scalaropts instcombine aggressiveinstcombine
        transformutils bitwriter x86desc mcdisassembler x86info
        x86utils mcjit executionengine target analysis profiledata runtimedyld
        object mcparser bitreader mc debuginfocodeview debuginfomsf core binaryformat
        support demangle irreader aarch64codegen amdgpucodegen armcodegen bpfcodegen
        hexagoncodegen lanaicodegen mipscodegen msp430codegen nvptxcodegen powerpccodegen
        sparccodegen systemzcodegen webassemblycodegen xcorecodegen)

include_directories(${PROJECT_ROOT_DIR}/include)
file(GLOB SRC_FILES lib/*.cpp)
add_library(DragonLoader SHARED ${SRC_FILES})
target_link_libraries(DragonLoader ${llvm_libs}
        clangFrontend
        clangSerialization
        clangDriver
        clangParse
        clangSema
        clangAnalysis
        clangAST
        clangBasic
        clangEdit
        clangLex
        clangTooling
        clangCodeGen)

add_subdirectory(unittests/googletest)
include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR})

file(GLOB TEST_FILES unittests/*.cpp)
add_executable(unittest ${TEST_FILES} ${SRC_FILES})
target_link_libraries(unittest gtest gtest_main DragonLoader)