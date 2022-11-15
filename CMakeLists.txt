cmake_minimum_required(VERSION 3.13)

project(qnemu LANGUAGES CXX)
enable_testing()
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/")
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(QNEMU_ENABLE_COVERAGE OFF CACHE BOOL "Enabel test coverage")

# C++
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    # if (${CMAKE_CXX_COMPILER_ID} STREQUAL MSVC)
    #     set(CMAKE_CXX_CLANG_TIDY clang-tidy --extra-arg=/EHsc -checks=-*,clang-analyzer-*,concurrency-*,google-*,llvm-*,misc-*)
    # else()
    #     set(CMAKE_CXX_CLANG_TIDY clang-tidy -checks=-*,clang-analyzer-*,concurrency-*,google-*,llvm-*,misc-*)
    # endif()
    set(CMAKE_CXX_CPPCHECK cppcheck --library=googletest)
endif()

if(QNEMU_ENABLE_COVERAGE)
    add_compile_options(--coverage)
    add_link_options(-lgcov --coverage)
endif()

add_subdirectory(${CMAKE_PROJECT_NAME})
add_subdirectory(tests)
