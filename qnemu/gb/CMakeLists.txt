# C++
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/cartridge CARTRIDGE_SRCS)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/cartridge/mbc CARTRIDGE_MBC_SRCS)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/cpu CPU_SRCS)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/interrupt INTERRUPT_SRCS)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/memory MEMORY_SRCS)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/gpu GPU_SRCS)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/joypad JOYPAD_SRCS)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/timer TIMER_SRCS)

add_library(
    gb OBJECT
    Gb.cpp
    ${CARTRIDGE_SRCS}
    ${CARTRIDGE_MBC_SRCS}
    ${CPU_SRCS}
    ${INTERRUPT_SRCS}
    ${MEMORY_SRCS}
    ${GPU_SRCS}
    ${JOYPAD_SRCS}
    ${TIMER_SRCS}
)

# Library
find_package(Qt6 COMPONENTS Core Gui)
if (${CMAKE_CXX_COMPILER_ID} STREQUAL MSVC)
    set_property(TARGET gb PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")
endif()
target_include_directories(gb PRIVATE ${CMAKE_SOURCE_DIR})
target_link_libraries(gb PRIVATE Qt6::Core Qt6::Gui)

# Special compile options
if(${CMAKE_CXX_COMPILER_ID} STREQUAL MSVC)
    target_compile_options(gb PRIVATE /W3 /WX /EHsc)
elseif(${CMAKE_CXX_COMPILER_ID} STREQUAL GNU OR ${CMAKE_CXX_COMPILER_ID} STREQUAL Clang OR ${CMAKE_CXX_COMPILER_ID} STREQUAL AppleClang)
    target_compile_options(gb PRIVATE -Wall -Wextra -Werror)
endif()
