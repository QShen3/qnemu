cmake_minimum_required(VERSION 3.16)

project(qnemu LANGUAGES CXX)

include(CTest)

option(QNEMU_WARNINGS_AS_ERRORS "Treat compiler warnings as errors" ON)
option(QNEMU_ENABLE_CLANG_TIDY "Run clang-tidy during compilation" OFF)
option(QNEMU_ENABLE_CPPCHECK "Run cppcheck during compilation" OFF)
option(QNEMU_ENABLE_COVERAGE "Build with coverage instrumentation and add a 'coverage' target" OFF)

if(QNEMU_ENABLE_COVERAGE AND NOT BUILD_TESTING)
    message(FATAL_ERROR "QNEMU_ENABLE_COVERAGE requires BUILD_TESTING=ON")
endif()

# C++20 is required by the code base. Anonymous structs/unions are used for
# hardware registers, so compiler extensions must stay enabled.
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Shared warning settings for all project targets.
add_library(qnemu_warnings INTERFACE)
if(MSVC)
    target_compile_options(qnemu_warnings INTERFACE /W3 /EHsc)
    if(QNEMU_WARNINGS_AS_ERRORS)
        target_compile_options(qnemu_warnings INTERFACE /WX)
    endif()
else()
    target_compile_options(qnemu_warnings INTERFACE -Wall -Wextra)
    if(QNEMU_WARNINGS_AS_ERRORS)
        target_compile_options(qnemu_warnings INTERFACE -Werror)
    endif()
endif()

# Optional static analysis. These are disabled by default because their
# diagnostics depend heavily on the local tool versions.
if(QNEMU_ENABLE_CLANG_TIDY)
    find_program(CLANG_TIDY_EXECUTABLE NAMES clang-tidy)
    if(NOT CLANG_TIDY_EXECUTABLE)
        message(FATAL_ERROR "QNEMU_ENABLE_CLANG_TIDY is ON, but clang-tidy was not found")
    endif()
    set(CMAKE_CXX_CLANG_TIDY
        "${CLANG_TIDY_EXECUTABLE}"
        "--checks=-*,clang-analyzer-*,concurrency-*,misc-*,-misc-include-cleaner")
    if(MSVC)
        list(APPEND CMAKE_CXX_CLANG_TIDY "--extra-arg=/EHsc")
    endif()
    message(STATUS "clang-tidy enabled: ${CLANG_TIDY_EXECUTABLE}")
endif()

if(QNEMU_ENABLE_CPPCHECK)
    find_program(CPPCHECK_EXECUTABLE NAMES cppcheck)
    if(NOT CPPCHECK_EXECUTABLE)
        message(FATAL_ERROR "QNEMU_ENABLE_CPPCHECK is ON, but cppcheck was not found")
    endif()
    set(CMAKE_CXX_CPPCHECK
        "${CPPCHECK_EXECUTABLE}"
        "--enable=style"
        "--std=c++20"
        "--inline-suppr"
        "--library=googletest"
        # cppcheck does not handle anonymous register structs/unions well and
        # reports their members as uninitialized even though they are.
        "--suppress=uninitMemberVarNoCtor"
        "--error-exitcode=1"
        "--quiet")
    message(STATUS "cppcheck enabled: ${CPPCHECK_EXECUTABLE}")
endif()

if(QNEMU_ENABLE_COVERAGE AND NOT MSVC)
    add_compile_options(--coverage -O0)
    add_link_options(--coverage)
endif()

find_package(Qt6 6.2 COMPONENTS
    Core Gui Multimedia
    Qml Quick QuickControls2 QuickDialogs2
    REQUIRED)

if(BUILD_TESTING)
    find_package(GTest REQUIRED)
endif()

add_subdirectory(qnemu)

if(BUILD_TESTING)
    add_subdirectory(tests)
endif()
