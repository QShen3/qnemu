# Executable
add_executable(
    qnemu
    ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
)

# Library
if (${CMAKE_CXX_COMPILER_ID} STREQUAL MSVC)
    set_property(TARGET qnemu PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")
endif()
target_include_directories(qnemu PRIVATE ${CMAKE_SOURCE_DIR})
target_link_libraries(qnemu $<TARGET_OBJECTS:gb>)

# Special compile options
if(${CMAKE_CXX_COMPILER_ID} STREQUAL MSVC)
    target_compile_options(qnemu PRIVATE /W3 /WX /EHsc)
elseif(${CMAKE_CXX_COMPILER_ID} STREQUAL GNU OR ${CMAKE_CXX_COMPILER_ID} STREQUAL Clang OR ${CMAKE_CXX_COMPILER_ID} STREQUAL AppleClang)
    target_compile_options(qnemu PRIVATE -Wall -Wextra -Werror)
endif()

add_subdirectory(gb)