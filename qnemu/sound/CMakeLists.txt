add_library(sound OBJECT DefaultSound.cpp)
target_include_directories(sound PRIVATE ${CMAKE_SOURCE_DIR})
target_link_libraries(sound PRIVATE Qt6::Gui Qt6::Multimedia)
set_target_properties(sound PROPERTIES
    CXX_STANDARD_REQUIRED ON
    CXX_STANDARD 20
    MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL"
    CXX_CPPCHECK "cppcheck;--enable=style;--std=c++20;--error-exitcode=1"
    CXX_CLANG_TIDY "clang-tidy;-checks=-*,clang-analyzer-*,concurrency-*,misc-*,-misc-include-cleaner")

# Special compile options
if(${CMAKE_CXX_COMPILER_ID} STREQUAL MSVC)
    target_compile_options(sound PRIVATE /W3 /WX /EHsc)
elseif(${CMAKE_CXX_COMPILER_ID} STREQUAL GNU OR ${CMAKE_CXX_COMPILER_ID} STREQUAL Clang OR ${CMAKE_CXX_COMPILER_ID} STREQUAL AppleClang)
    target_compile_options(sound PRIVATE -Wall -Wextra -Werror)
endif()
