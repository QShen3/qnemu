# C++
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/cartridge CARTRIDGE_SRCS)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/cartridge/mbc CARTRIDGE_MBC_SRCS)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/cpu CPU_SRCS)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/interrupt INTERRUPT_SRCS)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/memory MEMORY_SRCS)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/gpu GPU_SRCS)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/joypad JOYPAD_SRCS)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/timer TIMER_SRCS)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/mmu MMU_SRCS)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/apu APU_SRCS)

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
    ${MMU_SRCS}
    ${APU_SRCS}
)
target_include_directories(gb PRIVATE ${CMAKE_SOURCE_DIR})
target_link_libraries(gb PRIVATE Qt6::Core Qt6::Gui)
set_target_properties(gb PROPERTIES
    CXX_STANDARD_REQUIRED ON
    CXX_STANDARD 20
    MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL"
    CXX_CPPCHECK "cppcheck;--enable=style;--std=c++20;--error-exitcode=1;--inline-suppr"
    CXX_CLANG_TIDY "clang-tidy;-checks=-*,clang-analyzer-*,concurrency-*,misc-*,-misc-include-cleaner;--extra-arg=/EHsc")

# Special compile options
if(${CMAKE_CXX_COMPILER_ID} STREQUAL MSVC)
    target_compile_options(gb PRIVATE /W3 /WX /EHsc)
elseif(${CMAKE_CXX_COMPILER_ID} STREQUAL GNU OR ${CMAKE_CXX_COMPILER_ID} STREQUAL Clang OR ${CMAKE_CXX_COMPILER_ID} STREQUAL AppleClang)
    target_compile_options(gb PRIVATE -Wall -Wextra -Werror)
endif()
