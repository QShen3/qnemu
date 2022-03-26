# C++
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/gb/cartridge/mbc GB_CARTRIDGE_MBCTEST_SRCS)
aux_source_directory(${CMAKE_SOURCE_DIR}/${CMAKE_PROJECT_NAME}/gb/cartridge/mbc GB_CARTRIDGE_MBC_SRCS)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/gb/cartridge GB_CARTRIDGETEST_SRCS)
aux_source_directory(${CMAKE_SOURCE_DIR}/${CMAKE_PROJECT_NAME}/gb/cartridge GB_CARTRIDGE_SRCS)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/gb/interrupt GB_INTERRUPTTEST_SRCS)
aux_source_directory(${CMAKE_SOURCE_DIR}/${CMAKE_PROJECT_NAME}/gb/interrupt GB_INTERRUPT_SRCS)


# Executable
add_executable(
    ${GB_CARTRIDGE_MBCTEST_SRCS}
    ${GB_CARTRIDGE_MBC_SRCS}
    ${GB_CARTRIDGETEST_SRCS}
    ${GB_CARTRIDGE_SRCS}
    ${GB_INTERRUPTTEST_SRCS}
    ${GB_INTERRUPT_SRCS}
)
include(GoogleTest)
gtest_discover_tests(qnemu_test)

include(os_release_info)

get_os_release_info(OS_NAME OS_VERSION OS_CODENAME)
if("" STREQUAL OS_NAME)
    set(OS_NAME "UNKNOWN")
endif()

find_package(GTest REQUIRED)

# Library
if(${CMAKE_VERSION} VERSION_LESS 3.20 OR ${OS_NAME} STREQUAL centos OR ${OS_NAME} STREQUAL rocky)
    target_link_libraries(qnemu_test GTest::GTest gmock GTest::Main)
else()
    target_link_libraries(qnemu_test GTest::gtest GTest::gmock GTest::gtest_main)
endif()

if (${CMAKE_CXX_COMPILER_ID} STREQUAL MSVC)
    set_property(TARGET qnemu_test PROPERTY MSVC_RUNTIME_LIBRARY MultiThreaded$<$<CONFIG:Debug>:Debug>DLL)
endif()
target_include_directories(qnemu_test PRIVATE ${CMAKE_SOURCE_DIR})

# Special compile options
if(${CMAKE_CXX_COMPILER_ID} STREQUAL MSVC)
    target_compile_options(qnemu_test PRIVATE /W3 /WX /EHsc)
else(${CMAKE_CXX_COMPILER_ID} STREQUAL GNU OR ${CMAKE_CXX_COMPILER_ID} STREQUAL Clang OR ${CMAKE_CXX_COMPILER_ID} STREQUAL AppleClang)
    target_compile_options(qnemu_test PRIVATE -Wall -Wextra -Werror)
endif()