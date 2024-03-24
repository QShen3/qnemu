add_executable(
    qnemu
    ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/MainWindow.cpp)
target_include_directories(qnemu PRIVATE ${CMAKE_SOURCE_DIR})
target_link_libraries(qnemu PRIVATE display sound gb Qt6::Core Qt6::Gui Qt6::Widgets)
set_target_properties(qnemu PROPERTIES
    CXX_STANDARD_REQUIRED ON
    CXX_STANDARD 20
    MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL"
    CXX_CPPCHECK "cppcheck;--enable=style;--std=c++20;--error-exitcode=1;--suppress=preprocessorErrorDirective"
    CXX_CLANG_TIDY "clang-tidy;-checks=-*,clang-analyzer-*,concurrency-*,misc-*,-misc-include-cleaner")

# Special compile options
if(${CMAKE_CXX_COMPILER_ID} STREQUAL MSVC)
    target_compile_options(qnemu PRIVATE /W3 /WX /EHsc)
elseif(${CMAKE_CXX_COMPILER_ID} STREQUAL GNU OR ${CMAKE_CXX_COMPILER_ID} STREQUAL Clang OR ${CMAKE_CXX_COMPILER_ID} STREQUAL AppleClang)
    target_compile_options(qnemu PRIVATE -Wall -Wextra -Werror)
endif()

add_subdirectory(display)
add_subdirectory(sound)
add_subdirectory(gb)
