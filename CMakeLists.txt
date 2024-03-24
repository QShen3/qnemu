cmake_minimum_required(VERSION 3.13)

project(qnemu LANGUAGES CXX)
enable_testing()
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(QNEMU_ENABLE_COVERAGE OFF CACHE BOOL "Enabel test coverage")

if(${CMAKE_CXX_COMPILER_ID} STREQUAL GNU OR ${CMAKE_CXX_COMPILER_ID} STREQUAL Clang OR ${CMAKE_CXX_COMPILER_ID} STREQUAL AppleClang)
    if(QNEMU_ENABLE_COVERAGE)
        add_compile_options(--coverage -O0)
        add_link_options(--coverage)
        add_custom_command(OUTPUT coverage.xml COMMAND gcovr --root ${CMAKE_CURRENT_SOURCE_DIR})
        add_custom_target(coverage DEPENDS coverage.xml)
    endif()
endif()

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)
find_package(Qt6 COMPONENTS Core Gui Widgets Multimedia REQUIRED)
find_package(GTest REQUIRED)

add_subdirectory(${CMAKE_PROJECT_NAME})
add_subdirectory(tests)
