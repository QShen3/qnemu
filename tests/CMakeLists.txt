# C++
aux_source_directory(${CMAKE_SOURCE_DIR}/${CMAKE_PROJECT_NAME}/display DISPLAY_SRCS)
aux_source_directory(${CMAKE_SOURCE_DIR}/${CMAKE_PROJECT_NAME}/sound SOUND_SRCS)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/gb/cartridge/mbc GB_CARTRIDGE_MBCTEST_SRCS)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/gb/cartridge GB_CARTRIDGETEST_SRCS)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/gb/interrupt GB_INTERRUPTTEST_SRCS)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/gb/memory GB_MEMORYTEST_SRCS)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/gb/joypad GB_JOYPADTEST_SRCS)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/gb/timer GB_TIMERTEST_SRCS)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/gb/gpu GB_GPUTEST_SRCS)

# Qt
set(CMAKE_AUTOMOC ON)

# Executable
add_executable(
    qnemu_test
    ${DISPLAY_SRCS}
    ${SOUND_SRCS}
    ${GB_CARTRIDGE_MBCTEST_SRCS}
    ${GB_CARTRIDGETEST_SRCS}
    ${GB_INTERRUPTTEST_SRCS}
    ${GB_MEMORYTEST_SRCS}
    ${GB_JOYPADTEST_SRCS}
    ${GB_TIMERTEST_SRCS}
    ${GB_GPUTEST_SRCS}
)
include(GoogleTest)
gtest_discover_tests(qnemu_test)

find_package(GTest REQUIRED)
find_package(Qt6 COMPONENTS Widgets)

# Library
if(${CMAKE_CXX_COMPILER_ID} STREQUAL MSVC)
    target_link_libraries(qnemu_test GTest::gmock_main Qt6::Widgets)
else()
    target_link_libraries(qnemu_test GTest::gtest GTest::gmock GTest::gtest_main Qt6::Widgets)
endif()
target_link_libraries(qnemu_test gb)

if (${CMAKE_CXX_COMPILER_ID} STREQUAL MSVC)
    set_property(TARGET qnemu_test PROPERTY MSVC_RUNTIME_LIBRARY MultiThreaded$<$<CONFIG:Debug>:Debug>DLL)
endif()
target_include_directories(qnemu_test PRIVATE ${CMAKE_SOURCE_DIR})

# Special compile options
if(${CMAKE_CXX_COMPILER_ID} STREQUAL MSVC)
    target_compile_options(qnemu_test PRIVATE /W3 /WX /EHsc)
elseif(${CMAKE_CXX_COMPILER_ID} STREQUAL GNU OR ${CMAKE_CXX_COMPILER_ID} STREQUAL Clang OR ${CMAKE_CXX_COMPILER_ID} STREQUAL AppleClang)
    target_compile_options(qnemu_test PRIVATE -Wall -Wextra -Werror)
endif()