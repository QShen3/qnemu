add_executable(
    gb_test_runner
    main.cpp
    GbTestSystem.cpp)
target_include_directories(gb_test_runner PRIVATE ${PROJECT_SOURCE_DIR})
target_link_libraries(gb_test_runner PRIVATE qnemu_warnings gb_cpu)

# Optional integration tests against real ROMs.  Point this at a directory
# containing the blargg / Mooneye test ROMs; leaving it empty keeps the normal
# unit-test build free of external downloads.
set(QNEMU_TEST_ROMS_DIR "" CACHE PATH "Directory tree containing .gb test ROMs")
if(QNEMU_TEST_ROMS_DIR)
    file(GLOB_RECURSE QNEMU_TEST_ROMS "${QNEMU_TEST_ROMS_DIR}/*.gb")
    foreach(ROM IN LISTS QNEMU_TEST_ROMS)
        file(RELATIVE_PATH ROM_RELATIVE "${QNEMU_TEST_ROMS_DIR}" "${ROM}")
        string(MAKE_C_IDENTIFIER "rom_${ROM_RELATIVE}" TEST_NAME)
        add_test(
            NAME "${TEST_NAME}"
            COMMAND gb_test_runner "${ROM}" --max-cycles 300000000)
        set_tests_properties("${TEST_NAME}" PROPERTIES TIMEOUT 120 LABELS "rom")
    endforeach()
endif()
