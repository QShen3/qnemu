# C++
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/gb/cartridge/mbc GB_CARTRIDGE_MBCTEST_SRCS)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/gb/cartridge GB_CARTRIDGETEST_SRCS)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/gb/cpu GB_CPUTEST_SRCS)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/gb/interrupt GB_INTERRUPTTEST_SRCS)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/gb/memory GB_MEMORYTEST_SRCS)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/gb/mmu GB_MMUTEST_SRCS)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/gb/apu GB_APUTEST_SRCS)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/gb/joypad GB_JOYPADTEST_SRCS)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/gb/timer GB_TIMERTEST_SRCS)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/gb/gpu GB_GPUTEST_SRCS)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/gb/system GB_SYSTEMTEST_SRCS)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/gb/sgb GB_SGBTEST_SRCS)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/save SAVE_TEST_SRCS)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/ui UI_TEST_SRCS)

# Executable
add_executable(
    qnemu_test
    ${GB_CARTRIDGE_MBCTEST_SRCS}
    ${GB_CARTRIDGETEST_SRCS}
    ${GB_CPUTEST_SRCS}
    ${GB_INTERRUPTTEST_SRCS}
    ${GB_MEMORYTEST_SRCS}
    ${GB_MMUTEST_SRCS}
    ${GB_APUTEST_SRCS}
    ${GB_JOYPADTEST_SRCS}
    ${GB_TIMERTEST_SRCS}
    ${GB_GPUTEST_SRCS}
    ${GB_SYSTEMTEST_SRCS}
    ${GB_SGBTEST_SRCS}
    ${SAVE_TEST_SRCS}
    ${UI_TEST_SRCS}
    ${PROJECT_SOURCE_DIR}/qnemu/ui/GameLibrary.cpp
    ${PROJECT_SOURCE_DIR}/qnemu/ui/AppSettings.cpp
)
set_target_properties(qnemu_test PROPERTIES
    AUTOMOC ON AUTOUIC OFF AUTORCC OFF)
target_include_directories(qnemu_test PRIVATE ${PROJECT_SOURCE_DIR})
target_link_libraries(
    qnemu_test
    PRIVATE
    qnemu_warnings
    gb
    save
    GTest::gmock_main
    Qt6::Core
    Qt6::Gui)

include(GoogleTest)
gtest_discover_tests(qnemu_test DISCOVERY_TIMEOUT 30)

add_subdirectory(rom)

if(QNEMU_ENABLE_COVERAGE)
    if(MSVC)
        find_program(OPENCPPCOVERAGE_EXECUTABLE NAMES OpenCppCoverage)
        if(NOT OPENCPPCOVERAGE_EXECUTABLE)
            message(FATAL_ERROR "QNEMU_ENABLE_COVERAGE is ON, but OpenCppCoverage was not found")
        endif()
        add_custom_target(
            coverage
            COMMAND ${OPENCPPCOVERAGE_EXECUTABLE}
                --cover_children
                --sources ${PROJECT_SOURCE_DIR}/qnemu
                --excluded_sources ${PROJECT_SOURCE_DIR}/tests
                --export_type=html:${CMAKE_BINARY_DIR}/coverage
                -- $<TARGET_FILE:qnemu_test>
            BYPRODUCTS ${CMAKE_BINARY_DIR}/coverage/index.html
            DEPENDS qnemu_test
            COMMENT "Running tests and generating coverage report")
    else()
        find_program(GCOVR_EXECUTABLE NAMES gcovr)
        if(NOT GCOVR_EXECUTABLE)
            message(FATAL_ERROR "QNEMU_ENABLE_COVERAGE is ON, but gcovr was not found")
        endif()
        add_custom_target(
            coverage
            COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure
            COMMAND ${GCOVR_EXECUTABLE}
                --config ${PROJECT_SOURCE_DIR}/gcovr.cfg
                --root ${PROJECT_SOURCE_DIR}
                --filter ${PROJECT_SOURCE_DIR}/qnemu/
                --filter ${PROJECT_SOURCE_DIR}/tests/
                --xml ${CMAKE_BINARY_DIR}/coverage.xml
                --html-details ${CMAKE_BINARY_DIR}/coverage.html
            WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
            DEPENDS qnemu_test
            COMMENT "Running tests and generating coverage report")
    endif()
endif()
