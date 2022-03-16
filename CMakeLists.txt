cmake_minimum_required(VERSION 3.13)

project(qnemu LANGUAGES CXX)
enable_testing()
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/")
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# C++
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(CMAKE_CXX_CLANG_TIDY clang-tidy -checks=-*,clang-analyzer-*,concurrency-*,google-*,llvm-*,misc-*)
    set(CMAKE_CXX_CPPCHECK cppcheck --library=googletest)
endif()

add_subdirectory(${CMAKE_PROJECT_NAME})
add_subdirectory(tests)
